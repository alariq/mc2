//-------------------------------------------------------------------------------
//
// Multiple TG Shape Layer
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
// Replace MLR for a myriad number of reasons.
//
// Started 4/16/99
//
// FFS
//
//-------------------------------------------------------------------------------

#ifndef MSL_H
#define MSL_H
//-------------------------------------------------------------------------------
// Include Files

#ifndef TGL_H
#include"tgl.h"
#endif

#ifndef TXMMGR_H
#include"txmmgr.h"
#endif

#ifdef LINUX_BUILD
#include "platform_windows.h"
#include "platform_str.h"
#include "platform_stdlib.h"
#endif

// ARM
namespace Microsoft
{
	namespace Xna
	{
		namespace Arm
		{
			struct IProviderEngine;
		}
	}
}

//-------------------------------------------------------------------------------
// Structs used by layer.
//

class TG_MultiShape;
class TG_AnimateShape;
//-------------------------------------------------------------------------------
// The meat and Potatoes part.
// TG_TypeMultiShape
class TG_TypeMultiShape
{
	friend class TG_MultiShape;
	friend class TG_TypeShape;
	friend class TG_Shape;
	friend class TG_AnimateShape;
	
	//---------------------------------------------------------
	// This class runs a multi-shape appearance.  Anything
	// with an ASE file of more then one shape should use this
	// Class.  Uses everything from TG_Shape except parse.
	// That is local to here to get all of the pieces.  Each
	// Shape is transformed, rendered etc. in TG_Shape.
	// This will store animation information and use it, too!

	//-------------
	//Data Members
	protected:
		int                 numTG_TypeShapes;		//Number of TG_Shapes
		int                 numTextures;			//Total Textures for all shapes.
		TG_TypeNodePtr		*listOfTypeShapes;		//Memory holding all TG_TypeNodes and TypeShapes
		TG_TexturePtr		listOfTextures;			//List of texture Structures for all shapes.
		
#ifdef _DEBUG
		char				*shapeName;				//FileName of shape
#endif
		
	public:			
		Stuff::Vector3D		maxBox;					//Bounding Box Max Coords
		Stuff::Vector3D		minBox;					//Bounding Box Min Coords
		float				extentRadius;			//Bounding Sphere Coords
		
	//-----------------
	//Member Functions
	protected:

	public:
		void * operator new (size_t mySize);
		void operator delete (void * us);

		void init (void)
		{
			numTG_TypeShapes = 0;
			listOfTypeShapes = NULL;

			numTextures = 0;
			listOfTextures = NULL;

#ifdef _DEBUG
			shapeName = NULL;
#endif
			maxBox.x =
			maxBox.y =
			maxBox.z = -100000.0f;
			
			minBox.x =
			minBox.y =
			minBox.z = 100000.0f;
			
			extentRadius = 0.0f;
		}
		
		TG_TypeMultiShape (void)
		{
			init();
		}

		void destroy (void);

		~TG_TypeMultiShape (void)
		{
			destroy();
		}

		//Function returns 0 if OK.  -1 if file not found or file not ASE Format.
		//Function runs through each piece of ASE file and creates a separate
		//TG_Shape for each one.  First pass is count number of GEOMOBJECTS.
		//Second pass is load each one.
		long LoadTGMultiShapeFromASE (const char *fileName, bool forceMakeBinary = false, Microsoft::Xna::Arm::IProviderEngine * armProvider = NULL);

		//Function returns 0 if OK.  -1 if textureName is longer then nameLength-1.
		//This function digs the texture name(s) out of the ASE file so that the
		//User can load and manage them anyway they want to.
		long GetTextureName (DWORD textureNum, char *textureName, long nameLength);

		//Function returns 0 if OK.  -1 if textureNum is out of range of numTextures.
		//This function takes the gosTextureHandle passed in and assigns it to the
		//textureNum entry of the listOfTextures;
		long SetTextureHandle (DWORD textureNum, DWORD gosTextureHandle);

