//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#if !defined(MLR_MLRVERTEX_HPP)
	#define MLR_MLRVERTEX_HPP

	#if !defined(MLR_MLRSTUFF_HPP)
#include<mlr/mlrstuff.hpp>
	#endif

	//##########################################################################
	//##########################    MLRVertex    ###############################
	//##########################################################################

	class MLRVertex :
		public RegisteredClass
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
	protected:
		MLRVertex(
			MemoryStream *stream,
			int version
		);

	public:
		MLRVertex();
		MLRVertex(const Point3D*, const RGBAColor*, const Vector3D*, const Vector2DOf<Scalar>*);
		MLRVertex(const Point3D&, const RGBAColor&, const Vector3D&, const Vector2DOf<Scalar>&);
		~MLRVertex();

		static MLRVertex*
			Make(
				MemoryStream *stream,
				int version
			);

		void
			Save(MemoryStream *stream);

		inline MLRVertex&
			operator=(const Point3D& v)
				{
					Check_Pointer(this);

//					Tell_Value(v);
					
					coord = v;
					used |= 1;

					return *this;
				}

		inline MLRVertex&
			operator=(const RGBAColor& c)
				{
					Check_Pointer(this);

//					DEBUG_STREAM << "c = <" << c.alpha << ", " << c.red << ", ";
//					DEBUG_STREAM << c.green << ", " << c.blue << ">" << endl;

					color = c;
					used |= 2;

					return *this;
				}

		inline MLRVertex&
			operator=(const Vector3D& n)
				{
					Check_Pointer(this);

					normal = n;
					used |= 4;

					return *this;
				}

		inline MLRVertex&
			operator=(const Vector2DOf<Scalar>& uv)
				{
					Check_Pointer(this);

					texCoord[0] = uv[0];
					texCoord[1] = uv[1];
					used |= 8;

					return *this;
				}

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
		int used;

		Point3D		coord;
		RGBAColor	color;
		Vector3D	normal;
		Vector2DScalar	texCoord;
	};

#endif