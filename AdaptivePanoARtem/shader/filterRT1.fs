#version 430 core

layout (location = 0) out vec4 rtNewColor;
layout (location = 1) out vec4 rtIrrad;
layout (location = 2) out vec4 rtIrradRV;
layout (location = 3) out vec3 traceData;
layout (location = 4) out vec3 traceDataRV;
layout (location = 5) out vec4 rtTempRV;


in vec2 TexCoords;

uniform vec2 iResolution;
uniform sampler2D adaptStencilTex;
uniform sampler2D irradTex;
uniform sampler2D irradRVTex;
uniform sampler2D tempRVTex;
uniform sampler2D colorTex;
uniform sampler2D mat_prevCoordTex;

void main()
{
    float hitmat = texture(mat_prevCoordTex, TexCoords).x;
    vec2 adaptStencil = texture(adaptStencilTex, TexCoords).xy;
    vec2 invres = 1.f / iResolution;
    vec3 irrad,tempirradRV, irradRV, newColor;
    vec2 ipos = vec2(TexCoords * iResolution);

    if(hitmat == 0){
        if(adaptStencil.x == 0){
            if(adaptStencil.y == 2){
                vec2 tex1 = (floor(ipos/2) *2  + vec2(0.1)) * invres;
                if(texture(adaptStencilTex, tex1).x == 0) tex1 = (floor(ipos/4) *4 + vec2(0.1)) * invres;
                vec2 tex2 = ((floor(ipos/2) + vec2(0,1)) * 2 + vec2(0.1)) * invres;
                if(texture(adaptStencilTex, tex2).x == 0) tex2 = ((floor(ipos/4)+ vec2(0,1)) * 4  + vec2(0.1)) * invres;
                vec2 tex3 = ((floor(ipos/2) + vec2(1,0)) * 2 + vec2(0.1)) * invres;
                if(texture(adaptStencilTex, tex3).x == 0) tex3 = ((floor(ipos/4)+ vec2(1,0)) * 4  + vec2(0.1)) * invres;
                vec2 tex4 = ((floor(ipos/2) + vec2(1,1)) * 2 + vec2(0.1)) * invres;
                if(texture(adaptStencilTex, tex4).x == 0) tex4 = ((floor(ipos/4)+ vec2(1,1)) * 4  + vec2(0.1)) * invres;

                float dis1 = length(TexCoords - tex1);
                float dis2 = length(TexCoords - tex2);
                float dis3 = length(TexCoords - tex3);
                float dis4 = length(TexCoords - tex4);
                float tdis = dis1 + dis2 + dis3 + dis4;
                irrad = (dis1 *texture(irradTex, tex1).rgb + dis2 * texture(irradTex, tex2).rgb + 
                    dis3 * texture(irradTex, tex3).rgb + dis4 * texture(irradTex, tex4).rgb) / tdis;
                tempirradRV = (dis1 *texture(irradRVTex, tex1).rgb + dis2 * texture(irradRVTex, tex2).rgb + 
                    dis3 * texture(irradRVTex, tex3).rgb + dis4 * texture(irradRVTex, tex4).rgb) / tdis;
                irradRV = (dis1 *texture(tempRVTex, tex1).rgb + dis2 * texture(tempRVTex, tex2).rgb + 
                    dis3 * texture(tempRVTex, tex3).rgb + dis4 * texture(tempRVTex, tex4).rgb) / tdis;

                vec3 ratio =  min(vec3(2.f), irradRV / (irrad + 1e-4));
                if(ratio.x !=ratio.x || ratio.y !=ratio.y || ratio.z !=ratio.z) ratio = vec3(1.f);
                newColor = texture(colorTex,TexCoords).rgb * ratio;
                rtNewColor = vec4(texture(colorTex, TexCoords).rgb * ratio, min(ratio, 1.f));
            }
            else if(adaptStencil.y == 4){
                vec2 tex1 = (floor(ipos/4) *4 + vec2(0.1)) * invres;
                vec2 tex2 = ((floor(ipos/4)+ vec2(0,1)) * 4  + vec2(0.1)) * invres;
                vec2 tex3 = ((floor(ipos/4)+ vec2(1,0)) * 4  + vec2(0.1)) * invres;
                vec2 tex4 = ((floor(ipos/4)+ vec2(1,1)) * 4  + vec2(0.1)) * invres;
                //vec2 tex1 = (vec2(int(ipos.x/4) *4 + 0.1, int(ipos.y/4) * 4 + 0.1)) * invres;
                // vec2 tex2 = ((vec2(int(ipos.x/4), int(ipos.y/4))+ vec2(0,1)) * 4 ) * invres;
                // vec2 tex3 = ((vec2(int(ipos.x/4), int(ipos.y/4))+ vec2(1,0)) * 4 ) * invres;
                // vec2 tex4 = ((vec2(int(ipos.x/4), int(ipos.y/4))+ vec2(1,1)) * 4 ) * invres;

                float dis1 = length(TexCoords - tex1);
                float dis2 = length(TexCoords - tex2);
                float dis3 = length(TexCoords - tex3);
                float dis4 = length(TexCoords - tex4);
                float tdis = dis1 + dis2 + dis3 + dis4;
                irrad = (dis1 *texture(irradTex, tex1).rgb + dis2 * texture(irradTex, tex2).rgb + 
                    dis3 * texture(irradTex, tex3).rgb + dis4 * texture(irradTex, tex4).rgb) / tdis;
                tempirradRV = (dis1 *texture(irradRVTex, tex1).rgb + dis2 * texture(irradRVTex, tex2).rgb + 
                    dis3 * texture(irradRVTex, tex3).rgb + dis4 * texture(irradRVTex, tex4).rgb) / tdis;
                irradRV = (dis1 *texture(tempRVTex, tex1).rgb + dis2 * texture(tempRVTex, tex2).rgb + 
                    dis3 * texture(tempRVTex, tex3).rgb + dis4 * texture(tempRVTex, tex4).rgb) / tdis;
                // irrad = texture(irradTex, tex1).rgb;
                // tempirradRV = texture(irradRVTex, tex1).rgb;
                // irradRV = texture(tempRVTex, tex1).rgb;

                vec3 ratio =  min(vec3(2.f), irradRV / (irrad + 1e-4));
                if(ratio.x !=ratio.x || ratio.y !=ratio.y || ratio.z !=ratio.z) ratio = vec3(1.f);
                newColor = texture(colorTex,TexCoords).rgb * ratio;
                rtNewColor = vec4(texture(colorTex, TexCoords).rgb * ratio, min(ratio, 1.f));
            }
        }
        else{
            irrad = texture(irradTex, TexCoords).rgb;
            tempirradRV = texture(irradRVTex, TexCoords).rgb;
            irradRV = texture(tempRVTex, TexCoords).rgb;
            vec3 ratio =  min(vec3(2.f), irradRV / (irrad + 1e-4));
            if(ratio.x !=ratio.x || ratio.y !=ratio.y || ratio.z !=ratio.z) ratio = vec3(1.f);
            newColor = texture(colorTex,TexCoords).rgb * ratio;
            rtNewColor = vec4(texture(colorTex, TexCoords).rgb * ratio, min(ratio, 1.f));
        }

        rtIrrad = vec4(irrad, 1);
        rtIrradRV = vec4(tempirradRV, 1);
        rtTempRV = vec4(irradRV, 1);
        //rtNewColor = vec4(newColor, 1);
    }
}

