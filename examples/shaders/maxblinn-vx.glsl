#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 color;
layout (location = 3) in vec2 UV;

// struct VertexData {
//     float position[3];
//     float normal[3];
//     float color[3];
//     float uv[2];
// };

// layout(binding = 0, std430) readonly buffer vertex_buffer {
//     VertexData vertices[];
// };

// layout (std430, binding = 1) readonly buffer indices {
//     uint index[];
// };

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

layout (binding = 0) uniform sampler2D colorTexture;
layout (binding = 1) uniform sampler2D objectSpaceNormalTexture;

out vec3 vertexColor;
out vec4 modelPosition;
out vec4 worldPosition;
out vec4 worldNormal;
out vec2 textureCoords;

// vec3 get_pos() {
//     uint vertex = index[gl_VertexID];
//     vec3 pos = vec3(
//         vertices[vertex].position[0],
//         vertices[vertex].position[1],
//         vertices[vertex].position[2]
//     );
//     return pos;
// }

// vec3 get_normal() {
//     uint vertex = index[gl_VertexID];
//     vec3 normal = vec3(
//         vertices[vertex].normal[0],
//         vertices[vertex].normal[1],
//         vertices[vertex].normal[2]
//     );
//     return normal;
// }

// vec3 get_color() {
//     uint vertex = index[gl_VertexID];
//     vec3 color = vec3(
//         vertices[vertex].color[0],
//         vertices[vertex].color[1],
//         vertices[vertex].color[2]
//     );
//     return color;
// }

// vec2 get_uv() {
//     uint vertex = index[gl_VertexID];
//     vec2 uv = vec2(
//         vertices[vertex].uv[0],
//         vertices[vertex].uv[1]
//     );
//     return uv;
// }


void main()
{

    
// layout (location = 0) in vec3 aPos;
// layout (location = 1) in vec3 normal;
// layout (location = 2) in vec3 color;
// layout (location = 3) in vec2 UV;

    // vec3 aPos   = get_pos();
    // vec3 normal = get_normal();
    // vec3 color  = get_color();
    // vec2 UV     = get_uv();

    vec4 vertexPosition = vec4(aPos, 1.0);
    gl_Position = (transpose(projMatrix) * transpose(viewMatrix) * transpose(modelMatrix)) * vertexPosition;
    //vertexColor = vec3(UV, 0.0);
    vertexColor = color;

    modelPosition = vertexPosition;
    worldPosition = transpose(modelMatrix) * vertexPosition;

    if (useObjectSpaceNormalTexture) {
        // Convert from [0.0, 1.0] to [-1.0, 1.0] range
        worldNormal = modelMatrixInverse * ((texture(objectSpaceNormalTexture, UV) - 0.5) * 2.0);
    }
    else {
        worldNormal = transpose(modelMatrix) * vec4(normal, 0.0);
    }
    textureCoords = UV;
}