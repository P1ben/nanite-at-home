#version 420 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 color;
layout (location = 3) in vec2 UV;

layout (std140, binding = 2) uniform Camera {
    mat4 viewMatrix;
    mat4 projMatrix;
    vec3 cameraPosition;
};

layout (std140, binding = 3) uniform Object {
    mat4 modelMatrix;
    mat4 modelMatrixInverse;
    vec3 drawColor;
    bool useTrueColor;

    bool useColorTexture;
    bool useObjectSpaceNormalTexture;
};

layout (std140, binding = 4) uniform Tools {
    float uvXOffset;
    float uvYOffset;
};

out vec3 vertexColor;
out vec4 modelPosition;
out vec4 worldPosition;
out vec4 worldNormal;

void main()
{
    vec2 UV_screen = (UV * 2.0 - 1.0);
    UV_screen += vec2(uvXOffset, uvYOffset);
    vec4 vertexPosition = vec4(aPos, 1.0);
    gl_Position = vec4(vec3(UV_screen, 0.0), 1.0);
    vertexColor = vec3(UV_screen, 0.0);

    modelPosition = vertexPosition;
    worldPosition = modelMatrix * vertexPosition;
    // Convert from [-1.0, 1.0] to [0.0, 1.0] range
    worldNormal = vec4((normal / 2.0) + 0.5, 1.0);
}