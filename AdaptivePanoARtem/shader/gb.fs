#version 430 core

layout (location = 0) out vec4 gColor;
layout (location = 1) out vec4 gWorldPos;
layout (location = 2) out vec4 gNormal;
layout (location = 3) out vec4 gMat_PreCoord;
layout (location = 4) out float gmipmapedFrameDepth;
layout (location = 5) out vec4 gModelNormal;
layout (location = 6) out vec4 gAlbedo;
layout (location = 7) out vec4 gDiffTexTure;

in vec2 TexCoords;

uniform sampler2D colorTex;
uniform sampler2D worldPosTex;
uniform sampler2D normalTex;
uniform sampler2D mat_prevCoordTex;
uniform sampler2D mipmapedFrameDepthTex;
uniform sampler2D modelNormalTex;
uniform sampler2D diff_textureTex;

uniform sampler2D colorTex2;
uniform sampler2D worldPosTex2;
uniform sampler2D normalTex2;
uniform sampler2D mat_prevCoordTex2;
uniform sampler2D mipmapedFrameDepthTex2;
uniform sampler2D modelNormalTex2;
uniform sampler2D diff_textureTex2;

void main()
{
    vec3 pos1 = texture(worldPosTex, TexCoords).rgb;
    vec3 pos2 = texture(worldPosTex2, TexCoords).rgb;
    float depth1 = length(pos1);
    float depth2 = length(pos2);
    if(depth2 == 0 || depth2 > depth1) {
        gColor = texture(colorTex, TexCoords);
        gWorldPos = texture(worldPosTex, TexCoords);
        gNormal = texture(normalTex,TexCoords);
        gMat_PreCoord = texture(mat_prevCoordTex, TexCoords);
        gmipmapedFrameDepth = texture(mipmapedFrameDepthTex,TexCoords).r;
        gModelNormal = texture(modelNormalTex, TexCoords);
        gDiffTexTure = texture(diff_textureTex,TexCoords);
    }
    else{
        gColor = texture(colorTex2, TexCoords);
        gWorldPos = texture(worldPosTex2, TexCoords);
        gNormal = texture(normalTex2,TexCoords);
        gMat_PreCoord = texture(mat_prevCoordTex2, TexCoords);
        gmipmapedFrameDepth = texture(mipmapedFrameDepthTex2,TexCoords).r;
        gModelNormal = texture(modelNormalTex2, TexCoords);
        gDiffTexTure = vec4(texture(diff_textureTex2,TexCoords).rgb, 1);
    }
}