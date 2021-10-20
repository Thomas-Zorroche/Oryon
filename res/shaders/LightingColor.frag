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

// Vertex Shader Inputs
in vec3 vNormal;  
in vec3 vFragPos;


// Uniforms
uniform vec3 uColor;
uniform PointLight pointLight;
uniform vec3 uCameraPos;

vec3 ComputePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
    //vec3 color = uColor;
    vec3 color = vec3(0.0);

    vec3 normal = vNormal;
    vec3 viewDir = normalize(uCameraPos - vFragPos);

    color += ComputePointLight(pointLight, normal, vFragPos, viewDir);

    fFragColor = vec4(color, 1.0);
}


vec3 ComputePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    // TEMP
    vec3 materialAmbient = vec3(0.2);
    vec3 materialDiffuse = vec3(0.9);
    vec3 materialSpecular = vec3(0.9);
    // END EMP


    vec3 lightDirection = normalize(light.position - fragPos);
    float diffuseStrength = max(dot(normal, lightDirection), 0.0);
    
    vec3 reflectDir = reflect(-lightDirection, normal);
    
    float specularStrength = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    
    float distance = length(light.position - fragPos);

    float attenuation = 1.0 / (1.0 + light.linear * distance + light.quadratic * (distance * distance));  
    
    vec3 ambient = light.ambient * materialAmbient * attenuation * light.intensity;
    vec3 diffuse = light.diffuse * materialDiffuse * diffuseStrength * attenuation * light.intensity;
    vec3 specular = light.specular * materialSpecular * specularStrength * attenuation * light.intensity;

    return vec3(ambient + diffuse + specular);
}

