#version 420 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 color;
layout (location = 3) in vec2 UV;

layout (std140, binding = 1) uniform Camera {
    mat4 viewMatrix;
    mat4 projMatrix;
    vec3 cameraPosition;
};

layout (std140, binding = 2) uniform Object {
    mat4 modelMatrix;
    mat4 modelMatrixInverse;
    vec3 drawColor;
    bool useTrueColor;
};

out vec3 vertexColor;
out vec4 modelPosition;
out vec4 worldPosition;
out vec4 worldNormal;

void main()
{
    vec4 vertexPosition = vec4(aPos, 1.0);
    gl_Position = (transpose(projMatrix) * transpose(viewMatrix) * modelMatrix) * vertexPosition;
    vertexColor = vec3(UV, 0.0);

    modelPosition = vertexPosition;
    worldPosition = modelMatrix * vertexPosition;
    worldNormal = vec4(normal, 0.0) * modelMatrixInverse;
}