//#version 300 es
// using this because it is required if we want to use "binding" qualifier in layout (can be set in cpp code but it is easier to do in shader, so procedd like this and maybe change later)
//#version 420

#define PREC highp
// should be in sync with C++ code
#define     LIGHT_DATA_ATTACHMENT_SLOT      0
#
#define     MAX_LIGHTS_IN_WORLD             16

//layout (binding = LIGHT_DATA_ATTACHMENT_SLOT, std140) uniform LightsData
//{
//    mat4 light_to_world[MAX_LIGHTS_IN_WORLD];
//    vec4 light_dir[MAX_LIGHTS_IN_WORLD]; // w - light type
//    vec4 light_color[MAX_LIGHTS_IN_WORLD];
//} lights_data;


struct ObjectLights {
    mat4 light_to_world[MAX_LIGHTS_IN_WORLD];
    vec4 light_dir[MAX_LIGHTS_IN_WORLD]; // w - light type
    vec4 light_color[MAX_LIGHTS_IN_WORLD];
    ivec4 numLights;
};

layout (binding = LIGHT_DATA_ATTACHMENT_SLOT, std140) uniform LightsData
{
    ObjectLights light[128];
};

uniform vec4 light_offset_;

in PREC vec3 Normal;
//in PREC float FogValue;
in PREC vec2 Texcoord;
in PREC vec4 VertexLight;

layout (location=0) out PREC vec4 FragColor;

uniform sampler2D tex1;
uniform PREC vec4 fog_color;

void main(void)
{
    PREC vec4 c = vec4(1,1,1,1);//Color.bgra;
    PREC vec4 tex_color = texture(tex1, Texcoord);
    c *= tex_color;
    
#ifdef ALPHA_TEST
    if(tex_color.a == 0.5)
        discard;
#endif

	//if(fog_color.x>0.0 || fog_color.y>0.0 || fog_color.z>0.0 || fog_color.w>0.0)
    //	c.rgb = mix(fog_color.rgb, c.rgb, FogValue);

    const int lights_index = int(light_offset_.x);

#if ENABLE_VERTEX_LIGHTING
    FragColor = c*VertexLight;
#else
    ObjectLights ld = light[lights_index];
    float n_dot_l = clamp(dot(normalize(Normal), -ld.light_dir[0].xyz), 0.0, 1.0);
    vec4 diffuse = c * ld.light_color[0];
    vec4 ambient = c * ld.light_color[1];//ld.light_color[1];
	//FragColor = diffuse * n_dot_l + ambient;
    // this is how MC2 treats ambient :-/
	FragColor = diffuse * clamp(n_dot_l + ld.light_color[1], 0.0, 1.0);
#endif
}

