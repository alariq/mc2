#ifndef AANIMOBJECT_H
#define AANIMOBJECT_H
/*************************************************************************************************\
aAnimObject.h			: Interface for the aAnimObject component of the GUI library.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#ifndef ASYSTEM_H
#include"asystem.h"
#endif

#ifndef AANIM_H
#include"aanim.h"
#endif

//*************************************************************************************************

/**************************************************************************************************
CLASS DESCRIPTION
aAnimObject:
**************************************************************************************************/
class aAnimObject: public aObject
{
	public:

		aAnimObject();
		virtual ~aAnimObject();
		aAnimObject& operator=( const aAnimObject& AnimObject );


		int init( FitIniFile* file, const char* blockName, DWORD neverFlush = 0);

		virtual void update();
		virtual void render();

		void begin() { animInfo.begin(); }
		void end();
		void reverseBegin() { animInfo.reverseBegin(); }

		bool isDone() { return animInfo.isDone(); }

		aAnimation		animInfo;


	private:
	
		aAnimObject( const aAnimObject& src );	

};


//*************************************************************************************************
#endif  // end of file ( aAnimObject.h )
