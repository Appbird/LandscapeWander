//	Copyright (c) 2008-2023 Ryo Suzuki.
//	Copyright (c) 2016-2023 OpenSiv3D Project.
//	Licensed under the MIT License.

# version 410
#define M_PI 3.1415926535897932384626433832795

//
//	VS Input (nothing)
//

//
//	VS Output
//
layout(location = 0) out vec4 Color;
layout(location = 1) out vec2 UV;
layout(location = 2) out float radius;

out gl_PerVertex
{
	vec4 gl_Position;
};

//
//	Siv3D Functions
//
vec4 s3d_Transform2D(const vec2 pos, const vec4 t[2])
{
	return vec4(t[0].zw + (pos.x * t[0].xy) + (pos.y * t[1].xy), t[1].zw);
}

//
//	Constant Buffer
//
layout(std140) uniform VSConstants2D
{
	vec4 g_transform[2];
	vec4 g_colorMul;
};

layout(std140) uniform SoftShape
{
	float g_t;
};

//
//	Functions
//
vec2 Circular(float r, float theta)
{
	return vec2(sin(theta), -cos(theta)) * r;
}

void main()
{
	Color = vec4(0, 0, 0, 0);
	UV = vec2(0, 0);

	vec2 pos;

	if (gl_VertexID % 3 == 0)
	{
		pos = vec2(0, 0);
        radius = 0;
	}
	else
	{
		float angle = radians((gl_VertexID / 3) + ((gl_VertexID % 3) - 1));
		float r = 0.80
			+ cos((angle * 5.0) + sin(2 * M_PI / 0.6 * g_t)) * 0.01
			+ sin((angle * 4.0) + sin(2 * M_PI / 2.0 * g_t)) * 0.04
			+ cos((angle * 3.0) + sin(2 * M_PI / 1.5 * g_t)) * 0.05
			+ sin((angle * 2.0) + sin(2 * M_PI / 1.2 * g_t)) * 0.10;
		pos = vec2(0, 0) + Circular(r, angle);
        radius = 1;
	}

	gl_Position = s3d_Transform2D(pos, g_transform);	
}
