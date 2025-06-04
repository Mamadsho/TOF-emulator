#define _CRT_SECURE_NO_WARNINGS
#include <Yaml.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"
#include "model.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


#include <iostream>

// for debugging
//#define RENDER_LOOP

glm::vec3 LIGHT_COLOR = glm::vec3(1.0, 1.0, 1.0);
glm::vec3 CAMERA_POSITION = glm::vec3(0.0f, 4.0f, 3.0f);
glm::vec3 LIGHT_POSITION = glm::vec3(0.0f, 4.0f, -3.0f);
glm::vec3 BOX_SCALE = glm::vec3(10.0f, 10.0f, 10.0f);
glm::vec3 MODEL_COLOR = glm::vec3(1.0f, 0.5f, 0.5f);
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;
std::string MODELPATH = "data/suzanne.stl";
std::vector<std::string> BOX_TEXTURES( 6, "data/container.jpg");


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void SaveScreenToPNG(std::string filename);
void UpdateViewProjection(Shader* shader);
void ParseSettings();
unsigned int loadCubemap(vector<std::string> faces);

void SetupGL(GLFWwindow*& window);


// camera
Camera camera(CAMERA_POSITION);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
    ParseSettings();
    GLFWwindow* window;
    SetupGL(window);
    
    // update camera
    camera.LookAt(glm::vec3(0.0f, 0.0f, 0.0f));

    // build and compile shaders
    // -------------------------
    Shader boxShader("shaders/box.vs", "shaders/box.fs");
    Shader modelShader("shaders/model.vs", "shaders/model.fs");
    Shader tofBoxShader("shaders/tof.vs", "shaders/tof.fs");
    Shader tofModelShader("shaders/tof.vs", "shaders/tof.fs");

    // load Box
    // -----------
    Model box("data/cube.obj");
    // Box textures
    unsigned int cubemapTexture = loadCubemap(BOX_TEXTURES);

    // set box scale
    glm::mat4 boxModelTransform = glm::scale(glm::mat4(1.0f) , BOX_SCALE);
    boxShader.use();
    boxShader.setMat4("model", boxModelTransform);
    tofBoxShader.use();
    tofBoxShader.setMat4("model", boxModelTransform);
        
    Model model(MODELPATH, false, true);
    glm::mat4 modelRotate = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 modelTranslate = glm::translate(modelRotate, glm::vec3(-model.center.x, -model.center.y, -model.min.z));
    glm::mat4 modelTransform = modelTranslate;
    modelShader.use();
    modelShader.setVec3("color", glm::vec3(MODEL_COLOR));
    modelShader.setMat4("model", modelTransform);
    tofModelShader.use();
    tofModelShader.setMat4("model", modelTransform);

    Shader* light_shaders[2] = {&boxShader, &modelShader};
    for (Shader* shader : light_shaders) {
        // set up lights
        shader->use();
        shader->setVec3("light.position", LIGHT_POSITION);

        // light properties
        shader->setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
        shader->setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
        shader->setVec3("light.specular", 1.0f, 1.0f, 1.0f);
        shader->setFloat("shininess", 32.0f);
        shader->setVec3("light.color", LIGHT_COLOR);
    }

    Shader* tof_shaders[2] = { &tofBoxShader, &tofModelShader };
    for (Shader* shader : tof_shaders) {
        shader->use();
        shader->setFloat("factor", 1.0f / glm::length(camera.Position));
    }

    
#ifdef RENDER_LOOP
    while (!glfwWindowShouldClose(window))
    {
#endif

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (Shader* shader : light_shaders)
            UpdateViewProjection(shader);

        boxShader.use();
        box.Draw(boxShader);
        modelShader.use();
        model.Draw(modelShader);

#ifndef RENDER_LOOP
        SaveScreenToPNG("viz");
        camera.Position = glm::vec3(0.0f, BOX_SCALE.y, 0.0f);
        camera.LookAt(glm::vec3(0.0f, 0.0f, 0.0f));

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        for (Shader* shader : tof_shaders) {    
            UpdateViewProjection(shader);
            shader->setFloat("factor", 1.0f / glm::length(camera.Position));
        }

        tofBoxShader.use();
        box.Draw(tofBoxShader);
        tofModelShader.use();
        model.Draw(tofModelShader);
        SaveScreenToPNG("tof");
#endif


#ifdef RENDER_LOOP
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
#endif

    

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

void SetupGL(GLFWwindow*& window)
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    // glfw window creation
    // --------------------
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "TOF Emulator", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(false);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
}

unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void ParseSettings()
{
    Yaml::Node root;
        
    // Check if file parsing succeeded
    try {
        Yaml::Parse(root, "settings.yaml");
    }
    catch (const std::exception& e) {
        std::cerr << "✗ Failed to parse file: " << e.what() << std::endl;
        return;
    }

    // Check if specific fields exist
    if (root["resolution"].IsSequence()
        && root["resolution"][0].IsScalar()
        && root["resolution"][1].IsScalar()) {
        int width = root["resolution"][0].As<int>();
        int height = root["resolution"][1].As<int>();
        SCR_WIDTH = width;
        SCR_HEIGHT = height;
    }
    else {
        std::cout << "Field 'resolution' not found or not an integer[2]" << std::endl;
    }

    if (!root["modelPath"].IsNone())
        MODELPATH = root["modelPath"].As<string>();
    else
        std::cout << "Field 'modelPath' not found" << std::endl;

    if (!root["boxRight"].IsNone())
        BOX_TEXTURES[0] = root["boxRight"].As<std::string>();
    if (!root["boxLeft"].IsNone())
        BOX_TEXTURES[1] = root["boxLeft"].As<string>();
    if (!root["boxTop"].IsNone())
        BOX_TEXTURES[2] = root["boxTop"].As<string>();
    if (!root["boxBottom"].IsNone())
        BOX_TEXTURES[3] = root["boxBottom"].As<string>();
    if (!root["boxFront"].IsNone())
        BOX_TEXTURES[4] = root["boxFront"].As<string>();
    if (!root["boxBack"].IsNone())
        BOX_TEXTURES[5] = root["boxBack"].As<string>();

    if (root["lightColor"].IsSequence()
        && root["lightColor"][0].IsScalar()
        && root["lightColor"][1].IsScalar()
        && root["lightColor"][2].IsScalar()) {
        int x = root["lightColor"][0].As<float>();
        int y = root["lightColor"][1].As<float>();
        int z = root["lightColor"][2].As<float>();
        LIGHT_COLOR = glm::vec3(x, y, z);
    }
    else {
        std::cout << "Field 'lightColor' not found or not a float[3]" << std::endl;
    }

    if (root["cameraPosition"].IsSequence()
        && root["cameraPosition"][0].IsScalar()
        && root["cameraPosition"][1].IsScalar()
        && root["cameraPosition"][2].IsScalar()) {
        int x = root["cameraPosition"][0].As<float>();
        int y = root["cameraPosition"][1].As<float>();
        int z = root["cameraPosition"][2].As<float>();
        CAMERA_POSITION = glm::vec3(x, y, z);
        camera.Position = CAMERA_POSITION;
    }
    else {
        std::cout << "Field 'cameraPosition' not found or not a float[3]" << std::endl;
    }
    
    if (root["boxScale"].IsSequence()
        && root["boxScale"][0].IsScalar()
        && root["boxScale"][1].IsScalar()
        && root["boxScale"][2].IsScalar()) {
        float x = root["boxScale"][0].As<float>();
        float y = root["boxScale"][1].As<float>();
        float z = root["boxScale"][2].As<float>();
        BOX_SCALE = glm::vec3(x, y, z);
    }
    else {
        std::cout << "Field 'boxScale' not found or not a float[3]" << std::endl;
    }
   
    if (root["lightPosition"].IsSequence()
        && root["lightPosition"][0].IsScalar()
        && root["lightPosition"][1].IsScalar()
        && root["lightPosition"][2].IsScalar()) {
        float x = root["lightPosition"][0].As<float>();
        float y = root["lightPosition"][1].As<float>();
        float z = root["lightPosition"][2].As<float>();
        LIGHT_POSITION = glm::vec3(x, y, z);
    }
    else {
        std::cout << "Field 'lightPosition' not found or not a float[3]" << std::endl;
    }
    
    if (root["modelColor"].IsSequence()
        && root["modelColor"][0].IsScalar()
        && root["modelColor"][1].IsScalar()
        && root["modelColor"][2].IsScalar()) {
        float x = root["modelColor"][0].As<float>();
        float y = root["modelColor"][1].As<float>();
        float z = root["modelColor"][2].As<float>();
        MODEL_COLOR = glm::vec3(x, y, z);
    }
    else {
        std::cout << "Field 'modelColor' not found or not a float[3]" << std::endl;
    }
}

void UpdateViewProjection(Shader* shader)
{
    shader->use();

    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 2 * std::max({ BOX_SCALE.x, BOX_SCALE.y, BOX_SCALE.z }));
    glm::mat4 view = camera.GetViewMatrix();
    shader->setMat4("projection", projection);
    shader->setMat4("view", view);
    shader->setVec3("viewPos", camera.Position);
}

void SaveScreenToPNG(std::string filename)
{
    std::vector<unsigned char> pixels(SCR_WIDTH * SCR_HEIGHT * 3);
    glReadPixels(0, 0, SCR_WIDTH, SCR_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // Flip image vertically
    std::vector<unsigned char> flippedPixels(SCR_WIDTH * SCR_HEIGHT * 3);
    for (int y = 0; y < SCR_HEIGHT; y++) {
        for (int x = 0; x < SCR_WIDTH; x++) {
            int srcIndex = ((SCR_HEIGHT - 1 - y) * SCR_WIDTH + x) * 3;
            int dstIndex = (y * SCR_WIDTH + x) * 3;
            flippedPixels[dstIndex] = pixels[srcIndex];
            flippedPixels[dstIndex + 1] = pixels[srcIndex + 1];
            flippedPixels[dstIndex + 2] = pixels[srcIndex + 2];
        }
    }
    // Save to PNG
    stbi_write_png(("exports/" + filename + ".png").c_str(), SCR_WIDTH, SCR_HEIGHT, 3, flippedPixels.data(), SCR_WIDTH * 3);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    // per-frame time logic
        // --------------------
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

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

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}