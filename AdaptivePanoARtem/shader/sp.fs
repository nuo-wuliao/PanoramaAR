#version 430 core

layout (location = 0) out vec4 spIrrad;
layout (location = 1) out vec4 spIrradRV;

in vec2 TexCoords;

uniform float sigma_p;
uniform float sigma_c;
uniform float sigma_n;
uniform float sigma_d;
uniform float glo_ratio;
uniform float sdRatio;
uniform vec2 iResolution;
uniform int level;
uniform sampler2D rtIrradTex;
uniform sampler2D rtIrradRVTex;
uniform sampler2D normalTex;
uniform sampler2D worldPosTex;
uniform sampler2D mat_prevCoordsTex;
uniform sampler2D ratioTex;

const float PI = 3.141592653f;

float luminance(vec3 v){
    return 0.2126 * v.x + 0.7152 * v.y + 0.0722* v.z;
}

void ATrousFilter()
{
    vec3 fColor1, fColor2;
    // float h[25] = { 1.0 / 256.0, 1.0 / 64.0, 3.0 / 128.0, 1.0 / 64.0, 1.0 / 256.0,
    //                 1.0 / 64.0, 1.0 / 16.0, 3.0 / 32.0, 1.0 / 16.0, 1.0 / 64.0,
    //                 3.0 / 128.0, 3.0 / 32.0, 9.0 / 64.0, 3.0 / 32.0, 3.0 / 128.0,
    //                 1.0 / 64.0, 1.0 / 16.0, 3.0 / 32.0, 1.0 / 16.0, 1.0 / 64.0,
    //                 1.0 / 256.0, 1.0 / 64.0, 3.0 / 128.0, 1.0 / 64.0, 1.0 / 256.0 
    //             };
    float h[3] = {1.f, 2.f/3.f, 1.f/6.f};
    vec2 invRes = 1.f/iResolution;
    float weight = 0,  sum_weight1 = 0, sum_weight2 = 0, sigma_p = 1, new_sigma_c;
    vec3 sum2 = vec3(0), sum1 = vec3(0);
    vec3 icolor1 = texture(rtIrradTex, TexCoords).rgb, icolor2 = texture(rtIrradRVTex, TexCoords).rgb;
    //icolor1 /=(1+icolor1);icolor2 /=(1+icolor2);
    //icolor1/=PI;icolor2/=PI;
    icolor1 = clamp(icolor1, 0, 1);icolor2 = clamp(icolor2, 0, 1);
    icolor1 = pow(icolor1, vec3(1.f/2.2f));
    icolor2 = pow(icolor2, vec3(1.f/2.2f));
    float li1 = luminance(icolor1);
    float li2 = luminance(icolor2);
    vec3 inormal = texture(normalTex, TexCoords).rgb, ipos = texture(worldPosTex, TexCoords).rgb;
    float w_r = pow(texture(ratioTex, TexCoords).w, sdRatio);
    float imat = texture(mat_prevCoordsTex, TexCoords).x;
    if(imat == 2) new_sigma_c = sigma_c * glo_ratio;//-0.15pow(0.1, 4)pow(0.1, 2)
    else new_sigma_c = sigma_c;
    float gap = 1 << level; //pow(2, level);
    for(int i = -2; i<=2;i++){
        for(int j = -2; j<=2;j++){
            float di = i * gap;
            float dj = j * gap;
            vec2 dTexCoords = vec2(di *invRes.x, dj *invRes.y);
            vec2 sampTexCoords = TexCoords + dTexCoords;
            if(sampTexCoords.x >=0 && sampTexCoords.y >=0 && sampTexCoords.x < 1 && sampTexCoords.y < 1
                && imat == texture(mat_prevCoordsTex, sampTexCoords).x){//
                vec3 tcolor1 = texture(rtIrradTex, sampTexCoords).rgb;
                vec3 color1 = tcolor1;                
                //vec3 color1 = tcolor1/(1+tcolor1);
                //color1/=PI;
                color1 = clamp(color1, 0, 1);
                color1 = pow(color1, vec3(1.f/2.2f));
                vec3 tcolor2 = texture(rtIrradRVTex, sampTexCoords).rgb;
                vec3 color2 = tcolor2;
                //vec3 color2 = tcolor2/(1+tcolor2);
                //color2/=PI;
                color2 = clamp(color2, 0, 1);
                color2 = pow(color2, vec3(1.f/2.2f));
                vec3 normal = texture(normalTex, sampTexCoords).rgb; 
                vec3 pos = texture(worldPosTex, sampTexCoords).rgb;
                //float w_r = -pow(texture(ratioTex, sampTexCoords).w, sdRatio);//-texture(ratioTex, sampTexCoords).w * sdRatio;//

                float w_p = -(i*i + j*j)/(2.f * gap * gap); //
                // vec3 dcolor1 = icolor1 - color1;
                // vec3 dcolor2 = icolor2 - color2;
                float l1 = luminance(color1);
                float l2 = luminance(color2);
                //float w_c1 = -dot(dcolor1,dcolor1)/(2.f * new_sigma_c * new_sigma_c);// 
                //float w_c2 = -dot(dcolor2,dcolor2)/(2.f * new_sigma_c * new_sigma_c);//
                float w_c1 = -abs(li1-l1)/(2.f * new_sigma_c * new_sigma_c);// 
                float w_c2 = -abs(li2-l2)/(2.f * new_sigma_c * new_sigma_c);//
                float w_n = -pow(acos(clamp(dot(inormal, normal), 0,1)), 2.f)/(2.f *sigma_n * sigma_n);// 
                float w_d = -pow(clamp(dot(inormal, normalize(pos - ipos)), 0,1),2.f)/(2.f *sigma_d *sigma_d);//
                //int offset = (2 + i) + (2 + j) * 5;
                weight = exp(w_p + w_c1 + w_n + w_d) * h[abs(i)] * h[abs(j)];//
                sum_weight1 += weight;
                sum1 += weight * tcolor1;
                weight = exp(w_p + w_c2 + w_n + w_d) * h[abs(i)] * h[abs(j)];// + w_r
                sum_weight2 += weight;
                sum2 += weight * tcolor2;
            }
        }
    }


    fColor1 = sum1 / sum_weight1;
    fColor2 = sum2 / sum_weight2 * w_r;

    spIrrad = vec4(fColor1, 1);
    spIrradRV = vec4(fColor2, 1);
}

void main()
{
    ATrousFilter();
}