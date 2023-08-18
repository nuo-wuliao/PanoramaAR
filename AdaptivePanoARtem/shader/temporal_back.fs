#version 430 core

layout (location = 0) out vec4 temColorAcc;
layout (location = 1) out vec2 temMomentAcc;
layout (location = 2) out float temVarianceOut;
layout (location = 3) out float temHistoryLengthUpdate;

in vec2 TexCoords;

uniform float color_alpha_min;
uniform float moment_alpha_min;
uniform vec2 iResolution;
uniform sampler2D frameColorTex;
uniform sampler2D mat_prevCoordsTex;
uniform sampler2D modelNormalTex;
uniform sampler2D preMat_CoordsTex;
uniform sampler2D preNormalTex;
uniform sampler2D color_historyTex;
uniform sampler2D moment_historyTex;
uniform sampler2D history_lenthTex;


float getluminance(vec3 color)
{
	return dot(color, vec3(0.299, 0.587, 0.114));
}


vec3 RGBToYCgCo(vec3 rgb)
{
    float Y = dot(rgb, vec3(0.25f, 0.50f, 0.25f));
    float Cg = dot(rgb, vec3(-0.25f, 0.50f, -0.25f));
    float Co = dot(rgb, vec3(0.50f, 0.00f, -0.50f));
    return vec3(Y, Cg, Co);
}

vec3 YCgCoToRGB(vec3 YCgCo)
{
    float tmp = YCgCo.x - YCgCo.y;
    float r = tmp + YCgCo.z;
    float g = YCgCo.x + YCgCo.y;
    float b = tmp - YCgCo.z;
    return vec3(r, g, b);
}


bool isReprjValid(vec2 invres, vec2 prev_coord, float currentMatId, vec3 curr_normal) 
{
    vec2 prev_texcoord = prev_coord * invres;
    // reject if the pixel is outside the screen
	if (prev_coord.x < 0 || prev_coord.x >= iResolution.x || prev_coord.y < 0 || prev_coord.y >= iResolution.y) return false;
	// reject if the pixel is a different geometry
    float prev_matid = texture(preMat_CoordsTex, prev_texcoord).x;
	if (prev_matid != currentMatId) return false;
	// reject if the normal deviation is not acceptable
    vec3 prev_normal = texture(preNormalTex, prev_texcoord).rgb;
	if (length(prev_normal - curr_normal) > 0.2) return false;
	return true;
}


