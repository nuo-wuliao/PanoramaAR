#version 430 core

layout (location = 0) out float mipDepth;

in vec2 TexCoords;

uniform float inputLevel;
uniform sampler2D inputTex;
uniform vec2 texSize;

void main()
{
    //vec2 texSize = textureSize(inputTex, inputLevel);
    vec2 invTexSize = 1.f/texSize;
    float ratio = pow(2, inputLevel + 1);

    float v0 = textureLod(inputTex, TexCoords * ratio + invTexSize * vec2(-0.5,-0.5), inputLevel).r;
    float v1 = min(v0, textureLod(inputTex, TexCoords * ratio + invTexSize * vec2(0.5,-0.5), inputLevel).r);
    float v2 = min(v1, textureLod(inputTex, TexCoords * ratio + invTexSize * vec2(-0.5,0.5), inputLevel).r);
    float v3 = min(v2, textureLod(inputTex, TexCoords * ratio + invTexSize * vec2(0.5,0.5), inputLevel).r);

    mipDepth = v3;//TexCoords.x * 5.f;//
}