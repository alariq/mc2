#version 420

in vec4 Color;
in vec2 Texcoord;

layout (location=0) out vec4 FragColor;

#ifdef ENABLE_TEXTURE1
uniform sampler2D tex1;
#endif
uniform sampler2D tex2;
uniform sampler2D tex3;

void main(void)
{
    vec4 c = Color;
#ifdef ENABLE_TEXTURE1
    c *= texture(tex1, Texcoord);
#endif
    FragColor = c;//vec4(1,1,1,0.5);
}

