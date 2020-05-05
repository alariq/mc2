#ifdef PLATFORM_WINDOWS
#include <windows.h>
#endif
#include <GL/glew.h>
#include <assert.h>
#include <stdio.h>
#include <cstring>
#include <string>

#include "utils/stream.h"
#include "utils/logging.h"
#include "utils/gl_utils.h"
#include "utils/shader_builder.h"
#include "utils/timing.h"
#include "utils/file_utils.h"


std::map<std::string, glsl_shader*> glsl_shader::s_shaders[glsl_shader::NUM_SHADER_TYPES];

std::map<std::string, glsl_program*> glsl_program::s_programs;
UNIFORM_FUNC glsl_program::uniformFuncs[15] = {0};

const int constantSizes[] = {
    sizeof(float),
    sizeof(int),
    2*sizeof(float),
    3*sizeof(float),
    4*sizeof(float),
    sizeof(int) * 2,
    sizeof(int) * 3,
    sizeof(int) * 4,
    sizeof(int),
    sizeof(int) * 2,
    sizeof(int) * 3,
    sizeof(int) * 4,
    4*sizeof(float),
    9*sizeof(float),
    16*sizeof(float),
};

void init_func_ptrs(UNIFORM_FUNC (&uniformFuncs)[15])
{
	// changed fromARB variants, to work with CORE profile as well (because *ARB variants are not initialized in case of CORE profile)
    uniformFuncs[CONSTANT_FLOAT] = (UNIFORM_FUNC) glUniform1fv;
    uniformFuncs[CONSTANT_VEC2]  = (UNIFORM_FUNC) glUniform2fv;
    uniformFuncs[CONSTANT_VEC3]  = (UNIFORM_FUNC) glUniform3fv;
    uniformFuncs[CONSTANT_VEC4]  = (UNIFORM_FUNC) glUniform4fv;
    uniformFuncs[CONSTANT_INT]   = (UNIFORM_FUNC) glUniform1iv;
    uniformFuncs[CONSTANT_IVEC2] = (UNIFORM_FUNC) glUniform2iv;
    uniformFuncs[CONSTANT_IVEC3] = (UNIFORM_FUNC) glUniform3iv;
    uniformFuncs[CONSTANT_IVEC4] = (UNIFORM_FUNC) glUniform4iv;
    uniformFuncs[CONSTANT_BOOL]  = (UNIFORM_FUNC) glUniform1iv;
    uniformFuncs[CONSTANT_BVEC2] = (UNIFORM_FUNC) glUniform2iv;
    uniformFuncs[CONSTANT_BVEC3] = (UNIFORM_FUNC) glUniform3iv;
    uniformFuncs[CONSTANT_BVEC4] = (UNIFORM_FUNC) glUniform4iv;
    uniformFuncs[CONSTANT_MAT2]  = (UNIFORM_FUNC) glUniformMatrix2fv;
    uniformFuncs[CONSTANT_MAT3]  = (UNIFORM_FUNC) glUniformMatrix3fv;
    uniformFuncs[CONSTANT_MAT4]  = (UNIFORM_FUNC) glUniformMatrix4fv;
}

// true - error, false - no error
bool get_shader_error_status(GLuint shader, GLenum status_type)
{
    int status;
    glGetShaderiv(shader, status_type, &status);

    if(!status)
    {
        char* buf = 0;
        GLsizei len = 0, len2= 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
        buf = new char[len];

        glGetShaderInfoLog(shader, len, &len2, buf);
        if(len2!=0)
            log_error("CompileShader: %s\n", buf);
		delete[] buf;

        return true;
    }

    return false;
}

bool get_program_error_status(GLuint program, GLenum status_type)
{
    int status;
    glGetProgramiv(program, status_type, &status);

    if(!status)
    {
        char* buf = 0;
        GLsizei len = 0, len2= 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
        buf = new char[len];

        glGetProgramInfoLog(program, len, &len2, buf);
        if(len2!=0)
            log_error("CompileShader: %s\n", buf);

		delete[] buf;
        return true;
    }

    return false;
}

