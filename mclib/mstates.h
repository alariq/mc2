//---------------------------------------------------------------------
// Mech Gesture State Table for all Mecha.
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef MSTATES_H
#define MSTATES_H
//******************************************************************************************
// Gesture States which the mech can be in.  These are used to determine where the
// mech is looping and how to get to any other state.
// 0 -- Park
// 1 -- Stand 
// 2 -- Walk  (from normal walk speed to 1/2 walk speed)
// 3 -- Run
// 4 -- Reverse (1/2 walk speed)
// 5 -- Limp
// 6 -- Jump
// 7 -- Fallen Forward
// 8 -- Fallen Back
//
// Actual Gestures are:
//  0 -- Park 
//  1 -- ParkToStand 
//  2 -- Stand 
//  3 -- StandToWalk
//  4 -- Walk 
//  5 -- StandToPark
//  6 -- WalkToRun
//  7 -- Run
//  8 -- RunToWalk
//  9 -- Reverse
// 10 -- StandToReverse
// 11 -- LimpLeft
// 12 -- LimpRight
// 13 -- Idle
// 14 -- FallBackward 
// 15 -- FallForward 
// 16 -- HitFront
// 17 -- HitBack
// 18 -- HitLeft
// 19 -- HitRight
// 20 -- Jump
// 21 -- Rollover from FallBack to FallForward 
// 22 -- Get up from FallForward 
// 23 -- Fallen Forward (Single Frame, mech on ground)
// 24 -- Fallen Backward (Single Frame, mech on ground)

//---------------------------------------------------------------
// This array is pretty damned huge.  It shows how the mech can
// move from any one state to any other state.  Since there are
// 9 states which a mech can be in and 9 others it can get to,
// this table is 9*9*MAX_TRANSITION_GESTURES in size.

#define MAX_TRANSITION_GESTURES			10

char hotSpotFinderArray[28] = 
{
//  0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27
	0,0,0,1,2,3,4,5,6,2, 3, 7, 8, 9,10,11,12,13,14,15,16,17,18,11,10, 0,19,19
};

char mechSpeedStateArray[28] = {
	0, 0, 0, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0
};