void BackProjection()
{
    vec3 current_color = texture(frameColorTex, TexCoords).rgb;
    float N = texture(history_lenthTex, TexCoords).x;
    float luminance = 0.2126 * current_color.x + 0.7152 * current_color.y + 0.0722 * current_color.z;
    vec2 invResolution = 1.f/iResolution;

    vec3 curr_matCoords = texture(mat_prevCoordsTex, TexCoords).rgb;
    vec2 prevcoords = curr_matCoords.yz;
    float prevx = prevcoords.x;
    float prevy = prevcoords.y;
    vec2 loc = vec2(prevx, prevy);
    float curr_matid = curr_matCoords.x;
    vec3 prevColor = texture(color_historyTex, clamp(loc * invResolution,0,1)).rgb;
    float prevHistoryLength = texture(history_lenthTex, clamp(loc * invResolution,0,1)).x;
    vec3 curr_normal = texture(modelNormalTex, TexCoords).rgb;

    // vec3 curr_matCoords = texture(mat_prevCoordsTex, TexCoords).rgb;
    // vec2 prevcoords = curr_matCoords.yz;
    // float prevx = prevcoords.x;
    // float prevy = prevcoords.y;

    // bool v[4];
    // float floorx = floor(prevx);
    // float floory = floor(prevy);
    // float fracx = prevx - floorx;
    // float fracy = prevy - floory;

    // bool valid = (floorx >= 0 && floory >= 0 && floorx < iResolution.x && floory < iResolution.y);

    // // 2x2 tap bilinear filter
    // vec2 offset[4] = { vec2(0,0), vec2(1,0), vec2(0,1), vec2(1,1) };

    // vec3 curr_normal = texture(modelNormalTex, TexCoords).rgb;
    // float curr_matid = curr_matCoords.x;
    // // check validity
    // {
    //     for (int sampleIdx = 0; sampleIdx < 4; sampleIdx++) {
    //         vec2 loc = vec2(floorx, floory) + offset[sampleIdx];
    //         v[sampleIdx] = isReprjValid(invResolution, loc, curr_matid, curr_normal);
    //         valid = valid && v[sampleIdx];
    //     }
    // }

    // vec3 prevColor = vec3(0.0f);
    // vec2 prevMoments = vec2(0.0f);
    // float prevHistoryLength = 0.0f;

    // if (valid) {
    //     // interpolate?
    //     float sumw = 0.0f;
    //     float w[4] = { (1 - fracx) * (1 - fracy),
    //         fracx * (1 - fracy),
    //         (1 - fracx) * fracy,
    //         fracx * fracy };

    //     for (int sampleIdx = 0; sampleIdx < 4; sampleIdx++) {
    //         vec2 loc = vec2(floorx, floory) + offset[sampleIdx];
    //         if (v[sampleIdx]) {
    //             prevColor += w[sampleIdx] * texture(color_historyTex, clamp(loc * invResolution,0,1)).rgb; 
    //             prevMoments += w[sampleIdx] * texture(moment_historyTex, clamp(loc * invResolution,0,1)).xy;
    //             prevHistoryLength += w[sampleIdx] * texture(history_lenthTex, clamp(loc * invResolution,0,1)).x;
    //             sumw += w[sampleIdx];
    //         }
    //     }
    //     if (sumw >= 0.01) {
    //         prevColor /= sumw;
    //         prevMoments /= sumw;
    //         prevHistoryLength /= sumw;
    //         //prevHistoryLength = 1;
    //         valid = true;
    //     }
    // }

    // // find suitable samples elsewhere
    // if (!valid) {
    //     float cnt = 0.0f;
    //     int radius = 1;

    //     for (int yy = -radius; yy <= radius; yy++) {
    //         for (int xx = -radius; xx <= radius; xx++) {
    //             vec2 loc = vec2(floorx, floory) + vec2(xx, yy);
    //             if (isReprjValid(invResolution, loc, curr_matid, curr_normal)) {
    //                 prevColor += texture(color_historyTex, clamp(loc * invResolution,0,1)).rgb; 
    //                 prevMoments += texture(moment_historyTex, clamp(loc * invResolution,0,1)).xy;
    //                 prevHistoryLength += texture(history_lenthTex, clamp(loc * invResolution,0,1)).x;
    //                 cnt += 1.0f;
    //             }
    //         }
    //     }

    //     if (cnt > 0.0f) {
    //         prevColor /= cnt;
    //         prevMoments /= cnt;
    //         prevHistoryLength /= cnt;
    //         //prevHistoryLength = 0;
    //         valid = true;
    //     }
    // }

    if(isReprjValid(invResolution, loc, curr_matid, curr_normal))
    //if(valid)
    {
        temHistoryLengthUpdate = prevHistoryLength + 1;
        //float color_alpha = max(1.0f / (N + 1), color_alpha_min);
        vec2 c_offset[8] = { vec2(-1,-1), vec2(-1,1), vec2(1,-1), vec2(1,1),
                                vec2(1,0), vec2(0,-1), vec2(0,1), vec2(-1,0) };
        vec3 color = RGBToYCgCo(current_color);
        vec3 colorAvg = color;
        vec3 colorVar = color * color;
        float dominent = 1.f;

        for (int sampleIdx = 0; sampleIdx < 8; sampleIdx++) {
            vec2 loc = vec2(TexCoords * iResolution) + c_offset[sampleIdx];
            if (loc.x >= 0 && loc.y >= 0 && loc.x < iResolution.x && loc.y < iResolution.y && 
                curr_matid == texture(mat_prevCoordsTex, clamp(loc * invResolution,0,1)).r) {
                vec3 c = RGBToYCgCo(texture(frameColorTex, clamp(loc * invResolution,0,1)).rgb);
                colorAvg += c;
                colorVar += c*c;
                dominent+=1.f;
            }
        }

        float oneOverNine = 1.f / dominent;//9.f;//
        colorAvg *= oneOverNine;
        colorVar *= oneOverNine;

        vec3 sigma = vec3(sqrt(max(0.f, colorVar.x - colorAvg.x * colorAvg.x)),
            sqrt(max(0.f, colorVar.y - colorAvg.y * colorAvg.y)),
            sqrt(max(0.f, colorVar.z - colorAvg.z * colorAvg.z)));
        vec3 colorMin = colorAvg - 10.f * sigma;                                 //2.f is the colorboxsigma
        vec3 colorMax = colorAvg + 10.f * sigma;

        vec3 history = RGBToYCgCo(prevColor);
        float distToClamp = min(abs(colorMin.x - history.x), abs(colorMax.x - history.x));
        float new_alpha = clamp((color_alpha_min * distToClamp) / (distToClamp + colorMax.x - colorMin.x), 0.0f, 1.0f);// color_alpha;//

        history = clamp(history, colorMin, colorMax);
        vec3 ppColor_acc = YCgCoToRGB(color * new_alpha + history * (1.0f - new_alpha));//current_color * new_alpha + history * (1.0f - new_alpha);

        // float distToClamp = min(abs(getluminance(colorMin) - getluminance(prevColor)), abs(getluminance(colorMax) - getluminance(prevColor)));
        //float new_alpha = clamp((color_alpha * distToClamp) / (distToClamp + getluminance(colorMax) - getluminance(colorMin)), 0.0f, 1.0f); 

        //vec3 history = clamp(prevColor, colorMin, colorMax);
        //vec3 ppColor_acc = current_color * new_alpha + history * (1.0f - new_alpha);
        temColorAcc = vec4(ppColor_acc, 1);
    }
    else
    {
        temColorAcc = vec4(current_color, 1);
        temHistoryLengthUpdate = 1;
    }
}