GLenum get_gl_shader_type(glsl_shader::Shader_t type)
{
	static const  GLenum types[] =  {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER,  GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER, GL_GEOMETRY_SHADER };
	assert(type >=0  && type < sizeof(types)/sizeof(types[0]) );

	return types[type];
}

glsl_shader::Shader_t get_shader_type(GLenum type)
{
	switch(type)
	{
		case GL_VERTEX_SHADER:
			return glsl_shader::VERTEX;
		case GL_FRAGMENT_SHADER:
			return glsl_shader::FRAGMENT;
		case GL_TESS_CONTROL_SHADER:
			return glsl_shader::HULL;
		case GL_TESS_EVALUATION_SHADER:
			return glsl_shader::DOMAINE;
		case GL_GEOMETRY_SHADER:
			return glsl_shader::GEOMERTY;
		default:
			assert(0 && "This shader type is not supported yet!");
			return glsl_shader::NUM_SHADER_TYPES;
	}
}



const char* glsl_load(const char* fname, size_t* out_size = nullptr)
{
    assert(fname);
    stream* pstream = stream::makeFileStream();
    if(0 != pstream->open(fname,"rb"))
    {
        log_error("Can't open %s \n", fname);
        delete pstream;
        return 0;
    }

    pstream->seek(0, stream::S_END);
    size_t size = pstream->tell();
    pstream->seek(0, stream::S_SET);

    char* pdata = new char[size + 1];
    size_t rv = pstream->read(pdata, 1, size);
    assert(rv==size);
    pdata[size] = '\0';
    if(out_size)
        *out_size = size;

    pstream->close();
    delete pstream;

    return pdata;
}

bool parse_include(const char* str, const char*& include, size_t& size, const char*& ieol)
{
    const char* begin = strchr(str, '<');
    const char* end = strchr(str, '>');
    const char* eol = strchr(str, '\n');
    ieol = eol+1; // skip \n

    if(!begin || !end || (eol && end > eol) || end - begin <= 1)
        return false;

    begin++;

    while(*begin==' ')
        begin++;

    while(*(end-1)==' ')
        end--;

    include = begin;
    size = end - begin;
    return true;
}

bool load_shader(const char* fname, std::string& shader_source, std::vector<std::string>& includes);

size_t get_num_lines(const char* text)
{
    if(!text)
        return 0;

    size_t count = 1;
    const char* token = text;
    while(token && (token = strchr(token, '\n')))
    {
        token++;
        count++;
    }

    return count;
}

void append_line_directive(std::string& code, size_t line, const char* fname)
{
    char buf[512];
    snprintf(buf, sizeof(buf), "#line %zu // %s\n", line, fname);
    buf[510] = '\n'; // just in case our snprintf'ed line will be more than 512
    code.append(buf);
}

bool parse_includes(const char* fname, const char* psource, std::vector<std::string>& include_list, std::string& parsed_source)
{
    size_t current_line = 1;

    std::string base_path = filesystem::get_path(fname);

    static const char* INCLUDE = "#include";
    const char* token = psource;
    const char* start = psource;
    while((token = strstr(start, INCLUDE)))
    {
        std::string code = std::string(start, token - start);

        append_line_directive(parsed_source, current_line, fname);
        parsed_source.append(code);
        current_line += get_num_lines(code.c_str());

        const char* include;
        size_t size;
        if(!parse_include(token + strlen(INCLUDE), include, size, start))
            return false;

        std::string inc = std::string(include, size);
        std::string include_path = base_path + std::string(filesystem::kPathSeparator) + inc;
        include_list.push_back(include_path);

        // insert include contents to the shader source code
        std::string source;
        if(!load_shader(include_path.c_str(), source, include_list))
            return false;

        parsed_source.append(source);

        if(!start) // include was at last line 
            break;
    }

    if(*start)
    {
        append_line_directive(parsed_source, current_line, fname);
        parsed_source.append(std::string(start));
    }

    return true;
}

