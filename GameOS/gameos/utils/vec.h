#ifndef _VECTOR_H_
#define _VECTOR_H_

// ATTENTION: Include this file AFTER STL headers if us use STL
//#pragma message("Include " __FILE__  " AFTER STL headers")


/** \file
Standard 3D math definitions and routines
*/

#include <math.h>
#ifndef PI
#define PI 3.1415926535f
#endif
#ifndef M_PI
#define M_PI PI
#endif


struct half {
	unsigned short sh;

	half(){};
    half(const float x);
	operator float () const;
};

/* --------------------------------------------------------------------------------- */


/** 2D single-precision floating-point vector */
struct vec2 {
	float x, y;

	vec2():x(0),y(0) {}
	explicit vec2(const float iv){
		x = y = iv;
	}
	vec2(const float ix, const float iy){
		x = ix;
		y = iy;
	}

	operator float *(){ return &x; }
	operator const float *() const { return &x; }

	void operator += (const vec2 &v);
	void operator -= (const vec2 &v);
	void operator *= (const float s);
	void operator *= (const vec2 &v);
	void operator /= (const float s);
	void operator /= (const vec2 &v);
};

vec2 operator + (const vec2 &u, const vec2 &v);
vec2 operator + (const vec2 &v, const float s);
vec2 operator - (const vec2 &u, const vec2 &v);
vec2 operator - (const vec2 &v, const float s);
vec2 operator - (const vec2 &v);
vec2 operator * (const vec2 &u, const vec2 &v);
vec2 operator * (const float s, const vec2 &v);
vec2 operator * (const vec2 &v, const float s);
vec2 operator / (const vec2 &u, const vec2 &v);
vec2 operator / (const vec2 &v, const float s);

bool operator == (const vec2 &u, const vec2 &v);
bool operator != (const vec2 &u, const vec2 &v);

/** 3D single-precision floating-point vector */
struct vec3 {
	float x, y, z;

	vec3():x(0),y(0), z(0) {}
	explicit vec3(const float iv){
		x = y = z = iv;
	}
	vec3(const vec2 &xy, const float iz){
		x = xy.x;
		y = xy.y;
		z = iz;
	}
	vec3(const float ix, const float iy, const float iz){
		x = ix;
		y = iy;
		z = iz;
	}

	operator float *(){ return &x; }
	operator const float *() const { return &x; }

	vec2 xy() const { return vec2(x, y); }

	vec3 xzy() const { return vec3(x, z, y); }
	vec3 zyx() const { return vec3(z, y, x); }
	vec3 yxz() const { return vec3(y, x, z); }
	vec3 yzx() const { return vec3(y, z, x); }
	vec3 zxy() const { return vec3(z, x, y); }

	void operator += (const vec3 &v);
	void operator -= (const vec3 &v);
	void operator *= (const float s);
	void operator *= (const vec3 &v);
	void operator /= (const float s);
	void operator /= (const vec3 &v);

	// ps3 interface support
	float getX() const { return x;}
	float getY() const { return y;}
	float getZ() const { return z;}
};

vec3 operator + (const vec3 &u, const vec3 &v);
vec3 operator + (const vec3 &v, const float s);
vec3 operator - (const vec3 &u, const vec3 &v);
vec3 operator - (const vec3 &v, const float s);
vec3 operator - (const vec3 &v);
vec3 operator * (const vec3 &u, const vec3 &v);
vec3 operator * (const float s, const vec3 &v);
vec3 operator * (const vec3 &v, const float s);
vec3 operator / (const vec3 &u, const vec3 &v);
vec3 operator / (const vec3 &v, const float s);

bool operator == (const vec3 &u, const vec3 &v);
bool operator != (const vec3 &u, const vec3 &v);

/** 4D single-precision floating-point vector */
struct vec4 {
	float x, y, z, w;

