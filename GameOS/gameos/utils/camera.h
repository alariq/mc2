#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <memory.h>
#include "utils/vec.h"

struct camera
{
    camera();

	void set_projection(const mat4& proj);
	void get_projection(mat4* proj) const { *proj = proj_; }
	void update(float dt);
	void get_pos(float (*p)[4] ) const; 
	void set_pos(const vec3& world_pos);
	void get_view_proj_inv(mat4* vpi) const { *vpi = view_proj_inv_; }
	void get_view(mat4* view) const { *view = view_; }
	
	void set_view(const mat4& view_mat);

	static void compose_view_matrix(mat4* view, const vec3& right, vec3& up, vec3& front, vec3 world_pos);
	static void compose_view_matrix(mat4* view, const float (& mat)[3*4]);
	static void view_get_world_pos(const mat4& view, vec3* world_pos);

//private:

    float rot_x;
    float rot_y;
    float dist;
	float dx;
	float dy;
	float dz;
	float move_scale;

	float pos[4];
	float lookat[4];
	float right[4];
	float up[4];

	mat4 proj_;
	mat4 inv_proj_;
	mat4 view_;
	mat4 inv_view_;
	mat4 world_;
	mat4 view_proj_inv_;

	
	
};


#endif // __CAMERA_H__