bool load_shader(const char* fname, std::string& shader_source, std::vector<std::string>& includes)
{
    const char* psource = glsl_load(fname);
    if(!psource)
        return false;

    if(!parse_includes(fname, psource, includes, shader_source))
    {
		log_error("Shader filename: %s: failed to parse includes\n", fname);
        delete[] psource;
        return false;
    }

    delete[] psource;

    return true;
}

bool compile_shader(GLenum shader, const char** strings, size_t count)
{
    glShaderSource(shader, count, strings, 0);
    glCompileShader(shader);

	GLenum err = glGetError();
	if(err != GL_NO_ERROR)
	{
		log_error("OpenGL Error: %s\n", ogl_get_error_code_str(err));
	}

    bool error = get_shader_error_status(shader, GL_COMPILE_STATUS);
    return !error && err==GL_NO_ERROR;
}


glsl_shader* glsl_shader::makeShader(Shader_t stype, const char* fname, const char* prefix/* = nullptr*/)
{
    std::string shader_source;
    std::vector<std::string> shader_includes;

    if(!load_shader(fname, shader_source, shader_includes))
    {
		log_error("Shader filename: %s, failed to load shader\n", fname);
        return nullptr;
    }

    log_info("Loading shader: %s\n", fname);


    glsl_shader* pshader = new glsl_shader();

    char unique_id[256] = {0};
    snprintf(unique_id, 256, "%s_%p", fname, pshader);
    std::string uid = &unique_id[0];

#define DUMP_SHADER_PREPROCESSED_FILES 1
#if DUMP_SHADER_PREPROCESSED_FILES
    char dump_name[256] = {0};
    snprintf(dump_name, 256, "./dump/%s.glsl", uid.c_str());
    FILE* f = fopen(dump_name, "w");
    if(f) {
        fwrite(shader_source.c_str(), shader_source.size(), 1, f);
		fclose(f);
    }
#endif

	
    GLenum type = get_gl_shader_type(stype);
    GLuint shader = glCreateShader(type);
    if(0 == shader)
    {
        glDeleteShader(shader);
        delete pshader;
        return 0;
    }

    const char* strings[] = { prefix == nullptr ? "" : prefix, shader_source.c_str() };
    if(!compile_shader(shader, strings, sizeof(strings)/sizeof(strings[0])))
    {
        glDeleteShader(shader);
        delete pshader;
        return nullptr;
    }

    pshader->fname_ = fname;
    pshader->shader_ = shader;
    pshader->type_ = type;
    pshader->includes_ = shader_includes;

    if(s_shaders[stype].count(uid))
	{
        log_error("Duplicate shader name: %s\n", fname);
        delete pshader;
        return nullptr;

		//glsl_shader* pshader = s_shaders[stype][fname];
		//s_shaders[stype].erase(fname);
        //delete pshader;
	}
    s_shaders[stype].insert( std::make_pair(uid, pshader) );

     
    return pshader;
}

void glsl_shader::deleteShader(glsl_shader* psh)
{
	glsl_shader::Shader_t t = get_shader_type(psh->type_);
	if(s_shaders[t].count(psh->fname_))
    {   
        delete s_shaders[t][psh->fname_];
        s_shaders[t].erase(psh->fname_);
    }
}

glsl_shader::~glsl_shader()
{
    glDeleteShader(shader_);
}

bool glsl_shader::reload(const char* prefix)
{
    std::string shader_source;
    std::vector<std::string> shader_includes;

    if(!load_shader(fname_.c_str(), shader_source, shader_includes))
    {
		log_error("Shader filename: %s, failed to load shader\n", fname_.c_str());
        return false;
    }
	
    const char* strings[] = { prefix == nullptr ? "" : prefix, shader_source.c_str() };
    if(!compile_shader(shader_, strings, sizeof(strings)/sizeof(strings[0])))
    {
        return false;
    }

    includes_ = shader_includes;

    return true;
}

