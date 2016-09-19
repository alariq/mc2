//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"mlrstuff.hpp"

#if !defined(MLRVERTEX_HPP)
#include"mlrvertex.hpp"
#endif

//#############################################################################
//############################    MLRVertex    ################################
//#############################################################################

MLRVertex::ClassData*
	MLRVertex::DefaultData = NULL;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRVertex::InitializeClass()
{
	Verify(!DefaultData);
	DefaultData =
		new ClassData(
			MLRVertexClassID,
			"MLRVertex",
			RegisteredClass::DefaultData
		);
	Register_Object(DefaultData);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRVertex::TerminateClass()
{
	Unregister_Object(DefaultData);
	delete DefaultData;
	DefaultData = NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRVertex::MLRVertex(MemoryStream *stream):
	RegisteredClass(DefaultData)
{ 
	Check_Pointer(this);
	Check_Object(stream);

	*stream >> used;

	*stream >> coord;
	*stream >> color;
	*stream >> normal;
	*stream >> texCoord[0];
	*stream >> texCoord[1];
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRVertex::MLRVertex():
	RegisteredClass(DefaultData)
{ 
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRVertex::MLRVertex
	(const Point3D *v, const RGBAColor *c,
	 const Vector3D *n, const Vector2DOf<Scalar> *tc
	) : RegisteredClass(DefaultData)
{
	Verify(v);

	*this = *v;
	used = 1;

	if(c)
	{
		*this = *c;
		used |= 2;
	}

	if(n)
	{
		*this = *n;
		used |= 4;
	}

	if(tc)
	{
		*this = *tc;
		used |= 8;
	}
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRVertex::MLRVertex
	(const Point3D& v, const RGBAColor& c,
	 const Vector3D& n, const Vector2DOf<Scalar>& tc
	) : RegisteredClass(DefaultData)
{
	Verify(v);

	*this = v;
	*this = c;
	*this = n;
	*this = tc;
	used = 0xf;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRVertex::~MLRVertex()
{
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRVertex*
	MLRVertex::Make(MemoryStream *stream)
{
	return new MLRVertex(stream);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRVertex::Save(MemoryStream *stream)
{
	Check_Object(this);
	Check_Object(stream);

	*stream << used;

	*stream << coord;
	*stream << color;
	*stream << normal;
	*stream << texCoord[0];
	*stream << texCoord[1];
}



