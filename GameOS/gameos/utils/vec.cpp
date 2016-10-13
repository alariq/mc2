#include "vec.h"
#include <memory.h>

half::half(const float x){
	union {
		float floatI;
		unsigned int i;
	};
	floatI = x;

//	unsigned int i = *((unsigned int *) &x);
	int e = ((i >> 23) & 0xFF) - 112;
	int m =  i & 0x007FFFFF;

	sh = (i >> 16) & 0x8000;
	if (e <= 0){
		// Denorm
		m = ((m | 0x00800000) >> (1 - e)) + 0x1000;
		sh |= (m >> 13);
	} else if (e == 143){
		sh |= 0x7C00;
		if (m != 0){
			// NAN
			m >>= 13;
			sh |= m | (m == 0);
		}
	} else {
		m += 0x1000;
		if (m & 0x00800000){
			// Mantissa overflow
			m = 0;
			e++;
		}
		if (e >= 31){
			// Exponent overflow
			sh |= 0x7C00;
		} else {
			sh |= (e << 10) | (m >> 13);
		}
	}
}

half::operator float () const {
	union {
		unsigned int s;
		float result;
	};

	s = (sh & 0x8000) << 16;
	unsigned int e = (sh >> 10) & 0x1F;
	unsigned int m = sh & 0x03FF;

	if (e == 0){
		// +/- 0
		if (m == 0) return result;

		// Denorm
		while ((m & 0x0400) == 0){
			m += m;
			e--;
		}
		e++;
		m &= ~0x0400;
	} else if (e == 31){
		// INF / NAN
		s |= 0x7F800000 | (m << 13);
		return result;
	}

	s |= ((e + 112) << 23) | (m << 13);

	return result;
}
/* --------------------------------------------------------------------------------- */

void vec2::operator += (const vec2 &v){
	x += v.x;
	y += v.y;
}

void vec2::operator -= (const vec2 &v){
	x -= v.x;
	y -= v.y;
}

void vec2::operator *= (const float s){
	x *= s;
	y *= s;
}

void vec2::operator *= (const vec2 &v){
	x *= v.x;
	y *= v.y;
}

void vec2::operator /= (const float s){
	x /= s;
	y /= s;
}

void vec2::operator /= (const vec2 &v){
	x /= v.x;
	y /= v.y;
}

vec2 operator + (const vec2 &u, const vec2 &v){
	return vec2(u.x + v.x, u.y + v.y);
}

vec2 operator + (const vec2 &v, const float s){
	return vec2(v.x + s, v.y + s);
}

vec2 operator - (const vec2 &u, const vec2 &v){
	return vec2(u.x - v.x, u.y - v.y);
}

vec2 operator - (const vec2 &v, const float s){
	return vec2(v.x - s, v.y - s);
}

vec2 operator - (const vec2 &v){
	return vec2(-v.x, -v.y);
}

vec2 operator * (const vec2 &u, const vec2 &v){
	return vec2(u.x * v.x, u.y * v.y);
}

vec2 operator * (const float s, const vec2 &v){
	return vec2(v.x * s, v.y * s);
}

vec2 operator * (const vec2 &v, const float s){
	return vec2(v.x * s, v.y * s);
}

vec2 operator / (const vec2 &u, const vec2 &v){
	return vec2(u.x / v.x, u.y / v.y);
}

vec2 operator / (const vec2 &v, const float s){
	return vec2(v.x / s, v.y / s);
}

bool operator == (const vec2 &u, const vec2 &v){
	return (u.x == v.x && u.y == v.y);
}

bool operator != (const vec2 &u, const vec2 &v){
	return (u.x != v.x || u.y != v.y);
}


void vec3::operator += (const vec3 &v){
	x += v.x;
	y += v.y;
	z += v.z;
}

void vec3::operator -= (const vec3 &v){
	x -= v.x;
	y -= v.y;
	z -= v.z;
}

void vec3::operator *= (const float s){
	x *= s;
	y *= s;
	z *= s;
}

void vec3::operator *= (const vec3 &v){
	x *= v.x;
	y *= v.y;
	z *= v.z;
}

void vec3::operator /= (const float s){
	x /= s;
	y /= s;
	z /= s;
}

void vec3::operator /= (const vec3 &v){
	x /= v.x;
	y /= v.y;
	z /= v.z;
}

vec3 operator + (const vec3 &u, const vec3 &v){
	return vec3(u.x + v.x, u.y + v.y, u.z + v.z);
}

vec3 operator + (const vec3 &v, const float s){
	return vec3(v.x + s, v.y + s, v.z + s);
}

vec3 operator - (const vec3 &u, const vec3 &v){
	return vec3(u.x - v.x, u.y - v.y, u.z - v.z);
}

vec3 operator - (const vec3 &v, const float s){
	return vec3(v.x - s, v.y - s, v.z - s);
}

vec3 operator - (const vec3 &v){
	return vec3(-v.x, -v.y, -v.z);
}

vec3 operator * (const vec3 &u, const vec3 &v){
	return vec3(u.x * v.x, u.y * v.y, u.z * v.z);
}

