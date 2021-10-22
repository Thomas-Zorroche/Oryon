#version 330 core

out vec4 fFragColor;

struct PointLight 
{
    float intensity;
    vec3 position;  
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
	
    float constant;
    float linear;
    float quadratic;
};

struct DirectionalLight 
{
    float intensity;
    vec3 direction;  
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

// Constants
#define POINT_LIGHTS_COUNT 8

// Vertex Shader Inputs
in vec3 vNormal;  
in vec3 vFragPos;

// Uniforms
uniform vec3 uColor;
uniform float uShininess;
uniform vec3 uCameraPos;
uniform PointLight pointLights[POINT_LIGHTS_COUNT];
uniform DirectionalLight directionalLight;

vec3 ComputePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 ComputeDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir);

void main()
{
    //vec3 color = uColor;
    vec3 color = vec3(0.0);

    vec3 normal = vNormal;
    vec3 viewDir = normalize(uCameraPos - vFragPos);

    for (int i = 0; i < POINT_LIGHTS_COUNT; i++)
    {
        color += ComputePointLight(pointLights[i], normal, vFragPos, viewDir);
    }

    color += ComputeDirectionalLight(directionalLight, normal, viewDir);

    fFragColor = vec4(color, 1.0);
}


vec3 ComputePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    // TEMP
    vec3 materialAmbient = uColor;
    vec3 materialDiffuse = uColor;
    vec3 materialSpecular = vec3(0.5);
    // END TEMP

    vec3 lightDirection = normalize(light.position - fragPos);
    float diffuseStrength = max(dot(normal, lightDirection), 0.0);
    
    // Blinn Spec
    vec3 halfwayDir = normalize(lightDirection + viewDir);  
    float specularStrength = pow(max(dot(normal, halfwayDir), 0.0), uShininess);

    float distance = length(light.position - fragPos);

    float attenuation = 1.0 / (1.0 + light.linear * distance + light.quadratic * (distance * distance));  
    
    vec3 ambient = light.ambient * materialAmbient * attenuation * light.intensity;
    vec3 diffuse = light.diffuse * materialDiffuse * diffuseStrength * attenuation * light.intensity;
    vec3 specular = light.specular * materialSpecular * specularStrength * attenuation * light.intensity;

    return vec3(ambient + diffuse + specular);
}

vec3 ComputeDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir)
{
    // TEMP
    vec3 materialAmbient = uColor;
    vec3 materialDiffuse = uColor;
    vec3 materialSpecular = vec3(0.5);
    // END TEMP

    vec3 lightDirection = normalize(-light.direction);

    // diffuse shading
    float diffuseStrength = max(dot(normal, lightDirection), 0.0);

    // specular shading
    vec3 reflectDir = reflect(-lightDirection, normal);
    float specularStrength = pow(max(dot(viewDir, reflectDir), 0.0), uShininess);

    // combine results
    vec3 ambient  = light.ambient  * materialAmbient * light.intensity;
    vec3 diffuse  = light.diffuse  * diffuseStrength * materialDiffuse * light.intensity;
    vec3 specular = light.specular * specularStrength * materialSpecular * light.intensity;
    
    return (ambient + diffuse + specular);
}