	vec4():x(0),y(0), z(0), w(0) {}
	explicit vec4(const float iv){
		x = y = z = w = iv;
	}
	vec4(const float ix, const float iy, const float iz, const float iw){
		x = ix;
		y = iy;
		z = iz;
		w = iw;
	}
	vec4(const vec2 &xy, const float iz, const float iw){
		x = xy.x;
		y = xy.y;
		z = iz;
		w = iw;
	}
	vec4(const vec2 &xy, const vec2 &zw){
		x = xy.x;
		y = xy.y;
		z = zw.x;
		w = zw.y;
	}
	vec4(const vec3 &xyz, const float iw){
		x = xyz.x;
		y = xyz.y;
		z = xyz.z;
		w = iw;
	}

	// ps3 interface support
	explicit vec4(const vec3& xyz) {
		x = xyz.x;
		y = xyz.y;
		z = xyz.z;
		w = 0.0f;
	}

	vec3 getXYZ() {
		return vec3(x,y,z);
	}
	//

	operator float *(){ return &x; }
	operator const float *() const { return &x; }

	vec2 xy()  const { return vec2(x, y); }
	vec3 xyz() const { return vec3(x, y, z); }

	vec3 xzy() const { return vec3(x, z, y); }
	vec3 zyx() const { return vec3(z, y, x); }
	vec3 yxz() const { return vec3(y, x, z); }
	vec3 yzx() const { return vec3(y, z, x); }
	vec3 zxy() const { return vec3(z, x, y); }

	vec4 xzyw() const { return vec4(x, z, y, w); }
	vec4 zyxw() const { return vec4(z, y, x, w); }
	vec4 yxzw() const { return vec4(y, x, z, w); }
	vec4 yzxw() const { return vec4(y, z, x, w); }
	vec4 zxyw() const { return vec4(z, x, y, w); }

	void operator += (const vec4 &v);
	void operator -= (const vec4 &v);
	void operator *= (const float s);
	void operator *= (const vec4 &v);
	void operator /= (const float s);
	void operator /= (const vec4 &v);

	// ps3 interface support
	float getX() const { return x;}
	float getY() const { return y;}
	float getZ() const { return z;}
	float getW() const { return w;}
};

vec4 operator + (const vec4 &u, const vec4 &v);
vec4 operator + (const vec4 &v, const float s);
vec4 operator - (const vec4 &u, const vec4 &v);
vec4 operator - (const vec4 &v, const float s);
vec4 operator - (const vec4 &v);
vec4 operator * (const vec4 &u, const vec4 &v);
vec4 operator * (const float s, const vec4 &v);
vec4 operator * (const vec4 &v, const float s);
vec4 operator / (const vec4 &u, const vec4 &v);
vec4 operator / (const vec4 &v, const float s);

bool operator == (const vec4 &u, const vec4 &v);
bool operator != (const vec4 &u, const vec4 &v);


/** 4D integer vector */
struct ivec4 {
	int x, y, z, w;

	ivec4():x(0),y(0), z(0), w(0){}
	ivec4(const int iv){
		x = y = z = w = iv;
	}
	ivec4(const int ix, const int iy, const int iz, const int iw){
		x = ix;
		y = iy;
		z = iz;
		w = iw;
	}
	operator const int *() const { return &x;	}
};


float dot(const vec2 &u, const vec2 &v);
float dot(const vec3 &u, const vec3 &v);
float dot(const vec4 &u, const vec4 &v);

float lerp(const float u, const float v, const float x);
vec2  lerp(const vec2 &u, const vec2 &v, const float x);
vec3  lerp(const vec3 &u, const vec3 &v, const float x);
vec4  lerp(const vec4 &u, const vec4 &v, const float x);

/** Fit t to an S-Curve */
inline float sCurve(const float t){
	return t * t * (3 - 2 * t);
}


#ifdef min
#undef min
#endif


#ifdef max
#undef max
#endif

/** Returns the lesser of two vectors */
template <typename DATA_TYPE>
inline DATA_TYPE min(const DATA_TYPE x, const DATA_TYPE y){
	return (x < y)? x : y;
}

/** Returns the greater of two vectors */
template <typename DATA_TYPE>
inline DATA_TYPE max(const DATA_TYPE x, const DATA_TYPE y){
	return (x > y)? x : y;
}

