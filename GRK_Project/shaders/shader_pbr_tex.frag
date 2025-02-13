#version 430 core

float AMBIENT = 0.03;
float PI = 3.14;

uniform sampler2D depthMapSun;
uniform sampler2D depthMapTableLight;

uniform vec3 cameraPos;
uniform vec3 cameraStartPos;

uniform sampler2D colorTexture;

uniform vec3 sunDir;
uniform vec3 sunColor;

uniform vec3 lightPos;
uniform vec3 lightColor;



uniform float metallic;
uniform float roughness;
uniform float brightness;

uniform float exposition;

uniform vec3 pearlLightPos; 
uniform vec3 pearlLightColor;

in vec3 vecNormal;
in vec3 worldPos;
in vec2 vecTex;
in float visibility;

out vec4 outColor;



in vec4 sunSpacePos;

in vec3 sunDirTS;

vec3 color = texture(colorTexture, vecTex).xyz * brightness;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 dir, sampler2D shadowMap){
    float shadow = 0.0f;
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    if (projCoords.z <= 1.0f){
         projCoords = projCoords * 0.5 + 0.5;
         float currentDepth = projCoords.z;
         float bias = max(0.025f * (1.0f - dot(normal, dir)), 0.0005f);
         
         int sampleRadius = 2;
         vec2 pixelSize = 1.0 / textureSize(shadowMap, 0);
         for (int y = -sampleRadius; y <= sampleRadius; y++){
            for (int x = -sampleRadius; x <= sampleRadius; x++){
                float closestDepth = texture(shadowMap, projCoords.xy + vec2(x,y) * pixelSize).r;
                if (currentDepth > closestDepth + bias){
                    shadow += 1.0f;
                }
            }
         }
         shadow /= pow((sampleRadius*2 + 1), 2);
    }
     return shadow;
}

float DistributionGGX(vec3 normal, vec3 H, float roughness){
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(normal, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}
float GeometrySchlickGGX(float NdotV, float roughness){
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(vec3 normal, vec3 V, vec3 lightDir, float roughness){
    float NdotV = max(dot(normal, V), 0.0);
    float NdotL = max(dot(normal, lightDir), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}
vec3 fresnelSchlick(float cosTheta, vec3 F0){
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
} 

vec3 PBRLight(vec3 lightDir, vec3 radiance, vec3 normal, vec3 V){
	float diffuse=max(0,dot(normal,lightDir));

	//vec3 V = normalize(cameraPos-worldPos);
	vec3 F0 = vec3(0.04); 
    F0 = mix(F0, color, metallic);

    vec3 H = normalize(V + lightDir);    
        
    // cook-torrance brdf
    float NDF = DistributionGGX(normal, H, roughness);        
    float G   = GeometrySmith(normal, V, lightDir, roughness);      
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);       
        
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;	  
        
    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, V), 0.0) * max(dot(normal, lightDir), 0.0) + 0.0001;
    vec3 specular     = numerator / denominator;  
            
    // add to outgoing radiance Lo
    float NdotL = max(dot(normal, lightDir), 0.0);   
	return (kD * color / PI + specular) * radiance * NdotL;
}


void main()
{

	
    vec3 normal = normalize(vecNormal);

    
    vec3 viewDir = normalize(cameraPos-worldPos);
    vec3 viewDirSun = normalize(cameraStartPos - worldPos);


	vec3 lightDir = normalize(lightPos-worldPos);

    //main light
	vec3 ambient = AMBIENT*color;
	vec3 attenuatedlightColor = lightColor/pow(length(lightPos-worldPos),2);
	vec3 ilumination;
	ilumination = ambient+PBRLight(lightDir,attenuatedlightColor,normal,viewDir);


	//sun
    float shadowSun = ShadowCalculation(sunSpacePos, normal, sunDir, depthMapSun);
	ilumination=ilumination+PBRLight(sunDir,shadowSun*sunColor,normal,viewDirSun);

    // pearl
    vec3 pearlLightDir = normalize(pearlLightPos - worldPos);
    vec3 attenuatedPearlLightColor = pearlLightColor / pow(length(pearlLightPos - worldPos), 2.0);
    ilumination += PBRLight(pearlLightDir, attenuatedPearlLightColor, normal, viewDir);
    
    vec3 skyColor = vec3(0.0, 0.4, 0.8);
	outColor = vec4(vec3(1.0) - exp(-ilumination*exposition),1);
    outColor = mix(outColor, vec4(skyColor, 1.0), 1.0 - visibility);
}