uint64_t glsl_shader::getModTimeMs()
{
    using namespace filesystem;

	uint64_t mt = get_file_mod_time_ms(fname_.c_str());

    for(int i=0;i<includes_.size();++i)
    {
	    uint64_t t = get_file_mod_time_ms(includes_[i].c_str());
        mt = max(mt, t);
    }
    return mt;
}

void parse_uniforms(GLuint pprogram, glsl_program::UniArr_t* puniforms, glsl_program::SamplerArr_t* psamplers)
{
    GLint num_uni, max_name_len;
    glGetProgramiv(pprogram, GL_ACTIVE_UNIFORMS, &num_uni);
    glGetProgramiv(pprogram, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_name_len);
    char* buf = new char[max_name_len+1];
    GLsizei len;
    GLint size;
    GLenum type;
    for(GLint i=0;i<num_uni;++i)
    {
        glGetActiveUniform(pprogram, i, max_name_len+1, &len, &size, &type, buf);
        if(-1 == i) continue; // gl_ variable or does not correspond to an active uniform variable name in program

		if(type >=GL_SAMPLER_1D && type<= GL_SAMPLER_2D_SHADOW)
		{
			glsl_sampler* psampler = new glsl_sampler;
			psampler->index_ =  glGetUniformLocation(pprogram, buf);
			psampler->name_ = buf;
			psampler->type_ = (SamplerType)(type - GL_SAMPLER_1D);

			assert(psampler->type_ <= SAMPLER_2D_SHADOW);

			static const char *typeNames[] = {
				"sampler_1d", "sampler_2d", "sampler_3d", "sampler_cube", "sampler_1d_shadow", "sampler_2d_shadow"
			};

			log_info("name: %s type: %s\n", buf, typeNames[psampler->type_]);
	
			psamplers->insert(std::make_pair(psampler->name_, psampler));

			continue;
		}

        glsl_uniform* puni = new glsl_uniform(); 
        puni->name_ = buf;
        puni->is_dirty_ = true;
        puni->index_ = glGetUniformLocation(pprogram, buf);

        switch(type)
        {
            case GL_FLOAT:
                puni->type_ = CONSTANT_FLOAT;
                puni->num_el_ = 1;
                break;
            case GL_INT:
                puni->type_ = CONSTANT_INT;
                puni->num_el_ = 1;
                break;
            default:
                puni->type_ = (ConstantType)(CONSTANT_VEC2 + (type - GL_FLOAT_VEC2));
                puni->num_el_ = size;
                break;
        }


        size_t datasize = constantSizes[ puni->type_ ] * puni->num_el_;
        puni->data_ = new unsigned char[ datasize ];
        memset(puni->data_, 0, datasize);

        static const char *typeNames[] = {
            "float", "int  ", "vec2 ", "vec3 ", "vec4 ", "ivec2", "ivec3", "ivec4",
            "bool ", "bvec2", "bvec3", "bvec4", "mat2 ", "mat3 ", "mat4 "
        };
        log_info("name: %s type: %s  num_el: %d\n", buf, typeNames[puni->type_], puni->num_el_);

        puniforms->insert( std::make_pair(puni->name_, puni) );
    }

    delete[] buf;
}

