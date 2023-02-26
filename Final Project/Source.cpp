//---                Final Project                   ---
//---               Sarah C Jodrey                   ---
//---  CS-330-T3337:Comp Graphic & Visualization     ---
//---              February 19, 2023                 ---
//------------------------------------------------------

#include <iostream>         
#include <cstdlib>          
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Headers/camera.h" //Camera class
#define STB_IMAGE_IMPLEMENTATION
#include "Headers/stb_image.h" //Image loading for textures
#include "Headers/sphere.h" //Sphere class

using namespace std;

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    const char* const WINDOW_TITLE = "Final Project - Sarah C Jodrey"; // Macro for window title

    // Variables for window width and height
    const int WINDOW_WIDTH = 1200, WINDOW_HEIGHT = 900;

    //Store the GL data relative to a given mesh
    struct GLMesh
    {
        GLuint vao[6];         // Handle for the VAO
        GLuint vbo[6];         // Handle for the VBO
        GLuint nVertices[6];    // Number of vertices of the mesh
    };

    // Main GLFW window
    GLFWwindow* gWindow = nullptr;

    // Triangle mesh data
    GLMesh objectMesh;
    const GLuint floatsPerVertex = 3, floatsPerNormal = 3, floatsPerUV = 2;

    //Textures
    GLuint magmaTextureId, moonTextureId, barkTextureId, mossTextureId, rockTextureId;

    // Shader programs
    GLuint objectProgramId, lampProgramId;

    //Camera
    Camera camera(glm::vec3(0.0f, 0.0f, 6.0f));
    float lastX = WINDOW_WIDTH / 2.0f;
    float lastY = WINDOW_HEIGHT / 2.0f;
    bool  firstMouse = true;
    bool isPerspective = true; //defaults to 3D perspective

    // Timing
    float gDeltaTime = 0.0f, gLastFrame = 0.0f;

    //Object's position and scale
    glm::vec3 crustPosition(-0.0f, -1.50f, 0.0f), scaleCrust(2.0f, 1.0f, 2.0f);
    glm::vec3 magmaPosition(0.0f, -1.6f, -2.0f), scaleMagma(4.0f, -2.75f, 4.02f);
    glm::vec3 stumpPosition(-1.10f, -1.20f, -2.88f), scaleStump(0.1f, 0.35f, 0.10f);
    glm::vec3 treePosition(-1.10f, -1.03f, -2.90f), scaleTree(.50f, 2.20f, 0.50f);
    glm::vec3 logPosition(0.50f, -1.35f, -1.60f), scaleLog(1.3f, 0.08f, .120f);
    glm::vec3 moonPosition(-2.5f, 2.350f, -3.50f), scaleMoon(0.3f, 0.3, 0.3f);
    glm::vec3 scaleAsteroid(0.10f, 0.10f, 0.10f);
    //World space position of asteroids
    glm::vec3 asteroidPositions[] = {
        glm::vec3(-3.0f,  1.80f,  0.0f),
        glm::vec3(1.0f,  0.80f, -1.0f),
        glm::vec3(-1.5f, 2.50f, -2.5f),
        glm::vec3(-2.0f, 0.10f, -2.3f),
        glm::vec3(2.0f, 3.0f, -3.5f),
        glm::vec3(-1.7f, 1.0f, -0.5f),
        glm::vec3(-1.5f, 1.5f, -2.5f),
        glm::vec3(1.5f,  .20f, -2.5f),
        glm::vec3(1.0f,  1.0f, -2.5f),
        glm::vec3(4.0f,  0.50f, -3.0f),
         glm::vec3(-4.5f, .40f, -2.5f),
        glm::vec3(-3.0f,  1.0f, 1.5f),
        glm::vec3(4.50f,  0.50f, -1.0f)
    };

    //Key Light - moon light
    glm::vec3 keyLightPosition(-2.5f, 4.0f, -3.0f), keyLightColor(0.89f, 0.9f, 0.9f); //White key light   
    //Fill Light - space
    glm::vec3 fillLightPosition(2.35f, 0.0f, 1.0f), fillLightColor(0.0f, 0.0f, 0.0f); //Black fill light 
    //Lights scale   
    glm::vec3 lightScale(0.2f);
}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void UCreateObjectMesh(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);


/* Object Vertex Shader Source Code*/
const GLchar* objectVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data
layout(location = 1) in vec3 normal; // VAP position 1 for normals
layout(location = 2) in vec2 textureCoordinate; // VAP position 2 for texture

out vec3 vertexNormal; // For outgoing normals to fragment shader
out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
out vec2 vertexTextureCoordinate; // For outgoing texture / pixels to fragment shader

//Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); //Transform vertices into clip coordinates

    vertexFragmentPos = vec3(model * vec4(position, 1.0f)); //Get fragment/pixel position in world space only (exclude view & projection)

    vertexNormal = mat3(transpose(inverse(model))) * normal; //Get normal vectors in world space & exclude normal translation properties
    vertexTextureCoordinate = textureCoordinate;
}
);