// void BackProjection()
// {
//     vec3 current_color = texture(frameColorTex, TexCoords).rgb;
//     float N = texture(history_lenthTex, TexCoords).x;
//     float luminance = 0.2126 * current_color.x + 0.7152 * current_color.y + 0.0722 * current_color.z;
//     vec2 invResolution = 1.f/iResolution;

//     vec3 curr_matCoords = texture(mat_prevCoordsTex, TexCoords).rgb;
//     vec2 prevcoords = curr_matCoords.yz;
//     float prevx = prevcoords.x;
//     float prevy = prevcoords.y;
//     vec2 loc = vec2(prevx, prevy);
//     float curr_matid = curr_matCoords.x;
//     vec3 prevColor = texture(color_historyTex, clamp(loc * invResolution,0,1)).rgb;
//     float prevHistoryLength = texture(history_lenthTex, clamp(loc * invResolution,0,1)).x;
//     vec3 curr_normal = texture(modelNormalTex, TexCoords).rgb;
//     //temColorAcc = vec4(current_color * color_alpha_min + prevColor * (1.0f - color_alpha_min), 1);
//     // if(isReprjValid(invResolution, loc, curr_matid, curr_normal))
//     // {
//     //     vec2 c_offset[8] = { vec2(-1,-1), vec2(-1,1), vec2(1,-1), vec2(1,1),
//     //                             vec2(1,0), vec2(0,-1), vec2(0,1), vec2(-1,0) };
//     //     vec3 color = current_color;
//     //     vec3 colorAvg = color;
//     //     vec3 colorVar = color * color;
//     //     float dominent = 1.f;

//     //     for (int sampleIdx = 0; sampleIdx < 8; sampleIdx++) {
//     //         vec2 loc = vec2(TexCoords * iResolution) + c_offset[sampleIdx];
//     //         if (loc.x >= 0 && loc.y >= 0 && loc.x < iResolution.x && loc.y < iResolution.y && 
//     //             curr_matid == texture(mat_prevCoordsTex, clamp(loc * invResolution,0,1)).r) {
//     //             vec3 c = texture(frameColorTex, clamp(loc * invResolution,0,1)).rgb;
//     //             colorAvg += c;
//     //             colorVar += c*c;
//     //             dominent+=1.f;
//     //         }
//     //     }

//     //     float oneOverNine = 1.f / dominent;//9.f;//
//     //     colorAvg *= oneOverNine;
//     //     colorVar *= oneOverNine;

