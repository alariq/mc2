#pragma once
//===========================================================================//
// File:	 ControlManager.hpp												 //
// Contents: Control Manager routines										 //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//





//
// Mouse handling functions
//
void CMCreateMouse();
void CMUpdateMouse();
void ReInitControlManager();
gosEnum_KeyStatus GetStatus( gosEnum_KeyIndex index );


extern DWORD KeyBoardBuffer[256];
extern DWORD KeyCurrent;				// Where the next key to be read is
extern DWORD KeyPressed;				// Where the next key pressed will go
extern DWORD dbKeyBoardBuffer[256];
extern DWORD dbKeyCurrent;				// Where the next key to be read is
extern DWORD dbKeyPressed;				// Where the next key pressed will go
extern DWORD LastWMDown;				// Last WM_KEYDOWN message scan code
extern void SaveOldKeyState();
extern void DoKeyReleased( DWORD KeyCode );
extern void DealWithKey( DWORD wParam, DWORD lParam );
extern void AddKeyEvent(DWORD KeyEvent);

#define MAX_DI_DEVICES 16

typedef struct _cmd
{
	LPDIRECTINPUT7	m_lpDirectInput7;

	LPDIRECTINPUTDEVICE7	m_lpSysMouse7;
	LPDIRECTINPUTDEVICE7	m_lpJoystick7[MAX_DI_DEVICES];

	bool m_disabled[MAX_DI_DEVICES];
	bool m_validAxis[MAX_DI_DEVICES][12];
	bool m_pollMe[MAX_DI_DEVICES];
	float m_howOften[MAX_DI_DEVICES];
	double m_nextPoll[MAX_DI_DEVICES];
	bool m_bForceFeedback[MAX_DI_DEVICES];

	unsigned char
		m_numJoysticks,
		m_numMouseButtons,
		m_numMouseAxes,
		m_numPOVs[MAX_DI_DEVICES],
		m_numSliders[MAX_DI_DEVICES],
		m_numButtons[MAX_DI_DEVICES];

	LONG
		m_mousePos[3],
		m_mouseAxis[3],
		m_joyAxis[MAX_DI_DEVICES][12];

	unsigned char
		m_joyButton[MAX_DI_DEVICES][64];
	char
		m_joyName[MAX_DI_DEVICES][64];
	gosEnum_KeyStatus
		m_mouseButton[4];
	bool
		m_requiresAcquire;
	HANDLE
		m_hMouseEvent;
} ControlManagerDataStruct;

void CMInstall();

void CMUninstall();

void CMCreateKeyboard();

void CMReadJoystick(DWORD index);

void CMReadKeyboard();

void CMGetKeyState();

void CMRestore();

void CMReleaseControls();

void CMAcquireControls();

void CMUnacquireControls();

void CMUpdate();

void CMCreateJoysticks();

BOOL WINAPI CMCreateJoystick7( LPCDIDEVICEINSTANCE pdinst, LPVOID pvRef );

// This data structure holds all the information necessary for setting up, using,
// and uninstalling joysticks, a keyboard, and a mouse.
extern ControlManagerDataStruct g_CMData;


