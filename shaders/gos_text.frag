#version 420

in vec4 Color;
in vec2 Texcoord;

layout (location=0) out vec4 FragColor;

uniform sampler2D tex1;
uniform vec4 Foreground;

void main(void)
{
    vec4 c = Foreground;//Color;
    vec4 mask = texture(tex1, Texcoord);
    c *= mask.xxxx;
    FragColor = c;//vec4(1,1,1,0.5);
}

