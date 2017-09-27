//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include "platform_windows.h"
#include "gameos.hpp"

//#include "version.h"
static const char versionStamp[1024] = "00.05.10.2712";


#include "../ARM/Microsoft.Xna.Arm.h"
using namespace Microsoft::Xna::Arm;


#include "mclib.h"

HSTRRES gosResourceHandle = 0;

Stuff::MemoryStream *effectStream = NULL;

extern char CDInstallPath[];

bool hasGuardBand = false;
bool justResaveAllMaps = false;
Camera *eye = NULL;
//enum { CPU_UNKNOWN, CPU_PENTIUM, CPU_MMX, CPU_KATMAI } Processor = CPU_PENTIUM;		//Needs to be set when GameOS supports ProcessorID -- MECHCMDR2

float MaxMinUV = 8.0f;

DWORD BaseVertexColor = 0x00000000;

static LPCTSTR lpszAppName = "MechCmdr2";

UserHeapPtr systemHeap = NULL;
UserHeapPtr guiHeap = NULL;

float gosFontScale = 1.0f;

extern bool silentMode;
bool useLOSAngle = false;
static bool createARM = false;

IProviderEngine * armProvider = NULL;

unsigned long tglHeapSize = 16386000;

FastFile 	**fastFiles = NULL;
long 		numFastFiles = 0;
long		maxFastFiles = 0;

HWND		appWnd = NULL;

extern const char* MechAnimationNames[MaxGestures];

int ObjectTextureSize = 128;
bool reloadBounds = false;
MidLevelRenderer::MLRClipper * theClipper = NULL;
HGOSFONT3D gosFontHandle = 0;
extern HGOSFONT3D	FontHandle;
FloatHelpPtr globalFloatHelp = NULL;
unsigned long currentFloatHelp = 0;

char fileName[1024];
char listName[1024];
char prefixPath[1024];

//----------------------------------------------------------------------------
// Same command line Parser as MechCommander
void ParseCommandLine(char *command_line)
{
	int i;
	int n_args = 0;
	int index = 0;
	char *argv[30];

	char tempCommandLine[4096];
	memset(tempCommandLine, 0, 4096);
	strncpy(tempCommandLine, command_line, 4095);

	while (tempCommandLine[index] != '\0')  // until we null out
	{
		argv[n_args] = tempCommandLine + index;
		n_args++;
		while (tempCommandLine[index] != ' ' && tempCommandLine[index] != '\0')
		{
			index++;
		}
		while (tempCommandLine[index] == ' ')
		{
			tempCommandLine[index] = '\0';
			index++;
		}
	}

	i = 0;
	while (i<n_args)
	{
		if (strcmpi(argv[i], "-file") == 0)
		{
			i++;
			if (i < n_args)
			{
				if (argv[i][0] == '"')
				{
					// They typed in a quote, keep reading argvs
					// until you find the close quote
					strcpy(fileName, &(argv[i][1]));
					bool scanName = true;
					while (scanName && (i < n_args))
					{
						i++;
						if (i < n_args)
						{
							strcat(fileName, " ");
							strcat(fileName, argv[i]);

							if (strstr(argv[i], "\"") != NULL)
							{
								scanName = false;
								fileName[strlen(fileName) - 1] = 0;
							}
						}
						else
						{
							//They put a quote on the line with no space.
							//
							scanName = false;
							fileName[strlen(fileName) - 1] = 0;
						}
					}
				}
				else
					strcpy(fileName, argv[i]);
			}
		}
		if (strcmpi(argv[i], "-list") == 0)
		{
			i++;
			if (i < n_args)
			{
				if (argv[i][0] == '"')
				{
					// They typed in a quote, keep reading argvs
					// until you find the close quote
					strcpy(listName, &(argv[i][1]));
					bool scanName = true;
					while (scanName && (i < n_args))
					{
						i++;
						if (i < n_args)
						{
							strcat(listName, " ");
							strcat(listName, argv[i]);

							if (strstr(argv[i], "\"") != NULL)
							{
								scanName = false;
								listName[strlen(listName) - 1] = 0;
							}
						}
						else
						{
							//They put a quote on the line with no space.
							//
							scanName = false;
							listName[strlen(listName) - 1] = 0;
						}
					}
				}
				else
					strcpy(listName, argv[i]);
			}
		}
		if (strcmpi(argv[i], "-prefix-path") == 0)
		{
			i++;
			if (i < n_args)
			{
				if (argv[i][0] == '"')
				{
					// They typed in a quote, keep reading argvs
					// until you find the close quote
					strcpy(prefixPath, &(argv[i][1]));
					bool scanName = true;
					while (scanName && (i < n_args))
					{
						i++;
						if (i < n_args)
						{
							strcat(prefixPath, " ");
							strcat(prefixPath, argv[i]);

							if (strstr(argv[i], "\"") != NULL)
							{
								scanName = false;
								prefixPath[strlen(prefixPath) - 1] = 0;
							}
						}
						else
						{
							//They put a quote on the line with no space.
							//
							scanName = false;
							prefixPath[strlen(prefixPath) - 1] = 0;
						}
					}
				}
				else
					strcpy(prefixPath, argv[i]);
			}
		}

		if (strcmpi(argv[i], "-arm") == 0)
		{
			createARM = true;
		}

		i++;
	}
}

