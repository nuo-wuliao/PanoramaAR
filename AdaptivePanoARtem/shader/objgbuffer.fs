#version 430 core

layout (location = 0) out vec4 gColor;
layout (location = 1) out vec4 gWorldPos;
layout (location = 2) out vec4 gNormal;
layout (location = 3) out vec4 gMat_PreCoord;
layout (location = 4) out float gmipmapedFrameDepth;
layout (location = 5) out vec4 gModelNormal;
layout (location = 6) out vec4 gAlbedo;
layout (location = 7) out vec4 gDiffTexTure;

in vec3 oriPos;
in vec4 worldPos;
in vec3 normal;
in vec3 oriNormal;
in vec2 texCoords;

uniform mat4 lastProjection;
uniform mat4 lastCamView;
uniform mat4 lastModel;
uniform int frameWidth;
uniform int frameHeight;
uniform int mat;
uniform float alpha;
uniform vec3 rho;
uniform sampler2D panoPosTex;
uniform sampler2D texture_diffuse0;
// uniform sampler2D colorTex;
// uniform sampler2D worldPosTex;
// uniform sampler2D normalTex;
// uniform sampler2D mat_prevCoordTex;
// uniform sampler2D mipmapedFrameDepthTex;
// uniform sampler2D modelNormalTex;
// uniform sampler2D diff_textrureTex;

const vec2 invAtan = vec2(0.159154943f,0.318309886f);
const float PI=3.141592653f;

vec2 SamplerSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z,v.x),acos(v.y));
	uv.x = clamp(mod(uv.x, PI*2), 0.f,PI*2);
    uv *= invAtan;
    return uv;
}

vec2 getPrevCoords(vec3 pos)
{
	vec4 vertex = lastProjection * lastCamView * lastModel * vec4(pos, 1);//
	vertex.xy = vertex.xy / vertex.w;
	vertex.xy = vertex.xy * 0.5 + 0.5;
	vec2 screenCoord = vertex.xy;
//	screenCoord.x = vertex.x * (1.f * frameWidth) + 0.5;
//	screenCoord.y = vertex.y * (1.f * frameHeight) + 0.5;
	return screenCoord;
}

void main()
{
    //vec2 texCoords = gl_FragCoord.xy / vec2(frameWidth, frameHeight);
	// vec3 backposition = texture(panoPos, texCoords).rgb;
	// float backDepth = length(backposition);
	// vec3 objPosition = worldPos.xyz;
	// float objDepth = length(objPosition);

	// vec2 uv = SamplerSphericalMap(normalize(objPosition));
    // vec3 backposition = texture(panoPosTex, uv).bgr;
	// float backDepth = length(backposition);
	// if(objDepth > backDepth) {
	// 	//discard;
	// 	//return;
		
	// }
	// else{
		gWorldPos = worldPos;
		gmipmapedFrameDepth = length(worldPos.rgb);
		gNormal = vec4(normal, 1);
		gColor = vec4(0,0,0,1);
		vec2 coord = getPrevCoords(oriPos);
		gMat_PreCoord = vec4(mat, coord, 0);
		gModelNormal = vec4(oriNormal, 1);
		gDiffTexTure = vec4(texture(texture_diffuse0, texCoords).rgb, 1.f);//vec4(rho,1);//
		//return;
	//}
	
	// gWorldPos = texture(worldPosTex,texCoords);
	// gmipmapedFrameDepth = texture(mipmapedFrameDepthTex,texCoords).r;
	// gNormal = texture(normalTex,texCoords);
	// gColor = texture(colorTex,texCoords);
	// gMat_PreCoord = texture(mat_prevCoordTex,texCoords);
	// gModelNormal = texture(modelNormalTex,texCoords);
	// gDiffTexTure = texture(diff_textrureTex,texCoords);//vec4(1,0,0,1);//texture(texture_diffuse0, texCoords);//
}