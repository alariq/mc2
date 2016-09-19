#pragma once
//===========================================================================//
// File:	 Surface.hpp													 //
// Contents: DirectDraw Surface object										 //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

class DBCSSurface
{
	public:

		int m_height;
		int m_width;
		int m_offsety;
		IDirectDrawSurface7 * m_lpdds7;
		unsigned char * m_lpDynData;

		DBCSSurface(DWORD width, DWORD height);
		~DBCSSurface();
		void* GetDataPointer();
		void Clear(DWORD color);
};

