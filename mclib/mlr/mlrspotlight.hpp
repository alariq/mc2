//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once
#define MLR_MLRSPOTLIGHT_HPP

#include"mlr.hpp"
#include"mlrinfinitelightwithfalloff.hpp"

namespace MidLevelRenderer {

	//##########################################################################
	//########################    MLRSpotLight    ##############################
	//##########################################################################

	class MLRSpotLight:
		public MLRInfiniteLightWithFalloff
	{
	public:
		static void
			InitializeClass();
		static void
			TerminateClass();

		MLRSpotLight();
		MLRSpotLight(
			Stuff::MemoryStream *stream,
			int version
		);
		MLRSpotLight(Stuff::Page *page);
		~MLRSpotLight();

		void
			Save(Stuff::MemoryStream *stream);
		void
			Write(Stuff::Page *page);

		virtual LightType
			GetLightType() 
				{ Check_Object(this); return SpotLight; }


	//
	// spotlight spread.  This value is only valid if the light had falloff
	//
		bool
			GetSpreadAngle(Stuff::Radian *angle);
		void
			SetSpreadAngle(const Stuff::Radian &radian);

		void
			SetSpreadAngle(const Stuff::Degree &degree);

		Stuff::Scalar
			GetTanSpreadAngle()
				{ Check_Object(this); return tanSpreadAngle; }

		Stuff::Scalar
			GetCosSpreadAngle()
				{ Check_Object(this); return cosSpreadAngle; }

		virtual void
			LightVertex(const MLRVertexData&);

		void
			SetLightMap(MLRLightMap *light_map);

		MLRLightMap *
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

		Stuff::Radian
			spreadAngle;
		Stuff::Scalar
			tanSpreadAngle,
			cosSpreadAngle;
	};

}
