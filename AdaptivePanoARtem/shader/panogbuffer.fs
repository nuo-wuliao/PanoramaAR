#version 430 core

layout (location = 0) out vec4 gColor;
layout (location = 1) out vec4 gWorldPos;
layout (location = 2) out vec4 gNormal;
layout (location = 3) out vec4 gMat_PreCoord;
layout (location = 4) out float gmipmapedFrameDepth;
layout (location = 5) out vec4 gModelNormal;
layout (location = 6) out vec4 gAlbedo;
layout (location = 7) out vec4 gDiffTexTure;

in vec3 worldPos;

uniform sampler2D panoTex;
uniform sampler2D panoPosTex;
uniform sampler2D panoNormalTex;
uniform sampler2D panoAlbedoTex;

uniform mat4 lastProjection;
uniform mat4 lastCamView;

uniform int frameWidth;
uniform int frameHeight;


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
	vec4 vertex = lastProjection * lastCamView * vec4(worldPos, 1);//
	vertex.xy = vertex.xy / vertex.w;
	vertex.xy = vertex.xy * 0.5 + 0.5;
	vec2 screenCoord = vertex.xy;
//	screenCoord.x = vertex.x * frameWidth + 0.5;
//	screenCoord.y = vertex.y * frameHeight + 0.5;
	return screenCoord;
}

void main()
{
    vec2 uv = SamplerSphericalMap(normalize(worldPos));
    gColor = texture(panoTex, uv);
    vec4 panoPos = texture(panoPosTex, uv).bgra;
    gWorldPos = panoPos;
    gNormal = texture(panoNormalTex, uv).bgra;
	//gNormal = -normalize(panoPos);
    gmipmapedFrameDepth = length(panoPos.bgr);
    vec2 coord = getPrevCoords(worldPos);
    gMat_PreCoord = vec4(0, coord, 0);
    gModelNormal = texture(panoNormalTex, uv).bgra;
    gAlbedo  = texture(panoAlbedoTex,uv).rgba;  
    gDiffTexTure = vec4(0,0,0,1);
}