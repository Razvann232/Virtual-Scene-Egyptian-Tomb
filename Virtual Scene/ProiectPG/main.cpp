#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"

#include <iostream>
#include <iomanip>

// window
gps::Window myWindow;

//window dimensions
//GLuint windowWidth = 1024;
//GLuint windowHeight = 768;

GLuint windowWidth = 1920;
GLuint windowHeight = 1080;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;
glm::vec3 pointLightPosition;

// shader uniform locations
GLuint modelLoc;
GLuint viewLoc;
GLuint projectionLoc;
GLuint normalMatrixLoc;
GLuint lightDirLoc;
GLuint lightColorLoc;
GLuint pointLightPositionLoc;

// camera
gps::Camera myCamera(
    glm::vec3(-3.179, 1.49, -0.34),
    glm::vec3(0.0f, 0.0f, -10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.1f;

GLboolean pressedKeys[1024];

// models
gps::Model3D mainScene;
gps::Model3D sarcophageTop;
gps::Model3D sarcophageInner;
gps::Model3D flyingBook;
GLfloat angle;

// shaders
gps::Shader myBasicShader;

// for the initial animation
bool animation = true;
int stage = 0;
float aux = 0;

//for the object animation
float sarTopMove = 0;
float auxSarTop = -0.005;
float sarInnerMove = 0;
float auxSarInner = 0.005;
float bookMove = 0;
float auxBookMove = 0.005;
bool moveSarInner = false;
bool allowMovement = true;

//for mouse movement
float prevX = 0;
float prevY = 0;
float yaw = 0;
float pitch = 0;

GLenum glCheckError_(const char* file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
        case GL_INVALID_ENUM:
            error = "INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error = "INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            error = "INVALID_OPERATION";
            break;
        case GL_STACK_OVERFLOW:
            error = "STACK_OVERFLOW";
            break;
        case GL_STACK_UNDERFLOW:
            error = "STACK_UNDERFLOW";
            break;
        case GL_OUT_OF_MEMORY:
            error = "OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error = "INVALID_FRAMEBUFFER_OPERATION";
            break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key == GLFW_KEY_P && action == GLFW_RELEASE) { // move the sarcopgage in and out
        allowMovement = !allowMovement;
    }

    if (key == GLFW_KEY_H && action == GLFW_RELEASE) { // solid mode
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    
    if (key == GLFW_KEY_J && action == GLFW_RELEASE) { // wireframe mode
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    if (key == GLFW_KEY_K && action == GLFW_RELEASE) { // point mode
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    }

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        }
        else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (animation) return;  // movement not allowed during the initial animation
    yaw += (xpos - prevX) * 0.1;    // update with the offests
    pitch += (prevY - ypos) * 0.1;

    prevX = xpos;   // update previous position
    prevY = ypos;

    if (pitch > 89.99f) {   // stop the camera from doing a barrel roll
        pitch = 89.99f;
    }
    else if (pitch < -89.99f) {
        pitch = -89.99f;
    }

    myCamera.rotate(pitch, yaw);

    //update view matrix
    view = myCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
}

void processMovement() {
    if (animation) return;  // no movement allowed during the initial animation

    if (pressedKeys[GLFW_KEY_W]) {
        myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_S]) {
        myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_A]) {
        myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_D]) {
        myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_Q]) {
        angle -= 1.0f;
        // update model matrix for teapot
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_E]) {
        angle += 1.0f;
        // update model matrix for teapot
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }
}

void initOpenGLWindow() {
    myWindow.Create(windowWidth, windowHeight, "OpenGL Project Core");
    glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);  // disable the cursor
}

void setWindowCallbacks() {
    glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
}

