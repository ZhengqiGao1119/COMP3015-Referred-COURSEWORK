#version 460

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexcoord;

out vec3 Normal;
out vec2 Texcoord;
out vec3 FragPos;
out vec4 shadowCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightPV;


void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));

    Normal = normalize(mat3(transpose(inverse(model))) * aNormal);  
    Texcoord = aTexcoord;

    gl_Position = projection * view * vec4(FragPos, 1.0);

    //
    shadowCoord= lightPV * vec4(FragPos, 1);
}