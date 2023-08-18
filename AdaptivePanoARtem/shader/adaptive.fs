#version 430 core

layout (location = 0) out vec4 adaptStencil;

in vec2 TexCoords;

uniform vec2 iResolution;
uniform sampler2D posTex;
uniform sampler2D normalTex;

void main()
{
    vec3 normal = texture(normalTex, TexCoords).rgb;
    vec3 pos = texture(posTex, TexCoords).rgb;
    vec2 invRes = 1.f/iResolution;
    float maxDif = 0.f;
    vec2 loc = TexCoords * iResolution;

    for(int x = -2;x <= 2;x++){
        for(int y = -2;y<=2;y++){
            vec2 pTex = TexCoords + invRes * vec2(x,y);
            vec3 pN = texture(normalTex, pTex).rgb;
            vec3 pP = texture(posTex, pTex).rgb;
            float ndif = length(normal - pN);
            float pdif = 10.f * length(pP - pos);

            maxDif = min(1.f, max(maxDif, max(ndif, pdif)));//ndif
        }
    }
    float pixelType = 0;
    float isTracing  = 0;
    vec3 color;
    if(maxDif < 0.3) {
        pixelType = 4;
        if(int(loc.x) % 4 == 0 && int(loc.y) % 4 == 0) isTracing = 1;
        color = vec3(0.8, 0.2,0.3);
        //if(int(loc.y) % 4 == 0) isTracing = 1;
    }
    else if(maxDif < 0.5) {
        pixelType = 2;
        if(int(loc.x) % 2 == 0 && int(loc.y) % 2 == 0) isTracing = 1;
        color = vec3(0.2, 0.8,0.3);
        //if(int(loc.y) % 2 == 0) isTracing = 1;
    }
    else {
        pixelType = 0;
        isTracing = 1;
        color = vec3(0.2, 0.2,0.8);
    }

    adaptStencil = vec4(isTracing, pixelType,0,1.f);
    //adaptStencil = vec4(color, 1.f);
    //adaptStencil = vec4(maxDif,maxDif,maxDif, 1.f);
    //adaptStencil = vec4(isTracing,isTracing,isTracing, 1.f);
}


// void main()
// {
//     vec3 normal = texture(normalTex, TexCoords).rgb;
//     vec3 pos = texture(posTex, TexCoords).rgb;
//     vec2 invRes = 1.f/iResolution;
//     float maxDif = 0.f;
//     float pixelType = 0;
//     vec2 loc = TexCoords * iResolution;
//     if(int(loc.x) % 2 != 0) pixelType += 2;
//     if(int(loc.y) % 2 != 0) pixelType += 4;

//     for(int x = -2;x <= 2;x++){
//         for(int y = -2;y<=2;y++){
//             vec2 pTex = TexCoords + invRes * vec2(x,y);
//             vec3 pN = texture(normalTex, pTex).rgb;
//             vec3 pP = texture(posTex, pTex).rgb;
//             float ndif = length(normal - pN);
//             float pdif = 10.f * length(pP - pos);

//             maxDif = min(1.f, max(maxDif, max(ndif, pdif)));//ndif
//         }
//     }
//     if(maxDif < 0.5) maxDif = pixelType;//
//     else maxDif = 0.f;
//     adaptStencil = vec4(maxDif, maxDif,maxDif,1.f);
// }