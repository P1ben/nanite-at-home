#version 330 core
layout (location = 0) in vec3 aPos; // the position variable has attribute position 0
layout (location = 1) in vec3 normal; // the position variable has attribute position 1
layout (location = 2) in vec3 color; // the position variable has attribute position 2

uniform mat4 modelMatrix;
uniform mat4 modelMatrixInverse;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform vec3 cameraPosition;

out vec3 vertexColor; // specify a color output to the fragment shader

out vec4 modelPosition;
out vec4 worldPosition;
out vec4 worldNormal;

void main()
{
    vec4 vertexPosition = vec4(aPos, 1.0);
    gl_Position = (projMatrix * viewMatrix * modelMatrix) * vertexPosition; // see how we directly give a vec3 to vec4's constructor
    vertexColor = color; // set the output variable to a dark-red color

    modelPosition = vertexPosition;
    worldPosition = modelMatrix * vertexPosition;
    worldNormal = vec4(normal, 0.0) * modelMatrixInverse;
}