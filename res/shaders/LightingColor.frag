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

#define MAX_NUM_TOTAL_LIGHTS 200
layout (std140) uniform Lights
{
    PointLight uPointLights[MAX_NUM_TOTAL_LIGHTS];
};
uniform int uNumPointLights;

// Vertex Shader Inputs
in vec3 vNormal;  
in vec3 vFragPos;
in vec4 vFragPosLightSpace;

// Uniforms
uniform vec3 uColor;
uniform float uShininess;
uniform vec3 uCameraPos;
uniform DirectionalLight directionalLight;
uniform sampler2D shadowMap;
uniform int uSoftShadows; 
uniform int uBlockerSearchSamples;
uniform int uPCFFilteringSamples;
uniform sampler1D uBlockerSearchDist;
uniform sampler1D uPCFFilteringDist;

vec3 ComputeDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir, float shadow);
float ComputeShadow(vec4 fragPosLightSpace, vec3 normal);

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

void main()
{
    vec3 fColor = vec3(0, 0, 0);

    vec3 normal = vNormal;
    vec3 viewDir = normalize(uCameraPos - vFragPos);

    //float shadow = ComputeShadow(vFragPosLightSpace, normal);
    float shadow = 0.0;
    //fColor += ComputeDirectionalLight(directionalLight, normal, viewDir, shadow);
    for (int i = 0; i < MAX_NUM_TOTAL_LIGHTS; i++)
    {
        if (i == uNumPointLights)
            break;

        fColor += ComputePointLight(uPointLights[i], normal, vFragPos, viewDir, shadow, uColor);
    }


    fFragColor = vec4(fColor, 1.0);
}


vec3 ComputeDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir, float shadow)
{
    // TEMP
    vec3 materialAmbient = uColor;
    vec3 materialDiffuse = uColor;
    vec3 materialSpecular = vec3(0.5);
    // END TEMP

    // diffuse shading
    vec3 lightDirection = normalize(-light.direction);
    float diffuseStrength = max(dot(normal, lightDirection), 0.0);

    // specular shading
    vec3 halfwayDir = normalize(lightDirection + viewDir);  
    float specularStrength = pow(max(dot(normal, halfwayDir), 0.0), uShininess);

    // combine results
    vec3 ambient  = light.ambient  * materialAmbient * light.intensity;
    vec3 diffuse  = light.diffuse  * diffuseStrength * materialDiffuse * light.intensity;
    vec3 specular = light.specular * specularStrength * materialSpecular * light.intensity;
    
    return vec3(ambient + (1.0 - shadow) * (diffuse + specular));
}

float PenumbraSize(float zReceiver, float zBlocker) // Parallel plane estimation 
{ 
    return (zReceiver - zBlocker) / zBlocker; 
}

vec2 RandomDirection(sampler1D distribution, float u)
{
   return texture(distribution, u).xy * 2 - vec2(1);
}

void FindBlocker(out float avgBlockerDepth, out int numBlockers, vec2 uv, float zReceiver, float lightSizeUV)
{
    float searchWidth = lightSizeUV * (zReceiver - directionalLight.nearPlane) / zReceiver; 
    
    float blockerSum = 0; 
    numBlockers = 0; 

    for( int i = 0; i < uBlockerSearchSamples; ++i ) 
    { 
        float shadowMapDepth = texture(shadowMap, uv + RandomDirection(uBlockerSearchDist, i / float(uBlockerSearchSamples)) * searchWidth).r; 
        if ( shadowMapDepth < zReceiver - 0.005) { 
            blockerSum += shadowMapDepth; 
            numBlockers++; 
        } 
     } 
 
    if (numBlockers > 0)
        avgBlockerDepth = blockerSum / numBlockers;
}

float PCF_Filter( vec2 uv, float zReceiver, float filterRadiusUV ) 
{ 
    float sum = 0.0; 
    for ( int i = 0; i < uPCFFilteringSamples; ++i ) 
    { 
        vec2 offset = RandomDirection(uPCFFilteringDist, i / float(uPCFFilteringSamples)) * filterRadiusUV; 
        float shadowMapDepth = texture(shadowMap, uv + offset).r;
        sum += shadowMapDepth < (zReceiver - 0.005) ? 1 : 0; 
    } 
    return sum / uPCFFilteringSamples; 
}

float ComputeShadow(vec4 fragPosLightSpace, vec3 normal)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w; // perform perspective divide
    projCoords = projCoords * 0.5 + 0.5;  // [-1,1] to [0,1]
    if(projCoords.z > 1.0)
        return 0.0;
    
    float zReceiver = projCoords.z;
    vec2 uv = projCoords.xy;

    // HARD SHADOWS
    // -----------------------------------------------------------------------
    if (uSoftShadows != 1)
    {
        // BIAS 
        vec3 lightDirection = normalize(-directionalLight.direction);
        float bias = 0.005 * tan(acos(dot(normal, lightDirection)));
        bias = clamp(bias, 0,0.01);

        float zBlocker = texture(shadowMap, projCoords.xy).r; 
        return zReceiver - bias > zBlocker ? 1.0 : 0.0;
    }

    // SOFT SHADOWS
    // -----------------------------------------------------------------------

    // STEP 1: blocker search
    float avgBlockerDepth = 0; 
    int numBlockers = 0;
    float lightSizeUV = directionalLight.size / 20.0 /* FRUSTUM_WIDTH*/;
    FindBlocker(avgBlockerDepth, numBlockers, uv, zReceiver, lightSizeUV);

    //There are no occluders so early out (this saves filtering) 
    if( numBlockers < 1 )   
        return 0.0; 

    // STEP 2: penumbra size
    float penumbraRatio = PenumbraSize(zReceiver, avgBlockerDepth);     
    float filterRadiusUV = penumbraRatio * lightSizeUV * directionalLight.nearPlane / zReceiver;

    // STEP 3: filtering 
    return PCF_Filter( uv, zReceiver, filterRadiusUV );
}