void initOpenGLState() {
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
    glEnable(GL_CULL_FACE); // cull face
    glCullFace(GL_BACK); // cull back face
    glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels() {
    mainScene.LoadModel("models/mainScene.obj");    // the static scene
    sarcophageTop.LoadModel("models/sarcophage.obj");   // the piece covering the inner sarcophage
    sarcophageInner.LoadModel("models/sarcophageInner.obj");    // the inner sarcophage
    flyingBook.LoadModel("models/book.obj");    // a flying book
}

void initShaders() {
    myBasicShader.loadShader(
        "shaders/basic.vert",
        "shaders/basic.frag");
}

void initUniforms() {

    myBasicShader.useShaderProgram();

    // create model matrix for teapot
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

    // get view matrix for current camera
    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
    // send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

    // create projection matrix
    projection = glm::perspective(glm::radians(45.0f),
        (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
        0.1f, 20.0f);
    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    // send projection matrix to shader
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //set the light direction (direction towards the light)
    lightDir = glm::vec3(-1.0f, 1.0f, 0.0f);
    lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
    // send light dir to shader
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

    //set light color
    lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
    lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
    // send light color to shader
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    //set point light location
    pointLightPosition = glm::vec3(-3.179f, 1.49f, -0.34f);
    pointLightPositionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "Pposition");
    //send point light to shader
    glUniform3fv(pointLightPositionLoc, 1, glm::value_ptr(pointLightPosition));
}

void renderObjects(gps::Shader shader) {
    // =================================================== draw main scene

    // select active shader program
    shader.useShaderProgram();
    // create model matrix for the scene
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");
    //send model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //send normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    mainScene.Draw(shader);

    // =================================================== draw sarcophage top

    if (!moveSarInner && allowMovement && !animation) { // move the sarcophage top to the right
        if (sarTopMove < -0.63) {
            auxSarTop = -auxSarTop;
            moveSarInner = true;
        }
        else if (sarTopMove > 0) {
            auxSarTop = -auxSarTop;
            allowMovement = false;
        }
        sarTopMove += auxSarTop;
    }

    model = glm::translate(model, glm::vec3(sarTopMove, 0.0, 0.0)); // translate it's position accordingly
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    sarcophageTop.Draw(shader);

    // =================================================== draw sarcophage inner

    if (moveSarInner && allowMovement && !animation) {  // raise the inner sarcophage
        if (sarInnerMove > 0.75) {
            auxSarInner = -auxSarInner;
            allowMovement = false;
        }
        else if (sarInnerMove < 0) {
            auxSarInner = -auxSarInner;
            moveSarInner = false;
        }
        sarInnerMove += auxSarInner;
    }

    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, glm::vec3(0.0, sarInnerMove, 0.0));   // translate it accordingly
    modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");
    //send model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    sarcophageInner.Draw(shader);

    // =================================================== draw flying book

    if (bookMove > 0.65) {  // move the flying book up and down
        auxBookMove = -auxBookMove;
    }
    else if (bookMove < 0) {
        auxBookMove = -auxBookMove;
    }
    bookMove += auxBookMove;

    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, glm::vec3(0.0, bookMove, 0.0));   // translate it accordingly
    modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");
    //send model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    flyingBook.Draw(shader);
}

void initialAnimation() {
    if (animation) {
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

        switch (stage) {
        case 0: // set the camera in the initial position
            myCamera.rotate(180, 0);
            stage++;
            break;
        case 1: // move away from the stele
            myCamera.move(gps::MOVE_BACKWARD, 0.01);
            if (myCamera.getCameraPosition().x > 0.92) {
                stage++;
            }
            break;
        case 2: // rotate to see the entire room
            myCamera.rotate(prevX, prevY);
            prevY += aux;
            aux += 0.0025;
            if (aux >= 1.345) {
                aux = 0;
                stage++;
            }
            break;
        case 3: // move towards the door
            myCamera.move(gps::MOVE_FORWARD, 0.02);
            if (myCamera.getCameraPosition().x > 5.92) {
                stage++;
            }
            break;
        case 4: // rotate back to the tomb
            aux += 0.0025;
            prevY -= aux;
            myCamera.rotate(prevX, prevY);
            if (aux >= 0.95) stage++;
            break;
        default: // end the initial animation
            animation = false;
            break;
        }
    }
}

void renderScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // render the objects
    renderObjects(myBasicShader);

    // play the initial animation
    initialAnimation();
}

void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
}

int main(int argc, const char* argv[]) {

    try {
        initOpenGLWindow();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
    initModels();
    initShaders();
    initUniforms();
    setWindowCallbacks();

    glCheckError();
    // application loop
    while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processMovement();
        renderScene();

        glfwPollEvents();
        glfwSwapBuffers(myWindow.getWindow());

        glCheckError();
    }

    cleanup();

    return EXIT_SUCCESS;
}
