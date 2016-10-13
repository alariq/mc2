#ifndef __GL_UTILS_H__
#define __GL_UTILS_H__

#include <cassert>
#include <cstdio>
#include "utils/camera.h"
#include "utils/shader_builder.h"
#include "utils/render_constants.h"

#define BUFFER_OFFSET(bytes) ((GLubyte*) NULL + (bytes))

uint32_t vec4_to_uint32(const vec4& v);

struct Texture {
	Texture():id(0), w(0), h(0), depth(1), fmt_(TF_NONE) {}

	GLuint id;
	GLenum format;
	int w, h, depth;
    TexFormat fmt_;
};

uint32_t getTexFormatPixelSize(TexFormat fmt);

#define CHECK_GL_ERROR \
{ \
   	GLenum err = glGetError();\
	if(err != GL_NO_ERROR) \
	{ \
		printf("OpenGL Error: %s\n", ogl_get_error_code_str(err)); \
		printf("Location : %s : %d\n", __FILE__ , __LINE__); \
	}\
}

template<typename T>
const char* ogl_get_error_code_str(T input)
{
    int errorCode = (int)input;
    switch(errorCode)
    {
	case GL_NO_ERROR:
            return "GL_NO_ERROR";
        case GL_INVALID_ENUM:
            return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE:
            return "GL_INVALID_VALUE";               
        case GL_INVALID_OPERATION:
            return "GL_INVALID_OPERATION";           
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "GL_INVALID_FRAMEBUFFER_OPERATION";      
        case GL_OUT_OF_MEMORY:
            return "GL_OUT_OF_MEMORY";                    
        case GL_STACK_OVERFLOW:
            return "GL_STACK_OVERFLOW";                 
        case GL_STACK_UNDERFLOW:
            return "GL_STACK_UNDERFLOW";        
        default:
            return "unknown error code";
    }
}


template<typename T>
static int ogl_check_val(T input, T reference, const char* message)
{
    if(input==reference)
    {
        return true;
    }
    else
    {
	printf("OpenGL Error: %s Error code: %s\n", message, ogl_get_error_code_str(input));
        return false;
    }
}

Texture createDynamicTexture(int w, int h, TexFormat fmt);
Texture createDynamicTexture(int w, int h, GLenum fmt = GL_RED);
Texture create3DTextureF(int w, int h, int depth);
void updateTexture(const Texture& t , char* pdata);
Texture createPBO(int w, int h, GLenum fmt, int el_size);
void draw_quad(float x0, float y0, float x1, float y1);

struct glsl_program;
void applyTexture(glsl_program* program, int unit, const char* name, GLuint texid);
void applyPBO(glsl_program* program, int unit, const char* name, const Texture pbo, const Texture tex);

void normalize(float (&v)[3]);
void cross(float v1[3], float v2[3], float result[3]);
float dot(float (&v1)[3], float (&v2)[3]);

int glu_InvertMatrixf(const float m[16], float invOut[16]);
void glu_MakeIdentityf(GLfloat m[16]);
void glu_LookAt2(GLdouble eyex, GLdouble eyey, GLdouble eyez, GLdouble centerx,
          GLdouble centery, GLdouble centerz, GLdouble upx, GLdouble upy,
          GLdouble upz);


typedef void (*render_func_t)(int w, int h, void* puserdata);

void draw_in_2d(int w, int h, render_func_t prenderfunc, void* puserdata);

GLuint makeBuffer(GLenum target, const GLvoid* buffer_data, GLsizei buffer_size, GLenum type = GL_STATIC_DRAW);
void updateBuffer(GLuint buf, GLenum target, const GLvoid* buffer_data, GLsizei buffer_size);
void updateBuffer(GLuint buf, GLenum target, const GLvoid* buffer_data, GLsizei buffer_size, GLenum type);

// BUFFERS ETC.
template <typename VERTEX> 
struct glMesh {
	GLuint	    vb_;
	GLuint	    ib_;
	GLuint		instance_vb_;
	VERTEX*	    pvertices_;
	GLsizei     num_vertices_;
	int*	    pindices_;
	int	    num_indices_;

