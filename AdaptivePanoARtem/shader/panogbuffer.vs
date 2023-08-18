#version 430 core

layout (location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 camView;

out vec3 worldPos;

void main()
{
	worldPos = aPos;

	vec4 clipPos = projection * camView * vec4(aPos, 1);
	clipPos.z = 0.999999f * clipPos.w;

	gl_Position = clipPos;
}