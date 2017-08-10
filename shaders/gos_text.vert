#version 300 es

layout(location = 0) in vec4 pos;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 texcoord;

uniform mat4 mvp;
out vec4 Color;
out vec2 Texcoord;

void main(void)
{
    gl_Position = mvp * vec4(pos.xyz, 1);
    Color = color;
    Texcoord = texcoord;
}

