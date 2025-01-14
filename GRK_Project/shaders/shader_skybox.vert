#version 430 core

layout(location = 0) in vec3 vertexPosition;

uniform mat4 transformation;
uniform vec3 cameraPos;
uniform float density;
uniform float gradient;

out vec3 texCoord;
out float visibility;

void main()
{
	vec4 pos = transformation * vec4(vertexPosition, 1.0f);
	gl_Position = vec4(pos.x, pos.y, pos.w, pos.w);
	texCoord = vec3(vertexPosition.x, vertexPosition.y, vertexPosition.z);
	
    // Calculate visibility based on distance and fog parameters
    float distance = length(cameraPos - vertexPosition);
    visibility = exp(-pow(distance * density, gradient));
    visibility = clamp(visibility, 0.0, 0.3);
}