// void main()
// {
// 	float adaptStencil = texture(adaptStencilTex, TexCoords).x;
//     vec2 invres = 1.f / iResolution;
//     vec3 irrad,tempirradRV, irradRV, newColor;
//     if(adaptStencil == 0){
//         irrad = texture(irradTex, TexCoords).rgb;
//         tempirradRV = texture(irradRVTex, TexCoords).rgb;
//         irradRV = texture(tempRVTex, TexCoords).rgb;
//         vec3 ratio =  min(vec3(2.f), irradRV / (irrad + 1e-4));
//         if(ratio.x !=ratio.x || ratio.y !=ratio.y || ratio.z !=ratio.z) ratio = vec3(1.f);
//         newColor = texture(colorTex,TexCoords).rgb * ratio;
//     }
//     else if(adaptStencil == 2){
//         vec2 newTex1 = clamp(TexCoords + invres * vec2(1,0), 0,1);
//         vec2 newTex2 = clamp(TexCoords - invres * vec2(1,0), 0,1);

//         irrad = 0.5 *(texture(irradTex, newTex1).rgb + texture(irradTex, newTex2).rgb);
//         tempirradRV = 0.5 *(texture(irradRVTex, newTex1).rgb + texture(irradRVTex, newTex2).rgb);
//         irradRV = 0.5 *(texture(tempRVTex, newTex1).rgb + texture(tempRVTex, newTex2).rgb);

