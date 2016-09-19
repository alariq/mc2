#pragma once
//===========================================================================//
// File:	 Font3D_DBCS_Storage.hpp										 //
// Contents: Representation of DBCS in 3D environment						 //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"font3d_dbcs_surface.hpp"

// right hand
typedef struct _quad
{
	gos_VERTEX v[4];
	_quad * pNext;
	DWORD texture;
} quad;

typedef struct _paneTextures
{
	_paneTextures*	pNext;
	DWORD			Handle;
	DWORD			Size;
} paneTextures;


class gos_DBCS
{
	protected:
		int m_RefCount;				// used for whether to always cache

	public:
		unsigned long
			m_height,
			m_width,
			m_latticeX,
			m_latticeY,
			m_color;

		quad *
			m_pListOfQuads;

		DBCSSurface *m_surf;
		
		paneTextures*	m_pTextures;

		gos_DBCS * m_pNext;
		gos_DBCS * m_pPrev;
		float m_FontX;
		float m_FontY;
		int m_beginFontX;
		int m_beginFontY;
		float m_doneFontX;
		float m_doneFontY;
		float m_LastFontX;
		float m_LastFontY;
		char m_msg[8192];
		bool m_bRendered;
		bool embeddedColorChange;
		float m_FontSize;
		bool m_FontBold;
		DWORD m_FontColor;

	public:
		gos_DBCS(const char * msg, int w, int h, bool cc);
		~gos_DBCS();
		void Render();
		void Update();
		DBCSSurface* GetSurface(){return m_surf;}
		int GetWidth(){return m_width;}
		int GetHeight(){return m_height;}
		void PrepareTextures();
		void Texture(DBCSSurface* surf);
		void Translate(float x, float y, float z);
		void Color(DWORD c);
		void AlphaMode( enum gos_AlphaMode mode);
		void MakeQuads();
		bool NeedClip(float x, float y);
		void ShiftCoordinates(float x, float y);
		void Fill(float r, float g, float b, float a);

		DWORD AddTexture( DWORD Size );
		void AddQuad( DWORD X, DWORD Y, DWORD Width, DWORD Height, DWORD Texture, DWORD U, DWORD V, DWORD TextureSize );

		void AddRef (void)
		{ m_RefCount++; }
		void Release (void)
		{ m_RefCount--; if (m_RefCount < 0) m_RefCount = 0;}
		int RefCount (void)
		{ return m_RefCount; }
};


