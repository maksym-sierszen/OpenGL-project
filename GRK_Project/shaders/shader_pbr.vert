#version 430 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexTexCoord;
layout(location = 3) in vec3 vertexTangent;
layout(location = 4) in vec3 vertexBitangent;

uniform mat4 transformation;
uniform mat4 modelMatrix;
uniform mat4 sunVP;

out vec3 vecNormal;
out vec3 worldPos;

uniform vec3 lightPos;
uniform vec3 cameraPos;
uniform vec3 sunDir;

out vec3 viewDirTS;
out vec3 lightDirTS;
out vec3 sunDirTS;
out vec4 sunSpacePos;

void main()
{
    sunSpacePos    = sunVP * modelMatrix * vec4(vertexPosition, 1.0);
    worldPos       = (modelMatrix * vec4(vertexPosition, 1.0)).xyz;
    vecNormal      = (modelMatrix * vec4(vertexNormal, 0.0)).xyz;
    gl_Position    = transformation * vec4(vertexPosition, 1.0);

    // TBN-matrix (tangent/bitangent/normal)
    vec3 w_tangent  = normalize(mat3(modelMatrix) * vertexTangent);
    vec3 w_bitangent= normalize(mat3(modelMatrix) * vertexBitangent);
    mat3 TBN        = transpose(mat3(w_tangent, w_bitangent, vecNormal));

    // view direction
    vec3 V = normalize(cameraPos - worldPos);
    viewDirTS = TBN * V;

    // main light direction
    vec3 L = normalize(lightPos - worldPos);
    lightDirTS = TBN * L;

    // sun direction
    sunDirTS = TBN * sunDir;
}
