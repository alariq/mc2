//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"mlrheaders.hpp"

//#############################################################################
//#####################    MLRIndexedPrimitiveBase    #########################
//#############################################################################

DynamicArrayOf<unsigned short>
	*MLRIndexedPrimitiveBase::clipExtraIndex;

MLRIndexedPrimitiveBase::ClassData*
	MLRIndexedPrimitiveBase::DefaultData = NULL;

unsigned short
	*indexOffset;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRIndexedPrimitiveBase::InitializeClass()
{
	Verify(!DefaultData);
	Verify(gos_GetCurrentHeap() == StaticHeap);
	DefaultData =
		new ClassData(
			MLRIndexedPrimitiveBaseClassID,
			"MidLevelRenderer::MLRIndexedPrimitiveBase",
			MLRPrimitiveBase::DefaultData,
			NULL
		);
	Register_Object(DefaultData);
	
	clipExtraIndex = new DynamicArrayOf<unsigned short> (Limits::Max_Number_Vertices_Per_Mesh);
	Register_Pointer(clipExtraIndex);

	indexOffset = new unsigned short [Limits::Max_Number_Vertices_Per_Mesh+1];
	Register_Pointer(indexOffset);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRIndexedPrimitiveBase::TerminateClass()
{
	Unregister_Pointer(clipExtraIndex);
	delete clipExtraIndex;

	Unregister_Pointer(indexOffset);
	delete [] indexOffset;

	Unregister_Object(DefaultData);
	delete DefaultData;
	DefaultData = NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRIndexedPrimitiveBase::MLRIndexedPrimitiveBase(
	ClassData *class_data,
	MemoryStream *stream,
	int version
):
	MLRPrimitiveBase(class_data, stream, version)
{
	Check_Pointer(this);
	Check_Object(stream);
	Verify(gos_GetCurrentHeap() == Heap);

	switch(version)
	{
		case 1:
		case 2:
		{
			STOP(("This class got created only after version 2 !"));
		}
		break;
		default:
		{
			MemoryStreamIO_Read(stream, &index);
		}
		break;
	}

	visibleIndexedVerticesKey = false;
	visibleIndexedVertices.SetLength(coords.GetLength());
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRIndexedPrimitiveBase::Save(MemoryStream *stream)
{
	Check_Object(this);
	Check_Object(stream);

	MLRPrimitiveBase::Save(stream);

	MemoryStreamIO_Write(stream, &index);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRIndexedPrimitiveBase::MLRIndexedPrimitiveBase(ClassData *class_data):
	MLRPrimitiveBase(class_data), index(0)
{
	Verify(gos_GetCurrentHeap() == Heap);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRIndexedPrimitiveBase::~MLRIndexedPrimitiveBase()
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRIndexedPrimitiveBase::TestInstance() const
{
	Verify(IsDerivedFrom(DefaultData));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRIndexedPrimitiveBase::InitializeDrawPrimitive(unsigned char vis, int parameter)
{
	MLRPrimitiveBase::InitializeDrawPrimitive(vis, parameter);

	gos_indices = NULL;
	numGOSIndices = -1;

	visibleIndexedVerticesKey = false;
	int i, len = visibleIndexedVertices.GetLength();

	for(i=0;i<len;i++)
	{
		visibleIndexedVertices[i] = 0;
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRIndexedPrimitiveBase::SetCoordData(
		const Point3D *data,
		int dataSize
	)
{
	Check_Object(this); 
	Check_Pointer(data);
	Verify(gos_GetCurrentHeap() == Heap);

	Verify(texCoords.GetLength() == 0 || dataSize == texCoords.GetLength());

#if defined (MAX_NUMBER_VERTICES)
	Verify(dataSize <= MAX_NUMBER_VERTICES);
#endif
	coords.AssignData(data, dataSize);

	if(index.GetLength() > 0 && visibleIndexedVertices.GetLength() != dataSize)
	{
		visibleIndexedVertices.SetLength(dataSize);
	}


}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRIndexedPrimitiveBase::SetIndexData(
		unsigned short *index_array,
		int index_count
	)
{
	Check_Object(this); 
	Check_Pointer(index_array);
	Verify(gos_GetCurrentHeap() == Heap);

	if(coords.GetLength() > 0)
	{
		visibleIndexedVertices.SetLength(coords.GetLength());
	}

#ifdef _ARMOR
	int len = coords.GetLength();
	for(int i=0;i<index_count;i++)
	{
		Verify(index_array[i] < len);
	}
#endif

	index.AssignData(index_array, index_count);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRIndexedPrimitiveBase::GetIndexData(
		unsigned short **index_array,
		int *index_count
	)
{
	Check_Object(this); 

	*index_array = index.GetData();
	*index_count = index.GetLength();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRIndexedPrimitiveBase::Transform(Matrix4D *mat)
{
	Check_Object(this);

	int i, len = coords.GetLength();

	for(i=0;i<len;i++)
	{
		(*transformedCoords)[i].Multiply(coords[i], *mat);
	}
	
	#ifdef LAB_ONLY
		Set_Statistic(TransformedVertices, TransformedVertices+len);
	#endif
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
bool
	MLRIndexedPrimitiveBase::CheckIndicies()
{
	for(int i=0;i<numGOSIndices;i++)
	{
		if(gos_indices[i] >= numGOSVertices)
		{
			STOP(("Invalid indicies detected !"));
		}
	}
	return true;
}

