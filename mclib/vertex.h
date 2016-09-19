//---------------------------------------------------------------------------
//
// Vertex.h -- File contains class definitions for the Terrain Vertices
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef VERTEX_H
#define VERTEX_H
//---------------------------------------------------------------------------
// Include Files
#ifndef DVERTEX_H
#include"dvertex.h"
#endif

#include<stuff/stuff.hpp>

//---------------------------------------------------------------------------
// Macro Definitions
#define BOTTOMRIGHT			0
#define BOTTOMLEFT			1

#ifndef NO_ERR
#define NO_ERR				0
#endif

//---------------------------------------------------------------------------
// Classes
struct PostcompVertex
{
	//---------------------------------------------------
	// This Structure is used to store the data the 3D
	// version of the terrain uses so I don't have to 
	// recalculate it every frame!
	//
	// Replaces the pVertex pointer in Vertex Class
	
	Stuff::Vector3D		vertexNormal;		//Used for lighting

	float				elevation;			//Stored here so terrain can be locally deformed

	DWORD				textureData;		//Top word is Overlay TXM, Bottom Word is Base TXM

	DWORD				localRGBLight;		//aRGB format

	DWORD				terrainType;		//terrainTypeNumber.
	BYTE				selected;			// selection
	BYTE				water;				//Additional Storage to pull into 16 Byte Alignment
	BYTE				shadow;
	BYTE				highlighted;		//Used to highlight WHOLE FACES!!!

		float getElevation (void)
		{
			return elevation;
		}

	PostcompVertex& operator=( const PostcompVertex& src );
	PostcompVertex( const PostcompVertex& );
	PostcompVertex();
};

typedef PostcompVertex *PostcompVertexPtr;

//---------------------------------------------------------------------------
class Vertex
{
	//Data Members
	//-------------
	public:

		PostcompVertexPtr 	pVertex;			//Pointer to PostcompVertex for this Vertex
		float				vx,vy;				//Unrotated World Coordinates of Vertex
		float				px,py;				//Screen Coordinates of vertex.
		float				pz,pw;				//Depth of vertex.
		
		long				vertexNum;			//Physical Vertex Position in mapData

												//Used by new Object positioning system.
		long				blockVertex;		//What terrain block is this vertex part of.vertexNumber;		
												//What vertex number in the block

		long				posTile;			//Where are we on the tile!  Saves 24 divides per tile if overlay on tile!!!
												//Saves a mere 8 if no overlay!

		DWORD				clipInfo;			//Stores data on vertex clip information.

		DWORD				lightRGB;			//Light at this vertex.
		DWORD				fogRGB;				//Fog at this vertex.

		float				wx,wy;				//Screen Coordinates of water face if there is one!
		float				wz,ww;				//Depth of vertex for water if there is one!
		float				wAlpha;				//Used to environment Map Sky onto water.

		DWORD				calcThisFrame;		//Calced this vertex this frame?

		float				hazeFactor;			//Used to distance fog the terrain.
		
#ifdef _DEBUG
		bool				selected;			//Debug to show which triangle I am on.
#endif

	//Member Functions
	//-----------------
	public:

		void init (void)
		{
			pVertex = NULL;
			px = py = 0.0f;
			pz = pw = 0.0f;

			wx = wy = 0.0f;
			wz = ww = 0.0f;

			posTile = -1;
			
			vertexNum = -1;

			lightRGB = fogRGB = 0xffffffff;
			calcThisFrame = false;
			
		}

		Vertex (void)
		{
			init();
		}

		void destroy (void)
		{
		}

		~Vertex (void)
		{
			destroy();
		}

		long init (PostcompVertexPtr preVertex)
		{
			init();
			pVertex = preVertex;
			
			return(NO_ERR);
		}
		
		long getBlockNumber (void)
		{
			return (blockVertex>>16);
		}
		
		long getVertexNumber (void)
		{
			return (blockVertex & 0x0000ffff);
		}
};

//---------------------------------------------------------------------------
extern long numTerrainFaces;

//---------------------------------------------------------------------------
#endif