//     //     vec3 sigma = vec3(sqrt(max(0.f, abs(colorVar.x - colorAvg.x * colorAvg.x))),
//     //         sqrt(max(0.f, abs(colorVar.y - colorAvg.y * colorAvg.y))),
//     //         sqrt(max(0.f, abs(colorVar.z - colorAvg.z * colorAvg.z))));
//     //     vec3 colorMin = colorAvg - 10.f * sigma;                                 //2.f is the colorboxsigma
//     //     vec3 colorMax = colorAvg + 10.f * sigma;

//     //     vec3 history = prevColor;
//     //     float distToClamp = min(abs(getluminance(colorMin) - getluminance(history)), abs(getluminance(colorMax) - getluminance(history)));
//     //     float new_alpha = clamp((color_alpha_min * distToClamp) / (distToClamp + getluminance(colorMax) - getluminance(colorMin)), 0.0f, 1.0f);// color_alpha;//

//     //     history = clamp(history, colorMin, colorMax);
//     //     vec3 ppColor_acc = color * new_alpha + history * (1.0f - new_alpha);//current_color * new_alpha + history * (1.0f - new_alpha);

//     //     // float distToClamp = min(abs(getluminance(colorMin) - getluminance(prevColor)), abs(getluminance(colorMax) - getluminance(prevColor)));
//     //     //float new_alpha = clamp((color_alpha * distToClamp) / (distToClamp + getluminance(colorMax) - getluminance(colorMin)), 0.0f, 1.0f); 

//     //     //vec3 history = clamp(prevColor, colorMin, colorMax);
//     //     //vec3 ppColor_acc = current_color * new_alpha + history * (1.0f - new_alpha);
//     //     temColorAcc = vec4(ppColor_acc, 1);
//     // }
//     // else temColorAcc = vec4(current_color, 1);

//     if(isReprjValid(invResolution, loc, curr_matid, curr_normal))
//     {
//         temHistoryLengthUpdate = prevHistoryLength + 1;
//         float color_alpha = max(1.0f / (N + 1), color_alpha_min);
//         vec2 c_offset[8] = { vec2(-1,-1), vec2(-1,1), vec2(1,-1), vec2(1,1),
//                                 vec2(1,0), vec2(0,-1), vec2(0,1), vec2(-1,0) };
//         vec3 color = RGBToYCgCo(current_color);
//         vec3 colorAvg = color;
//         vec3 colorVar = color * color;
//         float dominent = 1.f;

//         for (int sampleIdx = 0; sampleIdx < 8; sampleIdx++) {
//             vec2 loc = vec2(TexCoords * iResolution) + c_offset[sampleIdx];
//             if (loc.x >= 0 && loc.y >= 0 && loc.x < iResolution.x && loc.y < iResolution.y && 
//                 curr_matid == texture(mat_prevCoordsTex, clamp(loc * invResolution,0,1)).r) {
//                 vec3 c = RGBToYCgCo(texture(frameColorTex, clamp(loc * invResolution,0,1)).rgb);
//                 colorAvg += c;
//                 colorVar += c*c;
//                 dominent+=1.f;
//             }
//         }

//         float oneOverNine = 1.f / dominent;//9.f;//
//         colorAvg *= oneOverNine;
//         colorVar *= oneOverNine;

//         vec3 sigma = vec3(sqrt(max(0.f, colorVar.x - colorAvg.x * colorAvg.x)),
//             sqrt(max(0.f, colorVar.y - colorAvg.y * colorAvg.y)),
//             sqrt(max(0.f, colorVar.z - colorAvg.z * colorAvg.z)));
//         vec3 colorMin = colorAvg - 10.f * sigma;                                 //2.f is the colorboxsigma
//         vec3 colorMax = colorAvg + 10.f * sigma;

//         vec3 history = RGBToYCgCo(prevColor);
//         float distToClamp = min(abs(colorMin.x - history.x), abs(colorMax.x - history.x));
//         float new_alpha = clamp((color_alpha * distToClamp) / (distToClamp + colorMax.x - colorMin.x), 0.0f, 1.0f);// color_alpha;//

//         history = clamp(history, colorMin, colorMax);
//         vec3 ppColor_acc = YCgCoToRGB(color * new_alpha + history * (1.0f - new_alpha));//current_color * new_alpha + history * (1.0f - new_alpha);

