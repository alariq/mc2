//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once
#define MLR_MLRPRIMITIVEBASE_HPP

#if !defined(MLR_MLR_HPP)
#include<mlr/mlr.hpp>
#endif

#if !defined(MLR_GOSVERTEXPOOL_HPP)
#include<mlr/gosvertexpool.hpp>
#endif

namespace MidLevelRenderer {

	struct ClipPolygon2
	{
		void Init(int);
		void Destroy();

		Stuff::DynamicArrayOf<Stuff::Vector4D> coords; // [Max_Number_Vertices_Per_Polygon]
#if COLOR_AS_DWORD
		Stuff::DynamicArrayOf<DWORD> colors; //[Max_Number_Vertices_Per_Polygon];
#else
		Stuff::DynamicArrayOf<Stuff::RGBAColor>  colors; //[Max_Number_Vertices_Per_Polygon];
#endif
		Stuff::DynamicArrayOf<Vector2DScalar> texCoords; //[2*Max_Number_Vertices_Per_Polygon];
		Stuff::DynamicArrayOf<MLRClippingState> clipPerVertex; //[Max_Number_Vertices_Per_Polygon];
	};


	class MLRPrimitiveBase__ClassData;

	//##########################################################################
	//####################    MLRPrimitiveBase    ##############################
	//##########################################################################
	// this is the abstract base class for all geometry. it has contains geometry
	// with one and only one renderer state !!!

	typedef Stuff::Vector2DOf<Stuff::Scalar> Vector2DScalar;
    
    // sebi: I;ve added it but do not understand why is this needed? 
    // MLRSHape is included anyway, but it does not want to compile 
    // without forward declaration :-/
    // maybe because of crossreference (this file is included in mlrshape.hpp)
	class MLRShape;

