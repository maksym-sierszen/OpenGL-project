#version 430 core

uniform samplerCube skybox;

in vec3 texCoord;
in float visibility;

out vec4 out_color;

void main()
{
	//out_color = texture(skybox,texCoord);
	vec4 color = texture(skybox, texCoord);
    vec3 skyColor = vec3(0.0, 0.4, 0.8);

    // Mix the skybox color with the sky color based on visibility
    out_color = mix(vec4(skyColor, 1.0), color, visibility);

}