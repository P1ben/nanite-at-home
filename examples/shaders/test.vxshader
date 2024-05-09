#version 330 core
layout (location = 0) in vec3 aPos; // the position variable has attribute position 0
layout (location = 1) in vec3 normal; // the position variable has attribute position 1
layout (location = 2) in vec3 color; // the position variable has attribute position 2
  
out vec3 vertexColor; // specify a color output to the fragment shader
out vec3 wNormal;
out vec3 FragPos;

uniform mat4 MVP;

void main()
{
    gl_Position = MVP * vec4(aPos, 1.0); // see how we directly give a vec3 to vec4's constructor
    vertexColor = color; // set the output variable to a dark-red color
    wNormal = normal;
    FragPos = aPos;
}