vec2 min(const vec2 u, const vec2 v);
vec2 max(const vec2 u, const vec2 v);
vec3 min(const vec3 u, const vec3 v);
vec3 max(const vec3 u, const vec3 v);
vec4 min(const vec4 u, const vec4 v);
vec4 max(const vec4 u, const vec4 v);

/** Clamps x to [lower-upper] */
template <typename DATA_TYPE>
inline DATA_TYPE clamp(const DATA_TYPE x, const float lower, const float upper){
	return max(min(x, DATA_TYPE(upper)), DATA_TYPE(lower));
}

/** Clamps x to [0-1] */
#define saturate(x) clamp(x, 0, 1)

int signP(const int x);     /**< Returns +1 if x > 0, -1 otherwise */
int signN(const int x);     /**< Returns +1 if x < 0, -1 otherwise */
int sign(const int x);      
float sign(const float x);
vec2 sign(const vec2 &v);
vec3 sign(const vec3 &v);
vec4 sign(const vec4 &v);

float smoothstep(const float edge0, const float edge1, const float x);

/** Compute the square of the input x */
template <typename DATA_TYPE>
inline DATA_TYPE sqr(const DATA_TYPE x){
	return x * x;
}

vec2 normalize(const vec2 &v);
vec3 normalize(const vec3 &v);
vec4 normalize(const vec4 &v);

float length(const vec2 &v);
float length(const vec3 &v);
float length(const vec4 &v);

float lengthSqr(const vec2 &v);
float lengthSqr(const vec3 &v);
float lengthSqr(const vec4 &v);

/** Compute the distance between two nD vectors, where n is [2-4] */
//#define distance(u, v) length((u) - (v))

/** Compute the cross product of two 3D vectors */
vec3 cross(const vec3 &u, const vec3 &v);

/** Convert RGBE format to RGB format */
vec3 rgbeToRGB(unsigned char *rgbe);

/** Convert 4 floats into a single DWORD value. @{ */
unsigned int toRGBA(const vec4 &u);
unsigned int toBGRA(const vec4 &u);
//@}



/** 2x2 matrix */
struct mat2 {
	float elem[2][2];

	mat2(){}
	mat2(const float m00, const float m01,
		const float m10, const float m11){
			elem[0][0] = m00; elem[0][1] = m01;
			elem[1][0] = m10; elem[1][1] = m11;
	}
	mat2(const vec2 &v0, const vec2 &v1){
		elem[0][0] = v0.x; elem[0][1] = v0.y;
		elem[1][0] = v1.x; elem[1][1] = v1.y;
	}

	operator const float *() const { return (const float *) elem; }

};

mat2 operator + (const mat2 &m, const mat2 &n);
mat2 operator - (const mat2 &m, const mat2 &n);
mat2 operator - (const mat2 &m);
mat2 operator * (const mat2 &m, const mat2 &n);
vec2 operator * (const mat2 &m, const vec2 &v);
mat2 operator * (const mat2 &m, const float x);

mat2 transpose(const mat2 &m);

/** 3x3 matrix */
struct mat3 {
	float elem[3][3];

	mat3(){}
	mat3(const float m00, const float m01, const float m02,
		const float m10, const float m11, const float m12,
		const float m20, const float m21, const float m22){
			elem[0][0] = m00; elem[0][1] = m01; elem[0][2] = m02;
			elem[1][0] = m10; elem[1][1] = m11; elem[1][2] = m12;
			elem[2][0] = m20; elem[2][1] = m21; elem[2][2] = m22;
	}
	mat3(const vec3 &v0, const vec3 &v1, const vec3 &v2){
		elem[0][0] = v0.x; elem[0][1] = v0.y; elem[0][2] = v0.z;
		elem[1][0] = v1.x; elem[1][1] = v1.y; elem[1][2] = v1.z;
		elem[2][0] = v2.x; elem[2][1] = v2.y; elem[2][2] = v2.z;
	}

	vec3 &getRow(const unsigned int index){ return *((vec3 *) elem[index]); }

	operator const float *() const { return (const float *) elem; }
};

