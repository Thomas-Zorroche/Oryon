#version 330 core

out vec4 fFragColor;

struct PointLight 
{
    vec3 position;  
    float intensity;
  
    vec3 ambient;
    float linear;

    vec3 diffuse;
    float quadratic;

    vec4 specular;
};

struct DirectionalLight 
{
    float intensity;
    vec3 direction;  
    float size;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float frustumSize;
    float nearPlane;
};


// Vertex Shader Inputs
in vec3 vNormal;  
in vec3 vFragPos;
in vec4 vFragPosLightSpace;
in vec2 vTexCoords;

// Uniforms
uniform float uShininess;
uniform vec3 uCameraPos;
uniform DirectionalLight directionalLight;
uniform sampler2D uBaseColorTexture;
uniform vec4 uBaseColorFactor;

#define MAX_NUM_TOTAL_LIGHTS 200
layout (std140) uniform Lights
{
    PointLight uPointLights[MAX_NUM_TOTAL_LIGHTS];
};
uniform int uNumPointLights;


vec3 ComputePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, float shadow, vec3 materialColor);
vec3 ComputeDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir, float shadow, vec3 materialColor);
float ComputeShadow(vec4 fragPosLightSpace, vec3 normal);

// Constants
const float GAMMA = 2.2;
const float INV_GAMMA = 1. / GAMMA;
vec3 LINEARtoSRGB(vec3 color) { 
    return pow(color, vec3(INV_GAMMA)); 
}
vec4 SRGBtoLINEAR(vec4 srgbIn) {
    return vec4(pow(srgbIn.xyz, vec3(GAMMA)), srgbIn.w);
}


void main()
{
    vec4 baseColorFromTexture = SRGBtoLINEAR(texture(uBaseColorTexture, vTexCoords));
    vec4 baseColor = baseColorFromTexture * uBaseColorFactor;
    
    vec3 normal = vNormal;
    vec3 viewDir = normalize(uCameraPos - vFragPos);
    
    //float shadow = ComputeShadow(vFragPosLightSpace, normal);
    float shadow = 0.0;
    
    //vec3 fColor = ComputeDirectionalLight(directionalLight, normal, viewDir, shadow, baseColor.rgb);
    vec3 fColor = vec3(0, 0, 0);
    for (int i = 0; i < MAX_NUM_TOTAL_LIGHTS; i++)
    {
        if (i == uNumPointLights)
            break;

        fColor += ComputePointLight(uPointLights[i], normal, vFragPos, viewDir, shadow, baseColor.rgb);
    }
    
    fFragColor = vec4(LINEARtoSRGB(fColor.rgb), 1.0);
}


vec3 ComputePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, float shadow, vec3 materialColor)
{
    // TEMP
    vec3 materialSpecular = vec3(0.5);
    vec3 materialAmbient = materialColor * 0.1;
    // END TEMP

    vec3 lightDirection = normalize(light.position - fragPos);
    float diffuseStrength = max(dot(normal, lightDirection), 0.0);
    
    // Blinn Spec
    vec3 halfwayDir = normalize(lightDirection + viewDir);  
    float specularStrength = pow(max(dot(normal, halfwayDir), 0.0), uShininess);

    float distance = length(light.position - fragPos);

    float attenuation = 1.0 / (1.0 + light.linear * distance + light.quadratic * (distance * distance));  
    
    vec3 ambient = light.ambient * materialAmbient * attenuation * light.intensity;
    vec3 diffuse = light.diffuse * materialColor * diffuseStrength * attenuation * light.intensity;
    vec3 specular = light.specular.rgb * materialSpecular * specularStrength * attenuation * light.intensity;

    return vec3(ambient + diffuse + specular);
}

vec3 ComputeDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir, float shadow, vec3 materialColor)
{
    // TEMP
    vec3 materialSpecular = vec3(0.5);
    // END TEMP

    // diffuse shading
    vec3 lightDirection = normalize(-light.direction);
    float diffuseStrength = max(dot(normal, lightDirection), 0.0);

    // specular shading
    vec3 halfwayDir = normalize(lightDirection + viewDir);  
    float specularStrength = pow(max(dot(normal, halfwayDir), 0.0), uShininess);

    // combine results
    vec3 ambient  = light.ambient  * materialColor * light.intensity;
    vec3 diffuse  = light.diffuse  * diffuseStrength * materialColor * light.intensity;
    vec3 specular = light.specular * specularStrength * materialSpecular * light.intensity;
    
    return vec3(ambient + (1.0 - shadow) * (diffuse + specular));
}