#version 430 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D accTex;
uniform sampler2D targetTex;
uniform float lod;

// void main()
// {
// 	float depth;// = texture(targetTex, TexCoords, lod).r;

// 	vec2 newTexCoords = TexCoords * vec2(2.f,1.f);
// 	if(newTexCoords.x >=1.f){
// 		depth = texture(targetTex, newTexCoords - vec2(1, 0)).r;
// 	}


// 	FragColor = vec4(vec3(depth / 6.f), 1);
// }

void main()
{
	vec3 color;
	vec2 newTexCoords = TexCoords * vec2(2.f,1.f);
	if(newTexCoords.x >=1.f){
		vec3 hdrColor = texture(targetTex, newTexCoords - vec2(1, 0)).rgb;
		color = clamp(hdrColor, 0 ,1);
		color = pow(color, vec3(1.f/2.2f));
	}
	else{
		vec3 hdrColor = vec3(texture(accTex, newTexCoords).rgb);
		//hdrColor*=vec3(0, 1.f/900, 1.f/900);
		//hdrColor -= vec3(0, TexCoords);
		color = clamp(hdrColor, 0 ,1);
		color = pow(color, vec3(1.f/2.2f));
	}
	FragColor = vec4(color, 1);
}

// void main()
// {
//     vec3 hdrColor = texture(targetTex, TexCoords).rgb;
// 	//vec3 color = pow(hdrColor, vec3(1.f/2.2f));
// 	//color = clamp(color, 0, 1);
// 	vec3 color = clamp(hdrColor, 0 ,1);
// 	color = pow(color, vec3(1.f/2.2f));
// 	FragColor = vec4(color, 1);

// 	//FragColor = vec4(1,0,0,1);
// }