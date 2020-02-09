//#version 300 es
// using this because it is required if we want to use "binding" qualifier in layout (can be set in cpp code but it is easier to do in shader, so procedd like this and maybe change later)
//#version 420

#include <include/lighting.hglsl>

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 aRGBLight;
layout(location = 3) in vec2 texcoord;

layout (binding = 1, std140) uniform mesh_data
{ 
  vec4 ambient;
  vec4 diffuse;
};

uniform vec4 light_offset_;

uniform mat4 mvp_; // TODO: remove, use wvp_ instead

uniform mat4 world_;
uniform mat4 view_;
uniform mat4 wvp_;

// common parameters
uniform mat4 projection_;
uniform vec4 vp; //viewport

uniform float forceZ; // baked in a wvp matrix

out vec3 Normal;
out float FogValue;
out vec2 Texcoord;
out vec4 VertexColor;
out vec3 VertexLight;
out vec3 WorldPos;
out vec3 CameraPos;

void main(void)
{
    vec4 p = wvp_ * vec4(pos.xyz, 1);
    float rhw = 1 / p.w;

    p.x = (p.x * rhw) * vp.z + vp.x + 100.0;
    p.y = (p.y * rhw) * vp.w + vp.y;
    p.z = p.z * rhw;
    p.w = abs(rhw);

    WorldPos = (world_ * vec4(pos.xyz, 1.0)).xyz;

    // something is wrong with this: check later
    //CameraPos = (inverse(view_) * vec4(0,0,0,1)).xyz;

    vec4 p2 = projection_ * vec4(p.xyz,1);

    //mat4 norm_view_mat = inverse(view_);

    gl_Position = p2 / p.w;
    //Normal = ((world_ * view_) * vec4(normal, 0)).xyz;
    Normal = (world_ * vec4(normal, 0)).xyz;
    //FogValue = fog.w;
    Texcoord = texcoord;

    // base light can be only calculated in VS because relies on exact vertex colors
    // TODO: pass correct parameters here
    vec3 base_light = get_base_light(aRGBLight.bgra, false, 0.0, false, false,
            vec3(0.0), vec3(0.0), vec3(0.0));

#if ENABLE_VERTEX_LIGHTING
    const int lights_index = int(light_offset_.x);
    VertexLight = calc_light(lights_index, Normal, base_light);
#else
    VertexLight = base_light;
#endif

    VertexColor = aRGBLight.bgra;
}

