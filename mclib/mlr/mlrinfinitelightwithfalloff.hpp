//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once
#define MLR_MLRINFINITELIGHTWITHFALLOFF_HPP

#include"mlr.hpp"

namespace MidLevelRenderer {

	//##########################################################################
	//##############    MLRInfiniteLightWithFalloff    #########################
	//##########################################################################

	class MLRInfiniteLightWithFalloff:
		public MLRLight
	{
	public:
		static void
			InitializeClass();
		static void
			TerminateClass();

		MLRInfiniteLightWithFalloff(ClassData *class_data=MLRInfiniteLightWithFalloff::DefaultData);
		MLRInfiniteLightWithFalloff(
			ClassData *class_data,
			Stuff::MemoryStream *stream,
			int version
		);
		MLRInfiniteLightWithFalloff(
			ClassData *class_data,
			Stuff::Page *page
		);
		~MLRInfiniteLightWithFalloff();

		void
			Save(Stuff::MemoryStream *stream);
		void
			Write(Stuff::Page *page);

		virtual void
			LightVertex(const MLRVertexData&);

		virtual LightType
			GetLightType() 
				{ Check_Object(this); return InfiniteLightWithFallOff; }

		//
		// light falloff.  The light is infinite if the GetFalloffDistance
		// function return false.  Lights default to infinite unless
		// SetFalloffDistance is called
		//
		void
			SetFalloffDistance(
				Stuff::Scalar n,
				Stuff::Scalar f
			);
		bool
			GetFalloffDistance(
				Stuff::Scalar& n,
				Stuff::Scalar& f
			);

		inline Stuff::Scalar
			GetFalloffNear()
				{ Check_Object(this); return innerRadius; }

		inline Stuff::Scalar
			GetFalloffFar()
				{ Check_Object(this); return outerRadius; }

		bool
			GetFalloff(const Stuff::Scalar& length, Stuff::Scalar& falloff)
		{
			Check_Object(this);

			Verify(length>0.0f);

			if(length <= innerRadius)
			{
				falloff = 1.0f;
				return true;
			}

			if (length >= outerRadius)
			{
				return false;
			}

			Verify(outerRadius - innerRadius > Stuff::SMALL);
			falloff = (outerRadius - length) * oneOverDistance;

			return true;
		}

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
		Stuff::Scalar
			innerRadius, outerRadius, oneOverDistance;

	};
}