//         // float distToClamp = min(abs(getluminance(colorMin) - getluminance(prevColor)), abs(getluminance(colorMax) - getluminance(prevColor)));
//         //float new_alpha = clamp((color_alpha * distToClamp) / (distToClamp + getluminance(colorMax) - getluminance(colorMin)), 0.0f, 1.0f); 

//         //vec3 history = clamp(prevColor, colorMin, colorMax);
//         //vec3 ppColor_acc = current_color * new_alpha + history * (1.0f - new_alpha);
//         temColorAcc = vec4(ppColor_acc, 1);
//     }
//     else
//     {
//         temColorAcc = vec4(current_color, 1);
//         temHistoryLengthUpdate = 1;
//     }


//     //if(N > 0)
//     //{
//         //temColorAcc = vec4(1,0,0,1);
//     //     vec3 curr_matCoords = texture(mat_prevCoordsTex, TexCoords).rgb;
//     //     vec2 prevcoords = curr_matCoords.yz;
//     //     float prevx = prevcoords.x;
//     //     float prevy = prevcoords.y;

//     //     bool v[4];
//     //     float floorx = floor(prevx);
//     //     float floory = floor(prevy);
//     //     float fracx = prevx - floorx;
//     //     float fracy = prevy - floory;

//     //     bool valid = (floorx >= 0 && floory >= 0 && floorx < iResolution.x && floory < iResolution.y);

//     //     // 2x2 tap bilinear filter
//     //     vec2 offset[4] = { vec2(0,0), vec2(1,0), vec2(0,1), vec2(1,1) };

//     //     vec3 curr_normal = texture(modelNormalTex, TexCoords).rgb;
//     //     float curr_matid = curr_matCoords.x;
//     //     // check validity
//     //     {
//     //         for (int sampleIdx = 0; sampleIdx < 4; sampleIdx++) {
//     //             vec2 loc = vec2(floorx, floory) + offset[sampleIdx];
//     //             v[sampleIdx] = isReprjValid(invResolution, loc, curr_matid, curr_normal);
//     //             valid = valid && v[sampleIdx];
//     //         }
//     //     }

//     //     vec3 prevColor = vec3(0.0f);
//     //     vec2 prevMoments = vec2(0.0f);
//     //     float prevHistoryLength = 0.0f;

//     //     if (valid) {
//     //         // interpolate?
//     //         float sumw = 0.0f;
//     //         float w[4] = { (1 - fracx) * (1 - fracy),
//     //             fracx * (1 - fracy),
//     //             (1 - fracx) * fracy,
//     //             fracx * fracy };

//     //         for (int sampleIdx = 0; sampleIdx < 4; sampleIdx++) {
//     //             vec2 loc = vec2(floorx, floory) + offset[sampleIdx];
//     //             if (v[sampleIdx]) {
//     //                 prevColor += w[sampleIdx] * texture(color_historyTex, clamp(loc * invResolution,0,1)).rgb; 
//     //                 prevMoments += w[sampleIdx] * texture(moment_historyTex, clamp(loc * invResolution,0,1)).xy;
//     //                 prevHistoryLength += w[sampleIdx] * texture(history_lenthTex, clamp(loc * invResolution,0,1)).x;
//     //                 sumw += w[sampleIdx];
//     //             }
//     //         }
//     //         if (sumw >= 0.01) {
//     //             prevColor /= sumw;
//     //             prevMoments /= sumw;
//     //             prevHistoryLength /= sumw;
//     //             //prevHistoryLength = 1;
//     //             valid = true;
//     //         }
//     //     }

//     //     // find suitable samples elsewhere
//     //     if (!valid) {
//     //         float cnt = 0.0f;
//     //         int radius = 1;

//     //         for (int yy = -radius; yy <= radius; yy++) {
//     //             for (int xx = -radius; xx <= radius; xx++) {
//     //                 vec2 loc = vec2(floorx, floory) + vec2(xx, yy);
//     //                 if (isReprjValid(invResolution, loc, curr_matid, curr_normal)) {
//     //                     prevColor += texture(color_historyTex, clamp(loc * invResolution,0,1)).rgb; 
//     //                     prevMoments += texture(moment_historyTex, clamp(loc * invResolution,0,1)).xy;
//     //                     prevHistoryLength += texture(history_lenthTex, clamp(loc * invResolution,0,1)).x;
//     //                     cnt += 1.0f;
//     //                 }
//     //             }
//     //         }