mat3 operator + (const mat3 &m, const mat3 &n);
mat3 operator - (const mat3 &m, const mat3 &n);
mat3 operator - (const mat3 &m);
mat3 operator * (const mat3 &m, const mat3 &n);
vec3 operator * (const mat3 &m, const vec3 &v);
mat3 operator * (const mat3 &m, const float x);

mat3 transpose(const mat3 &m);
mat3 operator ! (const mat3 &m);

/** 4x4 matrix */
struct mat4 {
	float elem[4][4];

	mat4(){}
	mat4(const float m00, const float m01, const float m02, const float m03,
		const float m10, const float m11, const float m12, const float m13,
		const float m20, const float m21, const float m22, const float m23,
		const float m30, const float m31, const float m32, const float m33){
			elem[0][0] = m00; elem[0][1] = m01; elem[0][2] = m02; elem[0][3] = m03;
			elem[1][0] = m10; elem[1][1] = m11; elem[1][2] = m12; elem[1][3] = m13;
			elem[2][0] = m20; elem[2][1] = m21; elem[2][2] = m22; elem[2][3] = m23;
			elem[3][0] = m30; elem[3][1] = m31; elem[3][2] = m32; elem[3][3] = m33;
	}
	mat4(const vec4 &v0, const vec4 &v1, const vec4 &v2, const vec4 &v3){
		elem[0][0] = v0.x; elem[0][1] = v0.y; elem[0][2] = v0.z; elem[0][3] = v0.w;
		elem[1][0] = v1.x; elem[1][1] = v1.y; elem[1][2] = v1.z; elem[1][3] = v1.w;
		elem[2][0] = v2.x; elem[2][1] = v2.y; elem[2][2] = v2.z; elem[2][3] = v2.w;
		elem[3][0] = v3.x; elem[3][1] = v3.y; elem[3][2] = v3.z; elem[3][3] = v3.w;
	}

	vec3 getRightVec()   const { return vec3(elem[0][0], elem[0][1], elem[0][2]); }
	vec3 getUpVec()      const { return vec3(elem[1][0], elem[1][1], elem[1][2]); }
	vec3 getForwardVec() const { return vec3(elem[2][0], elem[2][1], elem[2][2]); }

	operator const float *() const { return (const float *) elem; }
	void translate(const vec3 &v);

	// ps3 interface support
	static mat4 scale(const vec3 &v);
	static mat4 translation(const vec3 &v);
	static mat4 rotationX(const float angle);
	static mat4 rotationY(const float angle);
	static mat4 rotationZ(const float angle);

	void setCol0(const vec4& v){
		elem[0][0] = v.x; elem[1][0] = v.y; elem[2][0] = v.z; elem[3][0] = v.w;
	}
	void setCol1(const vec4& v){
		elem[0][1] = v.x; elem[1][1] = v.y; elem[2][1] = v.z; elem[3][1] = v.w;
	}
	void setCol2(const vec4& v){
		elem[0][2] = v.x; elem[1][2] = v.y; elem[2][2] = v.z; elem[3][2] = v.w;
	}
	void setCol3(const vec4& v){
		elem[0][3] = v.x; elem[1][3] = v.y; elem[2][3] = v.z; elem[3][3] = v.w;
	}

	vec4 getRow (int row) const {
		return vec4(elem[row][0], elem[row][1], elem[row][2], elem[row][3]);
	}
	void setRow(int idx, const vec4& row) {
		elem[idx][0] = row.x; elem[idx][1] = row.y; elem[idx][2] = row.z; elem[idx][3] = row.w;
	}

	vec4 getCol0() const {
		return vec4(elem[0][0], elem[1][0], elem[2][0], elem[3][0]);
	}
	vec4 getCol1() const {
		return vec4(elem[0][1], elem[1][1], elem[2][1], elem[3][1]);
	}
	vec4 getCol2() const {
		return vec4(elem[0][2], elem[1][2], elem[2][2], elem[3][2]);
	}
	vec4 getCol3() const {
		return vec4(elem[0][3], elem[1][3], elem[2][3], elem[3][3]);
	}
	void setElem(int c, int r, float el) { elem[r][c] = el; } 
	float getElem(int c, int r) const { return elem[r][c]; } 

