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
in vec4 vFragPosLightSpace;

// Uniforms
uniform vec3 uColor;
uniform float uShininess;
uniform vec3 uCameraPos;
uniform PointLight pointLights[POINT_LIGHTS_COUNT];
uniform DirectionalLight directionalLight;
uniform sampler2D shadowMap;

vec3 ComputePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, float shadow);
vec3 ComputeDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir, float shadow);
float ComputeShadow(vec4 fragPosLightSpace, vec3 normal);

void main()
{
    //vec3 color = uColor;
    vec3 color = vec3(0.0);

    vec3 normal = vNormal;
    vec3 viewDir = normalize(uCameraPos - vFragPos);

    float shadow = ComputeShadow(vFragPosLightSpace, normal);

    for (int i = 0; i < POINT_LIGHTS_COUNT; i++)
    {
        color += ComputePointLight(pointLights[i], normal, vFragPos, viewDir, shadow);
    }

    color += ComputeDirectionalLight(directionalLight, normal, viewDir, shadow);

    fFragColor = vec4(color, 1.0);
    //fFragColor = vec4(vec3(1.0 - shadow), 1.0);
}


vec3 ComputePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, float shadow)
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

vec3 ComputeDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir, float shadow)
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
    //vec3 reflectDir = reflect(-lightDirection, normal);
    //float specularStrength = pow(max(dot(viewDir, reflectDir), 0.0), uShininess);
    vec3 halfwayDir = normalize(lightDirection + viewDir);  
    float specularStrength = pow(max(dot(normal, halfwayDir), 0.0), uShininess);

    // combine results
    vec3 ambient  = light.ambient  * materialAmbient * light.intensity;
    vec3 diffuse  = light.diffuse  * diffuseStrength * materialDiffuse * light.intensity;
    vec3 specular = light.specular * specularStrength * materialSpecular * light.intensity;
    
    //return vec3(ambient + diffuse + specular);
    return vec3(ambient + (1.0 - shadow) * (diffuse + specular));
    
    
    
    //return vec3(ambient + (1.0 - shadow) * (diffuse + specular));
}

float ComputeShadow(vec4 fragPosLightSpace, vec3 normal)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // [-1,1] to [0,1]
    projCoords = projCoords * 0.5 + 0.5; 

    if(projCoords.z > 1.0)
        return 0.0;

    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    vec3 lightDirection = normalize(-directionalLight.direction);
    //float bias = max(0.05 * (1.0 - dot(normal, lightDirection)), 0.005);

    float bias = 0.005 * tan(acos(dot(normal, lightDirection)));
    bias = clamp(bias, 0,0.01);
    
    // PCF - average shadow - TODO uniform for control this
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    return shadow;
}

