#include <assert.h>

#include "utils/vec.h"
#include "utils/camera.h"

//#include <GL/glew.h>
//#include <graphics/gl_utils.h>
#include "utils/matrix.h"

camera::camera():rot_x(0), rot_y(0), dist(0)
{
	memset(pos, 0, sizeof(pos));
	pos[2] = -5;

	memset(lookat, 0, sizeof(lookat));
	memset(right, 0, sizeof(right));
	memset(up, 0, sizeof(up));
	lookat[2] = right[0] = up[1] = 1.0f;
	dx = dy = dz = 0;
	move_scale = .1f;

	proj_ = mat4::identity();
	inv_proj_ = mat4::identity();
	view_ = mat4::identity();
	inv_view_ = mat4::identity();
	world_ = mat4::identity();
	view_proj_inv_ = mat4::identity();
}

void camera::get_pos(float (*p)[4] ) const
{
	memcpy(p, pos, sizeof(pos) );
}

void camera::set_projection(const mat4& proj)
{
	proj_ = proj;
	float inv[16];
	glu_InvertMatrixf((const float*)proj_, inv);
	mat4 invProj( 
		inv[0], inv[1], inv[2], inv[3],
		inv[4], inv[5], inv[6], inv[7],	
		inv[8], inv[9], inv[10], inv[11],	
		inv[12], inv[13], inv[14], inv[15]
	);

	inv_proj_ = invProj;
}

void camera::update(float dt)
{
	mat4 rotX = rotateY4(rot_x);
	mat4 rotY = rotateX4(rot_y);

	mat4 matrot = rotY*rotX;

	//vec4 dpos = matrot*vec4(dx, 0, dz, 1);

	vec3 p(pos[0], pos[1], pos[2]);

	
	p += dx*matrot.getRightVec();
	p += dy*matrot.getUpVec();
	p += dz*matrot.getForwardVec();

	pos[0] = p.x;
	pos[1] = p.y;
	pos[2] = p.z;
	pos[3] = 0;

	// build view matrix
	view_ = mat4::identity();
	view_.setRow(0, vec4(matrot.getRightVec(), dot(-p, matrot.getRightVec() )));
	view_.setRow(1, vec4(matrot.getUpVec(), dot(-p, matrot.getUpVec() )));
	view_.setRow(2, vec4(matrot.getForwardVec(), dot(-p,matrot.getForwardVec() )));

	//view_.setRow(0, vec4(matrot.getRightVec(), -p.x));
	//view_.setRow(1, vec4(matrot.getUpVec(), -p.y));
	//view_.setRow(2, vec4(matrot.getForwardVec(), -p.z));

	// build inverted view matrix
	float inv[16];
	glu_InvertMatrixf((const float*)view_, inv);
	mat4 invView( 
		inv[0], inv[1], inv[2], inv[3],
		inv[4], inv[5], inv[6], inv[7],	
		inv[8], inv[9], inv[10], inv[11],	
		inv[12], inv[13], inv[14], inv[15]
	);
	inv_view_ = invView;

	view_proj_inv_ = inv_view_*inv_proj_;

	// test sample point: should be the same as in kernel
#if 0
	vec4 corner_pr(-1.f, -1.f, -1.0f, 1.0);
	vec4 corner_view = inv_proj_*corner_pr;
	vec4 corner_world = inv_view_*corner_view;
	corner_world.w = 0.0f;

	vec4 corner_world2 = inv_proj_view*corner_pr;
	corner_world2.w = 0.0f;

	assert( fabs(corner_world.getX()-corner_world2.getX()) < 0.00001f);
	assert( fabs(corner_world.getY()-corner_world2.getY()) < 0.00001f);
	assert( fabs(corner_world.getZ()-corner_world2.getZ()) < 0.00001f);
	assert( fabs(corner_world.getW()-corner_world2.getW()) < 0.00001f);
#endif
	
	vec3 v = invView.getForwardVec();
	lookat[0] = v.x; lookat[1] = v.y; lookat[2] = v.z; lookat[3] = 0.0f;
	v = invView.getRightVec();
	right[0] = v.x; right[1] = v.y; right[2] = v.z; right[3] = 0.0f;
	v = invView.getUpVec();
	up[0] = v.x; up[1] = v.y; up[2] = v.z; up[3] = 0.0f;
	
	dx = dy = dz = 0;

	
}