	static mat4 identity();
	vec3 getTranslation() const { return vec3(elem[0][3], elem[1][3], elem[2][3]); }
	void setTranslation(const vec3& tr) { setCol3(vec4(tr.x, tr.y, tr.z, elem[3][3])); }
};

mat4 operator + (const mat4 &m, const mat4 &n);
mat4 operator - (const mat4 &m, const mat4 &n);
mat4 operator - (const mat4 &m);
mat4 operator * (const mat4 &m, const mat4 &n);
vec4 operator * (const mat4 &m, const vec4 &v);
mat4 operator * (const mat4 &m, const float x);

mat4 transpose(const mat4 &m);
mat4 operator ! (const mat4 &m);

/** Computes a 2x2 rotation matrix about a given angle */
mat2 rotate2(const float angle);

/** Computes an NxN rotation matrix about a (set of) given angle(s). @{ */
mat3 rotateX3(const float angle);
mat3 rotateY3(const float angle);
mat3 rotateZ3(const float angle);
mat3 rotateXY3(const float angleX, const float angleY);
mat3 rotateZXY3(const float angleX, const float angleY, const float angleZ);

mat4 rotateX4(const float angle);
mat4 rotateY4(const float angle);
mat4 rotateZ4(const float angle);
mat4 rotateXY4(const float angleX, const float angleY);
mat4 rotateZXY4(const float angleX, const float angleY, const float angleZ);
//@}

/** Creates a 4x4 translation matrix by the input vector */
mat4 translate(const float x, const float y, const float z);
mat4 translate(const vec3 &v);

/** Creates an orthographic projection matrix */
mat4 orthoMatrix(const float left, const float right, const float top, const float bottom, const float zNear, const float zFar, const bool d3dStyle);

/** Creates a perspective-correct projection matrix */
mat4 perspectiveMatrixX(const float fov, const int width, const int height, const float zNear, const float zFar, const bool d3dStyle);
mat4 perspectiveMatrixY(const float fov, const int width, const int height, const float zNear, const float zFar, const bool d3dStyle);

/** Creates a perspective-correct projection matrix */
mat4 frustumProjMatrix(const float left, const float right, const float bottom, const float top, const float near, const float far);

/** Creates a shadow projection matrix */
mat4 shadowMatrix(const vec3 &planeNormal, const float planeOffset, const vec3 &lightPos);

/** Creates a mirror matrix for a given plane */
mat4 mirrorMatrix(const vec3 &planeNormal, const float planeOffset);

/** Creates a modelview matrix for rendering to a given cubemap face */
mat4 cubemapModelviewMatrix(const unsigned int face);

/** Creates a projection matrix for rendering to a cubemap */
mat4 cubemapProjectionMatrix(const float zNear, const float zFar, const bool d3dStyle);

/** Creates a standard view matrix */
mat4 lookAt(const vec3 &from, const vec3 &at);

/** Converts a column-major 4x4 matrix to a row-major 4x4 matrix */
mat4 glToD3DProjectionMatrix(const mat4 &m);

/** Pegs the geometry to the far clipping plane. Useful for instance for skyboxes. */
mat4 pegToFarPlane(const mat4 &m);

/** Returns an NxN identity matrix. @{ */
mat2 identity2();
mat3 identity3();
mat4 identity4();
//@}

/** Returns an NxN scaling matrix. @{ */
mat2 scale2(const float x, const float y);
mat3 scale3(const float x, const float y, const float z);
mat4 scale4(const float x, const float y, const float z);
//@}

/** HLSL (High-Level Shading Language) notation of basic types. @{ */
// needed to cemment this because of conflicts with mayas float2,3,4
//typedef vec2 float2;
//typedef vec3 float3;
//typedef vec4 float4;

typedef ivec4 int4;

typedef mat2 float2x2;
typedef mat3 float3x3;
typedef mat4 float4x4;
//@}




#endif // _VECTOR_H_
