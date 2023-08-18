#version 430 core

layout (location = 0) out vec4 newTemImageAcc;

in vec2 TexCoords;

uniform vec2 iResolution;
uniform float g_2;
uniform sampler2D imageTex;
uniform sampler2D imageAccTex;
uniform sampler2D mat_prevCoordsTex;

float grayscale(vec3 image) {
    return dot(image, vec3(0.3, 0.59, 0.11));
}

float normpdf(in float x, in float sigma)
{
	return 0.39894*exp(-0.5*x*x/(sigma*sigma))/sigma;
}


void main()
{
	vec3 curr_matCoords = texture(mat_prevCoordsTex, TexCoords).rgb;
	vec2 prevcoords = curr_matCoords.yz;
    float curr_matid = curr_matCoords.x;
	
	vec3 imageacc = max(texture(imageAccTex, prevcoords).rgb, vec3(0.f));
	vec3 image = max(texture(imageTex, TexCoords).rgb, vec3(0.f));

	//declare stuff
    const int mSize = 6;
    const int kSize = (mSize-1)/2 ;
    float kernel[mSize];
    vec3 imageblurred = vec3(0.0);

	//create the 1-D kernel
    float sigma = 2.;
    float Z = 0.0;
    for (int j = 0; j <= kSize; ++j)
    {
        kernel[kSize+j] = kernel[kSize-j] = normpdf(float(j), sigma);
    }

    //get the normalization factor (as the gaussian has been clamped)
    for (int j = 0; j < mSize; ++j)
    {
        Z += kernel[j];
    }

    //read out the texels
    for (int i=-kSize; i <= kSize; ++i)
    {
        for (int j=-kSize; j <= kSize; ++j)
        {
            imageblurred += kernel[kSize+j]*kernel[kSize+i]*texture(imageTex, TexCoords + (vec2(float(i),float(j)) / iResolution)).rgb;
        }
    }   

	imageblurred = imageblurred / (Z*Z);  
    
    image = min(image, imageblurred * 1.25); // reduce fireflies 
    //image = max(image, imageblurred * 0.75); // reduce darkflies
	
    // attempt to reduce ghosting
	float weight;
    if(curr_matid == 1) 
        weight = 0.5;//grayscale( pow( clamp( abs(imageacc - image) * 0.05, 0., 1.), vec3(0.35)));
    else if(curr_matid == 2) 
        weight = 0.50;//grayscale( pow( clamp( abs(imageacc - image) * g_2, 0., 1.), vec3(0.35)));//
    else 
        weight = 0.5;//grayscale( pow( clamp( abs(imageacc - image) * 1.0, 0., 1.), vec3(0.35)));//
    
    imageacc = mix(imageacc, image, clamp(weight + 0., 0.05, 1.));

	newTemImageAcc = vec4(imageacc, 1.f);
}