void parse_uniform_blocks(GLuint pprogram, glsl_program::UniBlockArr_t* puniforms)
{
	GLint num_uni_blocks, max_name_len;
	glGetProgramiv(pprogram, GL_ACTIVE_UNIFORM_BLOCKS, &num_uni_blocks);
	glGetProgramiv(pprogram, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &max_name_len);
	char* buf = new char[max_name_len + 1];
	GLsizei len;

	GLint binding;
	GLint data_size;
	GLint num_uniforms;
	for (GLint i = 0; i<num_uni_blocks; ++i)
	{
		glGetActiveUniformBlockName(pprogram, i, max_name_len + 1, &len, buf);
		glGetActiveUniformBlockiv(pprogram, i, GL_UNIFORM_BLOCK_BINDING, &binding);
		glGetActiveUniformBlockiv(pprogram, i, GL_UNIFORM_BLOCK_DATA_SIZE, &data_size);
		glGetActiveUniformBlockiv(pprogram, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &num_uniforms);

		// should be equal to i ?
		GLuint index = glGetUniformBlockIndex(pprogram, buf);
		
		// ???
		if (-1 == i) continue; // gl_ variable or does not correspond to an active uniform variable name in program

		glsl_uniform_block* uni_block = new glsl_uniform_block();
		uni_block->index_ = index;
		uni_block->binding_ = binding;
		uni_block->data_size_ = data_size;
		uni_block->is_dirty_ = true;
		uni_block->name_ = buf;
		uni_block->num_uniforms_ = num_uniforms;

		puniforms->insert(std::make_pair(buf, uni_block));
	}

	delete[] buf;
}

glsl_program* glsl_program::makeProgram2(const char* name, const char* vp, const char* hp, const char* dp, const char* gp, const char* fp, int count/* = 0*/, const char** xfb_variables/* = 0*/, const char* prefix/*=nullptr*/)
{
	if(!uniformFuncs[0])
        init_func_ptrs(uniformFuncs);

    assert(name);

	assert((count && xfb_variables) || 0==count);

    if(s_programs.count(name))
    {
        log_error("Program with this name (%s) already exists\n", name);
        return 0;
    }

    glsl_shader* vsh = glsl_shader::makeShader(glsl_shader::VERTEX, vp, prefix);
	if(!vsh)
		return 0;

	glsl_shader* hsh = 0, *dsh = 0, *gsh = 0, *fsh = 0;
	
	if(fp)
	{
		fsh = glsl_shader::makeShader(glsl_shader::FRAGMENT, fp, prefix);
		if(!fsh)
			return 0;
	}
    	
	if(hp)
	{
		hsh = glsl_shader::makeShader(glsl_shader::HULL, hp, prefix);
		if(!hsh)
			return 0;
	}

	if(dp)
	{
		dsh = glsl_shader::makeShader(glsl_shader::DOMAINE, dp, prefix);
		if(!dsh)
			return 0;
	}

	if(gp)
	{
		gsh = glsl_shader::makeShader(glsl_shader::GEOMERTY, gp, prefix);
		if(!gsh)
			return 0;
	}
			
	glsl_shader* pipeline[] = { vsh, hsh, dsh, gsh, fsh };

    GLuint shp = glCreateProgram();

	GLuint last_not_null = 0;
	for(size_t i=0; i< sizeof(pipeline)/sizeof(pipeline[0]); ++i)
	{
		if(!pipeline[i]) continue;
		last_not_null = pipeline[i]->shader_;

		glAttachShader(shp, pipeline[i]->shader_);
		if( GL_NO_ERROR != glGetError())
		{
	        glDeleteProgram(shp);
			log_error("glAttachShader: error during attaching %s\n", pipeline[i]->fname_.c_str());
			return 0;
		}
	}

	GLenum err = glGetError();
	if(err != GL_NO_ERROR)
	{
		log_error("Shader name: %s\n", name);
		log_error("OpenGL Error: %s\n", ogl_get_error_code_str(err));
	}

	if(count)
	{
		glTransformFeedbackVaryings(last_not_null, count, xfb_variables, GL_INTERLEAVED_ATTRIBS);
	}

	err = glGetError();
	if(err != GL_NO_ERROR)
	{
		log_error("Shader name: %s\n", name);
		log_error("OpenGL Error: %s\n", ogl_get_error_code_str(err));
	}

    glLinkProgram(shp);

	err = glGetError();
	if(err != GL_NO_ERROR)
	{
		log_error("Shader name: %s\n", name);
		log_error("OpenGL Error: %s\n", ogl_get_error_code_str(err));
	}

	CHECK_GL_ERROR
	if(get_program_error_status(shp, GL_LINK_STATUS))
    {
        glDeleteProgram(shp);
        return 0;
    }

    glsl_program* pprogram = new glsl_program();
    pprogram->shp_ = shp;
    pprogram->vsh_ = vsh;
    pprogram->fsh_ = fsh;
    pprogram->hsh_ = hsh;
    pprogram->dsh_ = dsh;
    pprogram->gsh_ = gsh;
    if(prefix) {
        size_t size = strlen(prefix) + 1;
        pprogram->prefix_ = new char[size];
        memcpy(pprogram->prefix_, prefix, size);
    } else {
        pprogram->prefix_ = nullptr;
    }
    pprogram->is_valid_ = true;

	for(size_t i=0; i< sizeof(pipeline)/sizeof(pipeline[0]); ++i)
	{
		if(!pipeline[i]) continue;
		glDetachShader(shp, pipeline[i]->shader_);
	}

    parse_uniforms(shp, &pprogram->uniforms_, &pprogram->samplers_);
    parse_uniform_blocks(shp, &pprogram->uniform_blocks_);

    pprogram->last_load_time_ = timing::get_wall_time_ms();

    s_programs.insert(std::make_pair(name, pprogram) );
    return pprogram;

}