	class _declspec(novtable) MLRPrimitiveBase:
		public Stuff::RegisteredClass
	{
		friend class MLRShape;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Initialization
	//
	public:
		static void
			InitializeClass();
		static void
			TerminateClass();
		typedef MLRPrimitiveBase__ClassData ClassData;
		static ClassData
			*DefaultData;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Constructors/Destructors
	//
	protected:
		MLRPrimitiveBase(
			ClassData *class_data,
			Stuff::MemoryStream *stream,
			int version
		);

		~MLRPrimitiveBase();

	public:
		MLRPrimitiveBase(ClassData *class_data);

		typedef MLRPrimitiveBase*
			(*Factory)(
				Stuff::MemoryStream *stream,
				int version
			);

		static MLRPrimitiveBase*
			Make(
				Stuff::MemoryStream *stream,
				int version
			);

		virtual void
			Save(Stuff::MemoryStream *stream);

	// Subprimitves are units in which this geometry is split off
	// ie. nr of polygons in a polygon mesh or number of tripstrips
	// in a tristriped mesh, every of this subprimitves has another
	// number which is type specific
	// ie. number of vertices in a polygon or number of vertices in
	// a tristrip
	// the data for the coord/color/texcoord/normal or index
	// ARE IN THIS ORDER
		virtual int
			GetNumPrimitives()
				{ Check_Object(this); return (int)lengths.GetLength(); }

		virtual void
			SetSubprimitiveLengths(
				unsigned char *length_array,
				int subprimitive_count
			) = 0;

	// returns the number of subprimitives
		void
			GetSubprimitiveLengths(unsigned char **length_array, int*);

		int
			GetSubprimitiveLength(int i) const;

	// ==============================================================

		virtual void	SetReferenceState(const MLRState& _state, int=0)
			{ Check_Object(this); referenceState = _state; };
		virtual const MLRState&
			GetReferenceState(int=0) const
				{ Check_Object(this); return referenceState; }; 
		virtual const MLRState&
			GetCurrentState(int=0) const
				{ Check_Object(this); return state; }; 

		virtual void
			CombineStates (const MLRState& master)
				{ Check_Object(this); state.Combine(master, referenceState); }; 

		int
			GetNumVertices()
				{ Check_Object(this); return (int)coords.GetLength(); }

		virtual void
			SetCoordData(
				const Stuff::Point3D *array,
				int point_count
			);
		virtual void
			GetCoordData(
				Stuff::Point3D **array,
				int *point_count
			);

		virtual void
			SetTexCoordData(
				const Vector2DScalar *array,
				int point_count
			);
		virtual void
			GetTexCoordData(
				Vector2DScalar **array,
				int *point_count
			);

	//	is to call befor clipping, parameter: camera point
		virtual int	FindBackFace(const Stuff::Point3D&) = 0;

		virtual void	Lighting(MLRLight* const*, int nrLights) = 0;

		static	void	InitializeDraw();

		virtual	void	InitializeDrawPrimitive(unsigned char, int=0);

		int	GetVisible () 
			{ Check_Object(this); return visible; }

		virtual GOSVertex*
			GetGOSVertices(int=0)
				{ Check_Object(this); return gos_vertices; }

		int
			GetNumGOSVertices()
				{ Check_Object(this); return numGOSVertices; }

		virtual GOSVertex2UV*
			GetGOSVertices2UV(int=0)
				{ Check_Object(this); return NULL; }

		int
			GetSortDataMode()
				{ Check_Object(this); return drawMode; }

		virtual bool
			CastRay(
				Stuff::Line3D *line,
				Stuff::Normal3D *normal
			);

		virtual void
			PaintMe(const Stuff::RGBAColor *paintMe) = 0;

		virtual int
			TransformAndClip(Stuff::Matrix4D*, MLRClippingState, GOSVertexPool*,bool=false) = 0;

		virtual void
			TransformNoClip(Stuff::Matrix4D*, GOSVertexPool*,bool=false) = 0;

		virtual int
			GetNumPasses()
				{ Check_Object(this); return passes; }

		virtual void
			HurtMe(const Stuff::LinearMatrix4D&, Stuff::Scalar radius)
				{ Check_Object(this); }

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// This functions using the static buffers
	//
		void
			SetClipCoord(Stuff::Point3D &point, int index)
				{
					Check_Object(this); Verify(clipExtraCoords->GetLength() > index);
					(*clipExtraCoords)[index].x = point.x;
					(*clipExtraCoords)[index].y = point.y;
					(*clipExtraCoords)[index].z = point.z;
				}
		void
			FlashClipCoords(int num)
				{
					Check_Object(this); Verify(clipExtraCoords->GetLength() > num);
					coords.SetLength(num);
					for(int i=0;i<num;i++)
					{
						coords[i].x = (*clipExtraCoords)[i].x;
						coords[i].y = (*clipExtraCoords)[i].y;
						coords[i].z = (*clipExtraCoords)[i].z;
					}
				}
		void
			SetClipTexCoord(Vector2DScalar &uvs, int index)
				{
					Check_Object(this); Verify(clipExtraTexCoords->GetLength() > index);
					Verify(	MLRState::GetHasMaxUVs() ? (uvs[0]>=-100.0 && uvs[0]<=100.0) : 1);
					Verify( MLRState::GetHasMaxUVs() ? (uvs[1]>=-100.0 && uvs[1]<=100.0) : 1);

					(*clipExtraTexCoords)[index] = uvs;
				}
		void
			FlashClipTexCoords(int num)
				{
					Check_Object(this); Verify(clipExtraTexCoords->GetLength() > num);
					texCoords.SetLength(num);
					Mem_Copy(texCoords.GetData(), clipExtraTexCoords->GetData(), sizeof(Vector2DScalar)*num, sizeof(Vector2DScalar)*num);
				}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Reference counting
	//
	public:
		void
			AttachReference()
				{Check_Object(this); ++referenceCount;}
		void
			DetachReference()
				{
					Check_Object(this); Verify(referenceCount > 0);
					if ((--referenceCount) == 0)
					{
						Unregister_Object(this);
						delete this;
					}
				}

		int
			GetReferenceCount()
				{return referenceCount;}

	protected:
		int
			referenceCount;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Testing
	//
	public:
		void
			TestInstance() const;

		virtual int
			GetSize()
		{ 
			Check_Object(this);
			size_t ret = 0;
			ret += coords.GetSize();
			ret += texCoords.GetSize();
			ret += lengths.GetSize();

			return (int)ret;
		}

		void
			GetExtend(Stuff::ExtentBox *box);

	protected:
		virtual void
			Transform(Stuff::Matrix4D*);

		static ClipPolygon2 *clipBuffer;

		unsigned char	visible;	//	primitive visibilty per frame
		unsigned char	passes;

//		int		numPrimitives;	// Number of primitives, e.g. - num quads 
//		Replaced by GetNumPrimitives

//		int		numVertices;	// number of verts for stats and vert arrays
//		Replaced by GetNumVertices

		Stuff::DynamicArrayOf<Stuff::Point3D> coords;	// Base address of coordinate list 
		Stuff::DynamicArrayOf<Vector2DScalar> texCoords;	// Base address of texture coordinate list 

		static Stuff::DynamicArrayOf<Stuff::Vector4D> *transformedCoords;

		Stuff::DynamicArrayOf<unsigned char>	lengths;	// List of strip lengths 

#if COLOR_AS_DWORD	// clipExtraColors for the future generations !!!
		static Stuff::DynamicArrayOf<DWORD> *clipExtraColors; // , Max_Number_Vertices_Per_Mesh
#else
		static Stuff::DynamicArrayOf<Stuff::RGBAColor> *clipExtraColors;  // , Max_Number_Vertices_Per_Mesh
#endif

		static Stuff::DynamicArrayOf<MLRClippingState> *clipPerVertex;  // , Max_Number_Vertices_Per_Mesh

		static Stuff::DynamicArrayOf<Stuff::Vector4D> *clipExtraCoords;  // , Max_Number_Vertices_Per_Mesh
		static Stuff::DynamicArrayOf<Vector2DScalar> *clipExtraTexCoords;  // , Max_Number_Vertices_Per_Mesh

		static Stuff::DynamicArrayOf<unsigned short> *clipExtraLength;  // , Max_Number_Primitives_Per_Frame

		MLRState	state, referenceState;

		int drawMode;

		GOSVertex *gos_vertices;
		unsigned short	numGOSVertices;
	};

