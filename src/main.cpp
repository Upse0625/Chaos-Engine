#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "GameObject.h"

// Prototipos
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
unsigned int loadTexture(const char* path);
void DrawUI(Shader& uiShader, unsigned int uiVAO, unsigned int uiVBO);

// --- Configuración ---
int scr_width = 1280;
int scr_height = 720;

Camera camera(glm::vec3(0.0f, 2.0f, 8.0f));
float lastX = scr_width / 2.0f;
float lastY = scr_height / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

// --- Estado de la UI ---
struct UIState {
    bool isDraggingSlider = false;
    int activeSliderID = -1;
} uiState;

// --- Gestión de la Escena ---
std::vector<GameObject> sceneObjects;
int selectedObjectIndex = -1;
float lightIntensity = 150.0f;
unsigned int nextId = 0;

int main()
{
    // --- Inicialización ---
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(scr_width, scr_height, "Chaos Engine - Editor Nativo", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_STENCIL_TEST);

    // --- Shaders ---
    Shader pbrShader("assets/shaders/basic.vert", "assets/shaders/basic.frag");
    Shader lightCubeShader("assets/shaders/light_cube.vert", "assets/shaders/light_cube.frag");
    Shader gridShader("assets/shaders/grid.vert", "assets/shaders/grid.frag");
    Shader uiShader("assets/shaders/ui.vert", "assets/shaders/ui.frag");
    Shader highlightShader("assets/shaders/highlight.vert", "assets/shaders/highlight.frag");
    Shader pickingShader("assets/shaders/picking.vert", "assets/shaders/picking.frag");


    // --- Geometría y VAOs (Cubo) ---
    float cube_vertices[] = {
        // positions          // normals           // texcoords  // tangent
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,  1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,  1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,  1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f, -1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f, -1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,  1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,  1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,  1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,  1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,  1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,  1.0f,  0.0f,  0.0f
    };
    unsigned int VBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);
    glBindVertexArray(cubeVAO);
    size_t stride = 11 * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (GLsizei)stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (GLsizei)stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, (GLsizei)stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, (GLsizei)stride, (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // --- Geometría para la Grid ---
    unsigned int gridVAO, gridVBO;
    glGenVertexArrays(1, &gridVAO);
    glGenBuffers(1, &gridVBO);
    std::vector<float> gridVertices;
    int gridSize = 20;
    for (int i = -gridSize; i <= gridSize; i++) {
        gridVertices.push_back((float)i); gridVertices.push_back(0.0f); gridVertices.push_back((float)-gridSize);
        gridVertices.push_back((float)i); gridVertices.push_back(0.0f); gridVertices.push_back((float)gridSize);
        gridVertices.push_back((float)-gridSize); gridVertices.push_back(0.0f); gridVertices.push_back((float)i);
        gridVertices.push_back((float)gridSize); gridVertices.push_back(0.0f); gridVertices.push_back((float)i);
    }
    glBindVertexArray(gridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
    glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), &gridVertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // --- Geometría para la UI ---
    unsigned int uiVAO, uiVBO;
    glGenVertexArrays(1, &uiVAO);
    glGenBuffers(1, &uiVBO);
    glBindVertexArray(uiVAO);
    glBindBuffer(GL_ARRAY_BUFFER, uiVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 2, NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    // --- Carga de Texturas PBR ---
    unsigned int albedoMap = loadTexture("assets/textures/albedo.png");
    unsigned int normalMap = loadTexture("assets/textures/normal.png");
    unsigned int metallicMap = loadTexture("assets/textures/metallic.png");
    unsigned int roughnessMap = loadTexture("assets/textures/roughness.png");

    pbrShader.use();
    pbrShader.setInt("albedoMap", 0);
    pbrShader.setInt("normalMap", 1);
    pbrShader.setInt("metallicMap", 2);
    pbrShader.setInt("roughnessMap", 3);

    // --- Gestión de la Escena ---
    sceneObjects.emplace_back(nextId++, "Luz Principal", ShapeType::Cube);
    sceneObjects[0].transform.position = glm::vec3(0.0f, 5.0f, 5.0f);
    sceneObjects.emplace_back(nextId++, "Cubo 1", ShapeType::Cube);
    sceneObjects[1].transform.position = glm::vec3(0.0f, 0.5f, 0.0f);

    // --- Bucle de Renderizado ---
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --- 1. RENDERIZAR LA ESCENA 3D ---
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)scr_width / (float)scr_height, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // Dibujar la grid
        gridShader.use();
        gridShader.setMat4("view", view);
        gridShader.setMat4("projection", projection);
        glBindVertexArray(gridVAO);
        glDrawArrays(GL_LINES, 0, gridVertices.size() / 3);

        // Dibujar los objetos de la escena
        for (const auto& object : sceneObjects)
        {
            if (object.name.find("Luz") != std::string::npos)
            {
                lightCubeShader.use();
                lightCubeShader.setMat4("view", view);
                lightCubeShader.setMat4("projection", projection);
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, object.transform.position);
                model = glm::scale(model, glm::vec3(0.5f));
                lightCubeShader.setMat4("model", model);
                lightCubeShader.setVec3("lightColor", glm::vec3(1.0f));
                glBindVertexArray(cubeVAO);
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
            else
            {
                pbrShader.use();
                pbrShader.setMat4("view", view);
                pbrShader.setMat4("projection", projection);
                pbrShader.setVec3("viewPos", camera.Position);
                pbrShader.setVec3("lightPositions[0]", sceneObjects[0].transform.position);
                pbrShader.setVec3("lightColors[0]", glm::vec3(lightIntensity));

                glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, albedoMap);
                glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, normalMap);
                glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, metallicMap);
                glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, roughnessMap);

                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, object.transform.position);
                model = glm::rotate(model, glm::radians(object.transform.rotation.x), glm::vec3(1, 0, 0));
                model = glm::rotate(model, glm::radians(object.transform.rotation.y), glm::vec3(0, 1, 0));
                model = glm::rotate(model, glm::radians(object.transform.rotation.z), glm::vec3(0, 0, 1));
                model = glm::scale(model, object.transform.scale);
                pbrShader.setMat4("model", model);
                pbrShader.setFloat("ao", 1.0f);
                glBindVertexArray(cubeVAO);
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
        }

        // --- 2. RENDERIZAR LA INTERFAZ NATIVA ---
        glDisable(GL_DEPTH_TEST);
        DrawUI(uiShader, uiVAO, uiVBO);
        glEnable(GL_DEPTH_TEST);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // --- Limpieza ---
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &gridVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &gridVBO);
    pbrShader.Delete();
    lightCubeShader.Delete();
    glfwTerminate();
    return 0;
}