//     //         if (cnt > 0.0f) {
//     //             prevColor /= cnt;
//     //             prevMoments /= cnt;
//     //             prevHistoryLength /= cnt;
//     //             //prevHistoryLength = 0;
//     //             valid = true;
//     //         }
//     //     }

//     //     if(valid)
//     //     {
//     //         temHistoryLengthUpdate = prevHistoryLength + 1;
//               //float color_alpha = max(1.0f / (N + 1), color_alpha_min);
//     //         vec2 c_offset[8] = { vec2(-1,-1), vec2(-1,1), vec2(1,-1), vec2(1,1),
//     //                                 vec2(1,0), vec2(0,-1), vec2(0,1), vec2(-1,0) };
//     //         vec3 color = RGBToYCgCo(current_color);
//     //         vec3 colorAvg = color;
//     //         vec3 colorVar = color * color;
//     //         float dominent = 1.f;

//     //         for (int sampleIdx = 0; sampleIdx < 8; sampleIdx++) {
//     //             vec2 loc = vec2(TexCoords * iResolution) + c_offset[sampleIdx];
//     //             if (loc.x >= 0 && loc.y >= 0 && loc.x < iResolution.x && loc.y < iResolution.y && 
//     //                 curr_matid == texture(mat_prevCoordsTex, clamp(loc * invResolution,0,1)).r) {
//     //                 vec3 c = RGBToYCgCo(texture(frameColorTex, clamp(loc * invResolution,0,1)).rgb);
//     //                 colorAvg += c;
//     //                 colorVar += c*c;
//     //                 dominent+=1.f;
//     //             }
//     //         }

//     //         float oneOverNine = 1.f / dominent;//9.f;//
//     //         colorAvg *= oneOverNine;
//     //         colorVar *= oneOverNine;

//     //         vec3 sigma = vec3(sqrt(max(0.f, colorVar.x - colorAvg.x * colorAvg.x)),
//     //             sqrt(max(0.f, colorVar.y - colorAvg.y * colorAvg.y)),
//     //             sqrt(max(0.f, colorVar.z - colorAvg.z * colorAvg.z)));
//     //         vec3 colorMin = colorAvg - 10.f * sigma;                                 //2.f is the colorboxsigma
//     //         vec3 colorMax = colorAvg + 10.f * sigma;

//     //         vec3 history = RGBToYCgCo(prevColor);
//     //         float distToClamp = min(abs(colorMin.x - history.x), abs(colorMax.x - history.x));
//     //         float new_alpha = clamp((color_alpha_min * distToClamp) / (distToClamp + colorMax.x - colorMin.x), 0.0f, 1.0f);// color_alpha;//

//     //         history = clamp(history, colorMin, colorMax);
//     //         vec3 ppColor_acc = YCgCoToRGB(color * new_alpha + history * (1.0f - new_alpha));//current_color * new_alpha + history * (1.0f - new_alpha);

//     //         // float distToClamp = min(abs(getluminance(colorMin) - getluminance(prevColor)), abs(getluminance(colorMax) - getluminance(prevColor)));
//     //         //float new_alpha = clamp((color_alpha * distToClamp) / (distToClamp + getluminance(colorMax) - getluminance(colorMin)), 0.0f, 1.0f); 

//     //         //vec3 history = clamp(prevColor, colorMin, colorMax);
//     //         //vec3 ppColor_acc = current_color * new_alpha + history * (1.0f - new_alpha);
//     //         temColorAcc = vec4(ppColor_acc, 1);
//     //         return;
//     //     }

//     //     // if (valid) {
//     //     //     // calculate alpha values that controls fade
//     //     //     float color_alpha = max(1.0f / (N + 1), color_alpha_min);
//     //     //     float moment_alpha = max(1.0f / (N + 1), moment_alpha_min);

//     //     //     // incresase history length
//     //     //     temHistoryLengthUpdate = prevHistoryLength + 1;

