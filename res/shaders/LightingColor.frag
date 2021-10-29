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

    bool softShadow;
    float frustumSize;
    float nearPlane;
};

// Constants
#define POINT_LIGHTS_COUNT 8

#define BLOCKER_SEARCH_NUM_SAMPLES 16 
#define PCF_NUM_SAMPLES 16

vec2 poissonDisk[16] = vec2[]( 
  vec2( -0.94201624, -0.39906216 ), 
  vec2( 0.94558609, -0.76890725 ), 
  vec2( -0.094184101, -0.92938870 ), 
  vec2( 0.34495938, 0.29387760 ), 
  vec2( -0.91588581, 0.45771432 ), 
  vec2( -0.81544232, -0.87912464 ), 
  vec2( -0.38277543, 0.27676845 ), 
  vec2( 0.97484398, 0.75648379 ), 
  vec2( 0.44323325, -0.97511554 ), 
  vec2( 0.53742981, -0.47373420 ), 
  vec2( -0.26496911, -0.41893023 ), 
  vec2( 0.79197514, 0.19090188 ), 
  vec2( -0.24188840, 0.99706507 ), 
  vec2( -0.81409955, 0.91437590 ), 
  vec2( 0.19984126, 0.78641367 ), 
  vec2( 0.14383161, -0.14100790 ) 
);

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
    
    //return vec3(ambient + diffuse + specular);
    return vec3(ambient + (1.0 - shadow) * (diffuse + specular));
    
    
    
    //return vec3(ambient + (1.0 - shadow) * (diffuse + specular));
}

float PenumbraSize(float zReceiver, float zBlocker) //Parallel plane estimation 
{ 
    return (zReceiver - zBlocker) / zBlocker; 
}

void FindBlocker(out float avgBlockerDepth, out int numBlockers, vec2 uv, float zReceiver, float lightSizeUV)
{
    float searchWidth = lightSizeUV * (zReceiver - directionalLight.nearPlane) / zReceiver; 
    
    float blockerSum = 0; 
    numBlockers = 0; 

    for( int i = 0; i < BLOCKER_SEARCH_NUM_SAMPLES; ++i ) 
    { 
        float shadowMapDepth = texture(shadowMap, uv + poissonDisk[i] * searchWidth).r; 
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
    for ( int i = 0; i < PCF_NUM_SAMPLES; ++i ) 
    { 
        vec2 offset = poissonDisk[i] * filterRadiusUV; 
        float shadowMapDepth = texture(shadowMap, uv + offset).r;
        sum += shadowMapDepth < (zReceiver - 0.005) ? 1 : 0; 
    } 
    return sum / PCF_NUM_SAMPLES; 
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
    if (!directionalLight.softShadow)
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