/* Object Fragment Shader Source Code*/
const GLchar* objectFragmentShaderSource = GLSL(440,

    in vec3 vertexNormal; //For incoming normals
in vec3 vertexFragmentPos; //For incoming fragment position
in vec2 vertexTextureCoordinate;//For incoming texture positions

out vec4 fragmentColor; //Outgoing object color to GPU

//Uniform / Global variables for light color, light position, & camera/view position
uniform vec3 lightColor; //Fill light color
uniform vec3 lightPos; //Fill light position
uniform vec3 keyColor; //Key light color
uniform vec3 keyPos; //Key light position
uniform vec3 viewPosition;
uniform bool layerTextures;
uniform sampler2D uTexture; //Base texture
uniform sampler2D texture2; //Layering texture    

void main()
{
    /*Phong lighting model calculations to generate ambient, diffuse, & specular components*/

    //Calculate Ambient lighting
    float ambientStrength = 0.25f; //Set ambient fill light to 25% 
    float keyStrength = 0.60f; //Set ambient key light to 60%
    vec3 ambient = ambientStrength * lightColor; // Generate ambient fill light color 
    vec3 ambientKey = keyStrength * keyColor; //Generate the ambient key light color

    //Calculate Diffuse lighting
    vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit
    vec3 lightDirection = normalize(lightPos - vertexFragmentPos); // Calculate distance (light direction) between light source & fragments/pixels on object  
    vec3 keyLightDir = normalize(keyPos - vertexFragmentPos);
    // Calculate diffuse impact by generating dot product of normal and light    
    float impact = max(dot(norm, lightDirection), 0.0);
    float keyImpact = max(dot(norm, keyLightDir), 0.0);
    vec3 diffuse = impact * lightColor; // Generate diffuse fill light color
    vec3 diffuseKey = keyImpact * keyColor; //Generate diffuse key light color    

    //Calculate Specular lighting*/
    float specularIntensity = 0.2f; // Set specular light strength
    float highlightSize = 32.0f; // Set specular highlight size
    vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
    vec3 reflectDir = reflect(-lightDirection, norm);// Calculate reflection vector
    //Calculate specular component
    float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
    vec3 specular = specularIntensity * specularComponent * lightColor;

    // Texture holds the color to be used for components
    vec4 textureColor = texture(uTexture, vertexTextureCoordinate);

    //Boolean to mix/layer textures if layerTextures is true
    if (layerTextures) {
        textureColor = mix(texture(uTexture, vertexTextureCoordinate), //Layer/mix multiple textures
            texture(texture2, vertexTextureCoordinate), 0.45);
        vec3 phong = (ambient + ambientKey + diffuse + diffuseKey + specular) * textureColor.xyz;
        fragmentColor = vec4(phong, 1.0); // Send lighting results to GPU
    }

    // Calculate phong result
    vec3 phong = (ambient + ambientKey + diffuse + diffuseKey + specular) * textureColor.xyz;

    fragmentColor = vec4(phong, 1.0); // Send lighting results to GPU
}
);


/* Lamp Shader Source Code*/
const GLchar* lampVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data

    //Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
}
);


/* Fragment Shader Source Code*/
const GLchar* lampFragmentShaderSource = GLSL(440,
    out vec4 fragmentColor; //Outgoing lamp color to the GPU
void main()
{
    fragmentColor = vec4(0.82f, 0.83f, 0.85f, 1.0f); // Set lamp color to soft white with alpha 1.0

}
);


// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
    for (int j = 0; j < height / 2; ++j)
    {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;

        for (int i = width * channels; i > 0; --i)
        {
            unsigned char tmp = image[index1];
            image[index1] = image[index2];
            image[index2] = tmp;
            ++index1;
            ++index2;
        }
    }
}

//Main program
int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    // Create the mesh
    UCreateObjectMesh(objectMesh);

    // Create the Object shader programs
    if (!UCreateShaderProgram(objectVertexShaderSource, objectFragmentShaderSource, objectProgramId))
        return EXIT_FAILURE;
    //Create the Lamp Shader Program
    if (!UCreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource, lampProgramId))
        return EXIT_FAILURE;

    // Load texture
    const char* magma = "C:/SNHU/3D_Project/lava2.jpg";
    if (!UCreateTexture(magma, magmaTextureId))
    {
        cout << "Failed to load texture " << magma << endl;
        return EXIT_FAILURE;
    }
    // Load texture
    const char* moon = "C:/SNHU/3D_Project/moon.jpg";
    if (!UCreateTexture(moon, moonTextureId))
    {
        cout << "Failed to load texture " << moon << endl;
        return EXIT_FAILURE;
    }
    // Load texture
    const char* bark = "C:/SNHU/3D_Project/bark4.jpg";
    if (!UCreateTexture(bark, barkTextureId))
    {
        cout << "Failed to load texture " << bark << endl;
        return EXIT_FAILURE;
    }
    // Load texture
    const char* moss = "C:/SNHU/3D_Project/moss.jpg";
    if (!UCreateTexture(moss, mossTextureId))
    {
        cout << "Failed to load texture " << moss << endl;
        return EXIT_FAILURE;
    }
    // Load texture
    const char* rock = "C:/SNHU/3D_Project/rock.png";
    if (!UCreateTexture(rock, rockTextureId))
    {
        cout << "Failed to load texture " << rock << endl;
        return EXIT_FAILURE;
    }
    //Tell OpenGL which texture unit each sampler belongs to (only done once)
    glUseProgram(objectProgramId);

    //Set the texture units
    glUniform1i(glGetUniformLocation(objectProgramId, "uTexture"), 0);
    glUniform1i(glGetUniformLocation(objectProgramId, "texture2"), 1);

    //Set the background color of the window
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    //Render loop
    while (!glfwWindowShouldClose(gWindow))
    {
        //Per-frame timing
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        //Input
        UProcessInput(gWindow);

        //Render this frame
        URender();

        glfwPollEvents();
    }

    // Release mesh data
    UDestroyMesh(objectMesh);

    // Release texture
    UDestroyTexture(magmaTextureId);
    UDestroyTexture(moonTextureId);
    UDestroyTexture(barkTextureId);
    UDestroyTexture(mossTextureId);
    UDestroyTexture(rockTextureId);

    // Release shader programs
    UDestroyShaderProgram(objectProgramId);
    UDestroyShaderProgram(lampProgramId);

    exit(EXIT_SUCCESS); // Terminates the program successfully
}


// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // GLFW: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW: window creation
    // ---------------------
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);
    glfwSetMouseButtonCallback(*window, UMouseButtonCallback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLEW: initialize
    // ----------------
    // Note: if using GLEW version 1.13 or earlier
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    // Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}


//Process keyboard input (glfw)
void UProcessInput(GLFWwindow* window)
{
    //Escape window
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    //Move around window
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, gDeltaTime);

    //Look Up / Down
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, gDeltaTime);

    //Toggle views between perspective (3D) and ortho (2D)
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        isPerspective = true;
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        isPerspective = false;
}


// Called when user or OS resizes window (glfw)
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


//Called whenever mouse moves (glfw)
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}


// Called for scroll wheel movement - changes movement speed of camera (glfw)
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

//Handle mouse button events (glfw)
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
    {
        if (action == GLFW_PRESS)
            cout << "Left mouse button pressed" << endl;
        else
            cout << "Left mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_MIDDLE:
    {
        if (action == GLFW_PRESS)
            cout << "Middle mouse button pressed" << endl;
        else
            cout << "Middle mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_RIGHT:
    {
        if (action == GLFW_PRESS)
            cout << "Right mouse button pressed" << endl;
        else
            cout << "Right mouse button released" << endl;
    }
    break;

    default:
        cout << "Unhandled mouse button event" << endl;
        break;
    }
}


