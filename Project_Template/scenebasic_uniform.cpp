#include "scenebasic_uniform.h"

#include <cstdio>
#include <cstdlib>

#include <string>
using std::string;

#include <iostream>
#include <vector>
using std::cerr;
using std::endl;

#include "helper/glutils.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "helper/stb/stb_image.h"

#define SHADOW_MAP_WIDTH 1024

using glm::vec3;

SceneBasic_Uniform::SceneBasic_Uniform() : angle(0.0f) {}


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

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

void SceneBasic_Uniform::initScene()
{
    compile();

    std::cout << std::endl;

    prog.printActiveUniforms();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FRAMEBUFFER_SRGB);

    /////////////////// Create the VBO ////////////////////
    float positiondata[] = {
        5.f, -0.0f,  5.f,
       -5.f, -0.0f,  5.f,
       -5.f, -0.0f, -5.f,
        5.f, -0.0f,  5.f,
       -5.f, -0.0f, -5.f,
        5.f, -0.0f, -5.f,
    };

    float normaldata[] = {
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
    };

    float texcoorddata[] = {
        5, 0.0f,
        0.0f, 0.0f,
        0.0f, 5,
         5, 0.0f,
         0.0f, 5,
         5, 5
    };

    // Create and populate the buffer objects
    GLuint vboHandles[3];
    glGenBuffers(3, vboHandles);
    GLuint positionBufferHandle = vboHandles[0];
    GLuint normalBufferHandle = vboHandles[1];
    GLuint texcoordBufferHandle = vboHandles[2];

    glBindBuffer(GL_ARRAY_BUFFER, positionBufferHandle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positiondata), positiondata, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, normalBufferHandle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normaldata), normaldata, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, texcoordBufferHandle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texcoorddata), texcoorddata, GL_STATIC_DRAW);


    // Create and set-up the vertex array object
    glGenVertexArrays(1, &vaoHandle);
    glBindVertexArray(vaoHandle);

    glEnableVertexAttribArray(0);  
    glEnableVertexAttribArray(1);  
    glEnableVertexAttribArray(2);  



    glBindBuffer(GL_ARRAY_BUFFER, vboHandles[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, vboHandles[1]);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, vboHandles[2]);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindVertexArray(0);
    stbi_set_flip_vertically_on_load(true);

    texFloor= loadTexture("model/square_tiles_diff_4k.jpg");
    texBaseColor = loadTexture("model/default_baseColor.png");
    texNormal = loadTexture("model/default_normal.jpeg");
    texMetallicRoughness = loadTexture("model/default_metallicRoughness.png");
    texEmissive = loadTexture("model/default_emissive.jpeg");
    texTransmission = loadTexture("model/default_transmission.png");

    stbi_set_flip_vertically_on_load(false);

    camera.Position = glm::vec3(0, 2, 6);

    pMesh = ObjMesh::load("model/dragon_ball_hover_car.obj");

    //create fbo
    glGenTextures(1, &shadowDepthTex);
    glBindTexture(GL_TEXTURE_2D, shadowDepthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, SHADOW_MAP_WIDTH, SHADOW_MAP_WIDTH, 0, GL_DEPTH_COMPONENT,
                 GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);

    glCreateFramebuffers(1, &shadowFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowDepthTex, 0);

    auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "failed to create framebuffer, err %x\n", glGetError());
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glEnable(GL_POLYGON_OFFSET_FILL);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void SceneBasic_Uniform::compile()
{
    try {
        prog.compileShader("shader/basic_uniform.vert");
        prog.compileShader("shader/basic_uniform.frag");
        prog.link();
        prog.use();
        prog.setUniform("texBaseColor", 0);
        prog.setUniform("texNormal", 1);
        prog.setUniform("texEmissive", 2);
        prog.setUniform("texMetallicRoughness", 3);
        prog.setUniform("texTransmission", 4);

        prog.setUniform("shadowTex", 5);
    }
    catch (GLSLProgramException& e) {
        cerr << e.what() << endl;
        exit(EXIT_FAILURE);
    }

    try {
        shadowProg.compileShader("shader/shadow.vert");
        shadowProg.compileShader("shader/shadow.frag");
        shadowProg.link();
        shadowProg.use();
    } catch (GLSLProgramException& e) {
        cerr << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}

void SceneBasic_Uniform::update(float t)
{
    if(isStartRotate)
     angle += 0.1;
    //update your angle here
}


bool SceneBasic_Uniform::processKey(int value) {
    float deltaTime  = 0.01;
    switch (value)
    {
    case 0:
        camera.ProcessKeyboard(FORWARD, deltaTime);
        break;
    case 1:
        camera.ProcessKeyboard(BACKWARD, deltaTime);
        break;
    case 2:
        camera.ProcessKeyboard(LEFT, deltaTime);
        break;
    case 3:
        camera.ProcessKeyboard(RIGHT, deltaTime);
        break;
    case 4:
        isStartRotate = true;
        break;
    case 5:
        isStartRotate = false;
    default:
        break;
    }

    return true;
}
void SceneBasic_Uniform::processMousePos(float xpos, float ypos) {
    static float xpos0 = xpos, ypos0 = ypos;
    camera.ProcessMouseMovement(xpos - xpos0, -ypos + ypos0);
    xpos0 = xpos;
    ypos0 = ypos;
}

void SceneBasic_Uniform::renderScene(GLSLProgram &program, glm::mat4 const &V,glm::mat4 const &P,glm::mat4 const &lightPV){
    program.use();

    glm::vec3 lightColor = glm::vec3(1.f, 1.f, 1.f) * 4000.f;
    glm::vec3 lightAmbient = glm::vec3(0.3f, 0.3f, 0.3f);
    glm::vec3 lightDiffuse = glm::vec3(1.f, 1.f, 1.f);
    glm::vec3 lightSpecular = glm::vec3(1.f, 1.f, 1.f);
    glm::vec3 matAmbient = glm::vec3(1.f, 1.f, 1.f);
    glm::vec3 matDiffuse = glm::vec3(1.f, 1.f, 1.f);
    glm::vec3 matSpecular = glm::vec3(1.f, 1.f, 1.f);
    float     matShininess = 64.f;
    program.setUniform("lightPosition", lightPosition);
    program.setUniform("lightColor", lightColor);
    program.setUniform("lightAmbient", lightAmbient);
    program.setUniform("lightDiffuse", lightDiffuse);
    program.setUniform("lightSpecular", lightSpecular);
    program.setUniform("matAmbient", matAmbient);
    program.setUniform("matDiffuse", matDiffuse);
    program.setUniform("matSpecular", matSpecular);
    program.setUniform("matShininess", matShininess);
    program.setUniform("viewPos", camera.Position);
    program.setUniform("useTexture", 1);


    //glm::mat4 view = glm::lookAt(glm::vec3(0, 2, 6), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    program.setUniform("projection", P);
    program.setUniform("view", V);
    program.setUniform("lightPV", lightPV);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, shadowDepthTex);

    glBindVertexArray(vaoHandle);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texFloor);
    glm::mat4 model = glm::mat4(1.f);
    model = glm::scale(model, glm::vec3(2, 1, 2));
    program.setUniform("model", model);
    program.setUniform("useTexture", 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(2, 2, 2));	// it's a bit too big for our scene, so scale it down
    program.setUniform("model", model);
    program.setUniform("useTexture", 1);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texBaseColor);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texEmissive);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, texMetallicRoughness);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, texTransmission);
    pMesh->render();
}
void SceneBasic_Uniform::renderShadow(GLSLProgram &program, glm::mat4 const V, glm::mat4 const P){
    glPolygonOffset(3,3);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowFbo);
    glClear(GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, SHADOW_MAP_WIDTH, SHADOW_MAP_WIDTH);

    program.use();
    renderScene(program, V, P, glm::mat4(1));

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glViewport(0, 0, width, height);
    glPolygonOffset(0,0);
}
void SceneBasic_Uniform::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    projection = glm::perspective(glm::radians(45.f), (float)width / (float)height, 0.1f, 100.0f);

    // rendershadow
    lightPosition = glm::vec3(30, 50, 30);

    auto lightV = glm::lookAt(lightPosition, {}, {0, 1, 0});
    auto lightP = glm::perspective(glm::radians(20.f), 1.f, 0.1f, 100.f);
    auto lightPV = lightP * lightV;
    renderShadow(shadowProg, lightV, lightP);

    //create the rotation matrix here and update the uniform in the shader
    view = camera.GetViewMatrix();
    view = glm::rotate(view, glm::radians(angle), glm::vec3(0, 1, 0));
    renderScene(prog, view, projection, lightPV);

    glBindVertexArray(0);
}

void SceneBasic_Uniform::resize(int w, int h)
{
    width = w;
    height = h;
    glViewport(0, 0, w, h);
}
