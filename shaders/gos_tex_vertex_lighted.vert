//#version 300 es
// using this because it is required if we want to use "binding" qualifier in layout (can be set in cpp code but it is easier to do in shader, so procedd like this and maybe change later)
#version 420

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 aRGBLight;
layout(location = 3) in vec2 texcoord;

layout (binding = 0, std140) uniform lights_data
{ 
  vec4 camera_position;
  vec4 light_position;
  vec4 light_diffuse;
};

layout (binding = 1, std140) uniform mesh_data
{ 
  vec4 ambient;
  vec4 diffuse;
};


#define MAX_LIGHTS_IN_WORLD 16

layout (binding = 0, std140) uniform lights_data_arrays
{
    mat4 light_to_shape[MAX_LIGHTS_IN_WORLD];
    vec4 light_dir[MAX_LIGHTS_IN_WORLD]; // w - light type
    vec4 root_light_dir[MAX_LIGHTS_IN_WORLD];
    vec4 spot_light_dir[MAX_LIGHTS_IN_WORLD];
};


uniform mat4 mvp_; // TODO: remove, use wvp_ instead

uniform mat4 wvp_;
uniform mat4 world_;
uniform mat4 projection_;
uniform vec4 vp; //viewport
uniform float forceZ;

out vec3 Normal;
out float FogValue;
out vec2 Texcoord;
out vec4 Light;


void main(void)
{
    vec4 p = wvp * vec4(pos.xyz, 1);
	float rhw = 1 / p.w;

	p.x = (p.x * rhw) * vp.z + vp.x + 100.0;
	p.y = (p.y * rhw) * vp.w + vp.y;
	p.z = p.z * rhw;
	p.w = abs(rhw);

	vec4 p2 = projection_ * vec4(p.xyz,1);

    gl_Position = p2 / p.w;
    Normal = normal;
	//FogValue = fog.w;
    Texcoord = texcoord;
	Light = aRGBLight;
}