vec3 operator * (const float s, const vec3 &v){
	return vec3(v.x * s, v.y * s, v.z * s);
}

vec3 operator * (const vec3 &v, const float s){
	return vec3(v.x * s, v.y * s, v.z * s);
}

vec3 operator / (const vec3 &u, const vec3 &v){
	return vec3(u.x / v.x, u.y / v.y, u.z / v.z);
}

vec3 operator / (const vec3 &v, const float s){
	return vec3(v.x / s, v.y / s, v.z / s);
}

bool operator == (const vec3 &u, const vec3 &v){
	return (u.x == v.x && u.y == v.y && u.z == v.z);
}

bool operator != (const vec3 &u, const vec3 &v){
	return (u.x != v.x || u.y != v.y || u.z != v.z);
}


void vec4::operator += (const vec4 &v){
	x += v.x;
	y += v.y;
	z += v.z;
	w += v.w;
}

void vec4::operator -= (const vec4 &v){
	x -= v.x;
	y -= v.y;
	z -= v.z;
	w -= v.w;
}

void vec4::operator *= (const float s){
	x *= s;
	y *= s;
	z *= s;
	w *= s;
}

void vec4::operator *= (const vec4 &v){
	x *= v.x;
	y *= v.y;
	z *= v.z;
	w *= v.w;
}

void vec4::operator /= (const float s){
	x /= s;
	y /= s;
	z /= s;
	w /= s;
}

void vec4::operator /= (const vec4 &v){
	x /= v.x;
	y /= v.y;
	z /= v.z;
	w /= v.w;
}

vec4 operator + (const vec4 &u, const vec4 &v){
	return vec4(u.x + v.x, u.y + v.y, u.z + v.z, u.w + v.w);
}

vec4 operator + (const vec4 &v, const float s){
	return vec4(v.x + s, v.y + s, v.z + s, v.w + s);
}

vec4 operator - (const vec4 &u, const vec4 &v){
	return vec4(u.x - v.x, u.y - v.y, u.z - v.z, u.w - v.w);
}

vec4 operator - (const vec4 &v, const float s){
	return vec4(v.x - s, v.y - s, v.z - s, v.w - s);
}

vec4 operator - (const vec4 &v){
	return vec4(-v.x, -v.y, -v.z, -v.w);
}

vec4 operator * (const vec4 &u, const vec4 &v){
	return vec4(u.x * v.x, u.y * v.y, u.z * v.z, u.w * v.w);
}

vec4 operator * (const float s, const vec4 &v){
	return vec4(v.x * s, v.y * s, v.z * s, v.w * s);
}

vec4 operator * (const vec4 &v, const float s){
	return vec4(v.x * s, v.y * s, v.z * s, v.w * s);
}

vec4 operator / (const vec4 &u, const vec4 &v){
	return vec4(u.x / v.x, u.y / v.y, u.z / v.z, u.w / v.w);
}

vec4 operator / (const vec4 &v, const float s){
	return vec4(v.x / s, v.y / s, v.z / s, v.w / s);
}

bool operator == (const vec4 &u, const vec4 &v){
	return (u.x == v.x && u.y == v.y && u.z == v.z && u.w == v.w);
}

bool operator != (const vec4 &u, const vec4 &v){
	return (u.x != v.x || u.y != v.y || u.z != v.z || u.w != v.w);
}

float dot(const vec2 &u, const vec2 &v){
	return u.x * v.x + u.y * v.y;
}

float dot(const vec3 &u, const vec3 &v){
	return u.x * v.x + u.y * v.y + u.z * v.z;
}

float dot(const vec4 &u, const vec4 &v){
	return u.x * v.x + u.y * v.y + u.z * v.z + u.w * v.w;
}

float lerp(const float u, const float v, const float x){
	return u * (1 - x) + v * x;
}

vec2 lerp(const vec2 &u, const vec2 &v, const float x){
	return u * (1 - x) + v * x;
}

vec3 lerp(const vec3 &u, const vec3 &v, const float x){
	return u * (1 - x) + v * x;
}

vec4 lerp(const vec4 &u, const vec4 &v, const float x){
	return u * (1 - x) + v * x;
}

vec2 min(const vec2 u, const vec2 v){
	return vec2((u.x < v.x)? u.x : v.x, (u.y < v.y)? u.y : v.y);
}

vec2 max(const vec2 u, const vec2 v){
	return vec2((u.x > v.x)? u.x : v.x, (u.y > v.y)? u.y : v.y);
}

vec3 min(const vec3 u, const vec3 v){
	return vec3((u.x < v.x)? u.x : v.x, (u.y < v.y)? u.y : v.y, (u.z < v.z)? u.z : v.z);
}

vec3 max(const vec3 u, const vec3 v){
	return vec3((u.x > v.x)? u.x : v.x, (u.y > v.y)? u.y : v.y, (u.z > v.z)? u.z : v.z);
}

vec4 min(const vec4 u, const vec4 v){
	return vec4((u.x < v.x)? u.x : v.x, (u.y < v.y)? u.y : v.y, (u.z < v.z)? u.z : v.z, (u.w < v.w)? u.w : v.w);
}