//-----------------------------
long convertASE2TGL(char *file)
{
	//---------------------------------------------------
	// Get all of the .ASE files in the tgl directory.
	char findString[1024];
	//char armFileName[1024];
	if (file[0] == '\0')
	{
		sprintf(findString, "%s*.ini", prefixPath);
	}
	else
	{
		strcpy(findString, file);
	}

	int count = 0;
	WIN32_FIND_DATA	findResult;
	HANDLE searchHandle = FindFirstFile(findString, &findResult);
	do
	{
		if (searchHandle != INVALID_HANDLE_VALUE && (findResult.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		{

			//-----------------------------------
			// Search for TGLData
			FullPathFileName iniName;
			//iniName.init(tglPath, findResult.cFileName, "");
			iniName.init(prefixPath, findResult.cFileName, "");

			FitIniFile iniFile;
			long result = iniFile.open(iniName);
			if (result != NO_ERR)
				return result;

			// ARM
			IProviderAssetPtr iniAsset = armProvider->OpenAsset(iniName, AssetType_Physical,
				ProviderType_Primary);
			iniAsset->AddProperty("Type", "Object Definition");
			iniAsset->AddProperty("Version", "1.0");


			TG_TypeMultiShape *shape = NULL;

			result = iniFile.seekBlock("TGLData");
			if (result == NO_ERR)
			{
				char fileName[1024];
				result = iniFile.readIdString("FileName", fileName, 1023);
				if (result != NO_ERR)
				{
					//---------------------------------------------
					// We have LODs -- handle differently
					// We will get animation from LAST LOD loaded
					long i = 0;
					char fileCheck[1024];
					sprintf(fileCheck, "FileName%d", i);
					result = iniFile.readIdString(fileCheck, fileName, 1023);

					while (result == NO_ERR)
					{
						if (shape)
						{
							delete shape;
							shape = NULL;
						}

						char aseName[1024];

						//sprintf(aseName, "%s%s%s", tglPath, fileName, ".ase");
						sprintf(aseName, "%s%s%s", prefixPath, fileName, ".ase");

						//---------------------------------------------------------------------------------------------
						// Load Base Shape or LOD 0 Shape.
						shape = new TG_TypeMultiShape;
						gosASSERT(shape != NULL);

						printf("Processing Main Shape %s\n", aseName);

						char lodID[4];
						sprintf(lodID, "%02d", count);

						IProviderRelationshipPtr armLink = iniAsset->AddRelationship("LOD Shape", aseName);
						armLink->AddProperty("LOD", lodID);

						shape->LoadTGMultiShapeFromASE(aseName, true, armProvider);

						i++;
						sprintf(fileCheck, "FileName%d", i);
						result = iniFile.readIdString(fileCheck, fileName, 1023);
					}
				}

				char aseName[1024];
				//sprintf(aseName, "%s%s%s", tglPath, fileName, ".ase");
				sprintf(aseName, "%s%s%s", prefixPath, fileName, ".ase");

				//---------------------------------------------------------------------------------------------
				// Load Base Shape or LOD 0 Shape.
				shape = new TG_TypeMultiShape;
				gosASSERT(shape != NULL);

				printf("Processing Main Shape %s\n", aseName);

				IProviderRelationshipPtr armLink = iniAsset->AddRelationship("Main Shape", aseName);

				shape->LoadTGMultiShapeFromASE(aseName, true, armProvider);

				//-------------------------------------------
				// Gotta make the special shadow shape now!!
				// MUST use its own shape or animation below
				// will go straight to HELL!!
				result = iniFile.readIdString("ShadowName", fileName, 1023);
				if (result == NO_ERR)
				{
					char aseName[1024];
					//sprintf(aseName, "%s%s%s", tglPath, fileName, ".ase");
					sprintf(aseName, "%s%s%s", prefixPath, fileName, ".ase");

					//---------------------------------------------------------------------------------------------
					// Load Base Shape or LOD 0 Shape.
					TG_TypeMultiShapePtr shadowShape = new TG_TypeMultiShape;
					gosASSERT(shadowShape != NULL);

					printf("Processing Shadow Shape %s\n", aseName);

					IProviderRelationshipPtr armLink = iniAsset->AddRelationship("Shadow Shape", aseName);

					shadowShape->LoadTGMultiShapeFromASE(aseName, true, armProvider);

					delete shadowShape;
					shadowShape = NULL;
				}

				long i = 0;
				char animCheck[1024];
				sprintf(animCheck, "Animation:%d", i);
				result = iniFile.seekBlock(animCheck);

				while (result == NO_ERR)		//This thing has animations.  Process them!
				{
					char fileName[1024];
					result = iniFile.readIdString("AnimationName", fileName, 1023);
					if (result == NO_ERR)
					{
						FullPathFileName aseName;
						//aseName.init(tglPath, fileName, ".ase");
						aseName.init(prefixPath, fileName, ".ase");

						TG_AnimateShape *anim = new TG_AnimateShape;
						gosASSERT(anim != NULL);

						//-----------------------------------------------
						// Skip this one if its already a binary file.
						// Happens ALOT!
						printf("Processing Animation %s\n", (const char*)aseName);

						IProviderRelationshipPtr armLink = iniAsset->AddRelationship("Animation", aseName);

						anim->LoadTGMultiShapeAnimationFromASE(aseName, shape, true);

						delete anim;
						anim = NULL;
					}

					i++;
					sprintf(animCheck, "Animation:%d", i);
					result = iniFile.seekBlock(animCheck);
				}

				if (!i)		//No Animations, BUT they may mean we are a MECH!!!
				{
					if (iniFile.seekBlock("Gestures0") == NO_ERR)
					{
						//We ARE a mech.  Load all of the animations for this mech and write 'em out.
						for (long i = 0; i<MaxGestures; i++)
						{
							char name[MAX_PATH];
							_splitpath(findResult.cFileName, NULL, NULL, name, NULL);

							char mechFileName[1024];
							//sprintf(mechFileName, "%s%s%s.ase", tglPath, name, MechAnimationNames[i]);
							sprintf(mechFileName, "%s%s%s.ase", prefixPath, name, MechAnimationNames[i]);

							TG_AnimateShape *anim = new TG_AnimateShape;
							gosASSERT(anim != NULL);

							//-----------------------------------------------
							// Skip this one if its already a binary file.
							// Happens ALOT!
							printf("Processing Animation %s\n", mechFileName);

							IProviderRelationshipPtr armLink = iniAsset->AddRelationship("Animation", mechFileName);

							anim->LoadTGMultiShapeAnimationFromASE(mechFileName, shape, true);

							delete anim;
							anim = NULL;
						}
					}
				}

				if (!i)		//No Animations, BUT they may mean we are a MECH!!!
				{
					if (iniFile.seekBlock("Gestures0") == NO_ERR)
					{
						//We ARE a mech.  Load all of the destroyed shapes for this mech and write 'em out.
						for (long i = MaxGestures; i<MaxGestures + 2; i++)
						{
							char name[MAX_PATH];
							_splitpath(findResult.cFileName, NULL, NULL, name, NULL);

							char mechFileName[1024];
							//sprintf(mechFileName, "%s%s%s.ase", tglPath, name, MechAnimationNames[i]);
							sprintf(mechFileName, "%s%s%s.ase", prefixPath, name, MechAnimationNames[i]);

							//-----------------------------------------------
							shape = new TG_TypeMultiShape;
							gosASSERT(shape != NULL);

							printf("Processing Animation %s\n", mechFileName);

							IProviderRelationshipPtr armLink = iniAsset->AddRelationship("Destroyed Shape", mechFileName);

							shape->LoadTGMultiShapeFromASE(mechFileName, true, armProvider);

							delete shape;
							shape = NULL;
						}
					}
				}

				if (iniFile.seekBlock("TGLDamage") == NO_ERR)
				{
					char fileName[1024];
					result = iniFile.readIdString("FileName", fileName, 1023);
					if (result == NO_ERR)
					{
						if (shape)
						{
							delete shape;
							shape = NULL;
						}

						char aseName[1024];
						//sprintf(aseName, "%s%s%s", tglPath, fileName, ".ase");
						sprintf(aseName, "%s%s%s", prefixPath, fileName, ".ase");

						//---------------------------------------------------------------------------------------------
						// Load Base Shape or LOD 0 Shape.
						shape = new TG_TypeMultiShape;
						gosASSERT(shape != NULL);

						printf("Processing Damage Shape %s\n", aseName);

						IProviderRelationshipPtr armLink = iniAsset->AddRelationship("Damage Shape", aseName);

						shape->LoadTGMultiShapeFromASE(aseName, true, armProvider);
					}

					//-------------------------------------------
					// Gotta make the special shadow shape now!!
					// MUST use its own shape or animation below
					// will go straight to HELL!!
					result = iniFile.readIdString("ShadowName", fileName, 1023);
					if (result == NO_ERR)
					{
						char aseName[1024];
						//sprintf(aseName, "%s%s%s", tglPath, fileName, ".ase");
						sprintf(aseName, "%s%s%s", prefixPath, fileName, ".ase");

						//---------------------------------------------------------------------------------------------
						// Load Base Shape or LOD 0 Shape.
						TG_TypeMultiShapePtr shadowShape = new TG_TypeMultiShape;
						gosASSERT(shadowShape != NULL);

						printf("Processing Damage Shadow Shape %s\n", aseName);

						IProviderRelationshipPtr armLink = iniAsset->AddRelationship("Damage Shadow Shape", aseName);

						shadowShape->LoadTGMultiShapeFromASE(aseName, true, armProvider);

						delete shadowShape;
						shadowShape = NULL;
					}
				}
			}

			delete shape;
			shape = NULL;

			iniAsset->Close();
		}

	} while (FindNextFile(searchHandle, &findResult) != 0);

	FindClose(searchHandle);

	return 0;
}

void usage(const char* program_name)
{
	printf("%s <-file file.ase | -prefix-path /path/to/ase/files/>\n", program_name);
}

//-----------------------------
int main(int argc, char** argv)
{
	globalHeapList = new HeapList;
	assert(globalHeapList != NULL);

	systemHeap = new UserHeap;
	assert(systemHeap != NULL);

	systemHeap->init(2048000);

	//---------------------------------------------------------
	// Start the Tiny Geometry Layer Heap.
	TG_Shape::tglHeap = new UserHeap;
	TG_Shape::tglHeap->init(tglHeapSize, "TinyGeom");

#if 0

	//--------------------------------------------------------------
	// Read in System.CFG
	FitIniFilePtr systemFile = new FitIniFile;

#ifdef _DEBUG
	long systemOpenResult =
#endif
		systemFile->open("system.cfg");

#ifdef _DEBUG
	assert(systemOpenResult == NO_ERR);
#endif

	{
#ifdef _DEBUG
		long systemPathResult =
#endif
			systemFile->seekBlock("systemPaths");
		assert(systemPathResult == NO_ERR);
		{
			long result = systemFile->readIdString("terrainPath", terrainPath, 79);
			assert(result == NO_ERR);

			result = systemFile->readIdString("artPath", artPath, 79);
			assert(result == NO_ERR);

			result = systemFile->readIdString("fontPath", fontPath, 79);
			assert(result == NO_ERR);

			result = systemFile->readIdString("savePath", savePath, 79);
			assert(result == NO_ERR);

			result = systemFile->readIdString("spritePath", spritePath, 79);
			assert(result == NO_ERR);

			result = systemFile->readIdString("shapesPath", shapesPath, 79);
			assert(result == NO_ERR);

			result = systemFile->readIdString("soundPath", soundPath, 79);
			assert(result == NO_ERR);

			result = systemFile->readIdString("objectPath", objectPath, 79);
			assert(result == NO_ERR);

			result = systemFile->readIdString("cameraPath", cameraPath, 79);
			assert(result == NO_ERR);

			result = systemFile->readIdString("tilePath", tilePath, 79);
			assert(result == NO_ERR);

			result = systemFile->readIdString("missionPath", missionPath, 79);
			assert(result == NO_ERR);

			result = systemFile->readIdString("warriorPath", warriorPath, 79);
			assert(result == NO_ERR);

			result = systemFile->readIdString("profilePath", profilePath, 79);
			assert(result == NO_ERR);

			result = systemFile->readIdString("interfacepath", interfacePath, 79);
			assert(result == NO_ERR);

			result = systemFile->readIdString("moviepath", moviePath, 79);
			assert(result == NO_ERR);

			result = systemFile->readIdString("tglpath", tglPath, 79);
			assert(result == NO_ERR);

			result = systemFile->readIdString("texturepath", texturePath, 79);
			assert(result == NO_ERR);

			result = systemFile->readIdString("CDsoundPath", CDsoundPath, 79);
			assert(result == NO_ERR);

			result = systemFile->readIdString("CDmoviepath", CDmoviePath, 79);
			assert(result == NO_ERR);

			result = systemFile->readIdString("CDspritePath", CDspritePath, 79);
			assert(result == NO_ERR);
		}
	}

	systemFile->close();
	delete systemFile;
	systemFile = NULL;

#endif


	//-------------------------------------------------------
	// Check if we are running this from the command line
	// with ASE2TGL as the command line parameter.  If so,
	// for each .INI file in data\tgl, find the corresponding
	// .ASE file and convert to .TGL.  Then Exit!
	silentMode = true;

	memset(fileName, 0, sizeof(fileName));
	memset(listName, 0, sizeof(listName));
	memset(prefixPath, 0, sizeof(prefixPath));

	// gather command line
	size_t cmdline_len = 0;
	for (int i = 0; i<argc; ++i) {
		cmdline_len += strlen(argv[i]);
		cmdline_len += 1; // ' '
	}
	char* cmdline = new char[cmdline_len + 1];
	uint32_t offset = 0;
	for (int i = 0; i<argc; ++i) {
		size_t arglen = strlen(argv[i]);
		memcpy(cmdline + offset, argv[i], arglen);
		cmdline[offset + arglen] = ' ';
		offset += arglen + 1;
	}
	cmdline[cmdline_len] = '\0';

	ParseCommandLine(cmdline);
	delete[] cmdline;

	// create prefix path from file name path
	if (strlen(fileName) > 0) {
		_splitpath(fileName, nullptr, prefixPath, nullptr, nullptr);
	}
	else if (0 == strlen(prefixPath)) {
		usage(argv[0]);
		return 1;
	}

	// tglPath used by Load*** functions when they compose filename for outpur binary file
	strcpy(tglPath, "./data/tgl/");

	// Initialize COM and create an instance of the InterfaceImplementation class:
	CoInitialize(NULL);
	armProvider = CreateProviderEngine("AseConv", versionStamp);

	assert(armProvider);

	if (listName[0] == 0)
	{
		convertASE2TGL(fileName);
	}
	else
	{
		//
		// A list file was provided
		//
		File file;
		if (file.open(listName) == NO_ERR)
		{
			while (!file.eof())
			{
				char line[1024];
				file.readLine((MemoryPtr)line, 1024);
				if (line[0] != 0)
					convertASE2TGL(line);
			}
			file.close();
		}
	}

	/*
	//Time BOMB goes here.
	// Set Date and write Binary data to registry under key
	// GraphicsDataInit!!
	SYSTEMTIME bombDate;
	DWORD dataSize = sizeof(SYSTEMTIME);
	gos_LoadDataFromRegistry("GraphicsDataInit", &bombDate, &dataSize);
	if (dataSize == 0)
	{
	bombDate.wYear = 2001;
	bombDate.wMonth = 3;
	bombDate.wDayOfWeek = 4;
	bombDate.wDay = 31;
	bombDate.wHour = 0;
	bombDate.wMinute = 0;
	bombDate.wSecond = 0;
	bombDate.wMilliseconds = 0;

	dataSize = sizeof(SYSTEMTIME);
	gos_SaveDataToRegistry("GraphicsDataInit", &bombDate, dataSize);
	}
	*/

	//
	// Exit app
	//
	CoUninitialize();

	//ExitGameOS();	

	return 0;
}

DWORD	Seed;

//
//
//
void __stdcall UpdateRenderers()
{
}

void __stdcall DoGameLogic()
{
}


//
// Setup the GameOS structure
//
void __stdcall GetGameOSEnvironment(const char* CommandLine)
{
	CommandLine = CommandLine;
	Environment.applicationName = "MechCmdr2";
	Environment.screenWidth = 640;
	Environment.screenHeight = 480;
	Environment.bitDepth = 16;

	Environment.DoGameLogic = DoGameLogic;
	Environment.UpdateRenderers = UpdateRenderers;

	Environment.version = versionStamp;

	Environment.checkCDForFiles = false;
}


