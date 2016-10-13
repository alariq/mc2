#pragma once

#include <string>
#include <map>
#include "utils/graphics.h"

typedef GLvoid (APIENTRY *UNIFORM_FUNC)(GLint location, GLsizei count, const void *value);
typedef GLvoid (APIENTRY *UNIFORM_MAT_FUNC)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

enum ConstantType {
    CONSTANT_FLOAT,
    CONSTANT_INT,

    CONSTANT_VEC2,
    CONSTANT_VEC3,
    CONSTANT_VEC4,
    CONSTANT_IVEC2,
    CONSTANT_IVEC3,
    CONSTANT_IVEC4,
    CONSTANT_BOOL,
    CONSTANT_BVEC2,
    CONSTANT_BVEC3,
    CONSTANT_BVEC4,
    CONSTANT_MAT2,
    CONSTANT_MAT3,
    CONSTANT_MAT4,
};

enum SamplerType {
	SAMPLER_1D,
	SAMPLER_2D,
	SAMPLER_3D,
	SAMPLER_CUBE,
	SAMPLER_1D_SHADOW,
	SAMPLER_2D_SHADOW
};

struct glsl_uniform
{
    ConstantType type_;
    int num_el_;
    std::string name_;
    GLint index_;
    unsigned char* data_;
    bool is_dirty_;
};

struct glsl_sampler
{
	SamplerType type_;
	std::string name_;
	GLint index_;
};

struct glsl_shader
{    

    enum Shader_t { VERTEX = 0, FRAGMENT = 1, HULL = 2, DOMAINE = 3, GEOMERTY = 4, NUM_SHADER_TYPES }; // E on purpose

    GLenum type_;
    std::string fname_;
    GLuint shader_;

    static std::map<std::string, glsl_shader*> s_shaders[glsl_shader::NUM_SHADER_TYPES];

    bool reload();
    static glsl_shader* makeShader(Shader_t type, const char* fname);
    static void deleteShader(glsl_shader* psh);

private:
    glsl_shader() {}
    ~glsl_shader();
};

struct glsl_program {

    GLuint shp_;
    glsl_shader* vsh_;
    glsl_shader* fsh_;
	glsl_shader* hsh_;
	glsl_shader* dsh_;
	glsl_shader* gsh_;

    typedef std::map<std::string, glsl_uniform*> UniArr_t;
	typedef std::map<std::string, glsl_sampler*> SamplerArr_t;
    UniArr_t uniforms_;
	SamplerArr_t samplers_;

    bool reload();
    void apply();

    bool setFloat(const char* name, const float v);
    bool setFloat2(const char* name, const float v[2]);
    bool setFloat3(const char* name, const float v[3]);
    bool setFloat4(const char* name, const float v[4]);
	bool setInt(const char* name, const int v);
    bool setInt2(const char* name, const int v[2]);
    bool setInt3(const char* name, const int v[3]);
    bool setInt4(const char* name, const int v[4]);
    bool setMat2(const char* name, const float v[4]);
    bool setMat3(const char* name, const float v[9]);
    bool setMat4(const char* name, const float v[16]);
   
    bool is_valid();

    static glsl_program* makeProgram(const char* name, const char* vsh, const char* fsh);
	static glsl_program* makeProgram2(const char* name, const char* vp, const char* hp, const char* dp, const char* gp, const char* fp, int count = 0, const char** xfb_variables = 0);
	static void deleteProgram(const char* name);
	GLint getAttribLocation(const char* pattrib);

    static std::map<std::string, glsl_program*> s_programs;

	
private:
    glsl_program():shp_(0), vsh_(0), fsh_(0), hsh_(0), dsh_(0), gsh_(0), is_valid_(false) {};
    ~glsl_program();

    static UNIFORM_FUNC uniformFuncs[15];

    bool is_valid_;
};

