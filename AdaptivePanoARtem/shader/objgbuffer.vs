#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;

uniform mat4 projection;
uniform mat4 camView;
uniform mat4 model;

out vec3 oriPos;
out vec4 worldPos;
out vec3 normal;
out vec3 oriNormal;
out vec2 texCoords;

void main()
{
	oriPos = aPos;
	worldPos = model * vec4(aPos, 1.f);
    mat3 normalMatrix = transpose(inverse(mat3(model)));
	normal = normalize(normalMatrix * aNormal);
	oriNormal = aNormal;
	texCoords = aTexCoords;

	gl_Position = projection * camView * worldPos;
}