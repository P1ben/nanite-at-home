#version 330 core

in vec3 vertexColor; // specify a color output to the fragment shader

in vec4 modelPosition;
in vec4 worldPosition;
in vec4 worldNormal;


uniform vec3 drawColor;
uniform bool useTrueColor;
uniform vec3 cameraPosition;


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
    Light pointLight = { vec4(cameraPosition, 1.0), vec3(1.0, 1.0, 1.0) };

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