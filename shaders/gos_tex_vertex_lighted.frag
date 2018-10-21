//#version 300 es
// using this because it is required if we want to use "binding" qualifier in layout (can be set in cpp code but it is easier to do in shader, so procedd like this and maybe change later)
//#version 420

#define PREC highp

#include <include/lighting.hglsl>

uniform vec4 light_offset_;

in PREC vec3 Normal;
//in PREC float FogValue;
in PREC vec2 Texcoord;
in PREC vec4 VertexColor;
in PREC vec3 VertexLight;
in PREC vec3 WorldPos;
in PREC vec3 CameraPos;

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

#if ENABLE_VERTEX_LIGHTING
	PREC vec3 lighting = VertexLight;
#else
    const int lights_index = int(light_offset_.x);
    PREC vec3 lighting = calc_light(lights_index, Normal, VertexLight);
#endif

    c.xyz = c.xyz * lighting;

    c.xyz = apply_fog(c.xyz, WorldPos.xyz, CameraPos);

	FragColor = vec4(c.xyz, c.a);
}

