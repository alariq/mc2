#version 420

in vec4 Color;
in vec2 Texcoord;

layout (location=0) out vec4 FragColor;

uniform sampler2D tex1;

void main(void)
{
    vec4 c = Color;
    c *= texture(tex1, Texcoord);
    FragColor = c;//vec4(1,1,1,0.5);
}