		//Function returns 0 if OK.  -1 if textureNum is out of range of numTextures.
		//This function takes the gosTextureHandle passed in and assigns it to the
		//textureNum entry of the listOfTextures;
		long SetTextureAlpha (DWORD textureNum, bool alphaFlag);

		DWORD GetTextureHandle (DWORD textureNum)
		{
			if (textureNum < numTextures)
				return listOfTextures[textureNum].mcTextureNodeIndex;

			return 0xffffffff;
		}

		long GetNumShapes (void)
		{
			return numTG_TypeShapes;
		}

		long GetNumTextures (void)
		{
			return numTextures;
		}

		char *GetNodeId (long shapeNum)
		{
			if ((shapeNum >= 0) && (shapeNum < numTG_TypeShapes))
				return (listOfTypeShapes[shapeNum]->getNodeId());
			
			return NULL;
		}

		Stuff::Point3D GetNodeCenter (long shapeNum)
		{
			Stuff::Point3D result;
			result.x = result.y = result.z = 0.0f;

			if ((shapeNum >= 0) && (shapeNum < numTG_TypeShapes))
				result = listOfTypeShapes[shapeNum]->GetNodeCenter();
			
			return result;
		}

		Stuff::Point3D GetNodeCenter (char * nodeId)
		{
			Stuff::Point3D result;
			result.x = result.y = result.z = 0.0f;

			for (int i=0;i<numTG_TypeShapes;i++)
			{
				if (S_stricmp(listOfTypeShapes[i]->getNodeId(),nodeId) == 0)
					result = listOfTypeShapes[i]->GetNodeCenter();
			}
			
			return result;
		}

		//This function creates an instance of the TG_MultiShape from the Type
		TG_MultiShape *CreateFrom (void);

		void SetAlphaTest (bool flag)
		{
			for (int i=0;i<numTG_TypeShapes;i++)
				listOfTypeShapes[i]->SetAlphaTest(flag);
		}

		void SetFilter (bool flag)
		{
			for (int i=0;i<numTG_TypeShapes;i++)
				listOfTypeShapes[i]->SetFilter(flag);
		}
		
		void SetLightRGBs (DWORD hPink, DWORD hGreen, DWORD hYellow)
		{
			for (int i=0;i<numTG_TypeShapes;i++)
				listOfTypeShapes[i]->SetLightRGBs(hPink,hGreen,hYellow);
		}

		long LoadBinaryCopy (const char *fileName);
		void SaveBinaryCopy (const char *fileName);
};

typedef TG_TypeMultiShape* TG_TypeMultiShapePtr;

//-------------------------------------------------------------------------------
// The meat and Potatoes part.
// TG_MultiShape
class TG_MultiShape
{
	//---------------------------------------------------------
	// This class runs a multi-shape appearance.  Anything
	// with an ASE file of more then one shape should use this
	// Class.  Uses everything from TG_Shape except parse.
	// That is local to here to get all of the pieces.  Each
	// Shape is transformed, rendered etc. in TG_Shape.
	// This will store animation information and use it, too!

	friend class TG_TypeMultiShape;
	friend class TG_Shape;
	friend class TG_TypeShape;
	friend class TG_AnimateShape;
	
	//-------------
	//Data Members
	protected:
		TG_TypeMultiShapePtr	myMultiType;			//Pointer to the type
		int                     numTG_Shapes;			//Number of TG_Shapes
		TG_ShapeRecPtr			listOfShapes;			//Memory holding all TG_ShapeRecs
		float					frameNum;				//Frame number of animation
		bool					d_useShadows;
		bool					isHudElement;
		BYTE					alphaValue;				//To fade shapes in and out
		bool					isClamped;				//So I can force a shape to clamp its textures

	//-----------------
	//Member Functions
	protected:

	public:
		void * operator new (size_t mySize);
		void operator delete (void * us);