char transitionArray[1000] = 
{
//--------------------------------------------------------------------------------
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//0,0	From Park to Park (NULL)
	 1, 2,-1,-1,-1,-1,-1,-1,-1,-1,		//0,1	From Park to Stand 
	 1, 2, 4,-1,-1,-1,-1,-1,-1,-1,		//0,2	From Park to Walk
	 1, 2, 4, 7,-1,-1,-1,-1,-1,-1,		//0,3	From Park to Run
	 1, 2, 9,-1,-1,-1,-1,-1,-1,-1,		//0,4	From Park to Reverse
	 1, 2,11,-1,-1,-1,-1,-1,-1,-1,		//0,5	From Park to LimpLeft
	 1, 2,20, 2,-1,-1,-1,-1,-1,-1,		//0,6	From Park to Jump
	15,23,-1,-1,-1,-1,-1,-1,-1,-1,		//0,7	From Park to Fall Forward 
	14,24,-1,-1,-1,-1,-1,-1,-1,-1,		//0,8	From Park to Fall Backward 
	 1, 2,12,-1,-1,-1,-1,-1,-1,-1,		//0,9	From Park to LimpRight
//--------------------------------------------------------------------------------
	 5, 0,-1,-1,-1,-1,-1,-1,-1,-1,		//1,0	From Stand to Park
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//1,1	From Stand to Stand (NULL)
	 4,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//1,2	From Stand to Walk
	 7,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//1,3	From Stand to Run
     9,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//1,4	From Stand to Reverse
    11,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//1,5	From Stand to LimpLeft
	20, 2,-1,-1,-1,-1,-1,-1,-1,-1,		//1,6	From Stand to Jump
	15,23,-1,-1,-1,-1,-1,-1,-1,-1,		//1,7	From Stand to Fall Forward 
	14,24,-1,-1,-1,-1,-1,-1,-1,-1,		//1,8	From Stand to Fall Backward 
    12,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//1,5	From Stand to LimpRight
//--------------------------------------------------------------------------------
	 2, 5, 0,-1,-1,-1,-1,-1,-1,-1,		//2,0	From Walk to Park 
	 2,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//2,1	From Walk to Stand 
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//2,2	From Walk to Walk (NULL)
	 7,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//2,3	From Walk to Run
	 9,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//2,4	From Walk to Reverse
	11,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//2,5	From Walk to LimpLeft
	20, 2,-1,-1,-1,-1,-1,-1,-1,-1,		//2,6	From Walk to Jump
	15,23,-1,-1,-1,-1,-1,-1,-1,-1,		//2,7	From Walk to Fall Forward 
	14,24,-1,-1,-1,-1,-1,-1,-1,-1,		//2,8	From Walk to Fall Backward 
	12,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//2,5	From Walk to LimpRight
//--------------------------------------------------------------------------------
	 2, 5, 0,-1,-1,-1,-1,-1,-1,-1,		//3,0	From Run to Park
	 2,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//3,1	From Run to Stand 
	 4,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//3,2	From Run to Walk
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//3,3	From Run to Run	(NULL)
	 9,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//3,4	From Run to Reverse
	11,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//3,5	From Run to LimpLeft
	20, 2,-1,-1,-1,-1,-1,-1,-1,-1,		//3,6	From Run to Jump
	15,23,-1,-1,-1,-1,-1,-1,-1,-1,		//3,7	From Run to Fall Forward 
	14,24,-1,-1,-1,-1,-1,-1,-1,-1,		//3,8	From Run to Fall Backward 
	12,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//3,5	From Run to LimpRight
//--------------------------------------------------------------------------------
	 2, 5, 0,-1,-1,-1,-1,-1,-1,-1,		//4,0	From Reverse to Park 
	 2,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//4,1	From Reverse to Stand 
	 4,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//4,2	From Reverse to Walk
	 7,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//4,3	From Reverse to Run
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//4,4	From Reverse to Reverse (NULL)
   	11,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//4,5	From Reverse to LimpLeft
	20, 2,-1,-1,-1,-1,-1,-1,-1,-1,		//4,6	From Reverse to Jump
	15,23,-1,-1,-1,-1,-1,-1,-1,-1,		//4,7	From Reverse to Fall Forward 
	14,24,-1,-1,-1,-1,-1,-1,-1,-1,		//4,8	From Reverse to Fall backward 
   	12,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//4,5	From Reverse to LimpRight
//--------------------------------------------------------------------------------
	 2, 5, 0,-1,-1,-1,-1,-1,-1,-1,		//5,0	From Limp to Park 
	 2,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//5,1	From Limp to Stand 
	 4,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//5,2	From Limp to Walk
	 7,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//5,3	From Limp to Run
     9,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//5,4	From Limp to Reverse
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//5,5	From Limp to LimpLeft	(NULL)
	20, 2,-1,-1,-1,-1,-1,-1,-1,-1,		//5,6	From Limp to Jump
	15,23,-1,-1,-1,-1,-1,-1,-1,-1,		//5,7	From Limp to Fall Forward 
	14,24,-1,-1,-1,-1,-1,-1,-1,-1,		//5,8	From Limp to Fall Backward 
	12,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//5,5	From Limp to LimpRight
//--------------------------------------------------------------------------------
	 5, 0,-1,-1,-1,-1,-1,-1,-1,-1,		//6,0	From Jump to Park
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//6,1	From Jump to Stand (NULL)
	 4,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//6,2	From Jump to Walk
	 7,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//6,3	From Jump to Run
	 9,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//6,4	From Jump to Reverse
	11,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//6,5	From Jump to LimpLeft
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//6,6	From Jump to Jump (NULL)
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//6,7	From Jump to Fall Forward 
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//6,8	From Jump to Fall Backward 
	12,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//6,5	From Jump to LimpRight
//--------------------------------------------------------------------------------
	22, 2, 5, 0,-1,-1,-1,-1,-1,-1,		//7,0	From Fall Forward to Park
	22, 2,-1,-1,-1,-1,-1,-1,-1,-1,		//7,1	From Fall Forward to Stand 
	22, 2, 4,-1,-1,-1,-1,-1,-1,-1,		//7,2	From Fall Forward to Walk	
	22, 2, 7,-1,-1,-1,-1,-1,-1,-1,		//7,3	From Fall Forward to Run
	22, 2, 9,-1,-1,-1,-1,-1,-1,-1,		//7,4	From Fall Forward to Reverse
	22, 2,11,-1,-1,-1,-1,-1,-1,-1,		//7,5	From Fall Forward to LimpLeft
	22, 2,20, 2,-1,-1,-1,-1,-1,-1,		//7,6	From Fall Forward to Jump
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//7,7	From Fall Forward to Fall Forward (NULL)
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//7,8	From Fall Forward to Fall Backward (NULL)
	22, 2,12,-1,-1,-1,-1,-1,-1,-1,		//7,5	From Fall Forward to LimpRight
//--------------------------------------------------------------------------------
	21, 2, 5, 0,-1,-1,-1,-1,-1,-1,		//8,0	From Fall Backward to Park 
	21, 2,-1,-1,-1,-1,-1,-1,-1,-1,		//8,1	From Fall Backward to Stand 
	21, 2, 4,-1,-1,-1,-1,-1,-1,-1,		//8,2	From Fall Backward to Walk
	21, 2, 7,-1,-1,-1,-1,-1,-1,-1,		//8,3	From Fall Backward to Run
	21, 2, 9,-1,-1,-1,-1,-1,-1,-1,		//8,4	From Fall Backward to Reverse
	21, 2,11,-1,-1,-1,-1,-1,-1,-1,		//8,5	From Fall Backward to LimpLeft
	21, 2,20, 2,-1,-1,-1,-1,-1,-1,		//8,6	From Fall Backward to Jump
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//8,7	From Fall Backward to Fall Forward  (NULL)
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//8,8	From Fall Backward to Fall Backward (NULL)
	21, 2,12,-1,-1,-1,-1,-1,-1,-1,		//8,5	From Fall Backward to LimpRight
//--------------------------------------------------------------------------------
	 2, 5, 0,-1,-1,-1,-1,-1,-1,-1,		//9,0	From LimpRight to Park 
	 2,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//9,1	From LimpRight to Stand 
	 4,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//9,2	From LimpRight to Walk
	 7,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//9,3	From LimpRight to Run
     9,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//9,4	From LimpRight to Reverse
	11,-1,-1,-1,-1,-1,-1,-1,-1,-1,		//9,5	From LimpRight to LimpLeft
	20, 2,-1,-1,-1,-1,-1,-1,-1,-1,		//9,6	From LimpRight to Jump
	15,23,-1,-1,-1,-1,-1,-1,-1,-1,		//9,7	From LimpRight to Fall Forward 
	14,24,-1,-1,-1,-1,-1,-1,-1,-1,		//9,8	From LimpRight to Fall Backward 
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1		//9,5	From LimpRight to LimpRight
//--------------------------------------------------------------------------------
};

#define GESTURE_OFFSET_SIZE		100
//-----------------------------------------------------------------------------
#endif