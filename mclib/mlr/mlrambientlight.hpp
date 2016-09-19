//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once
#define MLR_MLRAMBIENTLIGHT_HPP

#include"mlr.hpp"

namespace MidLevelRenderer {

	//##########################################################################
	//######################    MLRAmbientLight    #############################
	//##########################################################################

	class MLRAmbientLight:
		public MLRLight
	{
	public:
		static void
			InitializeClass();
		static void
			TerminateClass();

		MLRAmbientLight();
		MLRAmbientLight(
			Stuff::MemoryStream *stream,
			int version
		);
		MLRAmbientLight(Stuff::Page *page);
		~MLRAmbientLight();

		virtual void
			LightVertex(const MLRVertexData&);

		virtual LightType
			GetLightType() 
				{ Check_Object(this); return AmbientLight; }


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Class Data Support
	//
	public:
		static ClassData
			*DefaultData;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Testing
	//
	public:
		void
			TestInstance();

	protected:
	};

}
