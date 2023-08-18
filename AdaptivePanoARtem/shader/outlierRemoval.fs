#version 430 core

layout (location = 0) out vec4 orIrrad;
layout (location = 1) out vec4 orIrradRV;

in vec2 TexCoords;

uniform int halfKernel;
uniform vec2 iResolution;
uniform sampler2D rtIrradTex;
uniform sampler2D rtIrradRVTex;

void main()
{
    vec2 invRes = 1.f/iResolution;
    vec3 mean1 = vec3(0),mean2 = vec3(0),var1 = vec3(0),var2 = vec3(0);
    float dominent = 0.f;
    vec3 rtIrrad = texture(rtIrradTex, TexCoords).rgb;
    vec3 rtIrradRV = texture(rtIrradRVTex, TexCoords).rgb;
    for(int x = -halfKernel; x<=halfKernel;x++)
    {
        for(int y = -halfKernel;y<=halfKernel;y++)
        {
            vec2 tex = TexCoords + vec2(x,y) * invRes;
            if(tex.x>=0 && tex.y >=0 && tex.x<=1&&tex.y<=1)
            {
                vec3 c1 = texture(rtIrradTex, tex).rgb;
                vec3 c2 = texture(rtIrradRVTex, tex).rgb;
                mean1+=c1;
                mean2+=c2;
                var1+=(c1 *c1);
                var2+=(c2 *c2);
                dominent+=1;
            }
        }
    }
    float invDominent = 1.f/dominent;
    mean1*=invDominent; mean2*=invDominent;
    var1 *= invDominent; var2*=invDominent;

    vec3 sigma1 = vec3(sqrt(max(0, abs(var1.x - mean1.x * mean1.x))), sqrt(max(0, abs(var1.y - mean1.y * mean1.y))), sqrt(max(0, abs(var1.z - mean1.z * mean1.z))));
    vec3 sigma2 = vec3(sqrt(max(0, abs(var2.x - mean2.x * mean2.x))), sqrt(max(0, abs(var2.y - mean2.y * mean2.y))), sqrt(max(0, abs(var2.z - mean2.z * mean2.z))));
    vec3 colorMin1 = mean1 - 2.f * sigma1;
    vec3 colorMax1 = mean1 + 2.f * sigma1;
    vec3 colorMin2 = mean2 - 2.f * sigma2;
    vec3 colorMax2 = mean2 + 2.f * sigma2;

    vec3 result1 = clamp(rtIrrad, colorMin1,colorMax1);
    vec3 result2 = clamp(rtIrradRV, colorMin2, colorMax2);

    orIrrad = vec4(result1, 1);
    orIrradRV = vec4(result2, 1);
}