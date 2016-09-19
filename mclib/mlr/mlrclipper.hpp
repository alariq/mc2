//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once
#define MLR_MLRCLIPPER_HPP

#if !defined(MLR_MLR_HPP)
#include<mlr/mlr.hpp>
#endif

#if !defined(MLR_MLRSORTER_HPP)
#include<mlr/mlrsorter.hpp>
#endif

#if !defined(MLR_MLRLIGHT_HPP)
#include<mlr/mlrlight.hpp>
#endif

#if !defined(MLR_MLRSHAPE_HPP)
#include<mlr/mlrshape.hpp>
#endif

#if !defined(MLR_GOSVERTEXPOOL_HPP)
#include<mlr/gosvertexpool.hpp>
#endif

namespace MidLevelRenderer {

	typedef int AndyDisplay;

	class DrawShapeInformation
		#if defined(_ARMOR)
			: public Stuff::Signature
		#endif
	{
	public:
		DrawShapeInformation();

		MLRShape *shape;
		MLRState state;
		MLRClippingState clippingFlags;
		const Stuff::LinearMatrix4D *shapeToWorld;
		const Stuff::LinearMatrix4D *worldToShape;

		MLRLight *const *activeLights;
		int	nrOfActiveLights;

		void
			TestInstance() const
				{}
	};

	class DrawScalableShapeInformation : public DrawShapeInformation
	{
	public:
		DrawScalableShapeInformation();

		const Stuff::Vector3D *scaling;

		const Stuff::RGBAColor *paintMe;

		void
			TestInstance() const
				{}
	};

	class MLREffect;

	class DrawEffectInformation
		#if defined(_ARMOR)
			: public Stuff::Signature
		#endif
	{
	public:
		DrawEffectInformation();

		MLREffect *effect;

		MLRState state;
		MLRClippingState clippingFlags;
		const Stuff::LinearMatrix4D *effectToWorld;

#if 0 // for the time being no lights on the effects
		MLRLight *const *activeLights;
		int	nrOfActiveLights;
#endif
		void
			TestInstance() const
				{}
	};

	class DrawScreenQuadsInformation
		#if defined(_ARMOR)
			: public Stuff::Signature
		#endif
	{
	public:
		DrawScreenQuadsInformation();

		const Stuff::Vector4D *coords;
		const Stuff::RGBAColor *colors;
		const Vector2DScalar *texCoords;

		const bool *onOrOff;

		int
			nrOfQuads,
			currentNrOfQuads;


		MLRState state;

		void
			TestInstance() const
				{}
	};

	//##########################################################################
	//#########################    MLRClipper   #################################
	//##########################################################################

	class MLRClipper :
		public Stuff::RegisteredClass
	{
	public:
		static void
			InitializeClass();
		static void
			TerminateClass();

//	Camera gets attached to a film
		MLRClipper(AndyDisplay*, MLRSorter*);
		~MLRClipper();

//	lets begin the dance
		void StartDraw(
			const Stuff::LinearMatrix4D& camera_to_world,
			const Stuff::Matrix4D& clip_matrix,
			const Stuff::RGBAColor &fog_color,		// NOT USED ANYMORE
			const Stuff::RGBAColor *background_color,
			const MLRState &default_state,
			const Stuff::Scalar *z_value
		);

//	add another shape
		void DrawShape (DrawShapeInformation*);

//	add another shape
		void DrawScalableShape (DrawScalableShapeInformation*);

//	add screen quads
		void DrawScreenQuads (DrawScreenQuadsInformation*);

//	add another effect
		void DrawEffect (DrawEffectInformation*);
		
//	starts the action
		void RenderNow ()
			{ Check_Object(this); sorter->RenderNow(); }
    
//	clear the film
		void Clear (unsigned int flags);

		AndyDisplay* GetDisplay () const
			{ Check_Object(this); return display; };

// statistics and time
		unsigned int GetFrameRate () const
			{ Check_Object(this); return frameRate; }
		void SetTime (Stuff::Scalar t) 
			{ Check_Object(this); nowTime = t; }
		Stuff::Scalar GetTime () const
			{ Check_Object(this); return nowTime; }

		const Stuff::LinearMatrix4D&
			GetCameraToWorldMatrix()
				{Check_Object(this); return cameraToWorldMatrix;}
		const Stuff::LinearMatrix4D&
			GetWorldToCameraMatrix()
				{Check_Object(this); return worldToCameraMatrix;}
		const Stuff::Matrix4D&
			GetCameraToClipMatrix()
				{Check_Object(this); return cameraToClipMatrix;}

		void
			ResetSorter()
				{Check_Object(this); sorter->Reset();}

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
			TestInstance() const
				{};

	protected:
//	statistics and time
		unsigned int frameRate;
		Stuff::Scalar usedTime;
		Stuff::Scalar nowTime;

//	world-to-camera matrix
		Stuff::LinearMatrix4D
			worldToCameraMatrix;

		Stuff::LinearMatrix4D
			cameraToWorldMatrix;

		Stuff::Matrix4D
			cameraToClipMatrix;

		Stuff::Matrix4D
			worldToClipMatrix;

		Stuff::Point3D
			cameraPosition;

// this is the film
		AndyDisplay *display;

// this defines the sort order of the draw 
		MLRSorter *sorter;

		GOSVertexPool allVerticesToDraw;
	};

}
