//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"mlrstuff.hpp"

#include"ctype.h"

#if !defined(MSTRING_HPP)
#include"mstring.hpp"
#endif

#if !defined(MLRSTATE_HPP)
#include"mlrstate.hpp"
#endif

#if !defined(MLRPOLYMESH_HPP)
#include"mlrpolymesh.hpp"
#endif

#if !defined(MLRSHAPE_HPP)
#include"mlrshape.hpp"
#endif

// #define	FORGETFUL	1

// length of longest input line in ".obj" file (including continuations) 
const int BUFFER_SIZE = 8192;

// maximum number of vertices in a single polygon 
const int FACE_SIZE = 4096;

// initial allocation size for growable arrays 
const int CHUNK = 4096;

// how many different material library files 
const int MAX_MTL_FILES = 512;

// case insensitive string equality test 
#define        SAME(_a, _b)        (stricmp(_a,_b) == 0)

// list of textures defined by Wavefront material files 
typedef struct TEX
{
	char        *name;

//	pfTexture   *texture;
//	float        su;                // u-axis texture scale factor 
//	float        sv;                // v-axis texture scale factor 
} TEX;

/* list of materials defined by Wavefront material files typedef struct MTL
{
	char		*name;
	MLRState	*state;
//	pfMaterial      *material;
//	pfTexture       *texture;
//	int              refl_mapped;
} MTL;
*/

typedef struct Polygon
{

	Polygon() {nr = 0; };
	int nr;
	int *vIndex;
	int *cIndex;
	int *nIndex;
	int *tIndex;
} Poly;

// number of input lines skipped (recognized but not processed) or unknown 
static int	numSkip			= 0;
static int	numOther		= 0;

int	numPrimitives	= 0;		
MLRState	currentState;

// function type and argument declarations 
static void loadMtl (char *fileName);
static void useMtl (char *name);

#ifdef        FORGETFUL
	static void forgetMaterials (void);
	static void forgetMaterialFiles (void);
#endif

