//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"mlrheaders.hpp"

//#############################################################################
//#########################    MLRLookUpLight    ################################
//#############################################################################

MLRLookUpLight::ClassData*
	MLRLookUpLight::DefaultData = NULL;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRLookUpLight::InitializeClass()
{
	Verify(!DefaultData);
	Verify(gos_GetCurrentHeap() == StaticHeap);
	DefaultData =
		new ClassData(
			MLRLookUpLightClassID,
			"MidLevelRenderer::MLRLookUpLight",
			MLRInfiniteLight::DefaultData
		);
	Check_Object(DefaultData);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRLookUpLight::TerminateClass()
{
	Check_Object(DefaultData);
	delete DefaultData;
	DefaultData = NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRLookUpLight::MLRLookUpLight() :
	MLRInfiniteLight(DefaultData)
{
	Verify(gos_GetCurrentHeap() == Heap);

	mapOrigin.x = 0.0f;
	mapOrigin.y = 0.0f;
	mapOrigin.z = 0.0f;

	mapZoneCountX = 1, mapZoneCountZ = 1;

	zoneSizeX = 1260.0f, zoneSizeZ = 1260.0f;
	one_Over_zoneSizeX = 1.0f/zoneSizeX;
	one_Over_zoneSizeZ = 1.0f/zoneSizeZ;
	
	maps = NULL;
	mapName = "";
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRLookUpLight::MLRLookUpLight(
	Stuff::MemoryStream *stream,
	int version
) :
	MLRInfiniteLight(DefaultData, stream, version)
{
	Check_Object(stream);
	Verify(gos_GetCurrentHeap() == Heap);

	*stream >> mapOrigin;
	*stream >> mapZoneCountX >> mapZoneCountZ;
	*stream >> zoneSizeX >> zoneSizeZ;
	*stream >> mapName;

	one_Over_zoneSizeX = 1.0f/zoneSizeX;
	one_Over_zoneSizeZ = 1.0f/zoneSizeZ;

	maps = new unsigned char * [mapZoneCountX * mapZoneCountZ];
	Check_Pointer(maps);

	for(int i=0;i<mapZoneCountX*mapZoneCountZ;i++)
	{
		maps[i] = new unsigned char [256*256];
		Check_Pointer(maps[i]);

		stream->ReadBytes(maps[i], 256*256);
	}

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRLookUpLight::MLRLookUpLight(Stuff::Page *page):
	MLRInfiniteLight(DefaultData, page)
{
	Check_Object(page);
	Verify(gos_GetCurrentHeap() == Heap);

	maps = NULL;

	const char *data;

	mapOrigin.x = 0.0f;
	mapOrigin.y = 0.0f;
	mapOrigin.z = 0.0f;
	if (page->GetEntry("MapOrigin", &data))
	{
		sscanf(data, "%f %f %f", &mapOrigin.x, &mapOrigin.y, &mapOrigin.z);
	}

	mapZoneCountX = 1, mapZoneCountZ = 1;
	if(page->GetEntry("MapSize", &data))
	{
		sscanf(data, "%d %d", &mapZoneCountX, &mapZoneCountZ);
	}

	zoneSizeX = 1280.0f, zoneSizeZ = 1280.0f;
	if(page->GetEntry("ZoneSize", &data))
	{
		sscanf(data, "%f %f", &zoneSizeX, &zoneSizeX);
	}
	one_Over_zoneSizeX = 1.0f/zoneSizeX;
	one_Over_zoneSizeZ = 1.0f/zoneSizeZ;

	mapName = "";
	if(page->GetEntry("MapName", &data))
	{
		mapName = data;
	}

	LoadMap();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MLRLookUpLight::~MLRLookUpLight()
{
	if(maps!=NULL)
	{
		for(int i=0;i<mapZoneCountX*mapZoneCountZ;i++)
		{
			Check_Pointer(maps[i]);
			delete [] maps[i];
			maps[i] = NULL;
		}
		Check_Pointer(maps);
		delete [] maps;
		maps = NULL;
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRLookUpLight::Save(Stuff::MemoryStream *stream)
{
	Check_Object(this);
	Check_Object(stream);

	MLRInfiniteLight::Save(stream);

	*stream << mapOrigin;
	*stream << mapZoneCountX << mapZoneCountZ;
	*stream << zoneSizeX << zoneSizeZ;
	*stream << mapName;

	for(int i=0;i<mapZoneCountX*mapZoneCountZ;i++)
	{
		stream->WriteBytes(maps[i], 256*256);
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRLookUpLight::Write(Stuff::Page *page)
{
	Check_Object(this);
	Check_Object(page);

	MLRInfiniteLight::Write(page);

	char data[256];

	sprintf(data, "%f %f %f", mapOrigin.x, mapOrigin.y, mapOrigin.z);
	Verify(strlen(data) < sizeof(data));
	page->SetEntry("MapOrigin", data);

	sprintf(data, "%d %d", mapZoneCountX, mapZoneCountZ);
	Verify(strlen(data) < sizeof(data));
	page->SetEntry("MapSize", data);

	sprintf(data, "%f %f", zoneSizeX, zoneSizeZ);
	Verify(strlen(data) < sizeof(data));
	page->SetEntry("ZoneSize", data);

	page->SetEntry("MapName", mapName);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRLookUpLight::SetMapSizeAndName(int x, int z, const char *name)
{
	Check_Object(this);

	if(maps!=NULL)
	{
		for(int i=0;i<mapZoneCountX*mapZoneCountZ;i++)
		{
			Check_Pointer(maps[i]);
			delete [] maps[i];
			maps[i] = NULL;
		}
		Check_Pointer(maps);
		delete [] maps;
		maps = NULL;
	}

	mapZoneCountX = x;
	mapZoneCountZ = z;
	mapName = name;
	
	LoadMap();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
bool
	MLRLookUpLight::LoadMap()
{
	Check_Object(this);

	Stuff::FileStream element_stream(mapName);

	unsigned char *map = new unsigned char [mapZoneCountX*mapZoneCountZ*256*256];
	Check_Pointer(map);
	element_stream.ReadBytes(map, mapZoneCountX*mapZoneCountZ*256*256);

	Verify(maps==NULL);
	maps = new unsigned char * [mapZoneCountX * mapZoneCountZ];
	Check_Pointer(maps);

	int i, j, k;
	for(j=0;j<mapZoneCountZ;j++)
	{
		for(i=0;i<mapZoneCountX;i++)
		{
			maps[j*mapZoneCountX+i] = new unsigned char [256*256];
			Check_Pointer(maps[j*mapZoneCountX+i]);
		}
	}

	unsigned char *uptr = map;
	for(j=0;j<mapZoneCountZ;j++)
	{
		for(k=0;k<256;k++)
		{
			for(i=0;i<mapZoneCountX;i++)
			{
				Mem_Copy(&maps[j*mapZoneCountX+i][k*256], uptr, 256, (256-k)*256);
				uptr += 256;


//					&map[(256*j+k)*(mapZoneCountX*256) + i*256]
			}
		}
	}

	Check_Pointer(map);
	delete[] map;

	return false;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRLookUpLight::SetLightToShapeMatrix(const LinearMatrix4D& worldToShape)
{
	Check_Object(this);

	lightToShape.Multiply(lightToWorld, worldToShape);

	shapeToWorld.Invert(worldToShape);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRLookUpLight::TestInstance()
{
	Verify(IsDerivedFrom(DefaultData));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MLRLookUpLight::LightVertex(const MLRVertexData& vertexData)
{
	UnitVector3D light_z;

	GetInShapeDirection(light_z);

	const Scalar One_Over_255 = 1.f/255.0f;

	Point3D worldPoint;

	worldPoint.Multiply(*(vertexData.point), shapeToWorld);

	Scalar prep_x = mapZoneCountX*zoneSizeX - worldPoint.x + mapOrigin.x;
	Scalar prep_z = mapZoneCountZ*zoneSizeZ - worldPoint.z + mapOrigin.z;

	int map_x = Truncate_Float_To_Word(prep_x*one_Over_zoneSizeX);
	int map_z = Truncate_Float_To_Word(prep_z*one_Over_zoneSizeZ);
	Verify(map_x>=0 && map_x<mapZoneCountX);
	Verify(map_z>=0 && map_z<mapZoneCountZ);

	int off_x = Truncate_Float_To_Word((prep_x - map_x*zoneSizeX)*256.0f*one_Over_zoneSizeX);
	int off_z = Truncate_Float_To_Word((prep_z - map_z*zoneSizeZ)*256.0f*one_Over_zoneSizeZ);
	Verify(off_x>=0 && off_x < 256);
	Verify(off_z>=0 && off_z < 256);

	Scalar mapIntensity = maps[map_z*mapZoneCountX+map_x][(off_z<<8)+off_x]*One_Over_255;

	//
	//-------------------------------------------------------------------
	// Now we reduce the light level falling on the vertex based upon the
	// cosine of the angle between light and normal
	//-------------------------------------------------------------------
	//
	Scalar cosine = -(light_z * (*vertexData.normal))*mapIntensity*intensity;



#if COLOR_AS_DWORD
	TO_DO;
#else	// COLOR_AS_DWORD
	RGBColor light_color(color);

	if (cosine > SMALL)
	{
		light_color.red *= cosine;
		light_color.green *= cosine;
		light_color.blue *= cosine;

		vertexData.color->red += light_color.red;
		vertexData.color->green += light_color.green;
		vertexData.color->blue += light_color.blue;
	}
#endif	// COLOR_AS_DWORD
}