vec4 max(const vec4 u, const vec4 v){
	return vec4((u.x > v.x)? u.x : v.x, (u.y > v.y)? u.y : v.y, (u.z > v.z)? u.z : v.z, (u.w > v.w)? u.w : v.w);
}

int signP(const int x){
	return 1 - 2 * (x < 0);
}

int signN(const int x){
	return 2 * (x > 0) - 1;
}

int sign(const int x){
	return (x > 0) - (x < 0);
}

float sign(const float x){
	return (x < 0)? -1.0f : 1.0f;
}

vec2 sign(const vec2 &v){
	return vec2((v.x < 0)? -1.0f : 1.0f, (v.y < 0)? -1.0f : 1.0f);
}

vec3 sign(const vec3 &v){
	return vec3((v.x < 0)? -1.0f : 1.0f, (v.y < 0)? -1.0f : 1.0f, (v.z < 0)? -1.0f : 1.0f);
}

vec4 sign(const vec4 &v){
	return vec4((v.x < 0)? -1.0f : 1.0f, (v.y < 0)? -1.0f : 1.0f, (v.z < 0)? -1.0f : 1.0f, (v.w < 0)? -1.0f : 1.0f);
}

float smoothstep(const float edge0, const float edge1, const float x){
	float t = saturate((x - edge0) / (edge1 - edge0));
	return t * t * (3 - 2 * t);
}

vec2 normalize(const vec2 &v){
	float invLen = 1.0f / sqrtf(v.x * v.x + v.y * v.y);
	return v * invLen;
}

vec3 normalize(const vec3 &v){
	float invLen = 1.0f / sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	return v * invLen;
}

vec4 normalize(const vec4 &v){
	float invLen = 1.0f / sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
	return v * invLen;
}

float length(const vec2 &v){
	return sqrtf(v.x * v.x + v.y * v.y);
}