#define        GROW(_v, _t) \
		if (_v == 0) \
		{ \
				_v ## Available = CHUNK; \
				_v = (_t *)malloc(sizeof(_t)*_v ## Available); \
		} \
		else \
		if (_v ## Count >= _v ## Available) \
		{ \
				_v ## Available *= 2; \
				_v = (_t *)realloc(_v, sizeof(_t)*_v ## Available); \
		}

MLRPolyMesh*
	BuildPolyMesh(Poly *p, unsigned int& pCount,
				  Point3D *v, Vector3D *n,
				  Vector2DOf<Scalar> *t, RGBAColor *c)
{
	if(pCount <= 0)
	{
		return NULL;
	}

	MLRPolyMesh *pm = new MLRPolyMesh;

	int *length = new int [pCount];

	pm->SetPrimitiveLength (length, pCount);

	int i, j, k, count[4] = {0, 0, 0, 0}, indexCount = 0;
	static int list[10000];
	memset(&list, 0, 10000*sizeof(int));

	for(i=0;i<pCount;i++)
	{
		length[i] = p[i].nr;
		indexCount += p[i].nr;

		for(j=0;j<p[i].nr;j++)
		{
			list[p[i].vIndex[j]] |= 1;

			if(p[i].cIndex)
			{
				list[p[i].cIndex[j]] |= 2;
			}

			if(p[i].nIndex)
			{
				list[p[i].nIndex[j]] |= 4;
			}

			if(p[i].tIndex)
			{
				list[p[i].tIndex[j]] |= 8;
			}
		}
	}

	pm->SetPrimitiveLength(length, pCount);

	for(i=0;i<10000;i++)
	{
		if(list[i] & 1)
		{
			count[0]++;
		}
		if(list[i] & 2)
		{
			count[1]++;
		}
		if(list[i] & 4)
		{
			count[2]++;
		}
		if(list[i] & 8)
		{
			count[3]++;
		}
	}
	
	Point3D *coords = new Point3D [count[0]];

	for(i=0,j=0;i<10000;i++)
	{
		if(list[i] & 1)
		{
			list[i] |= j<<4;
			coords[j] = v[i];
			j++;
		}
	}

	int *vIndex = new int [indexCount];

	for(i=0,k=0;i<pCount;i++)
	{
		for(j=0;j<p[i].nr;j++)
		{
			vIndex[k++] = list[p[i].vIndex[j]]>>4;
		}
	}

	pm->SetData(MLRPrimitive::CoordType, coords, count[0], vIndex, indexCount);

	if(p[0].nIndex)
	{
		Vector3D *normals = new Vector3D [count[2]];

		for(i=0,j=0;i<10000;i++)
		{
			list[i] &= 0xf;

			if(list[i] & 4)
			{
				list[i] |= j<<4;
				normals[j] = n[i];
				j++;
			}
		}

		int *nIndex = new int [indexCount];

		for(i=0,k=0;i<pCount;i++)
		{
			for(j=0;j<p[i].nr;j++)
			{
				nIndex[k++] = list[p[i].nIndex[j]]>>4;
			}
		}

		pm->SetData(MLRPrimitive::NormalType, normals, count[2], nIndex, indexCount);
	}

	if(p[0].cIndex)
	{
		RGBAColor *colors = new RGBAColor [count[1]];

		for(i=0,j=0;i<10000;i++)
		{
			list[i] &= 0xf;

			if(list[i] & 4)
			{
				list[i] |= j<<2;
				colors[j] = c[i];
				j++;
			}
		}

		int *cIndex = new int [indexCount];

		for(i=0,k=0;i<pCount;i++)
		{
			for(j=0;j<p[i].nr;j++)
			{
				cIndex[k++] = list[p[i].cIndex[j]]>>4;
			}
		}

		pm->SetData(MLRPrimitive::ColorType, colors, count[1], cIndex, indexCount);
	}

	if(p[0].tIndex)
	{
		Vector2DOf<Scalar> *texCoords = new Vector2DOf<Scalar> [count[3]];

		for(i=0,j=0;i<10000;i++)
		{
			list[i] &= 0xf;

			if(list[i] & 8)
			{
				list[i] |= j<<4;
				texCoords[j] = t[i];
				j++;
			}
		}

		int *tIndex = new int [indexCount];

		for(i=0,k=0;i<pCount;i++)
		{
			for(j=0;j<p[i].nr;j++)
			{
				tIndex[k++] = list[p[i].tIndex[j]]>>4;
			}
		}

		pm->SetData(MLRPrimitive::TexCoordType, texCoords, count[3], tIndex, indexCount);
	}

	for(i=0;i<pCount;i++)
	{
		delete [] p[i].vIndex;

		if(p[i].nIndex)
		{
			delete [] p[i].nIndex;
		}

		if(p[i].cIndex)
		{
			delete [] p[i].cIndex;
		}

		if(p[i].tIndex)
		{
			delete [] p[i].tIndex;
		}
	}

	pCount = 0;
	return pm;
}

//
// pfdLoadFile_obj -- Load Wavefront ".obj" files into IRIS Performer
//
MLRShape*
	MLRLoadObj (const char *fileName)
{
	
	FILE         *objFile;

	char          buffer[BUFFER_SIZE];
	char          token[BUFFER_SIZE];
	char         *next       = 0;
	char         *backslash  = 0;

	MLRShape	*shape       = new MLRShape;

	int           width      = 0;

	int           numTris    = 0;
	int           numPolys   = 0;
	int           numGroups  = 0;

//	int			  colorsDefined = 0;

// growable vertex coordinate list (X, Y, Z) 
	Point3D       *v          = 0;
	unsigned int  vCount     = 0;
	unsigned int  vAvailable = 0;

// growable vertex normal list (Nx, Ny, Nz) 
	Vector3D	*n          = 0;
	unsigned int  nCount     = 0;
	unsigned int  nAvailable = 0;

// growable texture coordinate list (S, T )
	Vector2DOf<Scalar> *t          = 0;
	unsigned int  tCount     = 0;
	unsigned int  tAvailable = 0;

// growable color list (Cr, Cg, Cb, Ca) 
	RGBAColor	*c          = 0;
	unsigned int  cCount     = 0;
	unsigned int  cAvailable = 0;

// growable polygon list
	Poly		*p			= 0;
	unsigned int  pCount     = 0;
	unsigned int  pAvailable = 0;

// tmp count vars 
	int                 i;

	if ((objFile = fopen(fileName, "rt")) == 0)
		return 0;

	Logical backface_following_polygons = False;

// read Wavefront ".obj" file 
	while (fgets(buffer, BUFFER_SIZE, objFile) != 0)
	{
// concatenate continuation lines 
		while ((backslash = strchr(buffer, '\\')) != 0)
		{
// replace backslash with space (thanks to Ken Sakai) 
			*backslash++ = ' ';
			*backslash   = '\0';

// keep reading 
			if (fgets(backslash, (int)(BUFFER_SIZE - strlen(buffer)), objFile) == 0)
				break;
		}

// find first non-"space" character in line 
		for (next = buffer; *next != '\0' && isspace(*next); next++) {};

// skip blank lines and comments ('$' is comment in "cow.obj") 
		if (*next == '\0' || *next == '#' || *next == '!' || *next == '$')
			continue;

// extract token 
		sscanf(next, "%s%n", token, &width);
			next += width;

// identify token 
		if (SAME(token, "v"))
		{
// enlarge vertex coordinate list 
			GROW(v, Point3D);

// Set default values for vertex coordinate 
			v[vCount][0] = v[vCount][1] = v[vCount][2] = 0.0f;

// read vertex coordinate into list 
			sscanf(next, "%f %f %f",
					         &v[vCount][0], &v[vCount][1], &v[vCount][2]);
					  
/* Added to correct our OWN obj format
			float f = v[vCount][PF_Y];
			v[vCount][PF_Y] = -v[vCount][PF_Z];
			v[vCount][PF_Z] = f;
*/

// advance vertex count 
			++vCount;
		} else if (SAME(token, "vn"))
		{
// enlarge vertex normal list 
			GROW(n, Vector3D);

// Set default values for vertex normal 
			n[nCount][0] = n[nCount][1] = n[nCount][2] = 0.0f;

// read vertex normal into list 
			sscanf(next, "%f %f %f",
					      &n[nCount][0], &n[nCount][1], &n[nCount][2]);

/*
			float f = n[nCount][PF_Y];
			n[nCount][PF_Y] = -n[nCount][PF_Z];
			n[nCount][PF_Z] = f;
*/

// advance normal count 
			++nCount;
		} else if (SAME(token, "vt"))
		{
// enlarge texture coordinate list 
			GROW(t, Vector2DOf<Scalar>);

// Set default values for vertex normal 
			t[tCount][0] = t[tCount][1] = 0.0f;

// read texture coordinate into list 
			sscanf(next, "%f %f", &t[tCount][0], &t[tCount][1]);

// advance texture coordinate count 
			++tCount;
		} else if (SAME(token, "vc"))
		{
			static Scalar r, g, b, a;
// enlarge color list
			GROW(c, RGBAColor);

// Set default values for color
			r = g = b = 0.0f;
			a = 1.0f;

// read color into list
			sscanf(next, "%f %f %f %f", &r, &g, &b, &a);

			c[cCount] = RGBAColor(r, g, b, a);

// advance color count
			++cCount;
		} else if (SAME(token, "g"))
		{
			++numGroups;

			backface_following_polygons = (strstr(next, " bf_") != 0);
			
			//DEBUG_STREAM << "'g " << next << "' bf=" << 
			//	backface_following_polygons << endl;

		} else if (SAME(token, "f") || SAME(token, "fo"))
		{
			int          count;
			int          textureValid = 1;
			int          normalsValid = 1;
			int          colorsValid = 1;
			int          vi[FACE_SIZE];
			int          ti[FACE_SIZE];
			int          ni[FACE_SIZE];
			int          ci[FACE_SIZE];

			char        *slash;
			char         vertexData[256];

// parse vertex data from input buffer 
			for (count = 0; count < FACE_SIZE; count++)
			{
// read the next vertices' data packet 
				if (sscanf(next, "%s%n", vertexData, &width) != 1)
					break;

// advance next pointer past data packet ("n/n/n") 
				next += width;

// get vertex coordinate index 
				vi[count] = (int)strtol(vertexData, 0, 10);

// get texture coordinate index 
				ti[count] = 0;
				if ((slash = strchr(vertexData, '/')) == 0 ||
					  (ti[count] = (int)strtol(slash+1, 0, 10)) == 0)
					textureValid = 0;

// get vertex normal index 
				ni[count] = 0;
				if (slash == 0 || (slash = strchr(slash+1, '/')) == 0 ||
					  (ni[count] = (int)strtol(slash+1, 0, 10)) == 0)
					normalsValid = 0;

// get color index 
				ci[count] = 0;
				if (slash == 0 || (slash = strchr(slash+1, '/')) == 0 ||
					  (ci[count] = (int)strtol(slash+1, 0, 10)) == 0)
					colorsValid = 0;

// form cannonical indices:
// convert ".obj" 1-based indices to 0-based (subtract 1)
// convert negative indices to positive (count from 0)
					       
				if (vi[count] >= 0)
					vi[count] -= 1;
				else
					vi[count]  += vCount;

				if (ti[count] >= 0)
					ti[count] -= 1;
				else
					ti[count]  += tCount;

				if (ni[count] >= 0)
					ni[count] -= 1;
				else
					ni[count]  += nCount;

				if (ci[count] >= 0)
					ci[count] -= 1;
				else
					ci[count]  += cCount;
			}

			if(count > 2)
			{
// enlarge polygon list 
				GROW(p, Poly);

				p[pCount].nr = count;
				p[pCount].vIndex = new int [p[pCount].nr];

// Setup vertex position  information
				for (i = 0; i < count; i++)
				{
					p[pCount].vIndex[i] = vi[i];
				}

// Setup normal vector information 
				if (normalsValid)
				{
					for (i = 0; i < count; i++)
					{
						p[pCount].nIndex[i] = ni[i];
					}
				}

// Setup color information 
				if (colorsValid)
				{
					p[pCount].cIndex = new int [p[pCount].nr];
					for (i = 0; i < count; i++)
					{
						p[pCount].cIndex[i] = ci[i];
					}
				}

// Setup texture coordinates information 
				if (textureValid)
				{
					p[pCount].tIndex = new int [p[pCount].nr];
					for (i = 0; i < count; i++)
					{
						p[pCount].tIndex[i] = ti[i];
					}
				}

				++pCount;

				numTris += count - 2;
				numPolys++;
			}
		} else if (SAME(token, "usemtl"))
		{
			if(numPrimitives > 0)
			{
				MLRPolyMesh *pm = BuildPolyMesh(p, pCount, v, n, t, c);
				shape->Add(pm);
			}
			numPrimitives++;

			char        mtlName[1024];
			sscanf(next, "%s", mtlName);
			useMtl(mtlName);
		} else if (SAME(token, "mtllib"))
		{
			char        libName[1024];
			sscanf(next, "%s", libName);

			loadMtl(libName);
		} else if (
					  SAME(token, "bevel")      ||
					  SAME(token, "bmat")       ||
					  SAME(token, "bsp")        ||
					  SAME(token, "bzp")        ||
					  SAME(token, "c_interp")   ||
					  SAME(token, "cdc")        ||
					  SAME(token, "con")        ||
					  SAME(token, "cstype")     ||
					  SAME(token, "ctech")      ||
					  SAME(token, "curv")       ||
					  SAME(token, "curv2")      ||
					  SAME(token, "d_interp")   ||
					  SAME(token, "deg")        ||
					  SAME(token, "end")        ||
					  SAME(token, "hole")       ||
					  SAME(token, "l")          ||
					  SAME(token, "lod")        ||
					  SAME(token, "maplib")     ||
					  SAME(token, "mg")         ||
					  SAME(token, "o")          ||
					  SAME(token, "p")          ||
					  SAME(token, "param")      ||
					  SAME(token, "parm")       ||
					  SAME(token, "res")        ||
					  SAME(token, "s")          ||
					  SAME(token, "scrv")       ||
					  SAME(token, "shadow_obj") ||
					  SAME(token, "sp")         ||
					  SAME(token, "stech")      ||
					  SAME(token, "step")       ||
					  SAME(token, "surf")       ||
					  SAME(token, "trace_obj")  ||
					  SAME(token, "trim")       ||
					  SAME(token, "usemap")     ||
					  SAME(token, "vp"))
		{
			++numSkip;
		}
		else
		{
			fprintf(stderr, "unrecognized: %s", buffer);
			++numOther;
		}
	}

// close Wavefront ".obj" file 
	fclose(objFile);

	MLRPolyMesh *pm = BuildPolyMesh(p, pCount, v, n, t, c);
	shape->Add(pm);

// release dynamically allocated vertex, normal, and texture data 
	if (v != 0) free(v);
	if (n != 0) free(n);
	if (t != 0) free(t);
	if (c != 0) free(c);
	if (p != 0) free(p);

//  note -- if you define this, then material files will be read anew
//         for every file that references them. could be slow for big
//         material files and many models. on the other hand, if you
//         don't define it then the lists will linger after the files
//         have all been loaded -- wasting space.
		 
#ifdef        FORGETFUL
// discard material info 
	forgetMaterials();
	forgetMaterialFiles();
#endif

// print statistics 
	//pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfdLoadFile_obj: %s", fileName);
	fprintf(stdout, "  Input Data:");
	if (numSkip != 0)
		fprintf(stdout, "    Skipped tokens:     %8ld", numSkip);
	if (numOther != 0)
		fprintf(stdout, "    Unrecognized tokens:%8ld", numOther);
	if (numGroups != 0)
		fprintf(stdout, "    Groups processed:   %8ld", numGroups);
	if (vCount != 0)
		fprintf(stdout, "    Vertex coordinates: %8ld", vCount);
	if (nCount != 0)
		fprintf(stdout, "    Vertex normals:     %8ld", nCount);
	if (tCount != 0)
		fprintf(stdout, "    Texture coordinates:%8ld", tCount);
	if (cCount != 0)
		fprintf(stdout, "    Vertex colors:      %8ld", cCount);
	if (numTris != 0)
		fprintf(stdout, "    Input polygons:     %8ld", numPolys);
	if (numTris != 0)
		fprintf(stdout, "    Total triangles:    %8ld", numTris);

	return shape;
}

// ***
// ***        P R O C E S S    M A T E R I A L    F I L E S
// **

const int MTL_NOT_DEFINED    = 0x0001;
const int MTL_HAS_GEOSTATE   = 0x0002;
const int MTL_HAS_AMBIENT    = 0x0004;
const int MTL_HAS_DIFFUSE    = 0x0008;
const int MTL_HAS_ALPHA      = 0x0010;
const int MTL_HAS_SPECULAR   = 0x0020;
const int MTL_HAS_SHININESS  = 0x0040;
const int MTL_HAS_TEXTURE    = 0x0080;
const int MTL_HAS_REFLECTION = 0x0100;
const int MTL_IS_TWO_SIDED   = 0x0200;

// data for a single wavefront material
 
typedef struct objMaterial
{
	char	library[1024]; // library name
	char	name[1024];    // material name 
	int		defined;               // defined fields 
	RGBColor	ambient;               // Ka field 
	RGBColor diffuse;               // Kd field 
	RGBColor specular;              // Ks field 
	float  alpha;                 // Tr field (actually, 1-Tr) 
	float  shininess;             // Ns field 
	char   texture[1024]; // texture map name 
	float  su;                    // u-axis tc scale 
	float  sv;                    // v-axis tc scale 
	char   reflect[1024]; // reflection mode 
} objMaterial;

static objMaterial        *mtlList      = 0;
static int                 mtlListCount = 0;
static int                 mtlListSize  = 0;

static void
reSetMaterial (objMaterial *m)
{
	if (m == 0)
		return;

	memset(m->library, '\0', sizeof(m->library));
	memset(m->name, '\0', sizeof(m->name));
	m->defined = 0;
	m->ambient = RGBColor ( 0.0f, 0.0f, 0.0f);
	m->diffuse = RGBColor ( 0.0f, 0.0f, 0.0f);
	m->specular = RGBColor (0.0f, 0.0f, 0.0f);
	m->alpha = 1.0f;
	m->shininess = 0.0f;
	memset(m->texture, '\0', sizeof(m->texture));
	m->su = 0.0f;
	m->sv = 0.0f;
	memset(m->reflect, '\0', sizeof(m->reflect));
}

#ifdef        PRINT_MATERIALS_AS_PARSED
static void
	printMtl (objMaterial *m)
{
	if (m == 0)
		return;

	fprintf(stdout, "  library: %s\n", m->library);
	fprintf(stdout, "     name: %s\n", m->name);
	fprintf(stdout, "  defined: 0x%x\n", m->defined);
	fprintf(stdout, "  ambient: %g %g %g\n", 
					m->ambient[0],  m->ambient[1],  m->ambient[2]);
	fprintf(stdout, "  diffuse: %g %g %g\n", 
					m->diffuse[0],  m->diffuse[1],  m->diffuse[2]);
	fprintf(stdout, " specular: %g %g %g\n", 
					m->specular[0], m->specular[1], m->specular[2]);
	fprintf(stdout, "    alpha: %g\n", m->alpha);
	fprintf(stdout, "shininess: %g\n", m->shininess);
	fprintf(stdout, "  texture: %s\n", m->texture);
	fprintf(stdout, "       su: %g\n", m->su);
	fprintf(stdout, "       sv: %g\n", m->sv);
	fprintf(stdout, "  reflect: %s\n", m->reflect);
	fprintf(stdout, "\n");
	fflush (stdout);
}
#endif

static void
rememberMaterial (objMaterial *m)
{
	int                i;

	if (m == 0)
		return;

#ifdef        PRINT_MATERIALS_AS_PARSED
	printMtl (m);
#endif

// check for "empty" definition -- don't add junk 
	if (m->name[0] == '\0' || m->defined == 0)
		return;
		
// look for name in material list -- don't add duplicates 
	for (i = 0; i < mtlListCount; i++)
	if (strcmp(m->name, mtlList[i].name) == 0)
	{
// XXX the best thing is to do a compare and warn user 
// XXX if the old and new definitions are not identical 

		if (
			//*m != mtlList[i]
			(m->defined != (mtlList[i].defined & ~MTL_HAS_GEOSTATE)) ||
			!Close_Enough(m->ambient.red, mtlList[i].ambient.red, 1e-6f) ||
			!Close_Enough(m->ambient.green, mtlList[i].ambient.green, 1e-6f) ||
			!Close_Enough(m->ambient.blue, mtlList[i].ambient.blue, 1e-6f) ||
			!Close_Enough(m->diffuse.red, mtlList[i].diffuse.red, 1e-6f) ||
			!Close_Enough(m->diffuse.green, mtlList[i].diffuse.green, 1e-6f) ||
			!Close_Enough(m->diffuse.blue, mtlList[i].diffuse.blue, 1e-6f) ||
			!Close_Enough(m->specular.red, mtlList[i].specular.red, 1e-6f) ||
			!Close_Enough(m->specular.green, mtlList[i].specular.green, 1e-6f) ||
			!Close_Enough(m->specular.blue, mtlList[i].specular.blue, 1e-6f) ||
			!Close_Enough(m->alpha, mtlList[i].alpha, 1e-6f) ||
			!Close_Enough(m->shininess, mtlList[i].shininess, 1e-6f) ||
			strcmp(m->texture, mtlList[i].texture) ||
			!Close_Enough(m->su, mtlList[i].su, 1e-6f) ||
			!Close_Enough(m->sv, mtlList[i].sv, 1e-6f) ||
			strcmp(m->reflect, mtlList[i].reflect)
		)
		{
			DEBUG_STREAM << "* Warning:  Material '" << m->name << "' in " <<
				m->library << " ignored (already defined by " << mtlList[i].library << ")" << endl;
			DEBUG_STREAM << "    " << mtlList[i].library << " " << mtlList[i].name << " " << mtlList[i].texture << endl;
			DEBUG_STREAM << "    " << m->library << " " << m->name << " " << m->texture << endl;

		}

		return;
	}
		
// grow material list 
	if (mtlListSize == 0)
	{
// create material list 
		mtlListSize = 1;
		mtlList = (objMaterial *)malloc(mtlListSize*sizeof(objMaterial));
	} else if (mtlListCount >= mtlListSize)
	{
// grow material list 
		mtlListSize *= 2;
		mtlList = (objMaterial *)realloc(mtlList, mtlListSize*sizeof(objMaterial));
	}
#ifdef        PRINT_MATERIALS_AS_PARSED
	printf("mtlListSize=%d, mtlListCount=%d, sizeof(objMaterial)=%d\n",
				mtlListSize, mtlListCount, sizeof(objMaterial));
#endif

// add material to list 
	mtlList[mtlListCount++] = *m;
}

#ifdef        FORGETFUL
static void
	forgetMaterials (void)
{
// delete list 
	if (mtlList != 0)
		pfFree(mtlList);
	mtlList = 0;
	mtlListSize = 0;
	mtlListCount = 0;
}
#endif

static void
	defineMtl (objMaterial *m)
{
#if 0
	static char        *suffix[] = 
	{
		".pfi", 
		".sgi", 
		".rgb", 
		".rgba", 
		".int", 
		".inta", 
		".bw"
	};
	static int numSuffix = sizeof(suffix)/sizeof(suffix[0]);

	pfMaterial *material   = new pfMaterial;
	pfTexture  *texture    = new pfTexture;

	material->SetColorMode (PFMTL_FRONT, PFMTL_CMODE_OFF);
	pfdMakeDefaultObject((pfObject *)material);

	if (m->defined & (MTL_HAS_AMBIENT  | MTL_HAS_DIFFUSE  | 
				MTL_HAS_SPECULAR | MTL_HAS_ALPHA))
	{
		if (m->defined & MTL_HAS_AMBIENT)
		{
			material->SetColor (PFMTL_AMBIENT, 
					                m->ambient[0], m->ambient[1], m->ambient[2]);
		}

		if (m->defined & MTL_HAS_DIFFUSE)
		{
			material->SetColor (PFMTL_DIFFUSE, 
					                m->diffuse[0], m->diffuse[1], m->diffuse[2]);
		}

		if (m->defined & MTL_HAS_SPECULAR)
		{
			material->SetColor (PFMTL_SPECULAR, 
					                m->specular[0], m->specular[1], m->specular[2]);
		}

		if (m->defined & MTL_HAS_SHININESS)
		{
			material->SetShininess (m->shininess);
		}

		if (m->defined & MTL_HAS_ALPHA)
		{
			material->SetAlpha (m->alpha);
			pfdBldrStateMode(PFSTATE_TRANSPARENCY, PFTR_HIGH_QUALITY);
		}

		pfdBldrStateAttr(PFSTATE_FRONTMTL, material);

		if (m->defined & MTL_IS_TWO_SIDED)
		{
			lm->SetTwoSide (PF_ON);
			pfdBldrStateAttr (PFSTATE_LIGHTMODEL, lm);
			pfdBldrStateMode (PFSTATE_CULLFACE, PFCF_OFF);

			pfdBldrStateAttr (PFSTATE_BACKMTL, material);
		}
	}

	if (m->defined & (MTL_HAS_TEXTURE | MTL_HAS_REFLECTION))
	{
		int                  j, foo;
		static int				comp;
		unsigned int        *image;
		char                *sp;

// convert texture name from blah.rla to blah.rgb etc. 
		for (j = 0; j < numSuffix; j++)
		{
			char         texPath[PF_MAXSTRING];

// remove ".rla" or whatever suffix from file name 
			if ((sp = strrchr(m->texture, (int)'.')) != 0)
				*sp = '\0';

// append one of the known image file extensions 
			strcat(m->texture, suffix[j]);

// see if file is locatable with this extension 
			if (pfFindFile(m->texture, texPath, R_OK))
				break;
		}

		if (j < numSuffix)
		{
			pfTexture *realTexture;

			texture->SetName (m->texture);

			texture->SetRepeat(PFTEX_WRAP, PFTEX_REPEAT);

			pfdBldrStateAttr(PFSTATE_TEXTURE, texture);

			realTexture = (pfTexture *)pfdGetBldrStateAttr(PFSTATE_TEXTURE);
			realTexture->getImage (&image, &comp, &foo, &foo, &foo);

			if (comp == 2 || comp == 4)
			{
				pfdBldrStateMode(PFSTATE_TRANSPARENCY, PFTR_HIGH_QUALITY);
			}

			if (m->defined & MTL_HAS_REFLECTION)
			{
				pfTexGen   *texturegen = new pfTexGen;

				if (strcmp(m->reflect, "sphere") == 0)
				{
					texturegen->SetMode (PF_S, PFTG_SPHERE_MAP);
					texturegen->SetMode (PF_T, PFTG_SPHERE_MAP);
				}
				pfdBldrStateAttr(PFSTATE_TEXGEN, texturegen);
				pfdBldrStateMode(PFSTATE_ENTEXGEN, PF_ON);

				pfdReSetObject((pfObject *)texturegen);
			}
		} else {
// file not found -- Set name to 0 string 
			if ((sp = strrchr(m->texture, (int)'.')) != 0)
				*sp = '\0';
			pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
					      "can't find texture \"%s\" for material \"%s\"", 
					      m->texture, m->name);
		}
	}

	if (0 == (enableSwitches & TEXTURE_ENABLE))
	{
		pfdBldrStateMode (PFSTATE_ENTEXTURE, PF_OFF);
	}
	else
	{
		pfdBldrStateMode (PFSTATE_ENTEXTURE, PF_ON);
	}

	if (0 == (enableSwitches & LIGHTING_ENABLE))
	{
		pfdBldrStateMode (PFSTATE_ENLIGHTING, PF_OFF);
	}
	else
	{
		pfdBldrStateMode (PFSTATE_ENLIGHTING, PF_ON);
	}

	pfdSaveBldrState(strdup(m->name));

	pfdReSetObject((pfObject *)material);
	pfdReSetObject((pfObject *)texture);
	pfdReSetObject((pfObject *)lm);

	pfdReSetBldrState();
#endif

	m->defined |= MTL_HAS_GEOSTATE;

	if (m->defined & MTL_HAS_ALPHA)
	{
		currentState.SetAlpha(MLRState::AlphaOnMode);
	}

	if (m->defined & MTL_HAS_TEXTURE)
	{
		currentState.SetTexture(m->texture);
	}
}

static void
	useMtl (char *name)
{
	int i;

// reSet to default state 
//	pfdReSetBldrState();

// look for name in material list 
	for (i = 0; i < mtlListCount; i++)
	{
		if (strcmp(name, mtlList[i].name) == 0)
		{
// is this a "missing" material 
			if (mtlList[i].defined & MTL_NOT_DEFINED)
			{
			} else {
// define state before first use 
				if ((mtlList[i].defined & MTL_HAS_GEOSTATE) == 0)
					defineMtl(&mtlList[i]);

// specify the state by name 
//				pfdLoadBldrState(name);
			}

			return;
		}
	}
		
// if we can't find material, then use default 
	if (i >= mtlListCount)
	{
		objMaterial missing;

// print warning once 
		fprintf(stderr, "material \"%s\" not defined", name);

// remember missing material's name and status 
		reSetMaterial(&missing);
		strcpy(missing.name, name);
		missing.defined = MTL_NOT_DEFINED;
		rememberMaterial(&missing);
	}
}

// parse wafefront material file texture definition
 
static void
	parSetexture (char *next, objMaterial *m)
{
	int                 width = 0;

// Set default texture scale factors 
	m->su = 1.0f;
	m->sv = 1.0f;

// parse texture name 
	sscanf(next, "%s%n", m->texture, &width);

// parse texture option strings 
	do
	{
		next += width;

		if (strcmp(m->texture, "-mm") == 0)
		{
			sscanf(next, "%f %f %s%n", &m->su, &m->sv, m->texture, &width);
		} else {
			if (strcmp(m->texture, "-s") == 0)
			{
				sscanf(next, "%f %f %*f %s%n", &m->su, &m->sv, m->texture, &width);
			} else {
				if (strcmp(m->texture, "-t") == 0)
				{
					sscanf(next, "%f %f %*f %s%n", &m->su, &m->sv, m->texture, &width);
				} else {
					if (strcmp(m->texture, "-type") == 0)
					  sscanf(next, "%s %s%n", m->reflect, m->texture, &width);
					else
					  break;
				}
			}
		}
	} while (1);
}

static char        *mtlFileList[MAX_MTL_FILES];
static int         mtlCount = 0;

#ifdef        FORGETFUL
static void
	forgetMaterialFiles (void)
{
	int                i;

	for (i = 0; i < mtlCount; i++)
	if (mtlFileList[i] != 0)
		free(mtlFileList[i]);

	mtlCount = 0;
}
#endif

// load a wavefront-format material file (".mtl")
 
static void
	loadMtl (char *fileName)
{
	FILE        *mtlFile;
	char         buffer[BUFFER_SIZE];
	char         token[BUFFER_SIZE];
	char        *next;
	char        *backslash;
	int          width        = 0;
	int          i;
	int          inProgress = 0;
	objMaterial  current;

// have we already loaded this file ? 
	for (i = 0; i < mtlCount; i++)
	if (strcmp(fileName, mtlFileList[i]) == 0)
	{
		//DEBUG_STREAM << "(" << mtlCount << ") " << 
		//	"Material library '" << mtlFileList[i] << "' already loaded" << endl;
// bail out 
		return;
	}

// remember file name for future reference 
	if (i < MAX_MTL_FILES)
	{
		mtlFileList[mtlCount++] = strdup(fileName);
		DEBUG_STREAM << "> Reading material library '" << mtlFileList[mtlCount-1] <<
			"' (" << mtlCount << ") " << endl;
	} else {
		fprintf(stderr, "more than %d unique mtllibs. enlarge MAX_MTL_FILES", 
					  MAX_MTL_FILES);
	}

// open file 
	if ((mtlFile = fopen(fileName, "rt")) == 0)
	{
		fprintf(stderr, "can't open material library %s", fileName);
		return;
	}

// read Wavefront ".mtl" file 
	while (fgets(buffer, BUFFER_SIZE, mtlFile) != 0)
	{
// concatenate continuation lines 
		while ((backslash = strchr(buffer, '\\')) != 0)
		if (fgets(backslash, BUFFER_SIZE - strlen(buffer), mtlFile) == 0)
			break;

// find first non-"space" character in line 
		for (next = buffer; *next != '\0' && isspace(*next); next++)  {};

// skip blank lines and comments ('$' is comment in "cow.obj") 
		if (*next == '\0' || *next == '#' || *next == '!' || *next == '$')
			continue;

// extract token 
		sscanf(next, "%s%n", token, &width);
			next += width;

// identify token 
		if (SAME(token, "newmtl"))
		{
// save material definition 
			if (inProgress)
				rememberMaterial(&current);

			reSetMaterial(&current);
			inProgress = 1;
			strcpy(current.library, mtlFileList[mtlCount-1]);
			sscanf(next, "%s", current.name);
		} else if (SAME(token, "Ka"))
		{
			sscanf(next, "%f %f %f", 
					      &current.ambient.red, 
					      &current.ambient.green, 
					      &current.ambient.blue);
			current.defined |= MTL_HAS_AMBIENT;
		} else if (SAME(token, "Kd"))
		{
			sscanf(next, "%f %f %f", 
					      &current.diffuse.red, 
					      &current.diffuse.green, 
					      &current.diffuse.blue);
			current.defined |= MTL_HAS_DIFFUSE;
		} else if (SAME(token, "Ks"))
		{
			sscanf(next, "%f %f %f", 
					      &current.specular.red, 
					      &current.specular.green, 
					      &current.specular.blue);
			current.defined |= MTL_HAS_SPECULAR;
		} else if (SAME(token, "Tr"))
		{
			float alpha = 1.0f;
			sscanf(next, "%f", &alpha);
			current.alpha = 1.0f - alpha;
			current.defined |= MTL_HAS_ALPHA;
		} else if (SAME(token, "Ns"))
		{
			sscanf(next, "%f", &current.shininess);

// clamp shininess range 
			if (current.shininess <   0.0f)
				current.shininess =   0.0f;
			else
				if (current.shininess > 128.0f)
					current.shininess = 128.0f;

			current.defined |= MTL_HAS_SHININESS;
		} else if (SAME(token, "map_Kd"))
		{
			parSetexture(next, &current);
			current.defined |= MTL_HAS_TEXTURE;
		} else if (SAME(token, "refl"))
		{
			strcpy(current.reflect, "sphere");
			parSetexture(next, &current);
			current.defined |= MTL_HAS_REFLECTION;
		} else if (
					  SAME(token, "Ni")        ||
					  SAME(token, "Tf")        ||
					  SAME(token, "bump")      ||
					  SAME(token, "d")         ||
					  SAME(token, "decal")     ||
					  SAME(token, "illum")     ||
					  SAME(token, "map_Ka")    ||
					  SAME(token, "map_Ks")    ||
					  SAME(token, "map_Ns")    ||
					  SAME(token, "map_d")     ||
					  SAME(token, "sharpness") ||
					  SAME(token, "vp"))
		{
			numSkip++;
		}
#ifndef        STRICT_OBJ_FORMAT
// indicate that this material is two-sided
				 
		else
		if (SAME(token, "TWOSIDE"))
		{
			current.defined |= MTL_IS_TWO_SIDED;
		}
#endif
			else {
			fprintf(stderr, " unrecognized: %s", buffer);
			numOther++;
		}
	}
#if 0
	DEBUG_STREAM << fileName << endl << flush;
#endif

	if (inProgress)
		rememberMaterial(&current);

// close Wavefront ".mtl" file 
	fclose(mtlFile);
}
