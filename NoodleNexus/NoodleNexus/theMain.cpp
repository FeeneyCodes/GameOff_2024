//#define GLAD_GL_IMPLEMENTATION
//#include <glad/glad.h>
//
//#define GLFW_INCLUDE_NONE
//#include <GLFW/glfw3.h>
#include "GLCommon.h"

//#include "linmath.h"
#include <glm/glm.hpp>
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> 
// glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

#include <iostream>     // "input output" stream
#include <fstream>      // "file" stream
#include <sstream>      // "string" stream ("string builder" in Java c#, etc.)
#include <string>
#include <vector>

//void ReadPlyModelFromFile(std::string plyFileName);
#include "PlyFileLoaders.h"
#include "Basic_Shader_Manager/cShaderManager.h"
#include "sMesh.h"
#include "cVAOManager/cVAOManager.h"
#include "sharedThings.h"       // Fly camera
#include "cPhysics.h"
#include "cLightManager.h"
#include <windows.h>    // Includes ALL of windows... MessageBox
#include "cLightHelper/cLightHelper.h"
//
#include "cBasicTextureManager/cBasicTextureManager.h"

#include "cLowPassFilter.h"

//
//const unsigned int MAX_NUMBER_OF_MESHES = 1000;
//unsigned int g_NumberOfMeshesToDraw;
//sMesh* g_myMeshes[MAX_NUMBER_OF_MESHES] = { 0 };    // All zeros

std::vector<sMesh*> g_vecMeshesToDraw;

cPhysics* g_pPhysicEngine = NULL;
// This loads the 3D models for drawing, etc.
cVAOManager* g_pMeshManager = NULL;

cBasicTextureManager* g_pTextures = NULL;

cCommandGroup* g_pCommandDirector = NULL;
cCommandFactory* g_pCommandFactory = NULL;



//cLightManager* g_pLightManager = NULL;

void AddModelsToScene(cVAOManager* pMeshManager, GLuint shaderProgram);

void DrawMesh(sMesh* pCurMesh, GLuint program);

//glm::vec3 cameraEye = glm::vec3(0.0, 0.0, 4.0f);




// This is the function that Lua will call when 
//void g_Lua_AddSerialCommand(std::string theCommandText)
int g_Lua_AddSerialCommand(lua_State* L)
{
//    std::cout << "**************************" << std::endl;
//    std::cout << "g_Lua_AddSerialCommand() called" << std::endl;
//    std::cout << "**************************" << std::endl;
    // AddSerialCommand() has been called
    // eg: AddSerialCommand('New_Viper_Player', -50.0, 15.0, 30.0, 5.0)

    std::string objectFriendlyName = lua_tostring(L, 1);      // 'New_Viper_Player'
    float x = (float)lua_tonumber(L, 2);                   // -50.0
    float y = (float)lua_tonumber(L, 3);                   // 15.0
    float z = (float)lua_tonumber(L, 4);                   // 30.0
    float timeSeconds = (float)lua_tonumber(L, 5);                   // 5.0

    std::vector<std::string> vecCommandDetails;
    vecCommandDetails.push_back(objectFriendlyName);    // Object command controls
    vecCommandDetails.push_back(::g_floatToString(x));
    vecCommandDetails.push_back(::g_floatToString(y));
    vecCommandDetails.push_back(::g_floatToString(z));
    vecCommandDetails.push_back(::g_floatToString(timeSeconds));

    iCommand* pMoveViper = ::g_pCommandFactory->pCreateCommandObject(
        "Move Relative ConstVelocity+Time", vecCommandDetails);

    ::g_pCommandDirector->addSerial(pMoveViper);

    // We'll return some value to indicate if the command worked or not
    // Here, we'll push "true" if it worked
    lua_pushboolean(L, true);
    // return 1 because we pushed 1 thing onto the stack
    return 1;
}




static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

bool isControlDown(GLFWwindow* window);
//{
//    if ((glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) ||
//        (glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS))
//    {
//        return true;
//    }
//    return false;
//}

// START OF: TANK GAME
//#include "iTank.h"
//#include "cTank.h"
//#include "cSuperTank.h"
//#include "cTankFactory.h"
#include "cTankBuilder.h"
#include "cArena.h"
void SetUpTankGame(void);
void TankStepFrame(double timeStep);
std::vector< iTank* > g_vecTheTanks;
cArena* g_pTankArena = NULL;
sMesh* g_pTankModel = NULL;

// END OF: TANK GAME





void ConsoleStuff(void);

// https://stackoverflow.com/questions/5289613/generate-random-float-between-two-floats
float getRandomFloat(float a, float b) {
    float random = ((float)rand()) / (float)RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}

glm::vec3 getRandom_vec3(glm::vec3 min, glm::vec3 max)
{
    return glm::vec3(
        getRandomFloat(min.x, max.x),
        getRandomFloat(min.y, max.y),
        getRandomFloat(min.z, max.z));
}

std::string getStringVec3(glm::vec3 theVec3)
{
    std::stringstream ssVec;
    ssVec << "(" << theVec3.x << ", " << theVec3.y << ", " << theVec3.z << ")";
    return ssVec.str();
}

// Returns NULL if NOT found
sMesh* pFindMeshByFriendlyName(std::string theNameToFind)
{
    for (unsigned int index = 0; index != ::g_vecMeshesToDraw.size(); index++)
    {
        if (::g_vecMeshesToDraw[index]->uniqueFriendlyName == theNameToFind)
        {
            return ::g_vecMeshesToDraw[index];
        }
    }
    // Didn't find it
    return NULL;
}

void AABBOctTree(void);