float length(const vec3 &v){
	return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

float length(const vec4 &v){
	return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}

float lengthSqr(const vec2 &v){
	return v.x * v.x + v.y * v.y;
}
float lengthSqr(const vec3 &v){
	return v.x * v.x + v.y * v.y + v.z * v.z;
}
float lengthSqr(const vec4 &v){
	return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
}

vec3 cross(const vec3 &u, const vec3 &v){
	return vec3(u.y * v.z - v.y * u.z, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x);
}

vec3 rgbeToRGB(unsigned char *rgbe){
	if (rgbe[3]){
		return vec3(rgbe[0], rgbe[1], rgbe[2]) * ldexpf(1.0f, rgbe[3] - (int) (128 + 8));
	} else return vec3(0,0,0);
}

unsigned int toRGBA(const vec4 &u){
	return (int(u.x * 255) | (int(u.y * 255) << 8) | (int(u.z * 255) << 16) | (int(u.w * 255) << 24));
}

unsigned int toBGRA(const vec4 &u){
	return (int(u.z * 255) | (int(u.y * 255) << 8) | (int(u.x * 255) << 16) | (int(u.w * 255) << 24));
}

mat2 operator + (const mat2 &m, const mat2 &n){
	return mat2(
		m.elem[0][0] + n.elem[0][0], m.elem[0][1] + n.elem[0][1],
		m.elem[1][0] + n.elem[1][0], m.elem[1][1] + n.elem[1][1]);
}

mat2 operator - (const mat2 &m, const mat2 &n){
	return mat2(
		m.elem[0][0] - n.elem[0][0], m.elem[0][1] - n.elem[0][1],
		m.elem[1][0] - n.elem[1][0], m.elem[1][1] - n.elem[1][1]);
}

mat2 operator - (const mat2 &m){
	return mat2(-m.elem[0][0], -m.elem[0][1], -m.elem[1][0], -m.elem[1][1]);
}

mat2 operator * (const mat2 &m, const mat2 &n){
	return mat2(
		m.elem[0][0] * n.elem[0][0] + m.elem[0][1] * n.elem[1][0],   m.elem[0][0] * n.elem[0][1] + m.elem[0][1] * n.elem[1][1],
		m.elem[1][0] * n.elem[0][0] + m.elem[1][1] * n.elem[1][0],   m.elem[1][0] * n.elem[0][1] + m.elem[1][1] * n.elem[1][1]);
}

vec2 operator * (const mat2 &m, const vec2 &v){
	return vec2(m.elem[0][0] * v.x + m.elem[0][1] * v.y, m.elem[1][0] * v.x + m.elem[1][1] * v.y);
}

mat2 operator * (const mat2 &m, const float x){
	mat2 mat;
	mat.elem[0][0] = m.elem[0][0] * x;
	mat.elem[0][1] = m.elem[0][1] * x;
	mat.elem[1][0] = m.elem[1][0] * x;
	mat.elem[1][1] = m.elem[1][1] * x;
	return mat;
}

mat2 transpose(const mat2 &m){
	return mat2(
		m.elem[0][0], m.elem[1][0],
		m.elem[0][1], m.elem[1][1]);
}


mat3 operator + (const mat3 &m, const mat3 &n){
	return mat3(
		m.elem[0][0] + n.elem[0][0], m.elem[0][1] + n.elem[0][1], m.elem[0][2] + n.elem[0][2],
		m.elem[1][0] + n.elem[1][0], m.elem[1][1] + n.elem[1][1], m.elem[1][2] + n.elem[1][2],
		m.elem[2][0] + n.elem[2][0], m.elem[2][1] + n.elem[2][1], m.elem[2][2] + n.elem[2][2]);
}

mat3 operator - (const mat3 &m, const mat3 &n){
	return mat3(
		m.elem[0][0] - n.elem[0][0], m.elem[0][1] - n.elem[0][1], m.elem[0][2] - n.elem[0][2],
		m.elem[1][0] - n.elem[1][0], m.elem[1][1] - n.elem[1][1], m.elem[1][2] - n.elem[1][2],
		m.elem[2][0] - n.elem[2][0], m.elem[2][1] - n.elem[2][1], m.elem[2][2] - n.elem[2][2]);
}

mat3 operator - (const mat3 &m){
	return mat3(
		-m.elem[0][0], -m.elem[0][1], -m.elem[0][2],
		-m.elem[1][0], -m.elem[1][1], -m.elem[1][2],
		-m.elem[2][0], -m.elem[2][1], -m.elem[2][2]);
}

mat3 operator * (const mat3 &m, const mat3 &n){
	mat3 mat;

	for (unsigned int k = 0; k < 3; k++){
		for (unsigned int j = 0; j < 3; j++){
			mat.elem[k][j] = 0;
			for (unsigned int i = 0; i < 3; i++){
				mat.elem[k][j] += m.elem[k][i] * n.elem[i][j];
			}
		}
	}
	return mat;
}

vec3 operator * (const mat3 &m, const vec3 &v){
	return vec3(
		m.elem[0][0] * v.x + m.elem[0][1] * v.y + m.elem[0][2] * v.z,
		m.elem[1][0] * v.x + m.elem[1][1] * v.y + m.elem[1][2] * v.z,
		m.elem[2][0] * v.x + m.elem[2][1] * v.y + m.elem[2][2] * v.z);
}

mat3 operator * (const mat3 &m, const float x){
	mat3 mat;
	for (unsigned int j = 0; j < 3; j++){
		for (unsigned int i = 0; i < 3; i++){
			mat.elem[j][i] = m.elem[j][i] * x;
		}
	}
	return mat;
}

mat3 transpose(const mat3 &m){
	return mat3(
		m.elem[0][0], m.elem[1][0], m.elem[2][0],
		m.elem[0][1], m.elem[1][1], m.elem[2][1],
		m.elem[0][2], m.elem[1][2], m.elem[2][2]);
}

mat3 operator ! (const mat3 &m){
	float p0 = m.elem[1][1] * m.elem[2][2] - m.elem[2][1] * m.elem[1][2];
	float p1 = m.elem[1][2] * m.elem[2][0] - m.elem[1][0] * m.elem[2][2];
	float p2 = m.elem[1][0] * m.elem[2][1] - m.elem[1][1] * m.elem[2][0];

	float invDet = 1.0f / (m.elem[0][0] * p0 + m.elem[0][1] * p1 + m.elem[0][2] * p2);

	return mat3(
		p0, m.elem[2][1] * m.elem[0][2] - m.elem[0][1] * m.elem[2][2], m.elem[0][1] * m.elem[1][2] - m.elem[1][1] * m.elem[0][2],
		p1, m.elem[0][0] * m.elem[2][2] - m.elem[2][0] * m.elem[0][2], m.elem[1][0] * m.elem[0][2] - m.elem[0][0] * m.elem[1][2],
		p2, m.elem[2][0] * m.elem[0][1] - m.elem[0][0] * m.elem[2][1], m.elem[0][0] * m.elem[1][1] - m.elem[0][1] * m.elem[1][0]) * invDet;
}


void mat4::translate(const vec3 &v){
	elem[0][3] += (v.x * elem[0][0] + v.y * elem[0][1] + v.z * elem[0][2]);
	elem[1][3] += (v.x * elem[1][0] + v.y * elem[1][1] + v.z * elem[1][2]);
	elem[2][3] += (v.x * elem[2][0] + v.y * elem[2][1] + v.z * elem[2][2]);
	elem[3][3] += (v.x * elem[3][0] + v.y * elem[3][1] + v.z * elem[3][2]);
}



mat4 operator + (const mat4 &m, const mat4 &n){
	mat4 mat;
	for (unsigned int i = 0; i < 16; i++){
		((float *) &mat)[i] = ((float *) &m)[i] + ((float *) &n)[i];
	}
	return mat;
}

mat4 operator - (const mat4 &m, const mat4 &n){
	mat4 mat;
	for (unsigned int i = 0; i < 16; i++){
		((float *) &mat)[i] = ((float *) &m)[i] - ((float *) &n)[i];
	}
	return mat;
}

mat4 operator - (const mat4 &m){
	mat4 mat;
	for (unsigned int i = 0; i < 16; i++){
		((float *) &mat)[i] = -((float *) &m)[i];
	}
	return mat;
}

mat4 operator * (const mat4 &m, const mat4 &n){
	mat4 mat;
	for (unsigned int k = 0; k < 4; k++){
		for (unsigned int j = 0; j < 4; j++){
			mat.elem[k][j] = 0;
			for (unsigned int i = 0; i < 4; i++){
				mat.elem[k][j] += m.elem[k][i] * n.elem[i][j];
			}
		}
	}
	return mat;
}

vec4 operator * (const mat4 &m, const vec4 &v){
	return vec4(
		m.elem[0][0] * v.x + m.elem[0][1] * v.y + m.elem[0][2] * v.z + m.elem[0][3] * v.w,
		m.elem[1][0] * v.x + m.elem[1][1] * v.y + m.elem[1][2] * v.z + m.elem[1][3] * v.w,
		m.elem[2][0] * v.x + m.elem[2][1] * v.y + m.elem[2][2] * v.z + m.elem[2][3] * v.w,
		m.elem[3][0] * v.x + m.elem[3][1] * v.y + m.elem[3][2] * v.z + m.elem[3][3] * v.w);
}

mat4 operator * (const mat4 &m, const float x){
	mat4 mat;
	for (unsigned int j = 0; j < 4; j++){
		for (unsigned int i = 0; i < 4; i++){
			mat.elem[j][i] = m.elem[j][i] * x;
		}
	}
	return mat;
}

mat4 transpose(const mat4 &m){
	return mat4(
		m.elem[0][0], m.elem[1][0], m.elem[2][0], m.elem[3][0],
		m.elem[0][1], m.elem[1][1], m.elem[2][1], m.elem[3][1],
		m.elem[0][2], m.elem[1][2], m.elem[2][2], m.elem[3][2],
		m.elem[0][3], m.elem[1][3], m.elem[2][3], m.elem[3][3]);
}

mat4 operator ! (const mat4 &m){
	mat4 mat;

	float p00 = m.elem[2][2] * m.elem[3][3];
	float p01 = m.elem[3][2] * m.elem[2][3];
	float p02 = m.elem[1][2] * m.elem[3][3];
	float p03 = m.elem[3][2] * m.elem[1][3];
	float p04 = m.elem[1][2] * m.elem[2][3];
	float p05 = m.elem[2][2] * m.elem[1][3];
	float p06 = m.elem[0][2] * m.elem[3][3];
	float p07 = m.elem[3][2] * m.elem[0][3];
	float p08 = m.elem[0][2] * m.elem[2][3];
	float p09 = m.elem[2][2] * m.elem[0][3];
	float p10 = m.elem[0][2] * m.elem[1][3];
	float p11 = m.elem[1][2] * m.elem[0][3];

	mat.elem[0][0] = (p00 * m.elem[1][1] + p03 * m.elem[2][1] + p04 * m.elem[3][1]) - (p01 * m.elem[1][1] + p02 * m.elem[2][1] + p05 * m.elem[3][1]);
	mat.elem[0][1] = (p01 * m.elem[0][1] + p06 * m.elem[2][1] + p09 * m.elem[3][1]) - (p00 * m.elem[0][1] + p07 * m.elem[2][1] + p08 * m.elem[3][1]);
	mat.elem[0][2] = (p02 * m.elem[0][1] + p07 * m.elem[1][1] + p10 * m.elem[3][1]) - (p03 * m.elem[0][1] + p06 * m.elem[1][1] + p11 * m.elem[3][1]);
	mat.elem[0][3] = (p05 * m.elem[0][1] + p08 * m.elem[1][1] + p11 * m.elem[2][1]) - (p04 * m.elem[0][1] + p09 * m.elem[1][1] + p10 * m.elem[2][1]);
	mat.elem[1][0] = (p01 * m.elem[1][0] + p02 * m.elem[2][0] + p05 * m.elem[3][0]) - (p00 * m.elem[1][0] + p03 * m.elem[2][0] + p04 * m.elem[3][0]);
	mat.elem[1][1] = (p00 * m.elem[0][0] + p07 * m.elem[2][0] + p08 * m.elem[3][0]) - (p01 * m.elem[0][0] + p06 * m.elem[2][0] + p09 * m.elem[3][0]);
	mat.elem[1][2] = (p03 * m.elem[0][0] + p06 * m.elem[1][0] + p11 * m.elem[3][0]) - (p02 * m.elem[0][0] + p07 * m.elem[1][0] + p10 * m.elem[3][0]);
	mat.elem[1][3] = (p04 * m.elem[0][0] + p09 * m.elem[1][0] + p10 * m.elem[2][0]) - (p05 * m.elem[0][0] + p08 * m.elem[1][0] + p11 * m.elem[2][0]);

	float q00 = m.elem[2][0] * m.elem[3][1];
	float q01 = m.elem[3][0] * m.elem[2][1];
	float q02 = m.elem[1][0] * m.elem[3][1];
	float q03 = m.elem[3][0] * m.elem[1][1];
	float q04 = m.elem[1][0] * m.elem[2][1];
	float q05 = m.elem[2][0] * m.elem[1][1];
	float q06 = m.elem[0][0] * m.elem[3][1];
	float q07 = m.elem[3][0] * m.elem[0][1];
	float q08 = m.elem[0][0] * m.elem[2][1];
	float q09 = m.elem[2][0] * m.elem[0][1];
	float q10 = m.elem[0][0] * m.elem[1][1];
	float q11 = m.elem[1][0] * m.elem[0][1];

	mat.elem[2][0] = (q00 * m.elem[1][3] + q03 * m.elem[2][3] + q04 * m.elem[3][3]) - (q01 * m.elem[1][3] + q02 * m.elem[2][3] + q05 * m.elem[3][3]);
	mat.elem[2][1] = (q01 * m.elem[0][3] + q06 * m.elem[2][3] + q09 * m.elem[3][3]) - (q00 * m.elem[0][3] + q07 * m.elem[2][3] + q08 * m.elem[3][3]);
	mat.elem[2][2] = (q02 * m.elem[0][3] + q07 * m.elem[1][3] + q10 * m.elem[3][3]) - (q03 * m.elem[0][3] + q06 * m.elem[1][3] + q11 * m.elem[3][3]);
	mat.elem[2][3] = (q05 * m.elem[0][3] + q08 * m.elem[1][3] + q11 * m.elem[2][3]) - (q04 * m.elem[0][3] + q09 * m.elem[1][3] + q10 * m.elem[2][3]);
	mat.elem[3][0] = (q02 * m.elem[2][2] + q05 * m.elem[3][2] + q01 * m.elem[1][2]) - (q04 * m.elem[3][2] + q00 * m.elem[1][2] + q03 * m.elem[2][2]);
	mat.elem[3][1] = (q08 * m.elem[3][2] + q00 * m.elem[0][2] + q07 * m.elem[2][2]) - (q06 * m.elem[2][2] + q09 * m.elem[3][2] + q01 * m.elem[0][2]);
	mat.elem[3][2] = (q06 * m.elem[1][2] + q11 * m.elem[3][2] + q03 * m.elem[0][2]) - (q10 * m.elem[3][2] + q02 * m.elem[0][2] + q07 * m.elem[1][2]);
	mat.elem[3][3] = (q10 * m.elem[2][2] + q04 * m.elem[0][2] + q09 * m.elem[1][2]) - (q08 * m.elem[1][2] + q11 * m.elem[2][2] + q05 * m.elem[0][2]);

	return mat * (1.0f / (m.elem[0][0] * mat.elem[0][0] + m.elem[1][0] * mat.elem[0][1] + m.elem[2][0] * mat.elem[0][2] + m.elem[3][0] * mat.elem[0][3]));
}


mat2 rotate2(const float angle){
	float cosA = cosf(angle), sinA = sinf(angle);

	return mat2(
		cosA, -sinA,
		sinA,  cosA);
}

mat3 rotateX3(const float angle){
	float cosA = cosf(angle), sinA = sinf(angle);

	return mat3(
		1, 0,     0,
		0, cosA, -sinA,
		0, sinA,  cosA);
}

mat3 rotateY3(const float angle){
	float cosA = cosf(angle), sinA = sinf(angle);

	return mat3(
		cosA, 0, -sinA,
		0,    1,  0,
		sinA, 0,  cosA);
}

mat3 rotateZ3(const float angle){
	float cosA = cosf(angle), sinA = sinf(angle);

	return mat3(
		cosA, -sinA, 0,
		sinA,  cosA, 0,
		0,     0,    1);
}

mat3 rotateXY3(const float angleX, const float angleY){
	float cosX = cosf(angleX), sinX = sinf(angleX), 
		cosY = cosf(angleY), sinY = sinf(angleY);

	return mat3(
		cosY,        0,    -sinY,
		-sinX * sinY, cosX, -sinX * cosY,
		cosX * sinY, sinX,  cosX * cosY);
}

mat3 rotateZXY3(const float angleX, const float angleY, const float angleZ){
	float cosX = cosf(angleX), sinX = sinf(angleX), 
		cosY = cosf(angleY), sinY = sinf(angleY),
		cosZ = cosf(angleZ), sinZ = sinf(angleZ);

	return mat3(
		cosY * cosZ + sinX * sinY * sinZ,   -cosX * sinZ,    sinX * cosY * sinZ - sinY * cosZ,
		cosY * sinZ - sinX * sinY * cosZ,    cosX * cosZ,   -sinY * sinZ - sinX * cosY * cosZ,
		cosX * sinY,                         sinX,           cosX * cosY);
}

mat4 rotateX4(const float angle){
	float cosA = cosf(angle), sinA = sinf(angle);

	return mat4(
		1, 0,     0,    0,
		0, cosA, -sinA, 0,
		0, sinA,  cosA, 0,
		0, 0,     0,    1);
}

mat4 rotateY4(const float angle){
	float cosA = cosf(angle), sinA = sinf(angle);

	return mat4(
		cosA, 0, -sinA, 0,
		0,    1,  0,    0,
		sinA, 0,  cosA, 0,
		0,    0,  0,    1);
}

mat4 rotateZ4(const float angle){
	float cosA = cosf(angle), sinA = sinf(angle);

	return mat4(
		cosA, -sinA, 0, 0,
		sinA,  cosA, 0, 0,
		0,     0,    1, 0,
		0,     0,    0, 1);
}

mat4 rotateXY4(const float angleX, const float angleY){
	float cosX = cosf(angleX), sinX = sinf(angleX), 
		cosY = cosf(angleY), sinY = sinf(angleY);

	return mat4(
		cosY,        0,    -sinY,        0,
		-sinX * sinY, cosX, -sinX * cosY, 0,
		cosX * sinY, sinX,  cosX * cosY, 0,
		0,           0,     0,           1);
}

mat4 rotateZXY4(const float angleX, const float angleY, const float angleZ){
	float cosX = cosf(angleX), sinX = sinf(angleX), 
		cosY = cosf(angleY), sinY = sinf(angleY),
		cosZ = cosf(angleZ), sinZ = sinf(angleZ);

	return mat4(
		cosY * cosZ + sinX * sinY * sinZ,   -cosX * sinZ,    sinX * cosY * sinZ - sinY * cosZ,  0,
		cosY * sinZ - sinX * sinY * cosZ,    cosX * cosZ,   -sinY * sinZ - sinX * cosY * cosZ,  0,
		cosX * sinY,                         sinX,           cosX * cosY,                       0,
		0,                                   0,              0,                                 1);
}

mat4 translate(const float x, const float y, const float z){
	return mat4(1,0,0,x, 0,1,0,y, 0,0,1,z, 0,0,0,1);
}

mat4 translate(const vec3 &v){
	return mat4(1,0,0,v.x, 0,1,0,v.y, 0,0,1,v.z, 0,0,0,1);
}

mat4 orthoMatrix(const float left, const float right, const float top, const float bottom, const float zNear, const float zFar, const bool d3dStyle){
	float rl = right - left;
	float tb = top - bottom;
	float fn = zFar - zNear;

	mat4 mat(
		2.0f / rl, 0,         0,         -(right + left) / rl,
		0,         2.0f / tb, 0,         -(top + bottom) / tb,
		0,         0,        -2.0f / fn, -(zFar + zNear) / fn,
		0,         0,         0,         1);

	if (d3dStyle){
		mat.elem[2][2] = 0.5f * (mat.elem[2][2] + mat.elem[3][2]);
		mat.elem[2][3] = 0.5f * (mat.elem[2][3] + mat.elem[3][3]);
	}
	return mat;
}

mat4 perspectiveMatrixX(const float fov, const int width, const int height, const float zNear, const float zFar, const bool d3dStyle){
	float w = tanf(0.5f * fov);
	float h = (w * height) / width;

	mat4 mat(
		1.0f / w, 0,        0, 0,
		0,        1.0f / h, 0, 0,
		0,        0,        (zFar + zNear) / (zFar - zNear), -(2 * zFar * zNear) / (zFar - zNear),
		0,        0,        1, 0);

	if (d3dStyle){
		mat.elem[2][2] = 0.5f * (mat.elem[2][2] + mat.elem[3][2]);
		mat.elem[2][3] = 0.5f * (mat.elem[2][3] + mat.elem[3][3]);
	}
	return mat;
}

mat4 perspectiveMatrixY(const float fov, const int width, const int height, const float zNear, const float zFar, const bool d3dStyle){
	float h = tanf(0.5f * fov);
	float w = (h * width) / height;

	mat4 mat(
		1.0f / w, 0,        0, 0,
		0,        1.0f / h, 0, 0,
		0,        0,        (zFar + zNear) / (zFar - zNear), -(2 * zFar * zNear) / (zFar - zNear),
		0,        0,        1, 0);

	if (d3dStyle){
		mat.elem[2][2] = 0.5f * (mat.elem[2][2] + mat.elem[3][2]);
		mat.elem[2][3] = 0.5f * (mat.elem[2][3] + mat.elem[3][3]);
	}
	return mat;
}

mat4 frustumProjMatrix(const float left, const float right, const float bottom, const float top, const float near, const float far)
{
	float x,y,a,b,c,d;
	x = 2.0f*near / (right - left);
	y = 2.0f*near / (top - bottom);
	a = (right + left) / (right - left);
	b = (top + bottom) / (top - bottom);
	c = -(far + near) / (far - near);
	d = -(2.0f*far*near) / (far - near);

	mat4 mat(
		x, 0, a, 0,
		0, y, b, 0,
		0, 0, c, d,
		0, 0, -1, 0);

	return mat;
}

mat4 shadowMatrix(const vec3 &planeNormal, const float planeOffset, const vec3 &lightPos){
	float dist = dot(lightPos, planeNormal) + planeOffset;

	return mat4(
		dist - lightPos.x * planeNormal.x,
		- lightPos.x * planeNormal.y,
		- lightPos.x * planeNormal.z,
		- lightPos.x * planeOffset,

		- lightPos.y * planeNormal.x,
		dist - lightPos.y * planeNormal.y,
		- lightPos.y * planeNormal.z,
		- lightPos.y * planeOffset,

		- lightPos.z * planeNormal.x,
		- lightPos.z * planeNormal.y,
		dist - lightPos.z * planeNormal.z,
		- lightPos.z * planeOffset,

		- planeNormal.x,
		- planeNormal.y,
		- planeNormal.z,
		dist - planeOffset);
}

mat4 mirrorMatrix(const vec3 &planeNormal, const float planeOffset){
	return mat4(
		1 - 2 * planeNormal.x * planeNormal.x,
		- 2 * planeNormal.x * planeNormal.y,
		- 2 * planeNormal.x * planeNormal.z,
		- 2 * planeNormal.x * planeOffset,

		- 2 * planeNormal.y * planeNormal.x,
		1 - 2 * planeNormal.y * planeNormal.y,
		- 2 * planeNormal.y * planeNormal.z,
		- 2 * planeNormal.y * planeOffset,

		- 2 * planeNormal.z * planeNormal.x,
		- 2 * planeNormal.z * planeNormal.y,
		1 - 2 * planeNormal.z * planeNormal.z,
		- 2 * planeNormal.z * planeOffset,

		0, 0, 0, 1);
}

mat4 cubemapModelviewMatrix(const unsigned int face){
	switch (face){
		case 0:
			return mat4(
				0, 0, -1, 0,
				0, 1,  0, 0,
				1, 0,  0, 0,
				0, 0,  0, 1);
		case 1:
			return mat4(
				0, 0, 1, 0,
				0, 1, 0, 0,
				-1, 0, 0, 0,
				0, 0, 0, 1);
		case 2:
			return mat4(
				1, 0,  0, 0,
				0, 0, -1, 0,
				0, 1,  0, 0,
				0, 0,  0, 1);
		case 3:
			return mat4(
				1,  0, 0, 0,
				0,  0, 1, 0,
				0, -1, 0, 0,
				0,  0, 0, 1);
		case 4:
			return mat4(
				1, 0, 0, 0,
				0, 1, 0, 0,
				0, 0, 1, 0,
				0, 0, 0, 1);
		default:
			return mat4(
				-1, 0,  0, 0,
				0, 1,  0, 0,
				0, 0, -1, 0,
				0, 0,  0, 1);
	}
}

mat4 cubemapProjectionMatrix(const float zNear, const float zFar, const bool d3dStyle){
	mat4 mat(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, (zFar + zNear) / (zFar - zNear), -(2 * zFar * zNear) / (zFar - zNear),
		0, 0, 1, 0);

	if (d3dStyle){
		mat.elem[2][2] = 0.5f * (mat.elem[2][2] + mat.elem[3][2]);
		mat.elem[2][3] = 0.5f * (mat.elem[2][3] + mat.elem[3][3]);
	} else {
		mat.elem[1][1] = -1;
	}

	return mat;
}

mat4 lookAt(const vec3 &from, const vec3 &at){
	vec3 z = normalize(at - from);
	vec3 x = normalize(vec3(z.z, 0, -z.x));
	vec3 y = normalize(cross(z, x));
	mat4 mat(
		x.x, x.y, x.z, 0,
		y.x, y.y, y.z, 0,
		z.x, z.y, z.z, 0,
		0,   0,   0,   1);
	mat.translate(-from);

	return mat;
}

mat4 glToD3DProjectionMatrix(const mat4 &m){
	mat4 mat = m;

	mat.elem[2][0] = 0.5f * (mat.elem[2][0] + mat.elem[3][0]);
	mat.elem[2][1] = 0.5f * (mat.elem[2][1] + mat.elem[3][1]);
	mat.elem[2][2] = 0.5f * (mat.elem[2][2] + mat.elem[3][2]);
	mat.elem[2][3] = 0.5f * (mat.elem[2][3] + mat.elem[3][3]);

	return mat;
}

mat4 pegToFarPlane(const mat4 &m){
	mat4 mat;

	memcpy(mat.elem[0], m.elem[0], 8 * sizeof(float));
	memcpy(mat.elem[2], m.elem[3], 4 * sizeof(float));
	memcpy(mat.elem[3], m.elem[3], 4 * sizeof(float));

	return mat;
}

mat2 identity2(){
	return mat2(1,0, 0,1);
}

mat3 identity3(){
	return mat3(1,0,0, 0,1,0, 0,0,1);
}

mat4 identity4(){
	return mat4(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1);
}

mat2 scale2(const float x, const float y){
	return mat2(x,0, 0,y);
}

mat3 scale3(const float x, const float y, const float z){
	return mat3(x,0,0, 0,y,0, 0,0,z);
}

mat4 scale4(const float x, const float y, const float z){
	return mat4(x,0,0,0, 0,y,0,0, 0,0,z,0, 0,0,0,1);
}

mat4 mat4::scale(const vec3& v)
{
	return scale4(v.x, v.y, v.z);
}
mat4 mat4::translation(const vec3 &v) {
	mat4 m = identity4();
	m.translate(v);
	return m;
}

mat4 mat4::rotationX(const float angle)
{
	return rotateX4(angle);
}

mat4 mat4::rotationY(const float angle)
{
	return rotateY4(angle);
}

mat4 mat4::rotationZ(const float angle)
{
	return rotateZ4(angle);
}
mat4 mat4::identity() { return identity4(); }