	uint8_t*	pinstance_data_;
	int		instance_count_;
	int		instance_stride_;

	GLenum	    prim_type_;

				    glMesh();
				    ~glMesh();

	static glMesh<VERTEX>*	    makeMesh(int num_vertices, int num_indices, GLenum prim_type, int instance_array_size = 0, int instance_stride = 0);
	static void				    destroyMesh(glMesh<VERTEX>* pmesh);

	int			    gen_hw(GLenum type = GL_STREAM_DRAW);
	void			    update_hw(GLvoid* vertex_data, GLsizeiptr vsize, GLvoid* index_data, GLsizeiptr isize, GLvoid* instance_data = 0 , GLsizeiptr inst_size = 0);
	void			    update_hw(GLsizeiptr num_cvertices, GLsizeiptr num_indices, GLsizeiptr num_instances);

	typedef glMesh<VERTEX>	    myType;

};

template <typename VERTEX>
glMesh<VERTEX>::glMesh():
vb_(0),ib_(0),instance_vb_(0), pvertices_(0),num_vertices_(0), pindices_(0), num_indices_(0), 
pinstance_data_(0), instance_count_(0), instance_stride_(0), prim_type_(0)
{
}

template <typename VERTEX>
glMesh<VERTEX>* glMesh<VERTEX>::makeMesh(int num_vertices, int num_indices, GLenum prim_type, int instance_count/* = 0*/, int instance_stride/* = 0*/)
{
	glMesh* mesh = new glMesh();
	mesh->num_vertices_ = num_vertices;
	mesh->num_indices_ = num_indices;
	if(num_vertices>0)
	    mesh->pvertices_ = new VERTEX[mesh->num_vertices_];
	if(num_indices>0)
	    mesh->pindices_ = new int[mesh->num_indices_];
	mesh->prim_type_ = prim_type;

	assert(instance_count >= 0);
	if(instance_count > 0)
	{
		assert(instance_stride!=0);

		mesh->instance_count_ = instance_count;
		mesh->instance_stride_ = instance_stride;
		mesh->pinstance_data_ = new unsigned char [mesh->instance_count_ * mesh->instance_stride_];
	}

	return mesh;
}

template <typename VERTEX>
void glMesh<VERTEX>::destroyMesh(glMesh<VERTEX>* pmesh)
{
	assert(pmesh);
	delete pmesh;
}

template <typename VERTEX>
int glMesh<VERTEX>::gen_hw(GLenum type /*= GL_STATIC_DRAW*/)
{
	if(this->pvertices_)
	{
		if(this->vb_ > 0)
			glDeleteBuffers(1, &this->vb_);
		this->vb_ = makeBuffer(GL_ARRAY_BUFFER, this->pvertices_, sizeof(VERTEX)*this->num_vertices_, type);
	}
	if(this->pindices_)
	{
		if(this->ib_ > 0)
			glDeleteBuffers(1, &this->ib_);
		this->ib_ = makeBuffer(GL_ELEMENT_ARRAY_BUFFER, this->pindices_, sizeof(int)*this->num_indices_, type);
	}

	if(this->pinstance_data_)
	{
		if(this->instance_vb_ > 0)
			glDeleteBuffers(1, &this->instance_vb_);
		this->instance_vb_ = makeBuffer(GL_ARRAY_BUFFER, this->pinstance_data_, this->instance_count_ * this->instance_stride_, type);
	}

	return 1;
};

