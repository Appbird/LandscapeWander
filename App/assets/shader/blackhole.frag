//	Copyright (c) 2008-2023 Ryo Suzuki.
//	Copyright (c) 2016-2023 OpenSiv3D Project.
//	Licensed under the MIT License.

# version 410

//
//	PSInput
//
layout(location = 0) in vec4 Color;
layout(location = 2) in float radius;

//
//	PSOutput
//
layout(location = 0) out vec4 FragColor;

//
//	Constant Buffer
//
layout(std140) uniform PSConstants2D
{
	vec4 g_colorAdd;
	vec4 g_sdfParam;
	vec4 g_sdfOutlineColor;
	vec4 g_sdfShadowColor;
	vec4 g_internal;
};

//
//	Functions
//
void main()
{
    vec4[4] palette = vec4[4](
        vec4(0.1412, 0.0118, 0.0118, 1.0),
        vec4(0.6431, 0.051, 0.051, 1.0),
        vec4(0.8549, 0.5373, 0.0863, 1.0),
        vec4(0.6314, 0.5137, 0.1686, 1.0)
    );
    /**/
    /*
    vec4[4] palette = vec4[4](
        vec4(0.0745, 0.0039, 0.0745, 1.0),
        vec4(0.2314, 0.0235, 0.2314, 1.0),
        vec4(0.4353, 0.0745, 0.6196, 1.0),
        vec4(0.5451, 0.3059, 0.5176, 1.0)
    );
    /**/
    int index = radius > 0.80 ? 1 : 0;
    float p   = index == 0 ? (radius / 0.80)*(radius / 0.80) : (radius - 0.80) / 0.20;
    FragColor = mix(palette[index], palette[index + 1], p);
    float shine = radius > 0.95 ? (radius - 0.95) / 0.05 : 0;
    FragColor += mix(vec4(0, 0, 0, 1.0), palette[3], shine);
}
