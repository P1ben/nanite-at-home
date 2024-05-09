#version 330 core
out vec4 FragColor;
  
in vec3 vertexColor; // we set this variable in the OpenGL code.
in vec3 wNormal;
in vec3 FragPos;

uniform vec3 drawColor;
uniform bool useTrueColor;

void main()
{
    float ambientStrength = 0.4;
    vec3 lightColor = vec3(1, 1, 1);
    vec3 ambient = ambientStrength * lightColor;

    vec3 lightPos = vec3(0, 5, -5);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 diffuse  = max(dot(wNormal, lightDir), 0.0) * lightColor;

    FragColor = vec4(((ambient + diffuse) * (useTrueColor ? vertexColor : drawColor)), 1.0);
}