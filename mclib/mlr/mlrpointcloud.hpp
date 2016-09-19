//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once
#define MLR_MLRPOINTCLOUD_HPP

#if !defined(MLR_MLR_HPP)
#include<mlr/mlr.hpp>
#endif

#if !defined(MLR_MLREFFECT_HPP)
#include<mlr/mlreffect.hpp>
#endif

namespace MidLevelRenderer {

	//##########################################################################
	//#######################    MLRPointCloud    ##############################
	//##########################################################################


	class MLRPointCloud:
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
		MLRPointCloud(int, int=0);
		~MLRPointCloud();

		void
			SetData(
				const int *count,
				const Stuff::Point3D *point_data,
				const Stuff::RGBAColor *color_data
			);
		virtual int
			GetType(int) { return type; }

		void Draw (DrawEffectInformation*, GOSVertexPool*, MLRSorter*);

		void Transform(int, int);

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
		int type;
		const int *usedNrOfVertices;
	};

}