// Function called to render a frame
void URender()
{
    // Enable z-depth
    glEnable(GL_DEPTH_TEST);

    // Clear the frame and z buffers
    glClearColor(0.02f, 0.01f, 0.04f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //Set shader program
    glUseProgram(objectProgramId);

    //Camera/view transformation
    glm::mat4 view = camera.GetViewMatrix();

    //Check for user selected projection - default is perspective(3D).
    glm::mat4 projection;
    if (isPerspective) {
        //P for perspective view
        projection = glm::perspective(glm::radians(camera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
    }
    else {
        //O for ortho view
        projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 100.0f);
    }

#pragma region Cone / Tree

    // Model matrix: transformations are applied right-to-left order
    glm::mat4 model = glm::translate(treePosition) * glm::scale(scaleTree);

    //Retrieve & pass transform matrices to the Shader program
    GLint modelLoc = glGetUniformLocation(objectProgramId, "model");
    GLint viewLoc = glGetUniformLocation(objectProgramId, "view");
    GLint projLoc = glGetUniformLocation(objectProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Reference matrix uniforms from the Object Shader program for lights color, lights position, and camera position
    GLint fillLightColorLoc = glGetUniformLocation(objectProgramId, "lightColor");//fill light
    GLint fillLightPositionLoc = glGetUniformLocation(objectProgramId, "lightPos");
    GLint keyLightColorLoc = glGetUniformLocation(objectProgramId, "keyColor"); //key light
    GLint keyLightPositionLoc = glGetUniformLocation(objectProgramId, "keyPos");
    GLint viewPositionLoc = glGetUniformLocation(objectProgramId, "viewPosition");

    // Pass color, light, and camera data to the Object Shader program's corresponding uniforms   
    glUniform3f(fillLightColorLoc, fillLightColor.r, fillLightColor.g, fillLightColor.b); //fill light
    glUniform3f(fillLightPositionLoc, fillLightPosition.x, fillLightPosition.y, fillLightPosition.z);
    glUniform3f(keyLightColorLoc, keyLightColor.r, keyLightColor.g, keyLightColor.b); //key light
    glUniform3f(keyLightPositionLoc, keyLightPosition.x, keyLightPosition.y, keyLightPosition.z);
    const glm::vec3 cameraPosition = camera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    // Activate the cone VAO for the tree
    glBindVertexArray(objectMesh.vao[3]);

    //Matricies for multiple textures
    GLuint multipleTexturesLoc = glGetUniformLocation(objectProgramId, "layerTextures");
    glUniform1i(multipleTexturesLoc, false); //Set boolean layerTextures to false

    //Bind the moss texture on corresponding units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mossTextureId);

    //Draw the triangles
    glDrawArrays(GL_TRIANGLES, 0, objectMesh.nVertices[3]);
#pragma endregion

#pragma region Cube / Tree Stump
    glUniform1i(multipleTexturesLoc, false);
    //Bind bark texture on corresponding units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, barkTextureId);
    //Transform the cube to a tree stump
    model = glm::translate(stumpPosition) * glm::scale(scaleStump);
    // Reference matrix uniforms from the Object Shader program
    modelLoc = glGetUniformLocation(objectProgramId, "model");
    // Pass matrix data to the Object Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glBindVertexArray(objectMesh.vao[1]);
    glDrawArrays(GL_TRIANGLES, 0, objectMesh.nVertices[1]);

#pragma endregion

#pragma region Plane / Crust Layer  
    glUniform1i(multipleTexturesLoc, true);
    //Bind the moon & moss texture on corresponding units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, moonTextureId);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mossTextureId);
    //Transform the crust layer
    model = glm::translate(crustPosition) * glm::scale(scaleCrust);
    // Reference matrix uniforms from the Object Shader program
    modelLoc = glGetUniformLocation(objectProgramId, "model");
    // Pass matrix data to the Object Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glBindVertexArray(objectMesh.vao[2]);
    glDrawArrays(GL_TRIANGLES, 0, objectMesh.nVertices[2]);
#pragma endregion

#pragma region Plane / Magma Layer
    glUniform1i(multipleTexturesLoc, false);
    //Bind magma texture on corresponding units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rockTextureId);
    //Transform the magma layer
    model = glm::translate(magmaPosition) * glm::scale(scaleMagma);
    // Reference matrix uniforms from the Object Shader program
    modelLoc = glGetUniformLocation(objectProgramId, "model");
    // Pass matrix data to the Object Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glBindVertexArray(objectMesh.vao[0]);
    glDrawArrays(GL_TRIANGLES, 0, objectMesh.nVertices[0]);
#pragma endregion   

#pragma region Cube / Log
    glUniform1i(multipleTexturesLoc, false);
    //Bind bark texture on corresponding units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, barkTextureId);
    //Transform the cube to a log
    model = glm::translate(logPosition) * glm::scale(scaleLog);
    // Reference matrix uniforms from the Object Shader program
    modelLoc = glGetUniformLocation(objectProgramId, "model");
    // Pass matrix data to the Object Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glBindVertexArray(objectMesh.vao[1]);
    glDrawArrays(GL_TRIANGLES, 0, objectMesh.nVertices[1]);
#pragma endregion 

#pragma region Sphere / Moon
    glUniform1i(multipleTexturesLoc, false);
    Sphere Moon(0.35f, 36, 18);
    //Bind moon texture on corresponding units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, moonTextureId);
    //Transform the cube to a log
    model = glm::translate(moonPosition);
    // Reference matrix uniforms from the Object Shader program
    modelLoc = glGetUniformLocation(objectProgramId, "model");
    // Pass matrix data to the Object Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    Moon.Draw();
#pragma endregion 

#pragma region Icosahedron / Asteroids
    glUniform1i(multipleTexturesLoc, true);
    //Bind moon and magma texture on corresponding units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, moonTextureId);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, magmaTextureId);

    for (unsigned int i = 0; i < 13; i++)
    {
        model = glm::translate(asteroidPositions[i]) * glm::scale(scaleAsteroid);
        float angle = 20.0f * i;
        model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
        modelLoc = glGetUniformLocation(objectProgramId, "model");
        // Pass matrix data to the Object Shader program's matrix uniforms
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(objectMesh.vao[4]);
        glDrawArrays(GL_TRIANGLES, 0, objectMesh.nVertices[4]);
    }
#pragma endregion 

#pragma region Fill Light Lamp / Deep Space Abscence of Light
    glUseProgram(lampProgramId);

    //Transform the fill lamp
    model = glm::translate(fillLightPosition) * glm::scale(lightScale);

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(lampProgramId, "model");
    viewLoc = glGetUniformLocation(lampProgramId, "view");
    projLoc = glGetUniformLocation(lampProgramId, "projection");

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glBindVertexArray(objectMesh.vao[0]);

    //Draw fill light imitating darkness of space
    glDrawArrays(GL_TRIANGLES, 0, objectMesh.nVertices[0]);

#pragma endregion

#pragma region Key Light Lamp / Moon Light
    //Transform the pyramid key lamp
    model = glm::translate(keyLightPosition) * glm::scale(lightScale);
    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(lampProgramId, "model");
    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glBindVertexArray(objectMesh.vao[0]);
    //Draw key light imitating moon light
    glDrawArrays(GL_TRIANGLES, 0, objectMesh.nVertices[0]);

#pragma endregion

    // Deactivate the VAO & shader program
    glBindVertexArray(0);
    glUseProgram(0);

    //Swap buffers and poll IO events (glfw)
    glfwSwapBuffers(gWindow);  // Flips the the back buffer with the front buffer every frame
}


// Implements the mesh function
void UCreateObjectMesh(GLMesh& mesh)
{
    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);// The number of floats before each

    GLfloat pyramidVerts[] = {

        //Vertices Coordinates    //Normals         //Texture Coordinates    
        //Bottom Triangle        //Negative Y
        -0.5f, 0.0f, 0.5f,       0.0f,-1.0f,0.0f,    0.0f, 0.0f,
        -0.5f, 0.0f, -0.5f,      0.0f,-1.0f,0.0f,    2.0f, 0.0f,
        0.5f,  0.0f, -0.5f,      0.0f,-1.0f,0.0f,    1.0f, 2.0f,
        -0.5f,  0.0f, 0.5f,      0.0f,-1.0f,0.0f,    1.0f,2.0f,
        0.5f,  0.0f, -0.5f,      0.0f,-1.0f,0.0f,    0.0f,0.0f,
        0.5f, 0.0f, 0.5f,        0.0f,-1.0f,0.0f,    2.0f,0.0f,
        //Left Side Triangle    //Negative X
        -0.5f, 0.0f, 0.5f,      -1.0f, 0.0f,0.0f,    0.0f, 0.0f,
        -0.5f, 0.0f, -0.5f,     -1.0f, 0.0f,0.0f,    2.0f, 0.0f,
        0.0f, 0.75f, 0.0f,     -1.0f, 0.0f, 0.0f,   1.0f, 2.0f,
        //Back Side Triangle    //Negative Z
        -0.5f, 0.0f, -0.5f,      0.0f, 0.0f,-1.0f,   0.0f, 0.0f,
        0.5f,  0.0f, -0.5f,      0.0f, 0.0f,-1.0f,   2.0f, 0.0f,
        0.0f, 0.75f, 0.0f,       0.0f,0.0f,-1.0f,    1.0f, 2.0f,
        //Right Side Triangle   //Positive X
        0.5f, 0.0f, -0.5f,       1.0f, 0.0f,0.0f,    0.0f, 0.0f,
        0.5f, 0.0f, 0.5f,        1.0f, 0.0f,0.0f,    2.0f, 0.0f,
        0.0f, 0.75f, 0.0f,       1.0f, 0.0f,0.0f,    1.0f, 2.0f,
        //Front Side Triangle   //Positive Z
        0.5f, 0.0f, 0.5f,        0.0f, 0.0f,1.0f,    0.0f, 0.0f,
        -0.5f, 0.0f, 0.5f,       0.0f, 0.0f,1.0f,    2.0f, 0.0f,
        0.0f, 0.75f, 0.0f,       0.0f, 0.0f,1.0f,    1.0f, 2.0f
    };

    GLfloat cubeVerts[] = {
        //Positions          //Normals            //Texture Coords.
        //Back Face          //Negative Z Normal  
       -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
       -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
       -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

       //Front Face         //Positive Z Normal
      -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
       0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
       0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
       0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
      -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
      -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,

      //Left Face          //Negative X Normal
     -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
     -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
     -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
     -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

     //Right Face         //Positive X Normal
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

     //Bottom Face        //Negative Y Normal
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

    //Top Face           //Positive Y Normal
   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
    0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
    0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
   -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
    };

    GLfloat planeVerts[] = {
        // Vertex Positions  //Normals          //Textures 
        1.0f,  0.1f, 0.0f,   0.0f,.0f,1.0f,     0.0f, 0.0f,   // 0 Top Right Front
        1.0f, -0.1f, 0.0f,   0.0f,.0f,1.0f,     1.0f,0.0f,    // 1 Bottom Right
       -1.0f, -0.1f, 0.0f,   0.0f,.0f,1.0f,     1.0f,1.0f,    // 2 Bottom Left
       1.0f,  0.1f, 0.0f,    0.0f,.0f,1.0f,     0.0f, 0.0f,   // 0 Top Right Front
       -1.0f, -0.1f, 0.0f,   0.0f,.0f,1.0f,     1.0f,1.0f,    // 2 Bottom Left
       -1.0f,  0.1f, 0.0f,   0.0f,.0f,1.0f,     0.50f,1.0f,   // 3 Top Left Back

       1.0f,  0.1f, 0.0f,    1.0f,.0f,0.0f,     0.0f, 0.0f,   // 0 Top Right Front
       1.0f, -0.1f, 0.0f,    1.0f,.0f,0.0f,     1.0f,0.0f,    // 1 Bottom Right
       1.0f, -0.1f, -2.0f,   1.0f,.0f,0.0f,     0.50f,1.0f,  //4 Bottom Left Back
       1.0f,  0.1f, 0.0f,    1.0f,.0f,0.0f,     0.0f, 0.0f,   // 0 Top Right Front
       1.0f, -0.1f, -2.0f,   1.0f,.0f,0.0f,     0.50f,1.0f,  //4 Bottom Left Back
       1.0f,  0.1f, -2.0f,   1.0f,.0f,0.0f,     1.0f,1.0f,   //5 Top Right Back

       1.0f,  0.1f, 0.0f,    0.0f,1.0f,0.0f,    0.0f, 0.0f,   // 0 Top Right Front
       1.0f,  0.1f, -2.0f,   0.0f,1.0f,0.0f,    1.0f,1.0f,   //5 Top Right Back
       -1.0f, 0.1f, -2.0f,   0.0f,1.0f,0.0f,    1.0f,0.0f,   //6 Top Left Front
       1.0f,  0.1f, 0.0f,    0.0f,1.0f,0.0f,    0.0f, 0.0f,   // 0 Top Right Front
       -1.0f, 0.1f, 0.0f,    0.0f,1.0f,0.0f,    0.50f,1.0f,   // 3 Top Left Back
       -1.0f, 0.1f, -2.0f,   0.0f,1.0f,0.0f,    1.0f,0.0f,   //6 Top Left Front

       1.0f, -0.1f, -2.0f,   0.0f,.0f,-1.0f,    0.50f,1.0f,  //4 Bottom Left Back
       1.0f,  0.1f, -2.0f,   0.0f,.0f,-1.0f,    1.0f,1.0f,   //5 Top Right Back
       -1.0f,  0.1f, -2.0f,  0.0f,.0f,-1.0f,    1.0f,0.0f,   //6 Top Left Front
       1.0f, -0.1f, -2.0f,   0.0f,.0f,-1.0f,    0.50f,1.0f,  //4 Bottom Left Back
       -1.0f,  0.1f, -2.0f,  0.0f,.0f,-1.0f,    1.0f,0.0f,   //6 Top Left Front
       -1.0f, -0.1f, -2.0f,  0.0f,.0f,-1.0f,    0.0f, 0.0f,   // 7 Bottom Left Front 

       -1.0f, -0.1f, 0.0f,   -1.0f,0.0f,0.0f,    1.0f,1.0f,    // 2 Bottom Left
       -1.0f,  0.1f, 0.0f,   -1.0f,0.0f,0.0f,    0.50f,1.0f,   // 3 Top Left Back
       -1.0f,  0.1f, -2.0f,  -1.0f,0.0f,0.0f,    1.0f,0.0f,   //6 Top Left Front
       -1.0f, -0.1f, 0.0f,   -1.0f,0.0f,0.0f,    1.0f,1.0f,    // 2 Bottom Left
       -1.0f,  0.1f, -2.0f,  -1.0f,0.0f,0.0f,    1.0f,0.0f,   //6 Top Left Front
       -1.0f, -0.1f, -2.0f,  -1.0f,0.0f,0.0f,    0.0f, 0.0f,   // 7 Bottom Left Front

       1.0f, -0.1f, 0.0f,    0.0f,-1.0f,0.0f,    1.0f,0.0f,    // 1 Bottom Right
       1.0f, -0.1f, -2.0f,   0.0f,-1.0f,0.0f,    0.50f,1.0f,  //4 Bottom Left Back
       -1.0f, -0.1f, -2.0f,  0.0f,-1.0f,0.0f,    0.0f, 0.0f,   // 7 Bottom Left Front 
       1.0f, -0.1f, 0.0f,    0.0f,-1.0f,0.0f,    1.0f,0.0f,    // 1 Bottom Right
       -1.0f, -0.1f, 0.0f,   0.0f,-1.0f,0.0f,    1.0f,1.0f,    // 2 Bottom Left
       -1.0f, -0.1f, -2.0f,  0.0f,-1.0f,0.0f,    0.0f, 0.0f   // 7 Bottom Left Front
    };

    GLfloat coneVerts[]{
        // Vertex Positions  //Normals         //Textures
        //Base              //Negative Y
        0.0f,0.0f,0.0f,     0.0f,-1.0f,0.0f,   0.0f,0.0f, //0
        0.0f,0.0f,.80f,     0.0f,-1.0f,0.0f,   1.0f,0.0f,//1
        -0.6,0.0f,0.6f,     0.0f,-1.0f,0.0f,   0.5f,1.0f,//2
        0.0f,0.0f,0.0f,     0.0f,-1.0f,0.0f,   0.0f,0.0f,//0
        -0.6,0.0f,0.6f,     0.0f,-1.0f,0.0f,   1.0f,0.0f,//2
        -.80f,0.0f,0.0f,    0.0f,-1.0f,0.0f,   0.5f,1.0f,//3
        0.0f,0.0f,0.0f,     0.0f,-1.0f,0.0f,   0.0f,0.0f,//0
        -.80f,0.0f,0.0f,    0.0f,-1.0f,0.0f,   1.0f,0.0f,//3
        -0.6f,0.0f,-0.6f,   0.0f,-1.0f,0.0f,   0.5f,1.0f,//4
        0.0f,0.0f,0.0f,     0.0f,-1.0f,0.0f,   0.0f,0.0f,//0
        -0.6f,0.0f,-0.6f,   0.0f,-1.0f,0.0f,   1.0f,0.0f,//4
        0.0f,0.0f,-.80f,    0.0f,-1.0f,0.0f,   0.5f,1.0f,//5
        0.0f,0.0f,0.0f,     0.0f,-1.0f,0.0f,   0.0f,0.0f,//0
        0.0f,0.0f,-.80f,    0.0f,-1.0f,0.0f,   1.0f,0.0f,//5
        0.6f,0.0f,-0.6f,    0.0f,-1.0f,0.0f,   0.5f,1.0f,//6
        0.0f,0.0f,0.0f,     0.0f,-1.0f,0.0f,   0.0f,0.0f,//0
        0.6f,0.0f,-0.6f,    0.0f,-1.0f,0.0f,   1.0f,0.0f,//6
        .80f,0.0f,0.0f,     0.0f,-1.0f,0.0f,   0.5f,1.0f,//7
        0.0f,0.0f,0.0f,     0.0f,-1.0f,0.0f,   0.0f,0.0f,//0
        .80f,0.0f,0.0f,     0.0f,-1.0f,0.0f,   1.0f,0.0f,//7
        0.6f,0.0f,0.6f,     0.0f,-1.0f,0.0f,   0.5f,1.0f,//8
        0.0f,0.0f,0.0f,     0.0f,-1.0f,0.0f,   0.0f,0.0f,//0
        0.60f,0.0f,0.6f,    0.0f,-1.0f,0.0f,   1.0f,0.0f,//8
        0.0f,0.0f,.80f,     0.0f,-1.0f,0.0f,   0.5f,1.0f,//1
        //Side              //Positive Z
        0.0f,0.0f,.80f,     0.0f,0.0f,1.0f,    0.0f,0.0f,//1
        0.0f,1.0f,0.0f,     0.0f,0.0f,1.0f,    0.5f,1.0f,//9
        -0.6,0.0f,0.6f,     0.0f,0.0f,1.0f,    1.0f,0.0f,//2
        //Side              //Negative X
        -0.6,0.0f,0.6f,     -1.0f,0.0f,0.0f,   1.0f,0.0f,//2
        0.0f,1.0f,0.0f,     -1.0f,0.0f,0.0f,   0.5f,1.0f,//9
        -.80f,0.0f,0.0f,    -1.0f,0.0f,0.0f,   0.0f,0.0f,//3
        //Side              //Negative X
        -.80f,0.0f,0.0f,    -1.0f,0.0f,0.0f,   0.0f,0.0f,//3
        0.0f,1.0f,0.0f,     -1.0f,0.0f,0.0f,   0.5f,1.0f,//9
        -0.6f,0.0f,-0.6f,   -1.0f,0.0f,0.0f,   1.0f,0.0f,//4
        //Side              //Negative Z
        -0.6f,0.0f,-0.6f,   0.0f,0.0f,-1.0f,   1.0f,0.0f,//4
         0.0f,1.0f,0.0f,    0.0f,0.0f,-1.0f,   0.5f,1.0f,//9
        0.0f,0.0f,-.80f,    0.0f,0.0f,-1.0f,   0.0f,0.0f,//5
        //Side              //Negative Z
        0.0f,0.0f,-.80f,    0.0f,0.0f,-1.0f,   0.0f,0.0f,//5
        0.0f,1.0f,0.0f,     0.0f,0.0f,-1.0f,   0.5f,1.0f,//9
        0.6f,0.0f,-0.6f,    0.0f,0.0f,-1.0f,   1.0f,0.0f,//6
        //Side              //Positive X
        0.6f,0.0f,-0.6f,    1.0f,0.0f,0.0f,    1.0f,0.0f,//6
        0.0f,1.0f,0.0f,     1.0f,0.0f,0.0f,    0.5f,1.0f,//9
        .80f,0.0f,0.0f,     1.0f,0.0f,0.0f,    0.0f,0.0f,//7
        //Side              //Positive X
         .80f,0.0f,0.0f,    1.0f,0.0f,0.0f,    0.0f,0.0f,//7
        0.0f,1.0f,0.0f,     1.0f,0.0f,0.0f,    0.5f,1.0f,//9
        0.6f,0.0f,0.6f,     1.0f,0.0f,0.0f,    1.0f,0.0f,//8
        //Side              //Positive Z
        0.6f,0.0f,0.6f,     0.0f,0.0f,1.0f,    1.0f,0.0f,//8
        0.0f,1.0f,0.0f,     0.0f,0.0f,1.0f,    0.5f,1.0f,//9
        0.0f,0.0f,.80f,     0.0f,0.0f,1.0f,    0.0f,0.0f//1
    };

    const float X = .53f, Z = .85f;
    GLfloat icoVerts[]{
        -X, 0.0f, Z,     0.0, 0.0, 1.0,   0.0,0.0, //0 
        0.0, Z,  X,      0.0, 0.0, 1.0,   0.50,1.0f, //4
        X, 0.0,  Z,      0.0, 0.0, 1.0,    1.0,0.0f, //1     

        -X, 0.0f, Z,     0.0, 0.0, 1.0,   0.0,0.0, //0 
        -Z, X, 0.0,       0.0, 0.0, 1.0,   0.5,1.0, //9
        0.0, Z,  X,      0.0, 0.0, 1.0,   1.0,0.0,//4

        -Z, X, 0.0,      0.0, 1.0, 0.0,   0.5,1.0, //9
         0.0, Z, -X,     0.0, 1.0, 0.0,   1.0,0.0, //5
         0.0, Z, X,      0.0, 1.0, 0.0,   0.0,0.0, //4

         0.0, Z, X,      0.0, 1.0, 0.0,   0.50,1.0, //4
         0.0, Z, -X,     0.0, 1.0, 0.0,   1.0,0.0, //5
         Z, X, 0.0,      0.0, 1.0, 0.0,   0.0,0.0, //8

         0.0, Z, X,      0.0, 1.0, 0.0,   0.0,0.0,  //4
         Z, X, 0.0,      0.0, 1.0, 0.0,   0.50,1.0,//8
         X, 0.0, Z,      0.0, 1.0, 0.0,   1.0,0.0,  //1 

         Z, X, 0.0,      1.0, 0.0, 0.0,   0.0,0.0, //8
         Z, -X, 0.0,     1.0, 0.0, 0.0,   0.5,1.0, //10
         X, 0.0, Z,      1.0, 0.0, 0.0,   1.0,0.0,  //1

         Z, X, 0.0,      1.0, 0.0, 0.0,   0.0,0.0, //8
         X, 0.0, -Z,     1.0, 0.0, 0.0,   0.5,1.0,  //3
         Z, -X, 0.0,     1.0, 0.0, 0.0,   1.0,0.0, //10

        0.0, Z, -X,     1.0, 0.0, 0.0,   0.0,0.0, //5
        X, 0.0, -Z,     1.0, 0.0, 0.0,   0.5,1.0,  //3
         Z, X, 0.0,     1.0, 0.0, 0.0,   1.0,0.0, //8

        0.0, Z, -X,     0.0, 0.0, -1.0,   0.0,0.0, //5
        -X, 0.0, -Z,    0.0, 0.0, -1.0,   0.5,1.0,  //2
        X, 0.0, -Z,      0.0, 0.0, -1.0,   1.0,0.0,//3

        -X, 0.0, -Z,    0.0, 0.0, -1.0,   0.0,0.0,  //2 
         0.0, -Z, -X,   0.0, 0.0, -1.0,   0.5,1.0, //7
        X, 0.0, -Z,     0.0, 0.0, -1.0,   1.0,0.0,  //3

         0.0, -Z, -X,    1.0, 0.0, 0.0,   0.0,0.0, //7
         Z, -X, 0.0,    1.0, 0.0, 0.0,   0.5,1.0, //10
         X, 0.0, -Z,     1.0, 0.0, 0.0,   1.0,0.0,  //3

          0.0, -Z, -X,    0.0, -1.0, 0.0,   0.0,0.0, //7
          0.0, -Z, X,     0.0, -1.0, 0.0,   0.5,1.0, //6
          Z, -X, 0.0,     0.0, -1.0, 0.0,   1.0,0.0, //10

           0.0, -Z, -X,    0.0, -1.0, 0.0,   0.0,0.0,//7
           -Z, -X, 0.0,    0.0, -1.0, 0.0,   0.5,1.0, //11
           0.0, -Z, X,     0.0, -1.0, 0.0,   1.0,0.0, //6

           -Z, -X, 0.0,    0.0, 0.0, 1.0,   0.0,0.0, //11
           -X, 0.0f, Z,    0.0, 0.0, 1.0,   0.5,1.0, //0
           0.0, -Z, X,     0.0, 0.0, 1.0,   1.0,0.0, //6

           -X, 0.0f, Z,    0.0, 0.0, 1.0,   0.0,0.0, //0
           X, 0.0,  Z,     0.0, 0.0, 1.0,   0.5,1.0,  //1 
           0.0, -Z, X,     0.0, 0.0, 1.0,   1.0,0.0, //6

            0.0, -Z, X,     0.0,.0, 1.0,   0.0,0.0, //6
           X, 0.0, Z,     0.0, .0, 1.0,   0.5,1.0,  //1 
           Z, -X, 0.0,    0.0, .0, 1.0,   1.0,0.0, //10

         -Z, X, 0.0,     -1.0, 0.0, 0.0,    0.0,0.0,//9
         -X, 0.0f, Z,    -1.0, 0.0, 0.0,   .50,1.0,  //0
         -Z, -X, 0.0,    -1.0, 0.0, 0.0,   1.0,0.0, //11

        -Z, X, 0.0,     -1.0, 0.0, 0.0,   0.0,0.0, //9
        -Z, -X, 0.0,    -1.0, 0.0, 0.0,   0.5,1.0, //11
        -X, 0.0, -Z,    -1.0, 0.0, 0.0,   1.0,0.0,  //2

        -Z, X, 0.0,     0.0, 1.0, 0.0,   0.0,0.0, //9
        -X, 0.0, -Z,    0.0, 1.0, 0.0,   0.5,1.0,  //2
        0.0, Z, -X,     0.0, 1.0, 0.0,   1.0,0.0, //5

        0.0, -Z, -X,    0.0, -1.0, 0.0,   0.0,0.0, //7
        -X, 0.0, -Z,    0.0, -1.0, 0.0,   0.5,1.0,  //2
        -Z, -X, 0.0,    0.0, -1.0, 0.0,   1.0,0.0 //11      
    };

    mesh.nVertices[0] = sizeof(pyramidVerts) / (sizeof(pyramidVerts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));
    mesh.nVertices[1] = sizeof(cubeVerts) / (sizeof(cubeVerts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));
    mesh.nVertices[2] = sizeof(planeVerts) / (sizeof(planeVerts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));
    mesh.nVertices[3] = sizeof(coneVerts) / (sizeof(coneVerts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));
    mesh.nVertices[4] = sizeof(icoVerts) / (sizeof(icoVerts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));


#pragma region Pyramid
    glGenVertexArrays(1, &mesh.vao[0]);
    glGenBuffers(1, &mesh.vbo[0]);
    glBindVertexArray(mesh.vao[0]);

    // Create VBO for vertex data    
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVerts), pyramidVerts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);
    //VAP Normals
    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);
    //VAP Textures
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
#pragma endregion
#pragma region Cube

    glGenVertexArrays(1, &mesh.vao[1]);
    glGenBuffers(1, &mesh.vbo[1]);
    glBindVertexArray(mesh.vao[1]);

    // Create VBO for vertex data
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo[1]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), cubeVerts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);
    //VAP Normals
    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);
    //VAP Textures
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
#pragma endregion

