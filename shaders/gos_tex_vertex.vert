//#version 300 es

layout(location = 0) in vec4 pos;
layout(location = 1) in vec4 color;
layout(location = 2) in vec4 fog;
layout(location = 3) in vec2 texcoord;

uniform mat4 mvp;

out vec4 Color;
out float FogValue;
out vec2 Texcoord;

void main(void)
{
    vec4 p = mvp * vec4(pos.xyz, 1);
    gl_Position = p / pos.w;
    Color = color;
	FogValue = fog.w;
    Texcoord = texcoord;
}

