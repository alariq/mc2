#version 420

in vec4 Color;
in float FogValue;
in vec2 Texcoord;

layout (location=0) out vec4 FragColor;

uniform sampler2D tex1;
uniform vec4 fog_color;

void main(void)
{
    vec4 c = Color.bgra;
    c *= texture(tex1, Texcoord);
	if(fog_color!=0)
    	c.rgb = mix(fog_color.rgb, c.rgb, FogValue);
	FragColor = c;
}

