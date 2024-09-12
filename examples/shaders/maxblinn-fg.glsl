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

layout (binding = 0) uniform sampler2D colorTexture;
layout (binding = 1) uniform sampler2D objectSpaceNormalTexture;

out vec4 fragmentColor;

struct Light {
    vec4 position;
    vec3 powerDensity;
};

vec3 shade_phong_blinn(vec3 normal, vec3 lightDir, vec3 viewDir,
                       vec3 powerDensity, vec3 materialColor, vec3 specularColor, float shininess) {

    float cosa     = dot(normalize(normal), normalize(lightDir));
    float cosb     = dot(normalize(viewDir), normalize(normal));
    vec3  halfway  = normalize(normalize(lightDir) + normalize(viewDir));
    float cosDelta = dot(normalize(normal), halfway);

    cosDelta       = cosDelta > 0.0 ? cosDelta : 0.0;
    cosa           = clamp(cosa, 0.0, 1.0);
    cosb           = clamp(cosb, 0.0, 1.0);

    vec3 Mks = specularColor * pow(cosDelta, shininess);

    return powerDensity * (materialColor * cosa) + powerDensity * Mks * cosa / max(cosb, cosa);
}

void main() {
    //Light pointLight = { vec4(cameraPosition, 1.0), vec3(1.0, 1.0, 1.0) };
    Light pointLight = Light(vec4(-3.190557, 6.917207, -9.062605, 1.0), vec3(1.0, 1.0, 1.0));

    vec3 color = useTrueColor ? vertexColor : drawColor;
    vec3 normal = normalize(worldNormal.xyz);
    vec3 wP = worldPosition.xyz / worldPosition.w;

    vec3 x = worldPosition.xyz / worldPosition.w;
    vec3 viewDir = normalize(cameraPosition - x);

    vec3 resultColor = vec3(0, 0, 0);

    for (int i = 0; i < 1; i++) {
        vec3 lightDir = (pointLight.position.xyz - wP * pointLight.position.w);
        //resultColor += shade(normal, lightDir, lights[i].powerDensity, color);
        resultColor += shade_phong_blinn(normal, lightDir, viewDir, pointLight.powerDensity, color, vec3(1.0, 1.0, 1.0), 100.0);
    }

    fragmentColor = vec4(resultColor, 1);
}