	struct IcoInfo {
		int type;
		int depth;
		bool indexed;
		Stuff::Scalar radius;
		Stuff::Scalar all;
		bool onOff;
		const char *GetTypeName();
	};

    MLRShape*
		CreateIndexedIcosahedron(
			IcoInfo&,
			Stuff::DynamicArrayOf<MLRState>*
	);

	//##########################################################################
	//###################    MLRPrimitiveBase__ClassData    ####################
	//##########################################################################

	class MLRPrimitiveBase__ClassData:
		public Stuff::RegisteredClass::ClassData
	{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//
	public:
		MLRPrimitiveBase__ClassData(
			Stuff::RegisteredClass::ClassID class_id,
			const char* class_name,
			Stuff::RegisteredClass::ClassData *parent_class,
			MLRPrimitiveBase::Factory primitive_factory
		):
			RegisteredClass__ClassData(class_id, class_name, parent_class),
			primitiveFactory(primitive_factory)
				{}

		MLRPrimitiveBase::Factory 
			primitiveFactory;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//
	public:
		void
			TestInstance();
	};

	struct ClipData2
	{
		Stuff::Vector4D *coords;
#if COLOR_AS_DWORD
		DWORD *colors;
#else
		Stuff::RGBAColor *colors;
#endif
		Vector2DScalar *texCoords;
		MLRClippingState *clipPerVertex;

		unsigned short length;
	};

#if 0	// still defined in "MLRPrimitive.hpp"
	inline Stuff::Scalar
		GetBC(int nr, const Stuff::Vector4D& v4d) 
	{
		switch(nr)
		{
			case 0:
				return (v4d.w - v4d.y);
			case 1:
				return v4d.y;
			case 2:
				return (v4d.w - v4d.x);
			case 3:
				return v4d.x;
			case 4:
				return v4d.z;
			case 5:
				return (v4d.w - v4d.z);
		}
		return 0.0f;
	}

	inline void
		GetDoubleBC
			(
				int nr, 
				Stuff::Scalar& result1,
				Stuff::Scalar& result2,
				const Stuff::Vector4D& v4d1, 
				const Stuff::Vector4D& v4d2
			)
	{
		switch(nr)
		{
			case 0:
				result1 = (v4d1.w - v4d1.y);
				result2 = (v4d2.w - v4d2.y);
			break;
			case 1:
				result1 = v4d1.y;
				result2 = v4d2.y;
			break;
			case 2:
				result1 = (v4d1.w - v4d1.x);
				result2 = (v4d2.w - v4d2.x);
			break;
			case 3:
				result1 = v4d1.x;
				result2 = v4d2.x;
			break;
			case 4:
				result1 = v4d1.z;
				result2 = v4d2.z;
			break;
			case 5:
				result1 = (v4d1.w - v4d1.z);
				result2 = (v4d2.w - v4d2.z);
			break;
		}
	}

