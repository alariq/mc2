//#version 300 es
// using this because it is required if we want to use "binding" qualifier in layout (can be set in cpp code but it is easier to do in shader, so procedd like this and maybe change later)
#version 420

#define PREC highp
// should be in sync with C++ code
#define     LIGHT_DATA_ATTACHMENT_SLOT      0
#
#define     MAX_LIGHTS_IN_WORLD             16
layout (binding = LIGHT_DATA_ATTACHMENT_SLOT, std140) uniform LightsData
{
    mat4 light_to_world[MAX_LIGHTS_IN_WORLD];
    vec4 light_dir[MAX_LIGHTS_IN_WORLD]; // w - light type
    vec4 light_color[MAX_LIGHTS_IN_WORLD];
} lights_data;

in PREC vec3 Normal;
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

    float n_dot_l = clamp(dot(normalize(Normal), -lights_data.light_dir[0].xyz), 0.0, 1.0);
    vec4 diffuse = c * lights_data.light_color[0];
    vec4 ambient = lights_data.light_color[1];
	FragColor = diffuse * n_dot_l + ambient;
}