//     //     //     // color accumulation
//     //     //     //temColorAcc = vec4(current_color * color_alpha + prevColor * (1.0f - color_alpha), 1);
//     //     //     {

//     //     //        vec2 c_offset[8] = { vec2(-1,-1), vec2(-1,1), vec2(1,-1), vec2(1,1),
//     //     //                                vec2(1,0), vec2(0,-1), vec2(0,1), vec2(-1,0) };
//     //     //        vec3 color = RGBToYCgCo(current_color);
//     //     //        vec3 colorAvg = color;
//     //     //        vec3 colorVar = color * color;
//     //     //        float dominent = 1.f;

//     //     //        for (int sampleIdx = 0; sampleIdx < 8; sampleIdx++) {
//     //     //            vec2 loc = vec2(TexCoords * iResolution) + c_offset[sampleIdx];
//     //     //            if (loc.x >= 0 && loc.y >= 0 && loc.x < iResolution.x && loc.y < iResolution.y && 
//     //     //                curr_matid == texture(mat_prevCoordsTex, clamp(loc * invResolution,0,1)).r) {
//     //     //                vec3 c = RGBToYCgCo(texture(frameColorTex, clamp(loc * invResolution,0,1)).rgb);
//     //     //                colorAvg += c;
//     //     //                colorVar += c*c;
//     //     //                dominent+=1.f;
//     //     //            }
//     //     //        }

//     //     //        float oneOverNine = 1.f / dominent;//9.f;//
//     //     //        colorAvg *= oneOverNine;
//     //     //        colorVar *= oneOverNine;

//     //     //        vec3 sigma = vec3(sqrt(max(0.f, colorVar.x - colorAvg.x * colorAvg.x)),
//     //     //            sqrt(max(0.f, colorVar.y - colorAvg.y * colorAvg.y)),
//     //     //            sqrt(max(0.f, colorVar.z - colorAvg.z * colorAvg.z)));
//     //     //        vec3 colorMin = colorAvg - 5.f * sigma;                                 //2.f is the colorboxsigma
//     //     //        vec3 colorMax = colorAvg + 5.f * sigma;

//     //     //        vec3 history = RGBToYCgCo(prevColor);
//     //     //        float distToClamp = min(abs(colorMin.x - history.x), abs(colorMax.x - history.x));
//     //     //        float new_alpha = color_alpha_min;//clamp((color_alpha * distToClamp) / (distToClamp + colorMax.x - colorMin.x), 0.0f, 1.0f);// 

//     //     //        history = clamp(history, colorMin, colorMax);
//     //     //        vec3 ppColor_acc = YCgCoToRGB(color * new_alpha + history * (1.0f - new_alpha));//current_color * new_alpha + history * (1.0f - new_alpha);

//     //     //        // float distToClamp = min(abs(getluminance(colorMin) - getluminance(prevColor)), abs(getluminance(colorMax) - getluminance(prevColor)));
//     //     //        //float new_alpha = clamp((color_alpha * distToClamp) / (distToClamp + getluminance(colorMax) - getluminance(colorMin)), 0.0f, 1.0f); 

//     //     //         //vec3 history = clamp(prevColor, colorMin, colorMax);
//     //     //         //vec3 ppColor_acc = current_color * new_alpha + history * (1.0f - new_alpha);
//     //     //        temColorAcc = vec4(ppColor_acc, 1);
//     //     //     }


//     //     //     // moment accumulation
//     //     //     float first_moment = moment_alpha * prevMoments.x + (1.0f - moment_alpha) * luminance;
//     //     //     float second_moment = moment_alpha * prevMoments.y + (1.0f - moment_alpha) * luminance * luminance;
//     //     //     temMomentAcc = vec2(first_moment, second_moment);

//     //     //     // calculate variance from moments
//     //     //     float variance = second_moment - first_moment * first_moment;
//     //     //     temVarianceOut = variance > 0.0f ? variance : 0.0f;
//     //     //     return;
//     //     // }
//     // //}
//     // temHistoryLengthUpdate = 1;
//     // temColorAcc = vec4(current_color, 1);
//     // temMomentAcc = vec2(luminance, luminance * luminance);
//     // temVarianceOut = 100.f;
// }

void main()
{
    BackProjection();
}