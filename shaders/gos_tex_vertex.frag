//#version 300 es

#define PREC highp

in PREC vec4 Color;
in PREC vec2 Texcoord;
in PREC float FogValue;

layout (location=0) out PREC vec4 FragColor;

uniform sampler2D tex1;
uniform PREC vec4 fog_color;

void main(void)
{
    PREC vec4 c = Color.bgra;
    PREC vec4 tex_color = texture(tex1, Texcoord);
    c *= tex_color;
    
#ifdef ALPHA_TEST
    if(tex_color.a < 0.5)
        discard;
#endif

	if(fog_color.x>0.0 || fog_color.y>0.0 || fog_color.z>0.0 || fog_color.w>0.0)
    	c.rgb = mix(fog_color.rgb, c.rgb, FogValue);
	FragColor = c;
}