// strange function, ..heh, ..why do I need this?
template <typename VERTEX>
void glMesh<VERTEX>::update_hw(GLvoid* vertex_data, GLsizeiptr vsize, GLvoid* index_data, GLsizeiptr isize, GLvoid* instance_data/* = 0*/, GLsizeiptr inst_count/* = 0*/)
{
	if(this->vb_!=0 && vertex_data && vsize>0)
		updateBuffer(this->vb_, GL_ARRAY_BUFFER, vertex_data, vsize, GL_DYNAMIC_DRAW);

	if(this->ib_!=0 && index_data && isize>0)
		updateBuffer(this->ib_, GL_ELEMENT_ARRAY_BUFFER, index_data, isize, GL_DYNAMIC_DRAW);

	if(this->instance_vb_!=0 && instance_data && isize>0)
	{
		assert(inst_count >= 0);
		updateBuffer(this->instance_vb_, GL_ARRAY_BUFFER, instance_data, inst_count * this->instance_stride_, GL_DYNAMIC_DRAW);
	}
}

template <typename VERTEX>
void glMesh<VERTEX>::update_hw(GLsizeiptr num_vertices, GLsizeiptr num_indices, GLsizeiptr num_instances)
{
	if(this->vb_!=0 && num_vertices>=0)
	{
		GLsizeiptr vsize = (num_vertices!=0 ? num_vertices : num_vertices_) * sizeof(VERTEX); 
		updateBuffer(this->vb_, GL_ARRAY_BUFFER, pvertices_, vsize, GL_DYNAMIC_DRAW);
	}
	if(this->ib_!=0 && num_indices>=0)
	{
		GLsizeiptr isize = (num_indices!=0 ?  num_vertices : num_indices_) * sizeof(int);
		updateBuffer(this->ib_, GL_ELEMENT_ARRAY_BUFFER, pindices_, isize, GL_DYNAMIC_DRAW);
	}
	if(this->instance_vb_!=0 && num_instances>=0)
	{
		GLsizeiptr instsize = (num_instances!=0 ?  num_instances : instance_count_) * instance_stride_;
		updateBuffer(this->instance_vb_, GL_ARRAY_BUFFER, pinstance_data_, instsize, GL_DYNAMIC_DRAW);
	}
}

template <typename VERTEX>
glMesh<VERTEX>::~glMesh()
{
	delete[] pvertices_;
	delete[] pindices_;
	delete[] pinstance_data_;
    	if(this->vb_ > 0)
		glDeleteBuffers(1, &this->vb_);
	if(this->ib_ > 0)
		glDeleteBuffers(1, &this->ib_);
}

void draw_textured_cube(GLuint textureId);

Texture load_texture_from_file(const char* texName);

struct SIMPLE_VERTEX_PTN;
glMesh<SIMPLE_VERTEX_PTN>* make_mesh_from_file(const char* filepath);

template<typename T>
void  draw_mesh_indexed_pt(bool b_wireframe, camera* pcam, glsl_program* pmat, glMesh<T>* pmesh, int num_indices=0)
{
	GLint posaddr = pmat->getAttribLocation("pos");
	assert(posaddr!=-1);
	GLint tcaddr = pmat->getAttribLocation("texcoord");
	assert(tcaddr!=-1);

	if(pcam)
	{
		mat4 proj, view;
		pcam->get_projection(&proj);
		pcam->get_view(&view);
		mat4 viewproj = proj*view;
		pmat->setMat4("ModelViewProjectionMatrix", (const float*)viewproj);
		pmat->apply();
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, pmesh->vb_);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pmesh->ib_);

	glEnableVertexAttribArray(posaddr);
	glEnableVertexAttribArray(tcaddr);

	glVertexAttribPointer(posaddr, 3, GL_FLOAT, GL_FALSE, sizeof(T), (void*)0);
	glVertexAttribPointer(tcaddr, 2, GL_FLOAT, GL_FALSE, sizeof(T), BUFFER_OFFSET(3*sizeof(float)));

	if(b_wireframe)
	    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	int idx2draw = num_indices!=0 ? num_indices : pmesh->num_indices_;
	glDrawElements(pmesh->prim_type_, idx2draw , GL_UNSIGNED_INT, 0);
	if(b_wireframe)
	    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glDisableVertexAttribArray(posaddr);
	glDisableVertexAttribArray(tcaddr);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


