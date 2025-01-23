#version 430 core

float AMBIENT = 0.03;
float PI      = 3.14;

// sun shadow map
uniform sampler2D depthMapSun;

// main scene uniforms
uniform vec3 cameraPos;
uniform vec3 color;
uniform vec3 sunDir;
uniform vec3 sunColor;
uniform vec3 lightPos;
uniform vec3 lightColor;

// pbr material parameters
uniform float metallic;
uniform float roughness;
uniform float brightness;
uniform float exposition;

// pearl (anglerfish light)
uniform vec3 pearlLightPos;
uniform vec3 pearlLightColor;

// data from vertex shader
in vec3 vecNormal;
in vec3 worldPos;
in vec3 viewDirTS;
in vec3 lightDirTS;
in vec3 sunDirTS;
in vec4 sunSpacePos;

out vec4 outColor;

// color including brightness
vec3 colorB = vec3(0.0);

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 dir, sampler2D shadowMap)
{
    float shadow = 0.0;
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    if (projCoords.z <= 1.0)
    {
        projCoords = projCoords * 0.5 + 0.5;
        float currentDepth = projCoords.z;
        float bias = max(0.025 * (1.0 - dot(normal, dir)), 0.0005);

        int sampleRadius = 2;
        vec2 pixelSize = 1.0 / textureSize(shadowMap, 0);

        for (int y = -sampleRadius; y <= sampleRadius; y++)
        {
            for (int x = -sampleRadius; x <= sampleRadius; x++)
            {
                float closestDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * pixelSize).r;
                if (currentDepth > closestDepth + bias)
                {
                    shadow += 1.0;
                }
            }
        }
        shadow /= float((sampleRadius * 2 + 1) * (sampleRadius * 2 + 1));
    }
    return shadow;
}

float DistributionGGX(vec3 normal, vec3 H, float rough)
{
    float a  = rough * rough;
    float a2 = a * a;
    float NdotH  = max(dot(normal, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom       = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float rough)
{
    float r = (rough + 1.0);
    float k = (r * r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return num / denom;
}

float GeometrySmith(vec3 normal, vec3 V, vec3 L, float rough)
{
    float NdotV = max(dot(normal, V), 0.0);
    float NdotL = max(dot(normal, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, rough);
    float ggx1  = GeometrySchlickGGX(NdotL, rough);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// cook-torrance pbr
vec3 PBRLight(vec3 lightDir, vec3 radiance, vec3 normal, vec3 V)
{
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, colorB, metallic);

    vec3 H = normalize(V + lightDir);

    float NDF = DistributionGGX(normal, H, roughness);
    float G   = GeometrySmith(normal, V, lightDir, roughness);
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

    // ks: specular reflection factor
    vec3 kS = F;
    // kd: diffuse reflection factor
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, V), 0.0) * max(dot(normal, lightDir), 0.0) + 0.0001;
    vec3 specular     = numerator / denominator;

    float NdotL = max(dot(normal, lightDir), 0.0);
    // summation of diffuse and specular reflection
    return (kD * colorB / PI + specular) * radiance * NdotL;
}

void main()
{
    // set the final base color including brightness
    colorB = color * brightness;

    vec3 normal  = normalize(vecNormal);
    vec3 viewDir = normalize(cameraPos - worldPos);
    vec3 lightDir= normalize(lightPos - worldPos);

    // base: ambient + main light (lightpos)
    vec3 ambient = AMBIENT * colorB;
    vec3 attenuatedLightColor = lightColor / pow(length(lightPos - worldPos), 2.0);
    vec3 ilumination = ambient + PBRLight(lightDir, attenuatedLightColor, normal, viewDir);

    // sun + shadows (from depthmapsun)
    float shadowSun = ShadowCalculation(sunSpacePos, normal, sunDir, depthMapSun);
    ilumination += PBRLight(sunDir, shadowSun * sunColor, normal, viewDir);

    // 'pearl' (actually anglerfish light)
    vec3 pearlLightDir = normalize(pearlLightPos - worldPos);
    vec3 attenuatedPearlLightColor = pearlLightColor / pow(length(pearlLightPos - worldPos), 2.0);
    ilumination += PBRLight(pearlLightDir, attenuatedPearlLightColor, normal, viewDir);

    // reinhard tone mapping '1.0 - exp(...)'
    outColor = vec4(1.0 - exp(-ilumination * exposition), 1.0);
}