		void init (void)
		{
			myMultiType = NULL;
			numTG_Shapes = 0;
			listOfShapes = NULL;

			frameNum = 0.0f;
			d_useShadows = true;
			
			isHudElement = false;
			
			alphaValue = 0xff;
			
			isClamped = false;
		}
		
		TG_MultiShape (void)
		{
			init();
		}

		void destroy (void);

		~TG_MultiShape (void)
		{
			destroy();
		}

		//This function sets the list of lights used by the TransformShape function
		//to light the shape.
		//Function returns 0 if lightList entries are all OK.  -1 otherwise.
		//
		long SetLightList (TG_LightPtr *lightList, DWORD nLights);

		//This function sets the fog values for the shape.  Straight fog right now.
		void SetFogRGB (DWORD fRGB);

		//This function does the actual transform math, clip checks and lighting math.
		//The matrices passed in are the translation/rotation matrix for the shape and
		//Its inverse.  Also will animate.
		//Function returns -1 if all vertex screen positions are off screen.
		//Function returns 0 if any one vertex screen position is off screen.
		//Function returns 1 is all vertex screen positions are on screen.
		// NOTE:  THIS IS NOT A RIGOROUS CLIP!!!!!!!!!
		long TransformMultiShape (Stuff::Point3D *pos, Stuff::UnitQuaternion *rot);

		//This function rotates the heirarchy from this node down.  Used for torso twists, arms, etc.
		// SHould only be called once this way.  This way is DAMNED SLOW!!!  STRICMP!  IT returns the node num
		// Call that from then on!
		long SetNodeRotation (const char *nodeName, Stuff::UnitQuaternion *rot);

		void SetNodeRotation (long nodeNum, Stuff::UnitQuaternion *rot)
		{
			if ((nodeNum >= 0) && (nodeNum < numTG_Shapes))
				listOfShapes[nodeNum].baseRotation = *rot;
		}

		//This function takes the current listOfVisibleFaces and draws them using
		//gos_DrawTriangle.
		void Render (bool refreshTextures = false, float forceZ = -1.0f);

		//This function takes the current listOfShadowTriangles and draws them using
		//gos_DrawTriangle.
		void RenderShadows (bool refreshTextures = false);

		long GetNumShapes (void)
		{
			return numTG_Shapes;
		}

		void ScaleShape (float scaleFactor)
		{
			if (scaleFactor >= 0.0f)
			{
				for (int i=0;i<numTG_Shapes;i++)
					listOfShapes[i].node->shapeScalar = scaleFactor;
			}
		}
		
		long GetNumVerticesInShape (long i)
		{
			if ((i >= 0) && (i < numTG_Shapes))
			{
				return myMultiType->listOfTypeShapes[i]->GetNumTypeVertices();
			}
			
			return 0;
		}

		Stuff::Vector3D GetShapeVertexInWorld(long shapeNum, long vertexNum, float rotation);
		Stuff::Vector3D GetShapeVertexInEditor(long shapeNum, long vertexNum, float rotation);
		
		float GetFrameNum (void)
		{
			return frameNum;
		}

		void SetFrameNum (float frame)
		{
			frameNum = frame;
		}

		void SetIsClamped (bool value)
		{
			isClamped = value;
		}
		
		//Function returns 0 if OK.  -1 if textureName is longer then nameLength-1.
		//This function digs the texture name(s) out of the ASE file so that the
		//User can load and manage them anyway they want to.
		long GetTextureName (DWORD textureNum, char *textureName, long nameLength)
		{
			return myMultiType->GetTextureName(textureNum,textureName,nameLength);
		}

		long GetNumTextures (void)
		{
			return myMultiType->GetNumTextures();
		}

		//Function returns 0 if OK.  -1 if textureNum is out of range of numTextures.
		//This function takes the gosTextureHandle passed in and assigns it to the
		//textureNum entry of the listOfTextures;
		long SetTextureHandle (DWORD textureNum, DWORD gosTextureHandle)
		{
			return myMultiType->SetTextureHandle(textureNum,gosTextureHandle);
		}