template<typename T>
void  draw_mesh_indexed_pn(bool b_wireframe, camera* pcam, glsl_program* pmat, glMesh<T>* pmesh)
{
	GLint posaddr = pmat->getAttribLocation("pos");
	assert(posaddr!=-1);
	GLint normaddr = pmat->getAttribLocation("norm");
	assert(normaddr!=-1);

	mat4 proj, view;
	pcam->get_projection(&proj);
	pcam->get_view(&view);
	mat4 viewproj = proj*view;

	pmat->setMat4("ModelViewProjectionMatrix", (const float*)viewproj);
	pmat->apply();
	
	glBindBuffer(GL_ARRAY_BUFFER, pmesh->vb_);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pmesh->ib_);

	glEnableVertexAttribArray(posaddr);
	glEnableVertexAttribArray(normaddr);

	glVertexAttribPointer(posaddr, 3, GL_FLOAT, GL_FALSE, sizeof(T), (void*)0);
	glVertexAttribPointer(normaddr, 3, GL_FLOAT, GL_FALSE, sizeof(T), BUFFER_OFFSET(3*sizeof(float)));

	if(b_wireframe)
	    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawElements(pmesh->prim_type_, pmesh->num_indices_, GL_UNSIGNED_INT, 0);
	if(b_wireframe)
	    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glDisableVertexAttribArray(posaddr);
	glDisableVertexAttribArray(normaddr);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glUseProgram(0);
}

template<typename T>
void  draw_mesh_ptn(bool b_wireframe, camera* pcam, glsl_program* pmat, glMesh<T>* pmesh)
{
	GLint posaddr = pmat->getAttribLocation("pos");
	assert(posaddr!=-1);
	GLint normaddr = pmat->getAttribLocation("norm");
	//assert(normaddr!=-1);
	GLint tcaddr = pmat->getAttribLocation("texcoord");
	//assert(tcaddr!=-1);
	GLint instaddr = pmat->getAttribLocation("inst_data");

	mat4 proj, view;
	pcam->get_projection(&proj);
	pcam->get_view(&view);
	mat4 viewproj = proj*view;

	pmat->setMat4("ModelViewProjectionMatrix", (const float*)viewproj);
	pmat->apply();
	
	glBindBuffer(GL_ARRAY_BUFFER, pmesh->vb_);

	glEnableVertexAttribArray(posaddr);
	glVertexAttribPointer(posaddr, 3, GL_FLOAT, GL_FALSE, sizeof(T), (void*)0);
			
	if(tcaddr) {
		glEnableVertexAttribArray(tcaddr);
		glVertexAttribPointer(tcaddr, 2, GL_FLOAT, GL_FALSE, sizeof(T), BUFFER_OFFSET(3*sizeof(float)));
	}

	if(normaddr!=-1) {
		glEnableVertexAttribArray(normaddr);
		glVertexAttribPointer(normaddr, 3, GL_FLOAT, GL_FALSE, sizeof(T), BUFFER_OFFSET(5*sizeof(float)));
	}

	bool draw_instanced = false;
	if(instaddr!=-1 && pmesh->instance_vb_!=0)
	{
		draw_instanced = true;
		glBindBuffer(GL_ARRAY_BUFFER, pmesh->instance_vb_);
		glEnableVertexAttribArray(instaddr);
		glVertexAttribPointer(instaddr, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
		glVertexAttribDivisor(instaddr, 1);
	}
	
	if(b_wireframe)
	    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if(!draw_instanced)
		glDrawArrays(pmesh->prim_type_, 0, pmesh->num_vertices_);
	else
		glDrawArraysInstanced(pmesh->prim_type_, 0, pmesh->num_vertices_, pmesh->instance_count_);
	if(b_wireframe)
	    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glDisableVertexAttribArray(posaddr);
	glDisableVertexAttribArray(normaddr);
	glDisableVertexAttribArray(tcaddr);

	if(draw_instanced)
	{
		glDisableVertexAttribArray(instaddr);
		glVertexAttribDivisor(instaddr, 0);
	}


	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glUseProgram(0);
}

#endif // __GL_UTILS_H__
