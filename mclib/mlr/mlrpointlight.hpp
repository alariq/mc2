//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once
#define MLR_MLRPOINTLIGHT_HPP

#include"mlr.hpp"
#include"mlrinfinitelightwithfalloff.hpp"

namespace MidLevelRenderer {


	//##########################################################################
	//######################    MLRPointLight    ###############################
	//##########################################################################

	class MLRPointLight:
		public MLRInfiniteLightWithFalloff
	{
	public:
		static void
			InitializeClass();
		static void
			TerminateClass();

		MLRPointLight();
		MLRPointLight(
			Stuff::MemoryStream *stream,
			int version
		);
		MLRPointLight(Stuff::Page *page);
		~MLRPointLight();

		void
			Save(Stuff::MemoryStream *stream);
		void
			Write(Stuff::Page *page);

		virtual void
			LightVertex(const MLRVertexData&);

		virtual LightType
			GetLightType() 
				{ Check_Object(this); return PointLight; }

		void
			SetLightMap(MLRLightMap *light_map);

		virtual MLRLightMap *
			GetLightMap()
				{Check_Object(this); return lightMap; }

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
		MLRLightMap *lightMap;
	};

}