		DWORD GetTextureHandle (DWORD textureNum)
		{
			return myMultiType->GetTextureHandle(textureNum);
		}

		float GetExtentRadius (void)
		{
			return myMultiType->extentRadius;
		}
		
		Stuff::Vector3D GetMaxBox (void)
		{
			return myMultiType->maxBox;
		}
		
		Stuff::Vector3D GetMinBox (void)
		{
			return myMultiType->minBox;
		}
		
		//Function returns 0 if OK.  -1 if textureNum is out of range of numTextures.
		//This function takes the gosTextureHandle passed in and assigns it to the
		//textureNum entry of the listOfTextures;
		long SetTextureAlpha (DWORD textureNum, bool alphaFlag)
		{
			return myMultiType->SetTextureAlpha(textureNum,alphaFlag);
		}

		Stuff::Point3D GetRootNodeCenter (void)
		{
			Stuff::Point3D result;
			result.x = result.y = result.z = 0.0f;
			for (int i=0;i<numTG_Shapes;i++)
			{
				if (listOfShapes[i].parentNode == NULL)
				{
					result = listOfShapes[i].node->myType->GetNodeCenter();
				}
			}
			
			return result;
		}

		Stuff::Point3D GetRootRelativeNodeCenter (void)
		{
			Stuff::Point3D result;
			result.x = result.y = result.z = 0.0f;
			for (int i=0;i<numTG_Shapes;i++)
			{
				if (listOfShapes[i].parentNode == NULL)
				{
					result = listOfShapes[i].node->myType->GetRelativeNodeCenter();
				}
			}
			
			return result;
		}

		void SetCurrentAnimation (long shapeNum, TG_AnimationPtr anim)
		{
			if ((shapeNum >= 0) && (shapeNum < numTG_Shapes))
				listOfShapes[shapeNum].currentAnimation = anim;
		}

		//This function clears all of the animation pointers.
		//As such, the shape will stop animating and return to base pose.
		void ClearAnimation (void)
		{
			for (int i=0;i<numTG_Shapes;i++)
				listOfShapes[i].currentAnimation = NULL;
		}

		void SetARGBHighLight (DWORD argb)
		{
			for (int i=0;i<numTG_Shapes;i++)
				listOfShapes[i].node->SetARGBHighLight(argb);
		}
		
		void SetLightsOut (bool lightFlag)
		{
			for (int i=0;i<numTG_Shapes;i++)
				listOfShapes[i].node->SetLightsOut(lightFlag);
		}
		
 		Stuff::Vector3D GetTransformedNodePosition (Stuff::Point3D *pos, Stuff::UnitQuaternion *rot, const char *nodeId);
 		Stuff::Vector3D GetTransformedNodePosition (Stuff::Point3D *pos, Stuff::UnitQuaternion *rot, long nodeId);

		TG_TypeMultiShapePtr GetMultiType (void)
		{
			return myMultiType;
		}
		
		// This function takes the shape named nodeName and all of its children, detaches them
		// from the current heirarchy and stuffs them into a new MultiShape which it passes back
		// Uses are endless but for now limited to blowing the arms off of the mechs!
		TG_MultiShape* Detach (const char *nodeName);
		
		// This function takes the shape named nodeName and all of its children and stops processing
		// them forever.  Since we can never re-attach a mech arm in the field, this is OK!
		void StopUsing (const char *nodeName);
		
 		// Tells me if the passed in nodeName is a child of the parentName.
		bool isChildOf (const char *nodeName, const char* parentName);
		
		char *GetNodeId (long shapeNum)
		{
			if ((shapeNum >= 0) && (shapeNum < numTG_Shapes))
				return (listOfShapes[shapeNum].node->getNodeName());
			
			return NULL;
		}
		