void camera::set_view(const mat4& view_mat)
{
	view_ = view_mat;

	vec3 wp;
	camera::view_get_world_pos(view_, &wp);
	
	// build inverted view matrix
	float inv[16];
	glu_InvertMatrixf((const float*)view_, inv);
	mat4 invView( 
		inv[0], inv[1], inv[2], inv[3],
		inv[4], inv[5], inv[6], inv[7],	
		inv[8], inv[9], inv[10], inv[11],	
		inv[12], inv[13], inv[14], inv[15]
	);
	//inv_view_ = invView;
	inv_view_ = view_;
	inv_view_.setElem(3,0,0);
	inv_view_.setElem(3,1,0);
	inv_view_.setElem(3,2,0);
	inv_view_ = transpose(inv_view_);
	inv_view_.setElem(3,0,wp.x);
	inv_view_.setElem(3,1,wp.y);
	inv_view_.setElem(3,2,wp.z);
	

	view_proj_inv_ = inv_view_*inv_proj_;

	vec3 v = invView.getForwardVec();
	lookat[0] = v.x; lookat[1] = v.y; lookat[2] = v.z; lookat[3] = 0.0f;
	v = invView.getRightVec();
	right[0] = v.x; right[1] = v.y; right[2] = v.z; right[3] = 0.0f;
	v = invView.getUpVec();
	up[0] = v.x; up[1] = v.y; up[2] = v.z; up[3] = 0.0f;
	
	dx = dy = dz = 0;
	vec3 tr = inv_view_.getTranslation();
	pos[0] = tr.x;
	pos[1] = tr.y;
	pos[2] = tr.z;
	pos[3] = 0;

}

void camera::compose_view_matrix(mat4* view, const float (& mat)[3*4])
{
	vec3 pos(mat[3], mat[7], mat[11]);

	vec3 right = vec3(mat[0], mat[1], mat[2]);
	vec3 up = vec3(mat[4], mat[5], mat[6]);
	vec3 front = vec3(mat[8], mat[9], mat[10]);

	view->setRow(0, vec4(right, dot(-pos, right)));
	view->setRow(1, vec4(up, dot(-pos, up)));
	view->setRow(2, vec4(front, dot(-pos, front)));
}

void camera::compose_view_matrix(mat4* view, const vec3& right, vec3& up, vec3& front, vec3 world_pos)
{
	view->setRow(0, vec4(right, dot(-world_pos, right)));
	view->setRow(1, vec4(up, dot(-world_pos, up)));
	view->setRow(2, vec4(front, dot(-world_pos, front)));
}

void camera::view_get_world_pos(const mat4& view, vec3* world_pos)
{
	vec3 view_p = view.getTranslation();
	vec3 rxuxfx = view.getCol0().getXYZ(); 
	vec3 ryuyfy = view.getCol1().getXYZ();
	vec3 rzuzfz = view.getCol2().getXYZ();

	world_pos->x = -dot(view_p, rxuxfx);
	world_pos->y = -dot(view_p, ryuyfy);
	world_pos->z = -dot(view_p, rzuzfz);
}

void camera::set_pos(const vec3& world_pos)
{
	vec3 right = view_.getRow(0).getXYZ();
	vec3 up = view_.getRow(1).getXYZ();
	vec3 front = view_.getRow(2).getXYZ();

	view_.setElem(3, 0, dot(-world_pos, right));
	view_.setElem(3, 1, dot(-world_pos, up));
	view_.setElem(3, 2, dot(-world_pos, front));

	pos[0] = world_pos.x;
	pos[1] = world_pos.y;
	pos[2] = world_pos.z;
}
