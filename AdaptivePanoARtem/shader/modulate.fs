#version 430 core

layout (location = 0) out vec4 mColor;
layout (location = 1) out vec4 mAlbedo;
layout (location = 2) out vec4 mAccIrrad;
layout (location = 3) out vec4 mAccIrradRV;
layout (location = 4) out vec4 mAccColor;

in vec2 TexCoords;

//uniform vec3 rho;
uniform vec3 refspec;
uniform bool isAcc;
uniform int frameNum;
uniform int modu_sigma;
uniform sampler2D irradTex;
uniform sampler2D irradRVTex;
uniform sampler2D colorTex;
uniform sampler2D mat_prevCoordsTex;
uniform sampler2D rtIrradTex;
uniform sampler2D rtIrradRVTex;
uniform sampler2D accHisIrradTex;
uniform sampler2D accHisIrradRVTex;
uniform sampler2D albedoTex;
uniform sampler2D diffTextureTex;

float PI = 3.141592653;

float luminance(vec3 color)
{
	return dot(color, vec3(0.299, 0.587, 0.114));
}

void main()
{
    vec3 rho = texture(diffTextureTex,TexCoords).rgb;
    vec3 accColor;
    highp vec3 accIrrad,accIrradRV;
    if(isAcc) {
        if(frameNum == 600){
            accIrrad = texture(accHisIrradTex, TexCoords).rgb;
            accIrradRV = texture(accHisIrradRVTex, TexCoords).rgb;
        }
        else{
            accIrrad = (float(frameNum)/float(frameNum+1)) * texture(accHisIrradTex, TexCoords).rgb + texture(rtIrradTex, TexCoords).rgb/float(frameNum+1);
            accIrradRV = (float(frameNum)/float(frameNum+1)) * texture(accHisIrradRVTex, TexCoords).rgb + texture(rtIrradRVTex, TexCoords).rgb/float(frameNum+1);
        }
    }
    else{
        accIrrad =  texture(rtIrradTex, TexCoords).rgb;
        accIrradRV =  texture(rtIrradRVTex, TexCoords).rgb;
    }


    float hitmat = texture(mat_prevCoordsTex, TexCoords).r;
    vec3 color, albedo;
    if(hitmat == 0){
        vec3 temp = texture(colorTex, TexCoords).rgb;
        vec3 temp1 = texture(irradTex, TexCoords).rgb;
        vec3 temp2 = texture(irradRVTex, TexCoords).rgb;

        vec3 ratio = temp2/temp1;
        if(ratio.x < 1) ratio.x = pow(ratio.x, modu_sigma);
		if(ratio.y < 1) ratio.y = pow(ratio.y, modu_sigma);
		if(ratio.z < 1) ratio.z = pow(ratio.z, modu_sigma);
        
        color = temp * ratio; //clamp(temp1 / temp2, 1e-4, 2.f);
        accColor= temp * (accIrradRV/accIrrad);
        if(color.x !=color.x || color.y !=color.y ||color.z !=color.z ) color = temp;
        if(accColor.x !=accColor.x || accColor.y !=accColor.y ||accColor.z !=accColor.z ) accColor = temp;
        albedo = temp / (temp1 + 1e-4);
    }
    else if(hitmat == 1){
        vec3 temp = texture(irradRVTex, TexCoords).rgb;
        color = temp * rho / PI;
        accColor = accIrradRV * rho/PI;
        albedo = rho;
    }
    else if(hitmat == 2){
        color = texture(irradRVTex, TexCoords).rgb;
        accColor = accIrradRV;
        albedo = refspec;
    }

    mColor = vec4(color, 1);
    mAlbedo = vec4(albedo, 1);
    mAccIrrad = vec4(accIrrad, 1);
    mAccIrradRV = vec4(accIrradRV, 1);
    mAccColor = vec4(accColor, 1);
}