glsl_program* glsl_program::makeProgram(const char* name, const char* vp, const char* fp, const char* prefix /*= nullptr*/)
{
	return makeProgram2(name, vp, 0, 0, 0, fp, 0, nullptr, prefix);
}

void glsl_program::deleteProgram(const char* name)
{
    if(s_programs.count(name))
    {
        glsl_program* pprogram = s_programs[name];
        s_programs.erase(name);
        delete pprogram;
    }
}

glsl_program::~glsl_program()
{
    if(shp_)
    {
		glsl_shader* pipeline[] = { vsh_, hsh_, dsh_, gsh_, fsh_ };
		for(uint32_t i=0; i< sizeof(pipeline)/sizeof(pipeline[0]); ++i)
		{
			if(!pipeline[i]) continue;

			glDetachShader(shp_, pipeline[i]->shader_);
		}
        glDeleteProgram(shp_);
	}
}

void glsl_program::apply()
{
    glUseProgram(shp_);

    UniArr_t::iterator it = uniforms_.begin(); 
    UniArr_t::iterator end = uniforms_.end(); 
    for(;it!=end;++it)
    {
        glsl_uniform* puni = it->second;
        if(puni->is_dirty_)
        {
            if (puni->type_ >= CONSTANT_MAT2){
                ((UNIFORM_MAT_FUNC) uniformFuncs[puni->type_])(puni->index_, puni->num_el_, GL_TRUE, (float *) puni->data_);
            } else {
                uniformFuncs[puni->type_](puni->index_, puni->num_el_, (float *) puni->data_);
                if(GL_INVALID_OPERATION == glGetError())
                    log_error("Error setting variable\n");
            }
            puni->is_dirty_ = false;
        }
    }
}

