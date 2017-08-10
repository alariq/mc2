#version 300 es
#define PREC highp
#
layout (location=0) out PREC vec4 FragColor;

in PREC vec2 tc;
uniform sampler2D tex;


void main(void)
{
	PREC vec3 c = texture2D(tex, tc).rgb;
	FragColor = vec4(c + vec3(1.0,1.0,1.0), 1.0);
}
