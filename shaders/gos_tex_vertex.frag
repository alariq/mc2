#version 300 es

#define PREC highp

in PREC vec4 Color;
in PREC vec2 Texcoord;

layout (location=0) out PREC vec4 FragColor;

uniform sampler2D tex1;

void main(void)
{
    PREC vec4 c = Color.bgra;
    c *= texture(tex1, Texcoord);
    FragColor = c;//vec4(1,1,1,0.5);
}