bool glsl_program::reload()
{
    is_valid_ = false;

	//glDetachShader(shp_, vsh_->shader_);
	//glDetachShader(shp_, fsh_->shader_);

	bool rv = true;

	glsl_shader* const pipeline[] = { vsh_, hsh_, dsh_, gsh_, fsh_ };
	for(size_t i=0; i< sizeof(pipeline)/sizeof(pipeline[0]); ++i)
	{
		if(!pipeline[i]) continue;
		rv &= pipeline[i]->reload(prefix_);
	}
    if(!rv) return false;
	
	for(size_t i=0; i< sizeof(pipeline)/sizeof(pipeline[0]); ++i)
	{
		if(!pipeline[i]) continue;
		glAttachShader(shp_, pipeline[i]->shader_);
	}

    glLinkProgram(shp_);
    if(get_program_error_status(shp_, GL_LINK_STATUS))
    {
        return false;
    }

	for(size_t i=0; i< sizeof(pipeline)/sizeof(pipeline[0]); ++i)
	{
		if(!pipeline[i]) continue;
		glDetachShader(shp_, pipeline[i]->shader_);
	}

    std::map< std::string, glsl_uniform*>::iterator it = uniforms_.begin(); 
    std::map< std::string, glsl_uniform*>::iterator end = uniforms_.end(); 
    for(;it!=end;++it)
        delete it->second;
    uniforms_.clear();

    samplers_.clear();
    uniform_blocks_.clear();

    parse_uniforms(shp_, &uniforms_, &samplers_);
    parse_uniform_blocks(shp_, &uniform_blocks_);
    
    last_load_time_ = timing::get_wall_time_ms();

    is_valid_ = true;
    return true;

}

bool glsl_program::is_valid()
{
    return is_valid_;
}

//=====================================================================================================================================
bool glsl_program::setFloat(const char* name, const float v)
{
    UniArr_t::iterator it = uniforms_.find(name);
    if(it!=uniforms_.end() && it->second->type_ == CONSTANT_FLOAT)
    {
        memcpy(it->second->data_, &v, constantSizes[it->second->type_] * it->second->num_el_);
        it->second->is_dirty_ = true;
        return true;
    }
	log_error("Type mismatch: %s\n", name);
    return false;
}
bool glsl_program::setFloat2(const char* name, const float v[2])
{
    UniArr_t::iterator it = uniforms_.find(name);
    if(it!=uniforms_.end() && it->second->type_ == CONSTANT_VEC2)
    {
        memcpy(it->second->data_, v, constantSizes[it->second->type_] * it->second->num_el_);
        it->second->is_dirty_ = true;
        return true;
    }
	log_error("Type mismatch: %s\n", name);
    return false;
}
bool glsl_program::setFloat3(const char* name, const float v[3])
{
    UniArr_t::iterator it = uniforms_.find(name);
    if(it!=uniforms_.end() && it->second->type_ == CONSTANT_VEC3)
    {
        memcpy(it->second->data_, v, constantSizes[it->second->type_] * it->second->num_el_);
        it->second->is_dirty_ = true;
        return true;
    }
	log_error("Type mismatch: %s\n", name);
    return false;
}

bool glsl_program::setFloat4(const std::string& name, const float v[4])
{
    UniArr_t::iterator it = uniforms_.find(name);
    if(it!=uniforms_.end() && it->second->type_ == CONSTANT_VEC4)
    {
        memcpy(it->second->data_, v, constantSizes[it->second->type_] * it->second->num_el_);
        it->second->is_dirty_ = true;
        return true;
    }
	log_error("Type mismatch: %s\n", name.c_str());
    return false;
}

// TODO: need to change interface so that float* instead of float[4] is passed
bool glsl_program::setFloat4(const char* name, const float v[4])
{
	return setFloat4(std::string(name), v);
}

