#ifndef SIMPLECAMERA_H
#define SIMPLECAMERA_H
/*************************************************************************************************\
SimpleCamera.h		: Interface for the camera that renders a single object
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#include"camera.h"

class ObjectAppearance;

///////////////////////////////

class SimpleCamera : public Camera
{
public:
	SimpleCamera();
	~SimpleCamera();

	void setMech( const char* fileName, long base = 0xffff7e00, long highlight = 0xffff7e00, long h2 = 0xffbcbcbc );
	void setComponent( const char* fileName );
	void setBuilding( const char* fileName );
	void setVehicle( const char* vehicle, long base = 0xffff7e00, long highlight = 0xffff7e00, long h2 = 0xffbcbcbc );
	ObjectAppearance* getObjectAppearance() const { return pObject; }

	void init( float left, float right, float top, float bottom );

	void setObject( const char* fileName, long type, long base = 0xffff7e00, long highlight = 0xffff7e00, long h2 = 0xffbcbcbc );

	virtual void render();
	virtual void render(long xOffset, long yOffset);
	virtual long update();

	void setScale( float newScale );
	void setRotation( float rotation );
	void zoomIn( float howMuch ); // scale for things that can't 

	void setInMission (void)
	{
		bIsInMission = true;
	}

	void setColors( long base = 0xffff7e00, long highlight = 0xffff7e00, long h2 = 0xffbcbcbc );

    void pushContext() {
        oldCam = eye;
        eye = this;
    }
    void popContext() {
        eye = oldCam;
        oldCam = NULL;
    }

	float		bounds[4];


private:

	ObjectAppearance*	pObject;
	Camera*				oldCam;
	float				rotation;
	float				rotationIncrement;
	float				fudgeX;
	float				fudgeY;
	bool				bIsComponent;
	bool				bIsInMission;
	float				shapeScale;
};


#endif
