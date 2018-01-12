#version 300 es

#define PREC highp

in PREC vec4 Color;
//in PREC float FogValue;
in PREC vec2 Texcoord;
in PREC vec4 Light;

layout (location=0) out PREC vec4 FragColor;

uniform sampler2D tex1;
uniform PREC vec4 fog_color;

void main(void)
{
    PREC vec4 c = vec4(1,1,1,1);//Color.bgra;
    c *= texture(tex1, Texcoord);
	//if(fog_color.x>0.0 || fog_color.y>0.0 || fog_color.z>0.0 || fog_color.w>0.0)
    //	c.rgb = mix(fog_color.rgb, c.rgb, FogValue);
	FragColor = c;// * Light;
}