bool glsl_program::setInt(const char* name, const int v)
{
    UniArr_t::iterator it = uniforms_.find(name);
    if(it!=uniforms_.end() && it->second->type_ == CONSTANT_INT)
    {
        memcpy(it->second->data_, &v, constantSizes[it->second->type_] * it->second->num_el_);
        it->second->is_dirty_ = true;
        return true;
    }
	log_error("Type mismatch: %s\n", name);
    return false;
}
bool glsl_program::setInt2(const char* name, const int v[2])
{
    UniArr_t::iterator it = uniforms_.find(name);
    if(it!=uniforms_.end() && it->second->type_ == CONSTANT_IVEC2)
    {
        memcpy(it->second->data_, v, constantSizes[it->second->type_] * it->second->num_el_);
        it->second->is_dirty_ = true;
        return true;
    }
	log_error("Type mismatch: %s\n", name);
    return false;
}
bool glsl_program::setInt3(const char* name, const int v[3])
{
    UniArr_t::iterator it = uniforms_.find(name);
    if(it!=uniforms_.end() && it->second->type_ == CONSTANT_IVEC3)
    {
        memcpy(it->second->data_, v, constantSizes[it->second->type_] * it->second->num_el_);
        it->second->is_dirty_ = true;
        return true;
    }
	log_error("Type mismatch: %s\n", name);
    return false;
}
bool glsl_program::setInt4(const char* name, const int v[4])
{
    UniArr_t::iterator it = uniforms_.find(name);
    if(it!=uniforms_.end() && it->second->type_ == CONSTANT_IVEC4)
    {
        memcpy(it->second->data_, v, constantSizes[it->second->type_] * it->second->num_el_);
        it->second->is_dirty_ = true;
        return true;
    }
	log_error("Type mismatch: %s\n", name);
    return false;
}

bool glsl_program::setMat2(const char* name, const float v[4])
{
    UniArr_t::iterator it = uniforms_.find(name);
    if(it!=uniforms_.end() && it->second->type_ == CONSTANT_MAT2)
    {
        memcpy(it->second->data_, v, constantSizes[it->second->type_] * it->second->num_el_);
        it->second->is_dirty_ = true;
        return true;
    }
	log_error("Type mismatch: %s\n", name);
    return false;
}
bool glsl_program::setMat3(const char* name, const float v[9])
{
    UniArr_t::iterator it = uniforms_.find(name);
    if(it!=uniforms_.end() && it->second->type_ == CONSTANT_MAT3)
    {
        memcpy(it->second->data_, v, constantSizes[it->second->type_] * it->second->num_el_);
        it->second->is_dirty_ = true;
        return true;
    }
	log_error("Type mismatch: %s\n", name);
    return false;
}
bool glsl_program::setMat4(const char* name, const float v[16])
{
    UniArr_t::iterator it = uniforms_.find(name);
    if(it!=uniforms_.end() && it->second->type_ == CONSTANT_MAT4)
    {
        memcpy(it->second->data_, v, constantSizes[it->second->type_] * it->second->num_el_);
        it->second->is_dirty_ = true;
        return true;
    }
	log_error("Type mismatch: %s\n", name);
    return false;
}

bool glsl_program::setMat4(const std::string& name , const float v[16])
{
    UniArr_t::iterator it = uniforms_.find(name);
    if(it!=uniforms_.end() && it->second->type_ == CONSTANT_MAT4)
    {
        memcpy(it->second->data_, v, constantSizes[it->second->type_] * it->second->num_el_);
        it->second->is_dirty_ = true;
        return true;
    }
	log_error("Type mismatch: %s\n", name.c_str());
    return false;
}

GLint glsl_program::getAttribLocation(const char* pattrib)
{
	assert(pattrib);
	if(this->shp_)
		return glGetAttribLocation(this->shp_, pattrib);
	return -1;
}

uint64_t glsl_program::getModTimeMs()
{
    uint64_t least_recent_mt = 0;
    if(shp_)
    {
		glsl_shader* pipeline[] = { vsh_, hsh_, dsh_, gsh_, fsh_ };
		for(uint32_t i=0; i< sizeof(pipeline)/sizeof(pipeline[0]); ++i)
		{
			if(!pipeline[i]) continue;

			uint64_t mt = pipeline[i]->getModTimeMs();
            least_recent_mt = max(mt, least_recent_mt);
		}
	}

    return least_recent_mt;
}

bool glsl_program::needsReload()
{
    return last_load_time_ < getModTimeMs();
}