// ... El resto de las funciones (loadTexture, etc.) no cambian ...

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT, deltaTime);
    }
    else
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        firstMouse = true;
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            // Lógica de clics de la UI
        }
        else if (action == GLFW_RELEASE) {
            uiState.isDraggingSlider = false;
        }
    }
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {
        if (firstMouse)
        {
            lastX = xposIn;
            lastY = yposIn;
            firstMouse = false;
        }
        float xoffset = xposIn - lastX;
        float yoffset = lastY - yposIn;
        lastX = xposIn;
        lastY = yposIn;
        camera.ProcessMouseMovement(xoffset, yoffset);
    }
}

unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
        std::cout << "Texture loaded successfully: " << path << std::endl;
    }
    else
    {
        std::cout << "ERROR::TEXTURE::LOAD_FAILED\n" << "Path: " << path << std::endl;
        stbi_image_free(data);
        return 0;
    }

    return textureID;
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    scr_width = width;
    scr_height = height;
}

void DrawUI(Shader& uiShader, unsigned int uiVAO, unsigned int uiVBO)
{
    glm::mat4 ortho = glm::ortho(0.0f, (float)scr_width, 0.0f, (float)scr_height);
    uiShader.use();
    uiShader.setMat4("projection", ortho);

    // Panel de fondo
    float panelWidth = 300.0f;
    float panelX = scr_width - panelWidth;
    float panelY = 0.0f;
    float panelHeight = scr_height;

    float vertices[] = {
        panelX, panelY,
        panelX + panelWidth, panelY,
        panelX, panelY + panelHeight,
        panelX, panelY + panelHeight,
        panelX + panelWidth, panelY,
        panelX + panelWidth, panelY + panelHeight
    };

    glBindVertexArray(uiVAO);
    glBindBuffer(GL_ARRAY_BUFFER, uiVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    uiShader.setVec3("color", 0.2f, 0.2f, 0.2f);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
