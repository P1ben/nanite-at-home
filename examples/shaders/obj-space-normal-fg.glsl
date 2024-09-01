#version 420 core

in vec3 vertexColor;

in vec4 modelPosition;
in vec4 worldPosition;
in vec4 worldNormal;

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

    bool useColorTexture;
    bool useObjectSpaceNormalTexture;
};

out vec4 fragmentColor;

void main() {
    fragmentColor = worldNormal;
}