int main(void)
{
    
    AABBOctTree();


    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    GLFWwindow* window = glfwCreateWindow(640, 480, "OpenGL Triangle", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // Callback for keyboard, but for "typing"
    // Like it captures the press and release and repeat
    glfwSetKeyCallback(window, key_callback);

    // 
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetWindowFocusCallback(window, cursor_enter_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);



    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1);


    cShaderManager* pShaderManager = new cShaderManager();

    cShaderManager::cShader vertexShader;
    vertexShader.fileName = "assets/shaders/vertex01.glsl";

    cShaderManager::cShader fragmentShader;
    fragmentShader.fileName = "assets/shaders/fragment01.glsl";

    if ( ! pShaderManager->createProgramFromFile("shader01",
                                                 vertexShader, fragmentShader))
    {
        std::cout << "Error: " << pShaderManager->getLastError() << std::endl;
    }
    else
    {
        std::cout << "Shader built OK" << std::endl;
    }

    const GLuint program = pShaderManager->getIDFromFriendlyName("shader01");

    glUseProgram(program);


    ::g_pMyLuaMasterBrain = new cLuaBrain();


//    cVAOManager* pMeshManager = new cVAOManager();
    ::g_pMeshManager = new cVAOManager();

    ::g_pPhysicEngine = new cPhysics();
    // For triangle meshes, let the physics object "know" about the VAO manager
    ::g_pPhysicEngine->setVAOManager(::g_pMeshManager);


    ::g_pCommandDirector = new cCommandGroup();
    ::g_pCommandFactory = new cCommandFactory();
    // 
    // Tell the command factory about the phsyics and mesh stuff
    ::g_pCommandFactory->setPhysics(::g_pPhysicEngine);
    // (We are passing the address of this...)
    ::g_pCommandFactory->setVectorOfMeshes(&g_vecMeshesToDraw);

    // This also adds physics objects to the phsyics system
    AddModelsToScene(::g_pMeshManager, program);
    
     
    ::g_pFlyCamera = new cBasicFlyCamera();
    ::g_pFlyCamera->setEyeLocation(glm::vec3(0.0f, 5.0f, -50.0f));
    // To see the Galactica:
//    ::g_pFlyCamera->setEyeLocation(glm::vec3(10'000.0f, 25'000.0f, 160'000.0f));
    // Rotate the camera 180 degrees
//    ::g_pFlyCamera->rotateLeftRight_Yaw_NoScaling(glm::radians(180.0f));



    glUseProgram(program);

    // Enable depth buffering (z buffering)
    // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glEnable.xhtml
    glEnable(GL_DEPTH_TEST);

    cLowPassFilter frameTimeFilter;
//    frameTimeFilter.setNumSamples(30000);

    double currentFrameTime = glfwGetTime();
    double lastFrameTime = glfwGetTime();





    // Set up the lights
    ::g_pLightManager = new cLightManager();
    // Called only once
    ::g_pLightManager->loadUniformLocations(program);

    // Set up one of the lights in the scene
    ::g_pLightManager->theLights[0].position = glm::vec4(2'000.0f, 100'000.0f, 10'000.0f, 1.0f);
    ::g_pLightManager->theLights[0].diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    ::g_pLightManager->theLights[0].atten.y = 0.000006877f;
    ::g_pLightManager->theLights[0].atten.z = 0.0000000001184f;

    ::g_pLightManager->theLights[0].param1.x = 0.0f;    // Point light (see shader)
    ::g_pLightManager->theLights[0].param2.x = 1.0f;    // Turn on (see shader)


    // Set up one of the lights in the scene
    ::g_pLightManager->theLights[1].position = glm::vec4(0.0f, 20.0f, 0.0f, 1.0f);
    ::g_pLightManager->theLights[1].diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    ::g_pLightManager->theLights[1].atten.y = 0.01f;
    ::g_pLightManager->theLights[1].atten.z = 0.001f;

    ::g_pLightManager->theLights[1].param1.x = 1.0f;    // Spot light (see shader)
    ::g_pLightManager->theLights[1].direction = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
    ::g_pLightManager->theLights[1].param1.y = 5.0f;   //  y = inner angle
    ::g_pLightManager->theLights[1].param1.z = 10.0f;  //  z = outer angle

    ::g_pLightManager->theLights[1].param2.x = 1.0f;    // Turn on (see shader)




    ::g_pTextures = new cBasicTextureManager();

    ::g_pTextures->SetBasePath("assets/textures");
    ::g_pTextures->Create2DTextureFromBMPFile("bad_bunny_1920x1080.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("dua-lipa-promo.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("Puzzle_parts.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("Non-uniform concrete wall 0512-3-1024x1024.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("UV_Test_750x750.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("shape-element-splattered-texture-stroke_1194-8223.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("Grey_Brick_Wall_Texture.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("dirty-metal-texture_1048-4784.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("bad_bunny_1920x1080_24bit_black_and_white.bmp");
    //
    ::g_pTextures->Create2DTextureFromBMPFile("SurprisedChildFace.bmp");

    // Load the space skybox
    std::string errorString;
    ::g_pTextures->SetBasePath("assets/textures/CubeMaps");
    if (::g_pTextures->CreateCubeTextureFromBMPFiles("Space",
        "SpaceBox_right1_posX.bmp", 
        "SpaceBox_left2_negX.bmp",
        "SpaceBox_top3_posY.bmp", 
        "SpaceBox_bottom4_negY.bmp",
        "SpaceBox_front5_posZ.bmp", 
        "SpaceBox_back6_negZ.bmp", true, errorString))
    {
        std::cout << "Loaded space skybox" << std::endl;
    }
    else
    {
        std::cout << "ERROR: Didn't load space skybox because: " << errorString << std::endl;
    }
        
    //std::cout << "glTexStorage2D():" << glTexStorage2D << std::endl;
    //std::cout << "glCompileShader():" << glCompileShader << std::endl;

    //FARPROC glTexStorage2DProc = GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "glTexStorage2D");
    //std::cout << "glCompileShader():" << glTexStorage2DProc << std::endl;
    //FARPROC glCompileShaderProc = GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "glCompileShader");
    //std::cout << "glCompileShader():" << glCompileShaderProc << std::endl;

    // Load the sunny day cube map
    if (::g_pTextures->CreateCubeTextureFromBMPFiles("SunnyDay",
        "TropicalSunnyDayLeft2048.bmp",
        "TropicalSunnyDayRight2048.bmp",
        "TropicalSunnyDayUp2048.bmp",
        "TropicalSunnyDayDown2048.bmp",
        "TropicalSunnyDayFront2048.bmp",
        "TropicalSunnyDayBack2048.bmp",
        true, errorString))
    {
        std::cout << "Loaded space SunnyDay" << std::endl;
    }
    else
    {
        std::cout << "ERROR: Didn't load space SunnyDay because: " << errorString << std::endl;
    }
        
        
        
        


    //glGet with argument GL_ACTIVE_TEXTURE, or GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS.
    // 
    // void glGetIntegerv(GLenum pname, GLint* data);
    // 
    //GLint iActiveTextureUnits = 0;
    //glGetIntegerv(GL_ACTIVE_TEXTURE, &iActiveTextureUnits);
    //std::cout << "GL_ACTIVE_TEXTURE = " << (iActiveTextureUnits - GL_TEXTURE0) << std::endl;

    GLint iMaxCombinedTextureInmageUnits = 0;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &iMaxCombinedTextureInmageUnits);
    std::cout << "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS = " << iMaxCombinedTextureInmageUnits << std::endl;

    // data returns one value, the maximum number of components of the inputs read by the fragment shader, 
    // which must be at least 128.
    GLint iMaxFragmentInputComponents = 0;
    glGetIntegerv(GL_MAX_FRAGMENT_INPUT_COMPONENTS, &iMaxFragmentInputComponents);
    std::cout << "GL_MAX_FRAGMENT_INPUT_COMPONENTS = " << iMaxFragmentInputComponents << std::endl;
    

    // data returns one value, the maximum number of individual floating - point, integer, or boolean values 
    // that can be held in uniform variable storage for a fragment shader.The value must be at least 1024. 
    GLint iMaxFragmentUniformComponents = 0;
    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &iMaxFragmentUniformComponents);
    std::cout << "GL_MAX_FRAGMENT_UNIFORM_COMPONENTS = " << iMaxFragmentUniformComponents << std::endl;
        

    //  Turn on the blend operation
    glEnable(GL_BLEND);
    // Do alpha channel transparency
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    cLightHelper TheLightHelper;

    // Is the default (cull back facing polygons)
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    // HACK:
    unsigned int numberOfNarrowPhaseTrianglesInAABB_BroadPhaseThing = 0;

    while (!glfwWindowShouldClose(window))
    {
        float ratio;
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float)height;
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //        glm::mat4 m, p, v, mvp;
        glm::mat4 matProjection = glm::mat4(1.0f);

        matProjection = glm::perspective(0.6f,           // FOV
            ratio,          // Aspect ratio of screen
//            0.1f,           // Near plane (as far from the camera as possible)
//            1'000.0f);       // Far plane (as near to the camera as possible)
// For a "far" view of the large Galactica
            100.1f,           // Near plane (as far from the camera as possible)
            1'000'000.0f);       // Far plane (as near to the camera as possible)

        // View or "camera"
        glm::mat4 matView = glm::mat4(1.0f);

        //        glm::vec3 cameraEye = glm::vec3(0.0, 0.0, 4.0f);
        glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);

        matView = glm::lookAt(::g_pFlyCamera->getEyeLocation(),
            ::g_pFlyCamera->getTargetLocation(),
            upVector);
        //        matView = glm::lookAt( cameraEye,
        //                               cameraTarget,
        //                               upVector);


        const GLint matView_UL = glGetUniformLocation(program, "matView");
        glUniformMatrix4fv(matView_UL, 1, GL_FALSE, (const GLfloat*)&matView);

        const GLint matProjection_UL = glGetUniformLocation(program, "matProjection");
        glUniformMatrix4fv(matProjection_UL, 1, GL_FALSE, (const GLfloat*)&matProjection);


        // Calculate elapsed time
        // We'll enhance this
        currentFrameTime = glfwGetTime();
        double tempDeltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        // Set a limit on the maximum frame time
        const double MAX_FRAME_TIME = 1.0 / 60.0;   // 60Hz (16 ms)
        if (tempDeltaTime > MAX_FRAME_TIME)
        {
            tempDeltaTime = MAX_FRAME_TIME;
        }

        // Add this sample to the low pass filer ("averager")
        frameTimeFilter.addSample(tempDeltaTime);
        // 
        double deltaTime = frameTimeFilter.getAverage();


        // **************************************************************
// Sky box
// Move the sky sphere with the camera
        sMesh* pSkySphere = pFindMeshByFriendlyName("SkySphere");
        pSkySphere->positionXYZ = ::g_pFlyCamera->getEyeLocation();

        // Disable backface culling (so BOTH sides are drawn)
        glDisable(GL_CULL_FACE);
        // Don't perform depth buffer testing
        glDisable(GL_DEPTH_TEST);
        // Don't write to the depth buffer when drawing to colour (back) buffer
//        glDepthMask(GL_FALSE);
//        glDepthFunc(GL_ALWAYS);// or GL_LESS (default)
        // GL_DEPTH_TEST : do or not do the test against what's already on the depth buffer

        pSkySphere->bIsVisible = true;
        //        pSkySphere->bDoNotLight = true;

        pSkySphere->uniformScale = 1.0f;

        // Tell the shader this is the skybox, so use the cube map
        // uniform samplerCube skyBoxTexture;
        // uniform bool bIsSkyBoxObject;
        GLuint bIsSkyBoxObject_UL = glGetUniformLocation(program, "bIsSkyBoxObject");
        glUniform1f(bIsSkyBoxObject_UL, (GLfloat)GL_TRUE);
        
        // Set the cube map texture, just like we do with the 2D
        GLuint cubeSamplerID = ::g_pTextures->getTextureIDFromName("Space");
//        GLuint cubeSamplerID = ::g_pTextures->getTextureIDFromName("SunnyDay");
        // Make sure this is an unused texture unit
        glActiveTexture(GL_TEXTURE0 + 40);
        // *****************************************
        // NOTE: This is a CUBE_MAP, not a 2D
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubeSamplerID);
//        glBindTexture(GL_TEXTURE_2D, cubeSamplerID);
        // *****************************************
        GLint skyBoxTextureSampler_UL = glGetUniformLocation(program, "skyBoxTextureSampler");
        glUniform1i(skyBoxTextureSampler_UL, 40);       // <-- Note we use the NUMBER, not the GL_TEXTURE3 here


        DrawMesh(pSkySphere, program);

        pSkySphere->bIsVisible = false;

        glUniform1f(bIsSkyBoxObject_UL, (GLfloat)GL_FALSE);

        glEnable(GL_CULL_FACE);
        // Enable depth test and write to depth buffer (normal rendering)
        glEnable(GL_DEPTH_TEST);
        //        glDepthMask(GL_FALSE);
        //        glDepthFunc(GL_LESS);
                // **************************************************************




        ::g_pLightManager->updateShaderWithLightInfo();

        // *******************************************************************
        //    ____                       _                      
        //   |  _ \ _ __ __ ___      __ | |    ___   ___  _ __  
        //   | | | | '__/ _` \ \ /\ / / | |   / _ \ / _ \| '_ \ 
        //   | |_| | | | (_| |\ V  V /  | |__| (_) | (_) | |_) |
        //   |____/|_|  \__,_| \_/\_/   |_____\___/ \___/| .__/ 
        //                                               |_|            
        // // Will do two passes, one with "close" projection (clipping)
        // and one with "far away"

//        matProjection = glm::perspective(0.6f,           // FOV
//            ratio,          // Aspect ratio of screen
//            0.1f,           // Near plane (as far from the camera as possible)
//            500.0f);       // Far plane (as near to the camera as possible)
//        glUniformMatrix4fv(matProjection_UL, 1, GL_FALSE, (const GLfloat*)&matProjection);
//
//
//        // Draw all the objects
//        for (unsigned int meshIndex = 0; meshIndex != ::g_vecMeshesToDraw.size(); meshIndex++)
//        {
//            //            sMesh* pCurMesh = ::g_myMeshes[meshIndex];
//           sMesh* pCurMesh = ::g_vecMeshesToDraw[meshIndex];
////            pCurMesh->bDoNotLight = true;
//            DrawMesh(pCurMesh, program);
//
//        }//for (unsigned int meshIndex..


        //// For a "far" view of the large Galactica
        matProjection = glm::perspective(0.6f,           // FOV
            ratio,          // Aspect ratio of screen
            10.0f,           // Near plane (as far from the camera as possible)
            100'000.0f);       // Far plane (as near to the camera as possible)
        glUniformMatrix4fv(matProjection_UL, 1, GL_FALSE, (const GLfloat*)&matProjection);
 
        // Draw everything again, but this time far away things
        for (unsigned int meshIndex = 0; meshIndex != ::g_vecMeshesToDraw.size(); meshIndex++)
        {
            //            sMesh* pCurMesh = ::g_myMeshes[meshIndex];
            sMesh* pCurMesh = ::g_vecMeshesToDraw[meshIndex];
            //            pCurMesh->bDoNotLight = true;
            DrawMesh(pCurMesh, program);
 
        }//for (unsigned int meshIndex..

        // *******************************************************************



        // This should be inside the phsyics thing, I guess...
        // Which AABB bounding box of the broad phase is the viper now? 
        {
            cPhysics::sPhysInfo* pViperPhys = ::g_pPhysicEngine->pFindAssociateMeshByFriendlyName("New_Viper_Player");
            if (pViperPhys)
            {
                // The size of the AABBs that we sliced up the Galactical model in the broad phase
                const float AABBSIZE = 1000.0f;

                // Using the same XYZ location in space we used for the triangle vertices,
                //  we are going to pass the location of the viper to get an ID
                //  of an AABB/Cube that WOULD BE at that location (if there was one...)
                unsigned long long hypotheticalAABB_ID 
                    = ::g_pPhysicEngine->calcBP_GridIndex(
                                                 pViperPhys->position.x,
                                                 pViperPhys->position.y,
                                                 pViperPhys->position.z, AABBSIZE);

                // Where would that hypothetical AABB be in space
                glm::vec3 minXYZofHypotheticalCube = ::g_pPhysicEngine->calcBP_MinXYZ_FromID(hypotheticalAABB_ID, AABBSIZE);

                // Draw a cube at that location
                sMesh* pDebugAABB = pFindMeshByFriendlyName("AABB_MinXYZ_At_Origin");
                pDebugAABB->positionXYZ = minXYZofHypotheticalCube;
                pDebugAABB->bIsVisible = true;
                pDebugAABB->uniformScale = 1'000.0f;

                // Is this an AABB that's already part of the broad phase? 
                // i.e. is it already in the map?
                std::map< unsigned long long, cPhysics::cBroad_Cube* >::iterator
                    it_pCube = ::g_pPhysicEngine->map_BP_CubeGrid.find(hypotheticalAABB_ID);
                //
                if (it_pCube == ::g_pPhysicEngine->map_BP_CubeGrid.end())
                {
                    // NO, there is no cube there
                    pDebugAABB->objectColourRGBA = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
                    numberOfNarrowPhaseTrianglesInAABB_BroadPhaseThing = 0;
                }
                // NOT equal to the end
                if (it_pCube != ::g_pPhysicEngine->map_BP_CubeGrid.end())
                {
                    // YES, there is an AABB (full of triangles) there!
                    pDebugAABB->objectColourRGBA = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
                    // 
                    // 
                    cPhysics::cBroad_Cube* pTheAABB_Cube = it_pCube->second;
                    std::cout << pTheAABB_Cube->vec_pTriangles.size() << std::endl;
                    // Pass THIS smaller list of triangles to the narrow phase
                    numberOfNarrowPhaseTrianglesInAABB_BroadPhaseThing = pTheAABB_Cube->vec_pTriangles.size();
                }

                DrawMesh(pDebugAABB, program);
                pDebugAABB->bIsVisible = false;

            }
        }


        // For Debug, draw a cube where the smaller Cube/AABB/Regions on the broad phase 
        //  structrue is, in world space
        // 
        //        std::map< unsigned long long /*index*/, cBroad_Cube* > map_BP_CubeGrid;

        sMesh* pDebugAABB = pFindMeshByFriendlyName("AABB_MinXYZ_At_Origin");
        if (pDebugAABB)
        {
            pDebugAABB->bIsVisible = true;
            pDebugAABB->uniformScale = 1'000.0f;

            for (std::map< unsigned long long, cPhysics::cBroad_Cube* >::iterator
                it_pCube = ::g_pPhysicEngine->map_BP_CubeGrid.begin();
                it_pCube != ::g_pPhysicEngine->map_BP_CubeGrid.end();
                it_pCube++)
            {

                // Draw a cube at that location
                pDebugAABB->positionXYZ = it_pCube->second->getMinXYZ();
                pDebugAABB->objectColourRGBA = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
                DrawMesh(pDebugAABB, program);

            }

            pDebugAABB->bIsVisible = false;
        }//if (pDebugAABB)




        // *******************************************************************
        // Have camera follow the viper at a set distance
//        {
//            cPhysics::sPhysInfo* pViperPhys = ::g_pPhysicEngine->pFindAssociateMeshByFriendlyName("New_Viper_Player");
//            if (pViperPhys)
//            {
//                const float MAX_DISTANCE_FROM_VIPER = 500.0f;
//                const float MAX_CAMERA_SPEED = 5.0f;
//                // How far away are we?
//                if (glm::distance(pViperPhys->position, ::g_pFlyCamera->getEyeLocation()) > MAX_DISTANCE_FROM_VIPER)
//                {
//                    // Too far from viper.
//                    glm::vec3 vecCameraToViper = pViperPhys->position - ::g_pFlyCamera->getEyeLocation();
//
//                    glm::vec3 cameraDirection = glm::normalize(vecCameraToViper);
//
//                    glm::vec3 cameraSpeed = cameraDirection * MAX_CAMERA_SPEED;
//
//                    ::g_pFlyCamera->moveForward(cameraSpeed.z);
//                    ::g_pFlyCamera->moveLeftRight(cameraSpeed.x);
//                    ::g_pFlyCamera->moveUpDown(cameraSpeed.y);
//                }
//            }//if (pViperPhys)
//        }
//        // *******************************************************************


        //// OH NO! 
        //for (sMesh* pCurMesh : g_vecMeshesToDraw)
        //{
        //    pCurMesh->positionXYZ.z += 1000.0f;
        //}
        //glm::vec3 theEye = ::g_pFlyCamera->getEyeLocation();
        //theEye.z += 1000.0f;
        //g_pFlyCamera->setEyeLocation(theEye);


        // Draw the LASER beam
        cPhysics::sLine LASERbeam;
        glm::vec3 LASERbeam_Offset = glm::vec3(0.0f, -2.0f, 0.0f);

        if (::g_bShowLASERBeam)
        {
            // Draw a bunch of little balls along a line from the camera
            //  to some place in the distance

            // The fly camera is always "looking at" something 1.0 unit away
            glm::vec3 cameraDirection = ::g_pFlyCamera->getTargetRelativeToCamera();     //0,0.1,0.9


            LASERbeam.startXYZ = ::g_pFlyCamera->getEyeLocation();

            // Move the LASER below the camera
            LASERbeam.startXYZ += LASERbeam_Offset;
            glm::vec3 LASER_ball_location = LASERbeam.startXYZ;

            glm::mat4 matOrientation = glm::mat4(glm::quatLookAt(glm::normalize(LASERbeam.endXYZ - LASERbeam.startXYZ),
                                                                 glm::vec3(0.0f, 1.0f, 0.0f)));

            // Is the LASER less than 500 units long?
            // (is the last LAZER ball we drew beyond 500 units form the camera?)
            while ( glm::distance(::g_pFlyCamera->getEyeLocation(), LASER_ball_location) < 150.0f )
            {
                // Move the next ball 0.1 times the normalized camera direction
                LASER_ball_location += (cameraDirection * 0.10f);  
                DrawDebugSphere(LASER_ball_location, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), 0.05f, program);
            }

            // Set the end of the LASER to the last location of the beam
            LASERbeam.endXYZ = LASER_ball_location;

        }//if (::g_bShowLASERBeam)

        // Draw the end of this LASER beam
        DrawDebugSphere(LASERbeam.endXYZ, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.1f, program);

        // Now draw a different coloured ball wherever we get a collision with a triangle
        std::vector<cPhysics::sCollision_RayTriangleInMesh> vec_RayTriangle_Collisions;
        ::g_pPhysicEngine->rayCast(LASERbeam.startXYZ, LASERbeam.endXYZ, vec_RayTriangle_Collisions, false);

        glm::vec4 triColour = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
        float triangleSize = 0.25f;

        for (std::vector<cPhysics::sCollision_RayTriangleInMesh>::iterator itTriList = vec_RayTriangle_Collisions.begin();
            itTriList != vec_RayTriangle_Collisions.end(); itTriList++)
        {
            for (std::vector<cPhysics::sTriangle>::iterator itTri = itTriList->vecTriangles.begin();
                itTri != itTriList->vecTriangles.end(); itTri++)
            {
                // Draw a sphere at the centre of the triangle
//                glm::vec3 triCentre = (itTri->vertices[0] + itTri->vertices[1] + itTri->vertices[2]) / 3.0f;
//                DrawDebugSphere(triCentre, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 0.5f, program);

//                DrawDebugSphere(itTri->intersectionPoint, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), 0.25f, program);
                DrawDebugSphere(itTri->intersectionPoint, triColour, triangleSize, program);
                triColour.r -= 0.1f;
                triColour.g -= 0.1f;
                triColour.b += 0.2f;
                triangleSize *= 1.25f;


            }//for (std::vector<cPhysics::sTriangle>::iterator itTri = itTriList->vecTriangles

        }//for (std::vector<cPhysics::sCollision_RayTriangleInMesh>::iterator itTriList = vec_RayTriangle_Collisions


        // **********************************************************************************
        if (::g_bShowDebugSpheres)
        {

            DrawDebugSphere(::g_pLightManager->theLights[::g_selectedLightIndex].position,
                glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.1f, program);

            const float DEBUG_LIGHT_BRIGHTNESS = 0.3f;

            const float ACCURACY = 0.1f;       // How many units distance
            float distance_75_percent =
                TheLightHelper.calcApproxDistFromAtten(0.75f, ACCURACY, FLT_MAX,
                    ::g_pLightManager->theLights[::g_selectedLightIndex].atten.x,   // Const attent
                    ::g_pLightManager->theLights[::g_selectedLightIndex].atten.y,   // Linear attenuation
                    ::g_pLightManager->theLights[::g_selectedLightIndex].atten.z);  // Quadratic attenuation

            DrawDebugSphere(::g_pLightManager->theLights[::g_selectedLightIndex].position,
                glm::vec4(DEBUG_LIGHT_BRIGHTNESS, 0.0f, 0.0f, 1.0f),
                distance_75_percent,
                program);


            float distance_50_percent =
                TheLightHelper.calcApproxDistFromAtten(0.5f, ACCURACY, FLT_MAX,
                    ::g_pLightManager->theLights[::g_selectedLightIndex].atten.x,   // Const attent
                    ::g_pLightManager->theLights[::g_selectedLightIndex].atten.y,   // Linear attenuation
                    ::g_pLightManager->theLights[::g_selectedLightIndex].atten.z);  // Quadratic attenuation

            DrawDebugSphere(::g_pLightManager->theLights[::g_selectedLightIndex].position,
                glm::vec4(0.0f, DEBUG_LIGHT_BRIGHTNESS, 0.0f, 1.0f),
                distance_50_percent,
                program);

            float distance_25_percent =
                TheLightHelper.calcApproxDistFromAtten(0.25f, ACCURACY, FLT_MAX,
                    ::g_pLightManager->theLights[::g_selectedLightIndex].atten.x,   // Const attent
                    ::g_pLightManager->theLights[::g_selectedLightIndex].atten.y,   // Linear attenuation
                    ::g_pLightManager->theLights[::g_selectedLightIndex].atten.z);  // Quadratic attenuation

            DrawDebugSphere(::g_pLightManager->theLights[::g_selectedLightIndex].position,
                glm::vec4(0.0f, 0.0f, DEBUG_LIGHT_BRIGHTNESS, 1.0f),
                distance_25_percent,
                program);

            float distance_05_percent =
                TheLightHelper.calcApproxDistFromAtten(0.05f, ACCURACY, FLT_MAX,
                    ::g_pLightManager->theLights[::g_selectedLightIndex].atten.x,   // Const attent
                    ::g_pLightManager->theLights[::g_selectedLightIndex].atten.y,   // Linear attenuation
                    ::g_pLightManager->theLights[::g_selectedLightIndex].atten.z);  // Quadratic attenuation

            DrawDebugSphere(::g_pLightManager->theLights[::g_selectedLightIndex].position,
                glm::vec4(DEBUG_LIGHT_BRIGHTNESS, DEBUG_LIGHT_BRIGHTNESS, 0.0f, 1.0f),
                distance_05_percent,
                program);

        }
        // **********************************************************************************



               





        //sMesh* pBall = pFindMeshByFriendlyName("Ball");
        //if (pBall)
        //{
        //    pBall->positionXYZ.y -= 1.0f * deltaTime;
        //}

        // HACK: Update "shadow" of ball to be where the ball hits the large block ground
        sMesh* pBallShadow = pFindMeshByFriendlyName("Ball_Shadow");
        sMesh* pBall = pFindMeshByFriendlyName("Ball");
        pBallShadow->positionXYZ.x = pBall->positionXYZ.x;
        pBallShadow->positionXYZ.z = pBall->positionXYZ.z;
        // Don't update the y - keep the shadow near the plane


        // Physic update and test 
        ::g_pPhysicEngine->StepTick(deltaTime);


        // Update the commands, too
        ::g_pCommandDirector->Update(deltaTime);


        // Handle any collisions
        if (::g_pPhysicEngine->vec_SphereAABB_Collisions.size() > 0 )
        {
            // Yes, there were collisions

            for (unsigned int index = 0; index != ::g_pPhysicEngine->vec_SphereAABB_Collisions.size(); index++)
            {
                cPhysics::sCollision_SphereAABB thisCollisionEvent = ::g_pPhysicEngine->vec_SphereAABB_Collisions[index];

                if (thisCollisionEvent.pTheSphere->pPhysicInfo->velocity.y  < 0.0f)
                {
                    // Yes, it's heading down
                    // So reverse the direction of velocity
                    thisCollisionEvent.pTheSphere->pPhysicInfo->velocity.y = fabs(thisCollisionEvent.pTheSphere->pPhysicInfo->velocity.y);
                }

            }//for (unsigned int index
 
        }//if (::g_pPhysicEngine->vec_SphereAABB_Collisions


        // Point the spot light to the ball
        sMesh* pBouncy_5_Ball = pFindMeshByFriendlyName("Bouncy_5");
        if (pBouncy_5_Ball)
        {
            glm::vec3 directionToBal
                = pBouncy_5_Ball->positionXYZ - glm::vec3(::g_pLightManager->theLights[1].position);
    
            // Normalize to get the direction only
            directionToBal = glm::normalize(directionToBal);

            // Point the spot light at the bouncy ball
            ::g_pLightManager->theLights[1].direction = glm::vec4(directionToBal, 1.0f);
        }



        // Handle async IO stuff
        handleKeyboardAsync(window);
        handleMouseAsync(window);

        glfwSwapBuffers(window);
        glfwPollEvents();


        //std::cout << "Camera: "
        std::stringstream ssTitle;
        ssTitle << "Camera: "
            << ::g_pFlyCamera->getEyeLocation().x << ", "
            << ::g_pFlyCamera->getEyeLocation().y << ", "
            << ::g_pFlyCamera->getEyeLocation().z 
            << "   ";
        ssTitle << "light[" << g_selectedLightIndex << "] "
            << ::g_pLightManager->theLights[g_selectedLightIndex].position.x << ", "
            << ::g_pLightManager->theLights[g_selectedLightIndex].position.y << ", "
            << ::g_pLightManager->theLights[g_selectedLightIndex].position.z
            << "   "
            << "linear: " << ::g_pLightManager->theLights[0].atten.y
            << "   "
            << "quad: " << ::g_pLightManager->theLights[0].atten.z;

        ssTitle << " BP tris: " << numberOfNarrowPhaseTrianglesInAABB_BroadPhaseThing;

        // Add the viper info, too
        cPhysics::sPhysInfo* pViperPhys = ::g_pPhysicEngine->pFindAssociateMeshByFriendlyName("New_Viper_Player");
        if (pViperPhys)
        {
            ssTitle
                << " Viper XYZ:" << getStringVec3(pViperPhys->position)
                << " vel:" << getStringVec3(pViperPhys->velocity)
                << " acc:" << getStringVec3(pViperPhys->acceleration);

        }//if (pViperPhys)

        // Show frame time
        ssTitle << " deltaTime = " << deltaTime
            << " FPS: " << 1.0 / deltaTime;

 //       std::cout << " deltaTime = " << deltaTime << " FPS: " << 1.0 / deltaTime << std::endl;


//        glfwSetWindowTitle(window, "Hey!");
        glfwSetWindowTitle(window, ssTitle.str().c_str());


    }// End of the draw loop


    // Delete everything
    delete ::g_pFlyCamera;
    delete ::g_pPhysicEngine;

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}