//         vec3 ratio =  min(vec3(2.f), irradRV / (irrad + 1e-4));
//         if(ratio.x !=ratio.x || ratio.y !=ratio.y || ratio.z !=ratio.z) ratio = vec3(1.f);
//         newColor = texture(colorTex,TexCoords).rgb * ratio;
//     }
//     else if(adaptStencil == 4){
//         vec2 newTex1 = clamp(TexCoords + invres * vec2(0,1), 0,1);
//         vec2 newTex2 = clamp(TexCoords - invres * vec2(0,1), 0,1);

//         irrad = 0.5 *(texture(irradTex, newTex1).rgb + texture(irradTex, newTex2).rgb);
//         tempirradRV = 0.5 *(texture(irradRVTex, newTex1).rgb + texture(irradRVTex, newTex2).rgb);
//         irradRV = 0.5 *(texture(tempRVTex, newTex1).rgb + texture(tempRVTex, newTex2).rgb);

//         vec3 ratio =  min(vec3(2.f), irradRV / (irrad + 1e-4));
//         if(ratio.x !=ratio.x || ratio.y !=ratio.y || ratio.z !=ratio.z) ratio = vec3(1.f);
//         newColor = texture(colorTex,TexCoords).rgb * ratio;
//     }
//     else if(adaptStencil == 6){
//         vec2 newTex1 = clamp(TexCoords + invres * vec2(-1,-1), 0,1);
//         vec2 newTex2 = clamp(TexCoords - invres * vec2(-1,1), 0,1);
//         vec2 newTex3 = clamp(TexCoords + invres * vec2(1,-1), 0,1);
//         vec2 newTex4 = clamp(TexCoords - invres * vec2(1,1), 0,1);

//         irrad = 0.25 *(texture(irradTex, newTex1).rgb + texture(irradTex, newTex2).rgb + 
//             texture(irradTex, newTex3).rgb + texture(irradTex, newTex4).rgb);
//         tempirradRV = 0.25 *(texture(irradRVTex, newTex1).rgb + texture(irradRVTex, newTex2).rgb + 
//             texture(irradRVTex, newTex3).rgb + texture(irradRVTex, newTex4).rgb);
//         irradRV = 0.25 *(texture(tempRVTex, newTex1).rgb + texture(tempRVTex, newTex2).rgb + 
//             texture(tempRVTex, newTex3).rgb + texture(tempRVTex, newTex4).rgb);

//         vec3 ratio =  min(vec3(2.f), irradRV / (irrad + 1e-4));
//         if(ratio.x !=ratio.x || ratio.y !=ratio.y || ratio.z !=ratio.z) ratio = vec3(1.f);
//         newColor = texture(colorTex,TexCoords).rgb * ratio;
//     }

//     rtIrrad = vec4(irrad, 1);
//     rtIrradRV = vec4(tempirradRV, 1);
//     rtTempRV = vec4(irradRV, 1);
//     rtNewColor = vec4(newColor, 1);
// }