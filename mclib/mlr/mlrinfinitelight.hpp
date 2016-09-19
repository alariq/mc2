//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once
#define MLR_MLRINFINITELIGHT_HPP

#include"mlr.hpp"

namespace MidLevelRenderer {

	//##########################################################################
	//####################    MLRInfiniteLight    ##############################
	//##########################################################################

	class MLRInfiniteLight:
		public MLRLight
	{
	public:
		static void
			InitializeClass();
		static void
			TerminateClass();

		MLRInfiniteLight(ClassData *class_data=DefaultData);
		MLRInfiniteLight(
			ClassData *class_data,
			Stuff::MemoryStream *stream,
			int version
		);
		MLRInfiniteLight(
			ClassData *class_data,
			Stuff::Page *page
		);
		~MLRInfiniteLight();

		virtual void
			LightVertex(const MLRVertexData&);

		virtual LightType
			GetLightType() 
				{ Check_Object(this); return InfiniteLight; }

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
