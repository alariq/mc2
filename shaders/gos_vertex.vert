#version 420

layout(location = 0) in vec4 pos;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 texcoord;

out vec4 Color;
out vec2 Texcoord;

void main(void)
{
    vec2 p = pos.xy / vec2(800, 600);
    p.y = 1.0 - p.y;
    p = 2.0*p - 1.0;
	gl_Position 	= vec4(p.xy, pos.z, 1.0);

    Color = color;
    Texcoord = texcoord;
}

