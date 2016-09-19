//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once
#define MLR_MLRLIGHT_HPP

#include"mlr.hpp"

namespace MidLevelRenderer {

	class GOSVertexPool;
	class MLRLightMap;

	struct MLRVertexData {
		Stuff::Point3D *point;
#if COLOR_AS_DWORD
		DWORD *color;
#else
		Stuff::RGBAColor *color;
#endif
		Stuff::Vector3D *normal;

		int index;
	};

	//##########################################################################
	//#########################    MLRLight    #################################
	//##########################################################################

	class MLRLight:
		public Stuff::RegisteredClass
	{
	public:
		static void
			InitializeClass();
		static void
			TerminateClass();

		MLRLight(ClassData *class_data);
		MLRLight(
			ClassData *class_data,
			Stuff::MemoryStream *stream,
			int version
		);
		MLRLight(
			ClassData *class_data,
			Stuff::Page* page
		);
		~MLRLight();

		static MLRLight*
			Make(
				Stuff::MemoryStream *stream,
				int version
			);
		static MLRLight*
			Make(Stuff::Page *page);

		virtual void
			Save(Stuff::MemoryStream *stream);
		virtual void
			Write(Stuff::Page *page);

		enum LightType {
			AmbientLight = 0,
			InfiniteLight,
			InfiniteLightWithFallOff,
			PointLight,
			SpotLight,
			LookUpLight
		};

		virtual LightType
			GetLightType() = 0;

		virtual void
			LightVertex(const MLRVertexData&) = 0;

		void SetIntensity (Stuff::Scalar _int)
			{ Check_Object(this); intensity = _int; };
		Stuff::Scalar GetIntensity ()
			{ Check_Object(this); return intensity; };

		void SetColor(Stuff::RGBColor col)
			{ Check_Object(this); color = col; }
		void SetColor(Stuff::Scalar, Stuff::Scalar, Stuff::Scalar);

		void GetColor(Stuff::RGBColor& col)
			{ Check_Object(this); col = color; };
		void GetColor(Stuff::Scalar&, Stuff::Scalar&, Stuff::Scalar&);

		void GetInWorldPosition(Stuff::Point3D& pos)
			{ Check_Object(this); pos = lightToWorld; };

		void GetInWorldDirection(Stuff::UnitVector3D& dir)
			{ Check_Object(this); lightToWorld.GetLocalForwardInWorld(&dir); };

		void GetInShapePosition(Stuff::Point3D& pos)
			{ Check_Object(this); pos = lightToShape; };

		void GetInShapePosition(Stuff::LinearMatrix4D& pos)
			{ Check_Object(this); pos = lightToShape; };

		void GetInShapeDirection(Stuff::UnitVector3D& dir)
			{ Check_Object(this); lightToShape.GetLocalForwardInWorld(&dir); };

		void SetLightToWorldMatrix(const Stuff::LinearMatrix4D&);
		const Stuff::LinearMatrix4D&
			GetLightToWorldMatrix()
				{ Check_Object(this); return lightToWorld; };

		virtual
			void SetLightToShapeMatrix(const Stuff::LinearMatrix4D&);
		const Stuff::LinearMatrix4D&
			GetLightToShapeMatrix()
				{ Check_Object(this); return lightToShape; };

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Light Map Support
	//
		virtual MLRLightMap *
			GetLightMap()
				{Check_Object(this); return NULL; }

		int
			GetLightMask()
				{Check_Object(this); return lightMask;}
		void
			SetDynamicLight()
				{Check_Object(this); lightMask |= MLRState::TerrainLightingMode;}
		void
			SetStaticLight()
				{Check_Object(this); lightMask &= ~MLRState::TerrainLightingMode;}

		void
			SetName(const char *name)
				{Check_Object(this); lightName = name;}

		const char*
			GetName()
				{Check_Object(this); return lightName;}

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
			intensity;

		Stuff::RGBColor
			color;

		Stuff::LinearMatrix4D
			lightToWorld, lightToShape;

		int
			lightMask;

		Stuff::MString
			lightName;
	};

}