void AddModelsToScene(cVAOManager* pMeshManager, GLuint program)
{
    {
        sModelDrawInfo galacticaModel;
        ::g_pMeshManager->LoadModelIntoVAO("assets/models/Battlestar_Galactica_Res_0_(444,087 faces)_xyz_n_uv (facing +z, up +y).ply",
            galacticaModel, program);
        std::cout << galacticaModel.meshName << ": " << galacticaModel.numberOfVertices << " vertices loaded" << std::endl;
    }

    {
        sModelDrawInfo cubeMinXYZ_at_OriginInfo;
        ::g_pMeshManager->LoadModelIntoVAO("assets/models/Cube_MinXYZ_at_Origin_xyz_n_uv.ply",
            cubeMinXYZ_at_OriginInfo, program);
        std::cout << cubeMinXYZ_at_OriginInfo.meshName << ": " << cubeMinXYZ_at_OriginInfo.numberOfVertices << " vertices loaded" << std::endl;
    }

    {
        sModelDrawInfo warehouseModel;
        //    ::g_pMeshManager->LoadModelIntoVAO("assets/models/Warehouse_xyz_n.ply",
        ::g_pMeshManager->LoadModelIntoVAO("assets/models/Warehouse_xyz_n_uv.ply",
            warehouseModel, program);
        std::cout << warehouseModel.numberOfVertices << " vertices loaded" << std::endl;
    }

    {
        sModelDrawInfo tankModel;
        //    pMeshManager->LoadModelIntoVAO("assets/models/Low_Poly_Tank_Model_3D_model.ply", 
        pMeshManager->LoadModelIntoVAO("assets/models/Low_Poly_Tank_Model_3D_model_xyz_n_uv.ply",
            tankModel, program);
        std::cout << tankModel.meshName << " : " << tankModel.numberOfVertices << " vertices loaded" << std::endl;
    }

    //sModelDrawInfo carModelInfo;
    //pMeshManager->LoadModelIntoVAO("assets/models/VintageRacingCar_xyz_only.ply", 
    //                               carModelInfo, program);
    //std::cout << carModelInfo.numberOfVertices << " vertices loaded" << std::endl;

    //sModelDrawInfo dragonModel;
    //pMeshManager->LoadModelIntoVAO("assets/models/Dragon 2.5Edited_xyz_only.ply", 
    //    dragonModel, program);
    //std::cout << dragonModel.numberOfVertices << " vertices loaded" << std::endl;

    {
        sModelDrawInfo terrainModel;
        //    pMeshManager->LoadModelIntoVAO("assets/models/Simple_MeshLab_terrain_xyz_only.ply", 
    //    ::g_pMeshManager->LoadModelIntoVAO("assets/models/Simple_MeshLab_terrain_xyz_N.ply",
    //    ::g_pMeshManager->LoadModelIntoVAO("assets/models/Simple_MeshLab_terrain_xyz_N_uv.ply",
        ::g_pMeshManager->LoadModelIntoVAO("assets/models/Simple_MeshLab_terrain_x5_xyz_N_uv.ply",
            terrainModel, program);
        std::cout << terrainModel.numberOfVertices << " vertices loaded" << std::endl;
    }

    {
        sModelDrawInfo bunnyModel;
        //    ::g_pMeshManager->LoadModelIntoVAO("assets/models/bun_zipper_res2_10x_size_xyz_only.ply",
    //    ::g_pMeshManager->LoadModelIntoVAO("assets/models/bun_zipper_res2_10x_size_xyz_N_only.ply",
        ::g_pMeshManager->LoadModelIntoVAO("assets/models/bun_zipper_res2_10x_size_xyz_N_uv.ply",
            bunnyModel, program);
        std::cout << bunnyModel.numberOfVertices << " vertices loaded" << std::endl;
    }

    {
        sModelDrawInfo platPlaneDrawInfo;
        //    ::g_pMeshManager->LoadModelIntoVAO("assets/models/Flat_Plane_xyz.ply", 
    //    ::g_pMeshManager->LoadModelIntoVAO("assets/models/Flat_Plane_xyz_N.ply",
        ::g_pMeshManager->LoadModelIntoVAO("assets/models/Flat_Plane_xyz_N_uv.ply",
            platPlaneDrawInfo, program);
        std::cout << platPlaneDrawInfo.numberOfVertices << " vertices loaded" << std::endl;
    }

    {
        sModelDrawInfo sphereMesh;
        //    ::g_pMeshManager->LoadModelIntoVAO("assets/models/Sphere_radius_1_xyz.ply",
        //::g_pMeshManager->LoadModelIntoVAO("assets/models/Sphere_radius_1_xyz_N.ply",
        ::g_pMeshManager->LoadModelIntoVAO("assets/models/Sphere_radius_1_xyz_N_uv.ply",

            sphereMesh, program);
        std::cout << sphereMesh.numberOfVertices << " vertices loaded" << std::endl;
    }

    {
        sModelDrawInfo sphereShadowMesh;
        //    ::g_pMeshManager->LoadModelIntoVAO("assets/models/Sphere_radius_1_Flat_Shadow_xyz_N.ply",
        ::g_pMeshManager->LoadModelIntoVAO("assets/models/Sphere_radius_1_Flat_Shadow_xyz_N_uv.ply",
            sphereShadowMesh, program);
        std::cout << sphereShadowMesh.numberOfVertices << " vertices loaded" << std::endl;
    }

    {
        sModelDrawInfo newViperModelInfo;
        ::g_pMeshManager->LoadModelIntoVAO("assets/models/Viper_MkVII_xyz_n_uv.ply",
            newViperModelInfo, program);
        std::cout << newViperModelInfo.numberOfVertices << " vertices loaded" << std::endl;
    }

    {
        sModelDrawInfo cheeseMesh;
        ::g_pMeshManager->LoadModelIntoVAO("assets/models/Cheese_xyz_n_uv.ply",
            cheeseMesh, program);
        std::cout << cheeseMesh.numberOfVertices << " vertices loaded" << std::endl;
    }

    // Add a bunch of bunny rabbits
    float boxLimit = 500.0f;
    float boxStep = 50.0f;
    unsigned int ID_count = 0;
    for (float x = -boxLimit; x <= boxLimit; x += boxStep)
    {
        for (float z = -(2.0f * boxLimit); z <= boxLimit; z += boxStep)
        {
            sMesh* pBunny = new sMesh();
            //            pBunny->modelFileName = "assets/models/bun_zipper_res2_10x_size_xyz_only.ply";
            //            pBunny->modelFileName = "assets/models/bun_zipper_res2_10x_size_xyz_N_only.ply";
            pBunny->modelFileName = "assets/models/bun_zipper_res2_10x_size_xyz_N_uv.ply";
            pBunny->positionXYZ = glm::vec3(x, -35.0f, z);
            pBunny->uniformScale = 2.0f;
            pBunny->objectColourRGBA
                = glm::vec4(getRandomFloat(0.0f, 1.0f),
                    getRandomFloat(0.0f, 1.0f),
                    getRandomFloat(0.0f, 1.0f),
                    1.0f);
            // Set some transparency
            pBunny->alphaTransparency = getRandomFloat(0.25f, 1.0f);
            //            pBunny->alphaTransparency = 0.0f;
            std::stringstream ssName;
            ssName << "Bunny_" << ID_count;
            pBunny->uniqueFriendlyName = ssName.str();
            ID_count++;

            ::g_vecMeshesToDraw.push_back(pBunny);
        }
    }//for (float x = -boxLimit...


    // this is the object that the Lua script, etc. is going to handle
    {
        sMesh* pNewViper = new sMesh();
        pNewViper->modelFileName = "assets/models/Viper_MkVII_xyz_n_uv.ply";
        pNewViper->positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
        pNewViper->objectColourRGBA = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
        pNewViper->bOverrideObjectColour = true;
        pNewViper->uniqueFriendlyName = "New_Viper_Player";
        pNewViper->bIsVisible = true;
        pNewViper->uniformScale = 5.0f;
        pNewViper->textures[0] = "dirty-metal-texture_1048-4784.bmp";
        pNewViper->blendRatio[0] = 1.0f;

        ::g_vecMeshesToDraw.push_back(pNewViper);

        // Add a associated physics object to have the phsyics "move" this
        cPhysics::sPhysInfo* pViperPhysObject = new  cPhysics::sPhysInfo();
        pViperPhysObject->bDoesntMove = false;
        pViperPhysObject->position = pNewViper->positionXYZ;
        pViperPhysObject->velocity = glm::vec3(0.0f);
        pViperPhysObject->pAssociatedDrawingMeshInstance = pNewViper;
        g_pPhysicEngine->vecGeneralPhysicsObjects.push_back(pViperPhysObject);
    }

    // Place a bunny somewhere else in the scene
    sMesh* pBunny_15 = pFindMeshByFriendlyName("Bunny_15");
    if (pBunny_15)
    {
        pBunny_15->positionXYZ = glm::vec3(-50.0f, 15.0f, 30.0f);
        pBunny_15->rotationEulerXYZ.x = glm::radians(180.0f);
        pBunny_15->uniformScale = 10.0f;
    }
    // Place a bunny somewhere else in the scene
    sMesh* pBunny_27 = pFindMeshByFriendlyName("Bunny_27");
    if (pBunny_27)
    {
        pBunny_27->positionXYZ = glm::vec3(75.0f, 10.0f, -45.0f);
        pBunny_27->rotationEulerXYZ.x = glm::radians(180.0f);
        pBunny_27->uniformScale = 10.0f;
    }


        
   {
        sMesh* pGalactica = new sMesh();
        pGalactica->modelFileName = "assets/models/Battlestar_Galactica_Res_0_(444,087 faces)_xyz_n_uv (facing +z, up +y).ply";
        pGalactica->positionXYZ = glm::vec3(-25'000.0f, 0.0f, 0.0f);
        pGalactica->rotationEulerXYZ.y = 17.0f;
        pGalactica->rotationEulerXYZ.x = 23.0f;
        pGalactica->objectColourRGBA = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
        //pGalactica->bIsWireframe = true;
        pGalactica->bOverrideObjectColour = true;
        pGalactica->uniqueFriendlyName = "Galactica";
        //pGalactica->bDoNotLight = true;
        pGalactica->bIsVisible = true;
        pGalactica->uniformScale = 1.0f;
        //
        pGalactica->textures[0] = "Non-uniform concrete wall 0512-3-1024x1024.bmp";
        pGalactica->blendRatio[0] = 1.0f;

        ::g_vecMeshesToDraw.push_back(pGalactica);

        // This is just for testing to see if the xyz locations correctly map to a gridID and the other way around
        unsigned long long gridIndex = ::g_pPhysicEngine->calcBP_GridIndex(0.0f, 0.0f, 0.0f, 1000.0f); // 0, 0, 0
        glm::vec3 minXYZ = ::g_pPhysicEngine->calcBP_MinXYZ_FromID(gridIndex, 1000.0f);
        gridIndex = ::g_pPhysicEngine->calcBP_GridIndex(500.0f, 500.0f, 500.0f, 1000.0f);              // 0, 0, 0
        minXYZ = ::g_pPhysicEngine->calcBP_MinXYZ_FromID(gridIndex, 1000.0f);
        gridIndex = ::g_pPhysicEngine->calcBP_GridIndex(-500.0f, -500.0f, -500.0f, 1000.0f);           // 
        minXYZ = ::g_pPhysicEngine->calcBP_MinXYZ_FromID(gridIndex, 1000.0f);
        gridIndex = ::g_pPhysicEngine->calcBP_GridIndex(10.0f, 2500.0f, 10.0f, 1000.0f);               // 0, 2, 0
        minXYZ = ::g_pPhysicEngine->calcBP_MinXYZ_FromID(gridIndex, 1000.0f);
        gridIndex = ::g_pPhysicEngine->calcBP_GridIndex(2500.0f, 10.0f, 10.0f, 1000.0f);               // 2, 0, 0
        minXYZ = ::g_pPhysicEngine->calcBP_MinXYZ_FromID(gridIndex, 1000.0f);
        gridIndex = ::g_pPhysicEngine->calcBP_GridIndex(10.0f, 10.0f, 2500.0f, 1000.0f);               // 0, 0, 2
        minXYZ = ::g_pPhysicEngine->calcBP_MinXYZ_FromID(gridIndex, 1000.0f);
        gridIndex = ::g_pPhysicEngine->calcBP_GridIndex(8745.0f, 3723.0f, 2500.0f, 1000.0f);           // 8, 3, 2
        minXYZ = ::g_pPhysicEngine->calcBP_MinXYZ_FromID(gridIndex, 1000.0f);
        gridIndex = ::g_pPhysicEngine->calcBP_GridIndex(-8745.0f, -3723.0f, -2500.0f, 1000.0f);           // 8, 3, 2
        minXYZ = ::g_pPhysicEngine->calcBP_MinXYZ_FromID(gridIndex, 1000.0f);
        gridIndex = ::g_pPhysicEngine->calcBP_GridIndex(-999.0f, -999.0f, -999.0f, 1000.0f);           // -1, -1, -1
        minXYZ = ::g_pPhysicEngine->calcBP_MinXYZ_FromID(gridIndex, 1000.0f);



        // 1000x1000x1000 aabbs
        //::g_pPhysicEngine->initBroadPhaseGrid();
        ::g_pPhysicEngine->generateBroadPhaseGrid(
            "assets/models/Battlestar_Galactica_Res_0_(444,087 faces)_xyz_n_uv (facing +z, up +y).ply",
            1000.0f,                            // AABB Cube region size
            pGalactica->positionXYZ,
            pGalactica->rotationEulerXYZ,
            pGalactica->uniformScale);


        sMesh* pGalacticaWireframe = new sMesh();
        pGalacticaWireframe->modelFileName = "assets/models/Battlestar_Galactica_Res_0_(444,087 faces)_xyz_n_uv (facing +z, up +y).ply";
        pGalacticaWireframe->objectColourRGBA = glm::vec4(0.0f, 0.0f, 0.5f, 1.0f);
        pGalacticaWireframe->positionXYZ = pGalactica->positionXYZ;
        pGalacticaWireframe->rotationEulerXYZ = pGalactica->rotationEulerXYZ;
        pGalacticaWireframe->uniformScale = pGalactica->uniformScale;
        pGalacticaWireframe->bIsWireframe = true;
        pGalacticaWireframe->bOverrideObjectColour = true;
        pGalacticaWireframe->bDoNotLight = true;
        pGalacticaWireframe->bIsVisible = true;

        ::g_vecMeshesToDraw.push_back(pGalacticaWireframe);


        // Debug AABB shape
        sMesh* pAABBCube_MinAtOrigin = new sMesh();
        pAABBCube_MinAtOrigin->modelFileName = "assets/models/Cube_MinXYZ_at_Origin_xyz_n_uv.ply";
        pAABBCube_MinAtOrigin->bIsWireframe = true;
        pAABBCube_MinAtOrigin->objectColourRGBA = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        pAABBCube_MinAtOrigin->bOverrideObjectColour = true;
        pAABBCube_MinAtOrigin->bDoNotLight = true;
        pAABBCube_MinAtOrigin->bIsVisible = false;
        pAABBCube_MinAtOrigin->uniqueFriendlyName = "AABB_MinXYZ_At_Origin";

        ::g_vecMeshesToDraw.push_back(pAABBCube_MinAtOrigin);
    }

   {
       sMesh* pSkySphere = new sMesh();
       pSkySphere->modelFileName = "assets/models/Sphere_radius_1_xyz_N_uv.ply";
       pSkySphere->positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
       pSkySphere->objectColourRGBA = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
//       pSkySphere->bIsWireframe = true;
       pSkySphere->bOverrideObjectColour = true;
       pSkySphere->uniformScale = 25.0f;
       pSkySphere->uniqueFriendlyName = "SkySphere";
       pSkySphere->textures[0] = "bad_bunny_1920x1080.bmp";
       pSkySphere->blendRatio[0] = 1.0f;
       pSkySphere->bIsVisible = false;
       ::g_vecMeshesToDraw.push_back(pSkySphere);
   }



    {
        //    ____                _            __                   _     
        //   |  _ \ ___ _ __   __| | ___ _ __ / / __ ___   ___  ___| |__  
        //   | |_) / _ \ '_ \ / _` |/ _ \ '__/ / '_ ` _ \ / _ \/ __| '_ \ 
        //   |  _ <  __/ | | | (_| |  __/ | / /| | | | | |  __/\__ \ | | |
        //   |_| \_\___|_| |_|\__,_|\___|_|/_/ |_| |_| |_|\___||___/_| |_|
        //                                                                
        sMesh* pWarehouse = new sMesh();
        //        pWarehouse->modelFileName = "assets/models/Warehouse_xyz_n.ply";
        pWarehouse->modelFileName = "assets/models/Warehouse_xyz_n_uv.ply";
        pWarehouse->positionXYZ = glm::vec3(-200.0f, 5.0f, 0.0f);
        pWarehouse->rotationEulerXYZ.y = -90.0f;
        pWarehouse->objectColourRGBA = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
        //pWarehouse->bIsWireframe = true;
        pWarehouse->bOverrideObjectColour = true;
        pWarehouse->uniqueFriendlyName = "Warehouse";
        //
        pWarehouse->textures[0] = "bad_bunny_1920x1080.bmp";

        ::g_vecMeshesToDraw.push_back(pWarehouse);

        //    ____  _               _                  _     _           _   
        //   |  _ \| |__  _   _ ___(_) ___ ___    ___ | |__ (_) ___  ___| |_ 
        //   | |_) | '_ \| | | / __| |/ __/ __|  / _ \| '_ \| |/ _ \/ __| __|
        //   |  __/| | | | |_| \__ \ | (__\__ \ | (_) | |_) | |  __/ (__| |_ 
        //   |_|   |_| |_|\__, |___/_|\___|___/  \___/|_.__// |\___|\___|\__|
        //                |___/                           |__/               
        ::g_pPhysicEngine->addTriangleMesh(
            "assets/models/Warehouse_xyz_n_uv.ply",
            pWarehouse->positionXYZ,
            pWarehouse->rotationEulerXYZ,
            pWarehouse->uniformScale);

    }

    {
        sMesh* pTerrain = new sMesh();
        pTerrain->modelFileName = "assets/models/Simple_MeshLab_terrain_x5_xyz_N_uv.ply";
        pTerrain->positionXYZ = glm::vec3(0.0f, -150.0f, 0.0f);
        pTerrain->uniqueFriendlyName = "Terrain";
        pTerrain->rotationEulerXYZ.y = 90.0f;
        pTerrain->textures[0] = "Grey_Brick_Wall_Texture.bmp";
        pTerrain->blendRatio[0] = 1.0f;
        //

        ::g_vecMeshesToDraw.push_back(pTerrain);
    }
        
    {


        sMesh* pFlatPlane = new sMesh();
        pFlatPlane->modelFileName = "assets/models/Flat_Plane_xyz_N_uv.ply";
        pFlatPlane->positionXYZ = glm::vec3(0.0f, -5.5f, 0.0f);
        pFlatPlane->rotationEulerXYZ.y = 90.0f;
        pFlatPlane->objectColourRGBA = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        pFlatPlane->uniqueFriendlyName = "Ground";
        //
        pFlatPlane->textures[0] = "dua-lipa-promo.bmp";     // 1.0
        pFlatPlane->textures[1] = "Puzzle_parts.bmp";       // 0.0
        pFlatPlane->textures[2] = "shape-element-splattered-texture-stroke_1194-8223.bmp";
        pFlatPlane->textures[3] = "Grey_Brick_Wall_Texture.bmp";

//        pFlatPlane->alphaTransparency = 0.5f;

        pFlatPlane->blendRatio[0] = 0.0f;
        pFlatPlane->blendRatio[1] = 1.0f;

        pFlatPlane->bIsVisible = false;

        //
        //        pFlatPlane->bIsWireframe = true;
        //        ::g_myMeshes[::g_NumberOfMeshesToDraw] = pFlatPlane;
        //        ::g_NumberOfMeshesToDraw++;
        ::g_vecMeshesToDraw.push_back(pFlatPlane);


        // Add the "ground" to the physcs
        cPhysics::sAABB* pAABBGround = new cPhysics::sAABB();
        pAABBGround->centreXYZ = pFlatPlane->positionXYZ;
        sModelDrawInfo planeMeshInfo;
        ::g_pMeshManager->FindDrawInfoByModelName(pFlatPlane->modelFileName, planeMeshInfo);

       // Manually enter the AABB info:
        pAABBGround->centreXYZ = glm::vec3(0.0f, 0.0f, 0.0f);   
        // How far from the centre the XYZ min and max are
        // This information is from the mesh we loaded
        // WARNING: We need to be careful about the scale
        pAABBGround->minXYZ.x = -100.0f;
        pAABBGround->maxXYZ.x = 100.0f;

        pAABBGround->minXYZ.z = -100.0f;
        pAABBGround->maxXYZ.z = 100.0f;

        pAABBGround->minXYZ.y = -1.0f;
        pAABBGround->maxXYZ.y = 1.0f;

        // Copy the physics object position from the initial mesh position
        pAABBGround->pPhysicInfo->position = pFlatPlane->positionXYZ;

        // Don't move this ground (skip integration step)
        pAABBGround->pPhysicInfo->bDoesntMove = true;

        pAABBGround->pPhysicInfo->pAssociatedDrawingMeshInstance = pFlatPlane;

        ::g_pPhysicEngine->vecAABBs.push_back(pAABBGround);
    }
//    {
//        sMesh* pFlatPlane = new sMesh();
////        pFlatPlane->modelFileName = "assets/models/Flat_Plane_xyz.ply";
////        pFlatPlane->modelFileName = "assets/models/Flat_Plane_xyz_N.ply";
//        pFlatPlane->modelFileName = "assets/models/Flat_Plane_xyz_N_uv.ply";
//        pFlatPlane->positionXYZ = glm::vec3(0.0f, -5.0f, 0.0f);
//        pFlatPlane->bIsWireframe = true;
//        pFlatPlane->uniformScale = 1.01f;
//        pFlatPlane->objectColourRGBA = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
//
//        ::g_vecMeshesToDraw.push_back(pFlatPlane);
//    }



    {

        //    ____                _            __                   _     
        //   |  _ \ ___ _ __   __| | ___ _ __ / / __ ___   ___  ___| |__  
        //   | |_) / _ \ '_ \ / _` |/ _ \ '__/ / '_ ` _ \ / _ \/ __| '_ \ 
        //   |  _ <  __/ | | | (_| |  __/ | / /| | | | | |  __/\__ \ | | |
        //   |_| \_\___|_| |_|\__,_|\___|_|/_/ |_| |_| |_|\___||___/_| |_|
        //                                                                
        sMesh* pSphereMesh = new sMesh();
//        pSphereMesh->modelFileName = "assets/models/Sphere_radius_1_xyz.ply";
//        pSphereMesh->modelFileName = "assets/models/Sphere_radius_1_xyz_N.ply";
        pSphereMesh->modelFileName = "assets/models/Sphere_radius_1_xyz_N_uv.ply";
        pSphereMesh->positionXYZ = glm::vec3(-15.0f, -3.0f, -20.0f);
        //pSphereMesh->bIsWireframe = true;
        pSphereMesh->objectColourRGBA = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
        pSphereMesh->uniqueFriendlyName = "Ball";

        //::g_myMeshes[::g_NumberOfMeshesToDraw] = pSphere;
        //::g_NumberOfMeshesToDraw++;
        ::g_vecMeshesToDraw.push_back(pSphereMesh);

        {
            sMesh* pSphereShadowMesh = new sMesh();
//            pSphereShadowMesh->modelFileName = "assets/models/Sphere_radius_1_Flat_Shadow_xyz_N.ply";
            pSphereShadowMesh->modelFileName = "assets/models/Sphere_radius_1_Flat_Shadow_xyz_N_uv.ply";
            pSphereShadowMesh->positionXYZ = pSphereMesh->positionXYZ;
            pSphereShadowMesh->positionXYZ.y = -3.95f;  // JUST above the ground
            pSphereShadowMesh->objectColourRGBA = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            pSphereShadowMesh->uniqueFriendlyName = "Ball_Shadow";
            ::g_vecMeshesToDraw.push_back(pSphereShadowMesh);
        }


        //    ____  _               _                  _     _           _   
        //   |  _ \| |__  _   _ ___(_) ___ ___    ___ | |__ (_) ___  ___| |_ 
        //   | |_) | '_ \| | | / __| |/ __/ __|  / _ \| '_ \| |/ _ \/ __| __|
        //   |  __/| | | | |_| \__ \ | (__\__ \ | (_) | |_) | |  __/ (__| |_ 
        //   |_|   |_| |_|\__, |___/_|\___|___/  \___/|_.__// |\___|\___|\__|
        //                |___/                           |__/               
        // Add sphere
        cPhysics::sSphere* pSphereInfo = new cPhysics::sSphere();

        pSphereInfo->centre = glm::vec3(0.0f);  // Sphere's centre (i.e. an offset from the position)

        pSphereInfo->pPhysicInfo->position = pSphereMesh->positionXYZ;
        // HACK: We know this is 1.0 because...?
        // We could also have pulled that information from the mesh info
        pSphereInfo->radius = 1.0f;

        pSphereInfo->pPhysicInfo->velocity.y = 7.5f;
        
        // Set some x velocity
        pSphereInfo->pPhysicInfo->velocity.x = 1.0f;


        pSphereInfo->pPhysicInfo->acceleration.y = -3.0f;
        
        // Associate this drawing mesh to this physics object
        pSphereInfo->pPhysicInfo->pAssociatedDrawingMeshInstance = pSphereMesh;

        ::g_pPhysicEngine->vecSpheres.push_back(pSphereInfo);
    }


    for ( unsigned int ballCount = 0; ballCount != 10; ballCount++ )
    {
        //    ____                _            __                   _     
        //   |  _ \ ___ _ __   __| | ___ _ __ / / __ ___   ___  ___| |__  
        //   | |_) / _ \ '_ \ / _` |/ _ \ '__/ / '_ ` _ \ / _ \/ __| '_ \ 
        //   |  _ <  __/ | | | (_| |  __/ | / /| | | | | |  __/\__ \ | | |
        //   |_| \_\___|_| |_|\__,_|\___|_|/_/ |_| |_| |_|\___||___/_| |_|
        //                                                                
        sMesh* pSphereMesh = new sMesh();
        //        pSphereMesh->modelFileName = "assets/models/Sphere_radius_1_xyz.ply";
//        pSphereMesh->modelFileName = "assets/models/Sphere_radius_1_xyz_N.ply";
        pSphereMesh->modelFileName = "assets/models/Sphere_radius_1_xyz_N_uv.ply";
        pSphereMesh->positionXYZ.x = getRandomFloat(-30.0f, 30.0f);
        pSphereMesh->positionXYZ.z = getRandomFloat(-30.0f, 30.0f);
        pSphereMesh->positionXYZ.y = getRandomFloat(0.0f, 40.0f);
        //pSphereMesh->bIsWireframe = true;
        pSphereMesh->objectColourRGBA = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        pSphereMesh->objectColourRGBA.r = getRandomFloat(0.0f, 1.0f);
        pSphereMesh->objectColourRGBA.g = getRandomFloat(0.0f, 1.0f);
        pSphereMesh->objectColourRGBA.b = getRandomFloat(0.0f, 1.0f);
        std::stringstream ssBallName;
        ssBallName << "Bouncy_" << ballCount;
        pSphereMesh->uniqueFriendlyName = ssBallName.str();

        //
        pSphereMesh->textures[0] = "Non-uniform concrete wall 0512-3-1024x1024.bmp";

        ::g_vecMeshesToDraw.push_back(pSphereMesh);

        //    ____  _               _                  _     _           _   
        //   |  _ \| |__  _   _ ___(_) ___ ___    ___ | |__ (_) ___  ___| |_ 
        //   | |_) | '_ \| | | / __| |/ __/ __|  / _ \| '_ \| |/ _ \/ __| __|
        //   |  __/| | | | |_| \__ \ | (__\__ \ | (_) | |_) | |  __/ (__| |_ 
        //   |_|   |_| |_|\__, |___/_|\___|___/  \___/|_.__// |\___|\___|\__|
        //                |___/                           |__/               
        // Add sphere
        cPhysics::sSphere* pSphereInfo = new cPhysics::sSphere();
        pSphereInfo->centre = glm::vec3(0.0f);  // Sphere's centre (i.e. an offset from the position)
        pSphereInfo->pPhysicInfo->position = pSphereMesh->positionXYZ;
        pSphereInfo->radius = 1.0f;
        pSphereInfo->pPhysicInfo->velocity.y = getRandomFloat(2.0f, 10.0f);
        pSphereInfo->pPhysicInfo->velocity.x = getRandomFloat(-5.0f, 5.0f);
        pSphereInfo->pPhysicInfo->velocity.z = getRandomFloat(-5.0f, 5.0f);
        pSphereInfo->pPhysicInfo->acceleration.y = -3.0f;
        pSphereInfo->pPhysicInfo->pAssociatedDrawingMeshInstance = pSphereMesh;
        ::g_pPhysicEngine->vecSpheres.push_back(pSphereInfo);
    }//for ( unsigned int ballCount




    return;
}


// Add object to scene through Lua
// AddMeshToScene('plyname.ply', 'friendlyName', x, y, z);
int g_Lua_AddMeshToScene(lua_State* L)
{
//    std::cout << "g_Lua_AddMeshToScene" << std::endl;

    //{
    //    sModelDrawInfo galacticaModel;
    //    ::g_pMeshManager->LoadModelIntoVAO("assets/models/Battlestar_Galactica_Res_0_(444,087 faces)_xyz_n_uv (facing +z, up +y).ply",
    //        galacticaModel, program);
    //    std::cout << galacticaModel.meshName << ": " << galacticaModel.numberOfVertices << " vertices loaded" << std::endl;
    //}

    // AddMeshToScene('plyname.ply', 'friendlyName', x, y, z);

    sMesh* pNewMesh = new sMesh();
    pNewMesh->modelFileName = lua_tostring(L, 1);       // 'plyname.ply'
    pNewMesh->uniqueFriendlyName = lua_tostring(L, 2);  // Friendly name
    pNewMesh->positionXYZ.x = (float)lua_tonumber(L, 3);
    pNewMesh->positionXYZ.y = (float)lua_tonumber(L, 4);
    pNewMesh->positionXYZ.z = (float)lua_tonumber(L, 5);
    pNewMesh->textures[0] = lua_tostring(L, 6);
    pNewMesh->blendRatio[0] = (float)lua_tonumber(L, 7);
    //
    pNewMesh->bIsVisible = true;
    ::g_vecMeshesToDraw.push_back(pNewMesh);

    return 0;
}



























//using namespace std;

void ConsoleStuff(void)
{
    // "o" for output
//    std::ofstream myFile("someData.txt");
    // Write something
    //myFile << "Hello" << std::endl;
    //myFile << "there";
    //myFile.close();

    // Now read this file
//    std::ifstream myFile2("someData.txt");
//    std::string someString;
//    myFile2 >> someString;
//    std::cout << someString << std::endl;
//
    //std::string aword;
    //while (aword != "END_OF_FILE")
    //{
    //    myFile2 >> aword;
    //    std::cout << aword << std::endl;
    //};

    //std::string aword;
    //while (myFile2 >> aword)
    //{
    //    std::cout << aword << std::endl;
    //};

    std::ifstream myFile2("assets/models/bun_zipper_res3.ply");
    if (myFile2.is_open())
    {

        std::string aword;
        while (myFile2 >> aword)
        {
            std::cout << aword << std::endl;
        };
    }
    else
    {
        std::cout << "Can't find file" << std::endl;
    }


    // iostream
    std::cout << "Type a number:" << std::endl;

    int x = 0;
    std::cin >> x;

    std::cout << "You typed: " << x << std::endl;

    std::cout << "Type your name:" << std::endl;
    std::string name;
    std::cin >> name;

    std::cout << "Hello " << name << std::endl;
    return;
}


//int& getNumber(void)
//{
//    int p = 0;
//    return p;
//}

//cTankFactory* pTankFactory = NULL;
cTankBuilder* pTheTankBuilder = NULL;

// This is here for speed 
void SetUpTankGame(void)
{
 
    ::g_pTankArena = new cArena();

    if (!pTheTankBuilder)
    {
        pTheTankBuilder = new cTankBuilder();
    }



    

    std::vector<std::string> vecTankTpyes;
//    pTankFactory->GetTankTypes(vecTankTpyes);
//    cTankFactory::get_pTankFactory()->GetTankTypes(vecTankTpyes);
    pTheTankBuilder->GetTankTypes(vecTankTpyes);
    std::cout << "The tank factory can create "
        << vecTankTpyes.size() << " types of tanks:" << std::endl;
    for (std::string tankTypeString : vecTankTpyes)
    {
        std::cout << tankTypeString << std::endl;
    }
    std::cout << std::endl;

    // Create 1 super tank
//    iTank* pTheTank = cTankFactory::get_pTankFactory()->CreateATank("Super Tank");
    iTank* pTheTank = pTheTankBuilder->CreateATank("Super Tank!");
    if (pTheTank)
    {
        ::g_vecTheTanks.push_back(pTheTank);
    }

    // Create 10 tanks
    for (unsigned int count = 0; count != 50; count++)
    {
//        iTank* pTheTank = cTankFactory::get_pTankFactory()->CreateATank("Regular Tank");
        iTank* pTheTank = pTheTankBuilder->CreateATank("Regular Tank with Shield");
        if (pTheTank)
        {
            ::g_vecTheTanks.push_back(pTheTank);
        }
    }
    
    // Also a hover tank
//    iTank* pHoverTank = cTankFactory::get_pTankFactory()->CreateATank("Hover Tank");
    iTank* pHoverTank = pTheTankBuilder->CreateATank("Hover Tank");
    if (pHoverTank)
    {
        ::g_vecTheTanks.push_back(pHoverTank);
    }



    const float WORLD_SIZE(100.0f);

    for (iTank* pCurrentTank : ::g_vecTheTanks)
    {
        glm::vec3 tankLocXYZ;
        tankLocXYZ.x = getRandomFloat(-WORLD_SIZE, WORLD_SIZE);
        tankLocXYZ.y = -5.0f;
        tankLocXYZ.z = getRandomFloat(-WORLD_SIZE, WORLD_SIZE);

        pCurrentTank->setLocation(tankLocXYZ);
    }

    // Tell the tanks about the mediator
    for (iTank* pCurrentTank : ::g_vecTheTanks)
    {
        pCurrentTank->setMediator(::g_pTankArena);
    }


    for (iTank* pCurrentTank : ::g_vecTheTanks)
    {
        ::g_pTankArena->AddTank(pCurrentTank);
    }

    return;
}


void TankStepFrame(double timeStep)
{



    return;
}

// x = 5, y = 15    --> 0, 1
// x = 40.0, y = 80.0   --> [4][8]
// (40.0, 80.0) --> box size = 100
//   [0][0]   
void calcBoxXYFromCoord(float x, float y, int &xIndex, int &yIndex, float boxSize)
{
    xIndex = (int)(x / boxSize);
    yIndex = (int)(y / boxSize);
    return;
}


void AABBOctTree(void)
{
    struct sSquare
    {
        //       vector< cTriangles* > vecTriangleInThisSquare
        glm::vec2 minXY;
        glm::vec2 maxXY;
        float width;
        unsigned int indexColRow;
    };

    sSquare grid[10][10];
    float sqaureWidth = 10;

    for (unsigned int x = 0; x < 10; x++)
    {
        for (unsigned int y = 0; y < 10; y++)
        {
            grid[x][y].width = sqaureWidth;
            grid[x][y].minXY.x = sqaureWidth * x;
            grid[x][y].minXY.y = sqaureWidth * y;

            grid[x][y].maxXY.x = sqaureWidth * x + sqaureWidth;
            grid[x][y].maxXY.y = sqaureWidth * y + sqaureWidth;
        }
    }

    int xIndex, yIndex;
    calcBoxXYFromCoord(5.0f, 15.0f, xIndex, yIndex, sqaureWidth);
    std::cout << xIndex << ", " << yIndex << std::endl;


    calcBoxXYFromCoord(40.0f, 80.0f, xIndex, yIndex, sqaureWidth);
    std::cout << xIndex << ", " << yIndex << std::endl;




    return;
}