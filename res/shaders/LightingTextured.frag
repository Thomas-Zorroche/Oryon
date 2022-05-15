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
    float size;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float frustumSize;
    float nearPlane;
};

// Constants
#define POINT_LIGHTS_COUNT 8

// Vertex Shader Inputs
in vec3 vNormal;  
in vec3 vFragPos;
in vec4 vFragPosLightSpace;
in vec2 vTexCoords;

// Uniforms
uniform vec3 uColor;
uniform float uShininess;
uniform vec3 uCameraPos;
uniform PointLight pointLights[POINT_LIGHTS_COUNT];
uniform DirectionalLight directionalLight;
uniform sampler2D shadowMap;
uniform int uSoftShadows; 
uniform int uBlockerSearchSamples;
uniform int uPCFFilteringSamples;
uniform sampler1D uBlockerSearchDist;
uniform sampler1D uPCFFilteringDist;

uniform sampler2D uBaseColorTexture;
uniform vec4 uBaseColorFactor;

vec3 ComputePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, float shadow, vec3 materialColor);
vec3 ComputeDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir, float shadow, vec3 materialColor);
float ComputeShadow(vec4 fragPosLightSpace, vec3 normal);

// Constants
const float GAMMA = 2.2;
const float INV_GAMMA = 1. / GAMMA;

vec3 LINEARtoSRGB(vec3 color) 
{ 
    return pow(color, vec3(INV_GAMMA)); 
}

vec4 SRGBtoLINEAR(vec4 srgbIn)
{
    return vec4(pow(srgbIn.xyz, vec3(GAMMA)), srgbIn.w);
}


void main()
{
    vec4 baseColorFromTexture = SRGBtoLINEAR(texture(uBaseColorTexture, vTexCoords));
    vec4 baseColor = baseColorFromTexture;

    vec3 normal = vNormal;
    vec3 viewDir = normalize(uCameraPos - vFragPos);
    
    //float shadow = ComputeShadow(vFragPosLightSpace, normal);
    float shadow = 0.0f;
    
    vec3 dirLightColor = ComputeDirectionalLight(directionalLight, normal, viewDir, shadow, baseColor.rgb);
    for (int i = 0; i < POINT_LIGHTS_COUNT; i++)
    {
        dirLightColor += ComputePointLight(pointLights[i], normal, vFragPos, viewDir, shadow, baseColor.rgb);
    }
    

    fFragColor = vec4(LINEARtoSRGB(dirLightColor), 1.0);
}


vec3 ComputePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, float shadow, vec3 materialColor)
{
    // TEMP
    vec3 materialSpecular = vec3(0.5);
    // END TEMP

    vec3 lightDirection = normalize(light.position - fragPos);
    float diffuseStrength = max(dot(normal, lightDirection), 0.0);
    
    // Blinn Spec
    vec3 halfwayDir = normalize(lightDirection + viewDir);  
    float specularStrength = pow(max(dot(normal, halfwayDir), 0.0), uShininess);

    float distance = length(light.position - fragPos);

    float attenuation = 1.0 / (1.0 + light.linear * distance + light.quadratic * (distance * distance));  
    
    vec3 ambient = light.ambient * materialColor * attenuation * light.intensity;
    vec3 diffuse = light.diffuse * materialColor * diffuseStrength * attenuation * light.intensity;
    vec3 specular = light.specular * materialSpecular * specularStrength * attenuation * light.intensity;

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

