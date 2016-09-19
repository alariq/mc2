//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once
#define MLR_MLRNGONCLOUD_HPP

#if !defined(MLR_MLR_HPP)
#include<mlr/mlr.hpp>
#endif

#if !defined(MLR_MLREFFECT_HPP)
#include<mlr/mlreffect.hpp>
#endif

namespace MidLevelRenderer {

	//##########################################################################
	//#####################    MLRNGonCloud    #############################
	//##########################################################################


	class MLRNGonCloud:
		public MLREffect
	{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Initialization
	//
	public:
		static void
			InitializeClass();
		static void
			TerminateClass();

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Constructors/Destructors
	//
	public:
		MLRNGonCloud(int vertices, int number);
		~MLRNGonCloud();

		void
			SetData(
				const int *count,
				const Stuff::Point3D *point_data,
				const Stuff::RGBAColor *color_data
			);

		void Draw (DrawEffectInformation*, GOSVertexPool*, MLRSorter*);

		int	Clip(MLRClippingState, GOSVertexPool*);		

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
			TestInstance() const;

	protected:
		int numOfVertices;
		const int *usedNrOfNGons;

		static Stuff::DynamicArrayOf<Stuff::RGBAColor> *clipExtraColors;
		Stuff::DynamicArrayOf<Stuff::RGBAColor> specialClipColors;

		static Stuff::DynamicArrayOf<MLRClippingState> *clipPerVertex;

		static Stuff::DynamicArrayOf<Stuff::Vector4D> *clipExtraCoords;

		static Stuff::DynamicArrayOf<int> *clipExtraLength;
	};
}
