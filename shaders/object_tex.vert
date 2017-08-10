#version 300 es


layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 texcoord;

uniform mat4 ModelViewProjectionMatrix;

out vec2 tc;

void main(void)
{
	gl_Position     = ModelViewProjectionMatrix*vec4(pos, 1);
	//tc = gl_MultiTexCoord0.xy;
	tc = texcoord;
}