		long GetNodeNameId (const char * nodeId)
		{
			for (int  i=0;i<numTG_Shapes;i++)
			{
				//-----------------------------------------------------------------
				// Scan through the list of shapes and dig out the number needed.
				// DO NOT UPDATE THE HEIRARCHY!!!!
				// This thing may not have updated itself this turn yet!!!
				if (S_stricmp(listOfShapes[i].node->myType->getNodeId(),nodeId) == 0)
					return i;
			}

			return -1;
		}

		void SetIsHudElement (void)
		{
			isHudElement = true;
		}

		void SetAlphaValue (BYTE aVal)
		{
			alphaValue = aVal;
		}
		
		BYTE GetAlphaValue (void)
		{
			return alphaValue;
		}
		
 		bool PerPolySelect (long mouseX, long mouseY)
		{
			float mx = mouseX;
			float my = mouseY;

			for (int i=0;i<numTG_Shapes;i++)
			{
				if (listOfShapes[i].node->numVisibleFaces && listOfShapes[i].node->PerPolySelect(mx,my))
					return true;
			}
			
			return false;
		}
		
		void SetRecalcShadows (bool flag)
		{
			for (int i=0;i<numTG_Shapes;i++)
			{
				listOfShapes[i].node->SetRecalcShadows(flag);
			}
		}

		void SetUseShadow (bool flag)
		{
			d_useShadows = flag;
		}
};

typedef TG_MultiShape* TG_MultiShapePtr;

//-------------------------------------------------------------------------------
// The meat and Potatoes part of the animation system
// TG_AnimShape
class TG_AnimateShape
{
	friend class TG_TypeMultiShape;
	friend class TG_MultiShape;
	//--------------------------------------------------------------------
	//This class stores a single gesture worth of animation data.
	//Works by loading the animation data for each NODE in the multiShape
	//From an ASE file.  When we change an animation gesture, this class
	//will shove the correct pointers to the data down to the TG_ShapeRecs.
	//Other then data storage, parsing, and the pointer sets, does zipp!
	protected:
		TG_AnimationPtr		listOfAnimation;
		int                 count;
		int                 actualCount;
		bool				shapeIdsSet;

	public:
		void * operator new (size_t mySize);
		void operator delete (void * us);

		void init (void)
		{
			listOfAnimation = NULL;
			count = actualCount = 0;
			shapeIdsSet = false;
		}

		TG_AnimateShape (void)
		{
			init();
		}

		~TG_AnimateShape (void)
		{
			destroy();
		}

		long LoadBinaryCopy (const char *fileName);
		void SaveBinaryCopy (const char *fileName);

		//This function frees all of the RAM allocated by this class and resets vars to initial state.
		void destroy (void);

		//This function loads the animation data contained in the file passed in.
		//It sets up a pointer to the multi-shape so that animation data for each
		//Node in the Multi-Shape can be loaded.
		//It mallocs memory.
		long LoadTGMultiShapeAnimationFromASE (const char *filename, TG_TypeMultiShapePtr shape, bool skipIfBinary = false, bool forceMakeBinary = false);

		//This function copies the pointers to the animation data in this class to the
		//TG_MultiShape being drawn.  Nothing else happens.
		void SetAnimationState (TG_MultiShapePtr mShape);

		float GetFrameRate (void)
		{
			if (listOfAnimation)
				return listOfAnimation[0].frameRate;

			return 30.0f;
		}

		void SetFrameRate (float nFrameRate)
		{
			if (listOfAnimation)
				listOfAnimation[0].frameRate = nFrameRate;
		}

		void ReverseFrameRate (void)
		{
			if (listOfAnimation)
				listOfAnimation[0].frameRate = -listOfAnimation[0].frameRate;
		}

		long GetNumFrames (void)
		{
			if (listOfAnimation)
				return listOfAnimation[0].numFrames;

			return 0;
		}

		void resetShapeIds (void)
		{
			shapeIdsSet = false;
		}
};

typedef TG_AnimateShape *TG_AnimateShapePtr;
//-------------------------------------------------------------------------------
#endif
