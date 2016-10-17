#version 420

in vec2 tc;
uniform sampler2D tex;

void main(void)
{
	vec3 c = texture2D(tex, tc).rgb;
	gl_FragColor = vec4(c + vec3(1.0,1.0,1.0), 1.0);
}
