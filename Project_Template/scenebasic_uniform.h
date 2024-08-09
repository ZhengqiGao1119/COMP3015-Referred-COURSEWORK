#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"

#include <glad/glad.h>
#include "helper/glslprogram.h"

#include "helper/camera.h"
#include "helper/objmesh.h"

class SceneBasic_Uniform : public Scene
{
private:
    GLuint vaoHandle;
    GLSLProgram prog;
    float angle;
    bool isStartRotate;
    Camera camera;

    GLuint texFloor;
    GLuint texBaseColor;
    GLuint texNormal;
    GLuint texEmissive;
    GLuint texMetallicRoughness;
    GLuint texTransmission;

    std::unique_ptr<TriangleMesh> pMesh;

    void compile();

    void renderScene(GLSLProgram &program, glm::mat4 const &V,glm::mat4 const &P,glm::mat4 const &lightPV);

    void renderShadow(GLSLProgram &program, glm::mat4 const viewMatrix, glm::mat4 const proj);

    glm::vec3 lightPosition;

    GLSLProgram shadowProg;
    GLuint shadowFbo;
    GLuint shadowDepthTex;

public:
    SceneBasic_Uniform();

    void initScene();
    void update( float t );
    void render();
    void resize(int, int);
    bool processKey(int value);
    void processMousePos(float xpos, float ypos);
};

#endif // SCENEBASIC_UNIFORM_H