	inline Stuff::Scalar
		GetLerpFactor
			(
				int nr, 
				const Stuff::Vector4D& v4d1, 
				const Stuff::Vector4D& v4d2
			)
	{
		Stuff::Scalar result1, result2;

		switch(nr)
		{
			case 0:
				result1 = (v4d1.w - v4d1.y);
				result2 = (v4d2.w - v4d2.y);
			break;
			case 1:
				result1 = v4d1.y;
				result2 = v4d2.y;
			break;
			case 2:
				result1 = (v4d1.w - v4d1.x);
				result2 = (v4d2.w - v4d2.x);
			break;
			case 3:
				result1 = v4d1.x;
				result2 = v4d2.x;
			break;
			case 4:
				result1 = v4d1.z;
				result2 = v4d2.z;
			break;
			case 5:
				result1 = (v4d1.w - v4d1.z);
				result2 = (v4d2.w - v4d2.z);
			break;
			default:
				result1 = 0.0f;
				result2 = 0.0f;
				Abort_Program("Invalid plane number used !");
			break;
		}
		Verify(!Stuff::Close_Enough(result1, result2));
		return result1 / (result1 - result2);
	}

#endif

	inline Stuff::Scalar
		GetBC(int nr, const Stuff::Vector4D& v4d) 
	{
		switch(nr)
		{
			case 0:
				return (v4d.w - v4d.y);
			case 1:
				return v4d.y;
			case 2:
				return (v4d.w - v4d.x);
			case 3:
				return v4d.x;
			case 4:
				return v4d.z;
			case 5:
				return (v4d.w - v4d.z);
		}
		return 0.0f;
	}

	inline void
		GetDoubleBC
			(
				int nr, 
				Stuff::Scalar& result1,
				Stuff::Scalar& result2,
				const Stuff::Vector4D& v4d1, 
				const Stuff::Vector4D& v4d2
			)
	{
		switch(nr)
		{
			case 0:
				result1 = (v4d1.w - v4d1.y);
				result2 = (v4d2.w - v4d2.y);
			break;
			case 1:
				result1 = v4d1.y;
				result2 = v4d2.y;
			break;
			case 2:
				result1 = (v4d1.w - v4d1.x);
				result2 = (v4d2.w - v4d2.x);
			break;
			case 3:
				result1 = v4d1.x;
				result2 = v4d2.x;
			break;
			case 4:
				result1 = v4d1.z;
				result2 = v4d2.z;
			break;
			case 5:
				result1 = (v4d1.w - v4d1.z);
				result2 = (v4d2.w - v4d2.z);
			break;
		}
	}

	inline Stuff::Scalar
		GetLerpFactor
			(
				int nr, 
				const Stuff::Vector4D& v4d1, 
				const Stuff::Vector4D& v4d2
			)
	{
		Stuff::Scalar result1, result2;

		switch(nr)
		{
			case 0:
				result1 = (v4d1.w - v4d1.y);
				result2 = (v4d2.w - v4d2.y);
			break;
			case 1:
				result1 = v4d1.y;
				result2 = v4d2.y;
			break;
			case 2:
				result1 = (v4d1.w - v4d1.x);
				result2 = (v4d2.w - v4d2.x);
			break;
			case 3:
				result1 = v4d1.x;
				result2 = v4d2.x;
			break;
			case 4:
				result1 = v4d1.z;
				result2 = v4d2.z;
			break;
			case 5:
				result1 = (v4d1.w - v4d1.z);
				result2 = (v4d2.w - v4d2.z);
			break;
			default:
				result1 = 0.0f;
				result2 = 0.0f;
				STOP(("Invalid plane number used !"));
			break;
		}
		if(result1 == 0.0f)
		{
			return 0.0f;
		}
		Verify(!Stuff::Close_Enough(result1, result2, Stuff::SMALL*0.1f));
		return result1 / (result1 - result2);
	}
}
