//#version 300 es

#define PREC highp

in PREC vec4 Color;
in PREC vec2 Texcoord;
in PREC float FogValue;

layout (location=0) out PREC vec4 FragColor;

#ifdef ENABLE_TEXTURE1
uniform sampler2D tex1;
#endif
uniform sampler2D tex2;
uniform sampler2D tex3;

uniform PREC vec4 fog_color;

void main(void)
{
    PREC vec4 c = Color;
#ifdef ENABLE_TEXTURE1
    c *= texture(tex1, Texcoord);
#endif
	if(fog_color.x>0.0 || fog_color.y>0.0 || fog_color.z>0.0 || fog_color.w>0.0)
		c.rgb = mix(fog_color.rgb, c.rgb, FogValue);
    FragColor = c;
}

