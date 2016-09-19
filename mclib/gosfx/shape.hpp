//==========================================================================//
// File:	 gosFX_Shape.hpp											    //
// Contents: Shape Component											    //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
//
#pragma once

#include"gosfx.hpp"
#include"singleton.hpp"
#include<mlr/mlr.hpp>

namespace MidLevelRenderer {class MLRShape;}

namespace gosFX
{
	//############################################################################
	//########################  Shape__Specification  #############################
	//############################################################################

	class Shape;

	class Shape__Specification:
		public Singleton__Specification
	{
		friend class Shape;

	//----------------------------------------------------------------------
	// Constructors/Destructors
	//
	protected:
		Shape__Specification(
			Stuff::MemoryStream *stream,
			int gfx_version
		);

	public:
		Shape__Specification(MidLevelRenderer::MLRShape *shape);
		~Shape__Specification();

		void
			Save(Stuff::MemoryStream *stream);

		static Shape__Specification*
			Make(
				Stuff::MemoryStream *stream,
				int gfx_version
			);

		void
			Copy(Shape__Specification *spec);

		void
			SetShape(MidLevelRenderer::MLRShape *shape);

	protected:
		MidLevelRenderer::MLRShape
			*m_shape;
		Stuff::Scalar
			m_radius;
	};

	//############################################################################
	//#############################  Shape  #################################
	//############################################################################

	class Shape : public Singleton
	{
	//----------------------------------------------------------------------------
	// Class Registration Support
	//
	public:
		static void InitializeClass();
		static void	TerminateClass();

		typedef Shape__Specification Specification;

	//----------------------------------------------------------------------------
	// Class Data Support
	//
	protected:
		Shape(
			Specification *spec,
			unsigned flags
		);

	public:
		static Shape*
			Make(
				Specification *spec,
				unsigned flags
			);

		Specification*
			GetSpecification()
				{
					Check_Object(this);
					return
						Cast_Object(Specification*, m_specification);
				}

		static ClassData
			*DefaultData;

	//----------------------------------------------------------------------------
	// Testing
	//
	public:
		void
			TestInstance() const;

	//----------------------------------------------------------------------------
	// API
	//
	public:
		void
			Draw(DrawInfo *info);
	};
}