#pragma region Plane

    glGenVertexArrays(1, &mesh.vao[2]);
    glGenBuffers(1, &mesh.vbo[2]);
    glBindVertexArray(mesh.vao[2]);

    // Create VBO for vertex data
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo[2]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVerts), planeVerts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);
    //VAP Normals
    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);
    //VAP Textures
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
#pragma endregion

#pragma region Cone

    glGenVertexArrays(1, &mesh.vao[3]);
    glGenBuffers(1, &mesh.vbo[3]);
    glBindVertexArray(mesh.vao[3]);

    // Create VBO for vertex data
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo[3]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(coneVerts), coneVerts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);
    //VAP Normals
    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);
    //VAP Textures
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
#pragma endregion

#pragma region Icosahedron

    glGenVertexArrays(1, &mesh.vao[4]);
    glGenBuffers(1, &mesh.vbo[4]);
    glBindVertexArray(mesh.vao[4]);

    // Create VBO for vertex data
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo[4]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(icoVerts), icoVerts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);
    //VAP Normals
    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);
    //VAP Textures
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
#pragma endregion

}

void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(6, mesh.vao);
    glDeleteBuffers(6, mesh.vbo);
}

/*Generate and load the texture*/
bool UCreateTexture(const char* filename, GLuint& textureId)
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {
        flipImageVertically(image, width, height, channels);

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (channels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else
        {
            cout << "Not implemented to handle image with " << channels << " channels" << endl;
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

        return true;
    }

    // Error loading the image
    return false;
}


void UDestroyTexture(GLuint textureId)
{
    glGenTextures(1, &textureId);
}


// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[512];

    // Create a Shader program object.
    programId = glCreateProgram();

    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive the shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile the vertex shader, and print compilation errors (if any)
    glCompileShader(vertexShaderId); // compile the vertex shader
    // check for shader compile errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glCompileShader(fragmentShaderId); // compile the fragment shader
    // check for shader compile errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);   // links the shader program
    // check for linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);    // Uses the shader program

    return true;
}


void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}