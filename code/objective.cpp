#define OBJECTIVE_CPP
/*************************************************************************************************\
Objective.cpp			: Implementation of the Objective component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#include"terrain.h"

#include"objective.h"

#include"estring.h"
#include"echarstring.h"

#ifndef INIFILE_H
#include"inifile.h"
#endif

#ifndef MISSION_H
#include"mission.h"
#endif

#ifndef OBJMGR_H
#include"objmgr.h"
#endif

#ifndef BLDNG_H
#include"bldng.h"
#endif

#ifndef GAMESOUND_H
#include"gamesound.h"
#endif

#ifndef TACMAP_H
#include"tacmap.h"
#endif

#ifndef AFONT_H
#include"afont.h"
#endif

#include"logisticsdata.h"
#include"team.h"
#include"comndr.h"

#include"controlgui.h"

#include "../resource.h"

#include "platform_tchar.h"
#include<assert.h>

float CObjective::s_blinkLength = .5;
float CObjective::s_lastBlinkTime = 0.f;
unsigned long CObjective::s_blinkColor = SB_YELLOW;
aFont* CObjective::s_markerFont = 0;
float MaxExtractUnitDistance = 0.0f;

static const char *g_actionSpeciesStringArray[] = {
	"PlayBIK",
	"PlayWAV",
	"DisplayTextMessage",
	"DisplayResourceTextMessage",
	"SetBooleanFlag",
	"MakeNewTechnologyAvailable",
	"_RemoveStructure",
};

const char *g_conditionSpeciesStringArray[] = {
	"DestroyAllEnemyUnits",
		"DestroyNumberOfEnemyUnits",
		"DestroyEnemyUnitGroup",
		"DestroySpecificEnemyUnit",
		"DestroySpecificStructure",

		"CaptureOrDestroyAllEnemyUnits",
		"CaptureOrDestroyNumberOfEnemyUnits",
		"CaptureOrDestroyEnemyUnitGroup",
		"CaptureOrDestroySpecificEnemyUnit",
		"CaptureOrDestroySpecificStructure",

		"DeadOrFledAllEnemyUnits",
		"DeadOrFledNumberOfEnemyUnits",
		"DeadOrFledEnemyUnitGroup",
		"DeadOrFledSpecificEnemyUnit",

		"CaptureSpecificUnit",
		"CaptureSpecificStructure",

		"GuardSpecificUnit",
		"GuardSpecificStructure",

		"MoveAnyUnitToArea",
		"MoveAllUnitsToArea",
		"MoveAllSurvivingUnitsToArea",
		"MoveAllSurvivingMechsToArea",

		"BooleanFlagIsSet",
		"ElapsedMissionTime",
};


static bool MoverIsDeadOrDisabled(const long pMoverWID) 
{
	bool retval = false;

	Mover *pMover = (MoverPtr)ObjectManager->getByWatchID(pMoverWID);
	assert(pMover);

	if (pMover->isDisabled() || pMover->isDestroyed()) {
		retval = true;
	} else {
		assert(pMover->getPilot());
		long wstatus = pMover->getPilot()->getStatus();
		if ((WARRIOR_STATUS_DEAD == wstatus)
			|| (WARRIOR_STATUS_EJECTED == wstatus)) {
			retval = true;
		}
	}
	return retval;
}

static bool MoverIsDeadOrDisabled(const MoverPtr pMover) 
{
	bool retval = false;
	assert(pMover);

	if (pMover->isDisabled() || pMover->isDestroyed()) {
		retval = true;
	} else {
		assert(pMover->getPilot());
		long wstatus = pMover->getPilot()->getStatus();
		if ((WARRIOR_STATUS_DEAD == wstatus)
			|| (WARRIOR_STATUS_EJECTED == wstatus)) {
			retval = true;
		}
	}
	return retval;
}

static bool MoverIsDeadOrDisabledOrFled(const MoverPtr pMover) {
	assert(pMover);
	bool retval = false;
	if (pMover->isDisabled() || pMover->isDestroyed()) {
		retval = true;
	} else {
		assert(pMover->getPilot());
		long wstatus = pMover->getPilot()->getStatus();
		if ((WARRIOR_STATUS_DEAD == wstatus)
			|| (WARRIOR_STATUS_EJECTED == wstatus)) {
			retval = true;
		} else if (WARRIOR_STATUS_WITHDRAWN == wstatus) {
			retval = true;
		}
	}
	return retval;
}

static bool MoverIsCaptured(const MoverPtr pMover, const long teamID) {
	assert(pMover);
	bool retval = false;
	if (pMover->isDestroyed()) {
		retval = false;
	} else {
		if ( pMover->getTeamId() == teamID ) {
			retval = true;
		}
	}
	return retval;
}

static bool MoverIsCaptured(const long pMoverWID, const long teamID) 
{
	Mover *pMover = (MoverPtr)ObjectManager->getByWatchID(pMoverWID);
	assert(pMover);

	bool retval = false;
	if (pMover->isDestroyed()) 
	{
		retval = false;
	} 
	else 
	{
		if ( pMover->getTeamId() == teamID ) 
		{
			retval = true;
		}
	}

	return retval;
}

static long sReadIdFloat(FitIniFile* missionFile, const char *varName, float &value) {
	long result = 0;
	float tmpFloat;
	result = missionFile->readIdFloat((char *)varName, tmpFloat);
	if (NO_ERR != result) {
		//assert(false);
	} else {
		value = tmpFloat;
	}
	return result;
}

static long sReadIdBoolean(FitIniFile* missionFile, const char *varName, bool &value) {
	long result = 0;
	bool tmpBool;
	result = missionFile->readIdBoolean((char *)varName, tmpBool);
	if (NO_ERR != result) {
		//assert(false);
	} else {
		value = tmpBool;
	}
	return result;
}

static long sReadIdWholeNum(FitIniFile* missionFile, const char *varName, int &value) {
	long result = 0;
	unsigned long tmpULong;
	result = missionFile->readIdULong((char *)varName, tmpULong);
	if (NO_ERR != result) {
		//assert(false);
	} else {
		value = tmpULong;
	}
	return result;
}

static long sReadIdLongInt(FitIniFile* missionFile, const char *varName, int &value) {
	long result = 0;
	long tmpLong;
	result = missionFile->readIdLong((char *)varName, tmpLong);
	if (NO_ERR != result) {
		//assert(false);
	} else {
		value = tmpLong;
	}
	return result;
}

static int sReplace(ECharString &ECStr, const char *szOldSub, const char *szNewSub) {
	if ((!szOldSub) || (0 >= strlen(szOldSub)) || (!szNewSub)) { return -1; }
	int retval = 0;
	ECharString newStr;
	const int lengthOfOldSub = strlen(szOldSub);
	int endIndexOfLastOldSub = -1;
	int startIndexOfOldSub = ECStr.Find(szOldSub, endIndexOfLastOldSub + 1);
	while ((ECharString::INVALID_INDEX != startIndexOfOldSub) && (0 <= startIndexOfOldSub) && (ECStr.Length() - lengthOfOldSub >= startIndexOfOldSub)) {
		if (1 <= startIndexOfOldSub) {
			newStr += ECStr.SubString(endIndexOfLastOldSub + 1, startIndexOfOldSub - 1);
		}
		newStr += szNewSub;
		retval += 1;
		endIndexOfLastOldSub = startIndexOfOldSub + lengthOfOldSub -1;
		startIndexOfOldSub = ECStr.Find(szOldSub, endIndexOfLastOldSub + 1);
	}
	if (0 < retval) {
		if (endIndexOfLastOldSub + 1 < ECStr.Length() - 1) {
			newStr += ECStr.SubString(endIndexOfLastOldSub + 1, ECStr.Length() - 1);
		}
		ECStr = newStr;
	}
	return retval;
}

static long sReadIdString(FitIniFile* missionFile, const char *varName, ECharString &ECStr) {
	long result = 0;
	char buffer[2001/*buffer size*/]; buffer[0] = '\0';
	result = missionFile->readIdString((char *)varName, buffer, 2001/*buffer size*/ -1);
	if ((NO_ERR != result) && (BUFFER_TOO_SMALL != result)) {
		//assert(false);
	} else {
		ECStr = buffer;
		/*readIdString can't read in "\r\n"*/
		sReplace(ECStr, "\n", "\r\n");
	}
	return result;
}

static long sWriteIdString(FitIniFile* missionFile, const char *varName, const char *szStr) {
	if (!szStr) { return !(NO_ERR); }
	long result = 0;
	ECharString ECStr = szStr;
	/*readIdString can't read in "\r\n"*/
	sReplace(ECStr, "\r\n", "\n");
	result = missionFile->writeIdString(varName, ECStr.Data());
	return result;
}

static bool areCloseEnough(float f1, float f2)
{
	bool retval = false;
	float diff = fabs(f1 - f2);
	float bigger = fabs(f1);
	if (fabs(f2) > bigger) {
		bigger = fabs(f2);
	}
	float proportion_diff = 0.0;
	if (0.0 != bigger) {
		proportion_diff = diff / bigger;
	}
	if ((pow(10.0, -FLT_DIG) * 10.0/*arbitrary*/) > proportion_diff) {
		retval = true;
	}
	return retval;
}

void CBooleanArray::save (long alignment, FitIniFile *file)
{
	char blockID[256];
	sprintf(blockID, "Team%dBooleanFlags",alignment);
	long result = file->writeBlock(blockID); (void)result;

	CEStringList::EIterator flagIDListIter;
	long count = 0;
	for (flagIDListIter = m_FlagIDList.Begin(); !flagIDListIter.IsDone(); flagIDListIter++) 
	{
		char stringName[1024];
		sprintf(stringName,"BooleanString%d",count);
		sWriteIdString(file,stringName,(*flagIDListIter).Data());

		char booleanName[1024];
		sprintf(booleanName,"BooleanValue%d",count);
		file->writeIdBoolean(booleanName,m_valueList[count]);

		count++;
	}
}

void CBooleanArray::load (long alignment, FitIniFile *file)
{
	char blockID[256];
	sprintf(blockID, "Team%dBooleanFlags",alignment);
	long result = file->seekBlock(blockID);
	if (result == NO_ERR)
	{
		ECharString tmpECStr;
		EString element;
		bool value;
	
		long count = 0;
		char stringName[1024];
		sprintf(stringName,"BooleanString%d",count);
		result = sReadIdString(file,stringName,tmpECStr);
		while (result == NO_ERR)
		{
			char booleanName[1024];
			sprintf(booleanName,"BooleanValue%d",count);
			result = file->readIdBoolean(booleanName,value);
	
			element = tmpECStr.Data();
			m_FlagIDList.Append(element);
			m_valueList.Append(value);
	
			count++;
			sprintf(stringName,"BooleanString%d",count);
			result = sReadIdString(file,stringName,tmpECStr);
		}
	}
}


objective_status_type CDestroyAllEnemyUnits::Status() {
	long numMovers = ObjectManager->getNumMovers();
	long i;
	for (i = 0; i < numMovers; i+= 1) {
		MoverPtr pMover = ObjectManager->getMover(i);
		assert(pMover);
		if (pMover->getTeamId() != Alignment())
			if (!(MoverIsDeadOrDisabled(pMover) || MoverIsCaptured(pMover, Alignment()))) {
				return OS_UNDETERMINED;
		}
	}
	return OS_SUCCESSFUL;
}

bool CNumberOfUnitsObjectiveCondition::Read( FitIniFile* missionFile )
{
	long result = 0;
	result = sReadIdWholeNum(missionFile, "Num", m_num);
	if (NO_ERR != result) { return false; }

	return true;
}

EString CNumberOfUnitsObjectiveCondition::InstanceDescription() {
	EString tmpEStr;
	tmpEStr.Format("%d", m_num);
	return tmpEStr;
}

objective_status_type CDestroyNumberOfEnemyUnits::Status() {
	int numDestroyed = 0;
	long numMovers = ObjectManager->getNumMovers();
	long i;
	for (i = 0; i < numMovers; i+= 1) {
		MoverPtr pMover = ObjectManager->getMover(i);
		assert(pMover);
		if (pMover->getTeamId() != Alignment())
			if (MoverIsDeadOrDisabled(pMover) || MoverIsCaptured(pMover, Alignment())) {
				numDestroyed += 1;
			}
	}

	if (numDestroyed >= m_num) {
		return OS_SUCCESSFUL;
	} else {
		return OS_UNDETERMINED;
	}
}

EString CSpecificUnitObjectiveCondition::InstanceDescription() {
	EString tmpEStr;
	long partId = m_pUnitWID;

	tmpEStr.Format("partId: %ld", partId);
	return tmpEStr;
}

bool CSpecificEnemyUnitObjectiveCondition::Read( FitIniFile* missionFile )
{
	long result = 0;

	result = missionFile->readIdInt("MoverWID", m_pUnitWID);
	if (result != NO_ERR)
	{
		float positionX, positionY;
		result = sReadIdFloat(missionFile, "PositionX", positionX);
		if (NO_ERR != result) { return false; }
		result = sReadIdFloat(missionFile, "PositionY", positionY);
		if (NO_ERR != result) { return false; }
		MoverPtr pMover = NULL;
		if (ObjectManager) {
			long numMovers = ObjectManager->getNumMovers();
			long i;
			for (i = 0; i < numMovers; i+= 1) {
				pMover = ObjectManager->getMover(i);
				bool close_x = areCloseEnough(pMover->getPosition().x, positionX);
				bool close_y = areCloseEnough(pMover->getPosition().y, positionY);
				// FIXME: sebi: one of the missions has bad data and so this condition never worked which led to assert(false)
				// I could ont change it in data, so had to disable this check in the code ;-(
				if (pMover && /*(pMover->getTeamId() != Alignment()) &&*/ close_x && close_y) {
					break;
				}
				printf("i=%d\n", i);
			}
			if (!(i < numMovers)) { assert(false); return false; }
		}
		if (pMover)
			m_pUnitWID = pMover->getWatchID();
		else
			m_pUnitWID = 0;
	}

	return true;
}

objective_status_type CDestroySpecificEnemyUnit::Status() {
	assert(m_pUnitWID);
	if (MoverIsDeadOrDisabled(m_pUnitWID) || MoverIsCaptured(m_pUnitWID, Alignment())) {
		return OS_SUCCESSFUL;
	} else {
		return OS_UNDETERMINED;
	}
}

EString CSpecificStructureObjectiveCondition::InstanceDescription() {
	EString tmpEStr;
	Building * m_pBuilding = (Building *)ObjectManager->getByWatchID(m_pBuildingWID);
	long partId = m_pBuilding->getPartId();

	tmpEStr.Format("partId: %ld", partId);
	return tmpEStr;
}

bool CSpecificStructureObjectiveCondition::Read( FitIniFile* missionFile )
{
	long result = 0;

	result = missionFile->readIdLong("BuildingWID", m_pBuildingWID);
	if (result != NO_ERR)
	{
		float positionX, positionY;
		result = sReadIdFloat(missionFile, "PositionX", positionX);
		if (NO_ERR != result) { return false; }
		result = sReadIdFloat(missionFile, "PositionY", positionY);
		if (NO_ERR != result) { return false; }
		BuildingPtr pBuilding = 0;
		if (ObjectManager) {
			pBuilding = ObjectManager->getBuilding(0);
			long i;
			for (i = 0; (0 != pBuilding); i+= 1) {
				pBuilding = ObjectManager->getBuilding(i);
				if ((0 != pBuilding) && areCloseEnough(pBuilding->getPosition().x, positionX)  && areCloseEnough(pBuilding->getPosition().y, positionY)) {
					break;
				}
			}
			if (!pBuilding) { assert(false); return false; }
		}
		if (pBuilding)
			m_pBuildingWID = pBuilding->getWatchID();
		else
			m_pBuildingWID = 0;
	}

	return true;
}

objective_status_type CDestroySpecificStructure::Status() 
{
	Building *m_pBuilding = (Building *)ObjectManager->getByWatchID(m_pBuildingWID);
	if (m_pBuilding->isDestroyed()) {
		return OS_SUCCESSFUL;
	} else {
		return OS_UNDETERMINED;
	}
}

objective_status_type CCaptureOrDestroyAllEnemyUnits::Status() {
	long numMovers = ObjectManager->getNumMovers();
	long i;
	for (i = 0; i < numMovers; i+= 1) {
		MoverPtr pMover = ObjectManager->getMover(i);
		assert(pMover);
		if (pMover->getTeamId() != Alignment())
			if (!MoverIsDeadOrDisabled(pMover)) {
				if (!MoverIsCaptured(pMover, Alignment())) { /*Is this necessary? When a mover is captured, is it no longer returned as a "BadMover"? */
					return OS_UNDETERMINED;
				}
		}
	}
	return OS_SUCCESSFUL;
}

objective_status_type CCaptureOrDestroyNumberOfEnemyUnits::Status() {
	int numCapturedOrDestroyed = 0;
	long numMovers = ObjectManager->getNumMovers();
	long i;
	for (i = 0; i < numMovers; i+= 1) {
		MoverPtr pMover = ObjectManager->getMover(i);
		assert(pMover);
		if (pMover->getTeamId() != Alignment())
			if (MoverIsDeadOrDisabled(pMover) || MoverIsCaptured(pMover, Alignment())/*redundant?*/) {
				numCapturedOrDestroyed += 1;
			}
	}

	if (numCapturedOrDestroyed >= m_num) {
		return OS_SUCCESSFUL;
	} else {
		return OS_UNDETERMINED;
	}
}

bool CCaptureOrDestroySpecificEnemyUnit::Read( FitIniFile* missionFile )
{
	long result = 0;

	result = missionFile->readIdInt("MoverWID", m_pUnitWID);
	if (result != NO_ERR)
	{
		float positionX, positionY;
		result = sReadIdFloat(missionFile, "PositionX", positionX);
		if (NO_ERR != result) { return false; }
		result = sReadIdFloat(missionFile, "PositionY", positionY);
		if (NO_ERR != result) { return false; }
		MoverPtr pMover = 0;
		if (ObjectManager) {
			long numMovers = ObjectManager->getNumMovers();
			long i;
			for (i = 0; i < numMovers; i+= 1) {
				pMover = ObjectManager->getMover(i);
				if (pMover && (pMover->getTeamId() != Alignment()) && areCloseEnough(pMover->getPosition().x, positionX)  && areCloseEnough(pMover->getPosition().y, positionY)) {
					break;
				}
			}
			if (!(i < numMovers)) { assert(false); return false; }
			pMover->setFlag(OBJECT_FLAG_CAPTURABLE, true);
		}
		if (pMover)
			m_pUnitWID = pMover->getWatchID();
		else
			m_pUnitWID = 0;
	}

	return true;
}

objective_status_type CCaptureOrDestroySpecificEnemyUnit::Status() 
{
	MoverPtr m_pUnit = (MoverPtr)ObjectManager->getByWatchID(m_pUnitWID);
	assert(m_pUnit);
	if (MoverIsDeadOrDisabled(m_pUnit)|| MoverIsCaptured(m_pUnit, Alignment())/*redundant?*/) {
		return OS_SUCCESSFUL;
	} else {
		return OS_UNDETERMINED;
	}
}

bool CCaptureOrDestroySpecificStructure::Read( FitIniFile* missionFile )
{
	long result = 0;

	result = missionFile->readIdLong("BuildingWID",m_pBuildingWID);
	if (result != NO_ERR)
	{
		float positionX, positionY;
		result = sReadIdFloat(missionFile, "PositionX", positionX);
		if (NO_ERR != result) { return false; }
		result = sReadIdFloat(missionFile, "PositionY", positionY);
		if (NO_ERR != result) { return false; }
		BuildingPtr pBuilding = 0;
		if (ObjectManager) {
			pBuilding = ObjectManager->getBuilding(0);
			long i;
			for (i = 0; (0 != pBuilding); i+= 1) {
				pBuilding = ObjectManager->getBuilding(i);
				if ((0 != pBuilding) && areCloseEnough(pBuilding->getPosition().x, positionX)  && areCloseEnough(pBuilding->getPosition().y, positionY)) {
					break;
				}
			}
			if (!pBuilding) { assert(false); return false; }
			pBuilding->setFlag(OBJECT_FLAG_CAPTURABLE, true);
		}
		if (pBuilding)
			m_pBuildingWID = pBuilding->getWatchID();
		else
			m_pBuildingWID = 0;
	}

	return true;
}

objective_status_type CCaptureOrDestroySpecificStructure::Status() 
{
	Building* m_pBuilding = (Building *)ObjectManager->getByWatchID(m_pBuildingWID);
	if (m_pBuilding->isDestroyed()) 
	{
		return OS_SUCCESSFUL;
	} 
	else if ( m_pBuilding->getTeamId() == Alignment() ) 
	{
		return OS_SUCCESSFUL;
	} 
	else 
	{
		return OS_UNDETERMINED;
	}
}

objective_status_type CDeadOrFledAllEnemyUnits::Status() {
	long numMovers = ObjectManager->getNumMovers();
	long i;
	for (i = 0; i < numMovers; i+= 1) {
		MoverPtr pMover = ObjectManager->getMover(i);
		assert(pMover);
		if (!MoverIsDeadOrDisabledOrFled(pMover)) {
			return OS_UNDETERMINED;
		}
	}
	return OS_SUCCESSFUL;
}

objective_status_type CDeadOrFledNumberOfEnemyUnits::Status() {
	int numDeadOrFled = 0;
	long numMovers = ObjectManager->getNumMovers();
	long i;
	for (i = 0; i < numMovers; i+= 1) {
		MoverPtr pMover = ObjectManager->getMover(i);
		assert(pMover);
		if (MoverIsDeadOrDisabledOrFled(pMover)) {
			numDeadOrFled += 1;
		}
	}

	if (numDeadOrFled >= m_num) {
		return OS_SUCCESSFUL;
	} else {
		return OS_UNDETERMINED;
	}
}

objective_status_type CDeadOrFledSpecificEnemyUnit::Status() 
{
	MoverPtr m_pUnit = (MoverPtr)ObjectManager->getByWatchID(m_pUnitWID);
	assert(m_pUnit);
	if (MoverIsDeadOrDisabledOrFled(m_pUnit)) {
		return OS_SUCCESSFUL;
	} else {
		return OS_UNDETERMINED;
	}
}

bool CCaptureUnit::Read( FitIniFile* missionFile )
{
	long result = 0;

	result = missionFile->readIdInt("MoverWID", m_pUnitWID);
	if (result != NO_ERR)
	{
		float positionX, positionY;
		result = sReadIdFloat(missionFile, "PositionX", positionX);
		if (NO_ERR != result) { return false; }
		result = sReadIdFloat(missionFile, "PositionY", positionY);
		if (NO_ERR != result) { return false; }
		MoverPtr pMover = 0;
		if (ObjectManager) {
			long numMovers = ObjectManager->getNumMovers();
			long i;
			for (i = 0; i < numMovers; i+= 1) {
				pMover = ObjectManager->getMover(i);
				if (pMover && (pMover->getTeamId() != Alignment()) && areCloseEnough(pMover->getPosition().x, positionX)  && areCloseEnough(pMover->getPosition().y, positionY)) {
					break;
				}
			}
			if (!(i < numMovers)) { assert(false); return false; }
			pMover->setFlag(OBJECT_FLAG_CAPTURABLE, true);
		}
		if (pMover)
			m_pUnitWID = pMover->getWatchID();
		else
			m_pUnitWID = 0;
	}

	return true;
}

objective_status_type CCaptureUnit::Status() 
{
	MoverPtr m_pUnit = (MoverPtr)ObjectManager->getByWatchID(m_pUnitWID);

	if (!m_pUnit || m_pUnit->isDestroyed()) 
	{
		return OS_FAILED;
	} 
	else if (m_pUnit && m_pUnit->getTeamId() == Alignment() ) 
	{
		return OS_SUCCESSFUL;
	} 
	else 
	{
		return OS_UNDETERMINED;
	}
}

bool CCaptureStructure::Read( FitIniFile* missionFile )
{
	long result = 0;
	result = missionFile->readIdLong("BuildingWID", m_pBuildingWID);
	if (result != NO_ERR)
	{
		float positionX, positionY;
		result = sReadIdFloat(missionFile, "PositionX", positionX);
		if (NO_ERR != result) { return false; }
		result = sReadIdFloat(missionFile, "PositionY", positionY);
		if (NO_ERR != result) { return false; }
		BuildingPtr pBuilding = 0;
		if (ObjectManager) 
		{
			pBuilding = ObjectManager->getBuilding(0);
			long i;
			for (i = 0; (0 != pBuilding); i+= 1) 
			{
				pBuilding = ObjectManager->getBuilding(i);
				if ((0 != pBuilding) && areCloseEnough(pBuilding->getPosition().x, positionX)  && areCloseEnough(pBuilding->getPosition().y, positionY)) 
				{
					break;
				}
			}
			if (!pBuilding) { assert(false); return false; }
			pBuilding->setFlag(OBJECT_FLAG_CAPTURABLE, true);
		}
		if (pBuilding)
			m_pBuildingWID = pBuilding->getWatchID();
		else
			m_pBuildingWID = 0;
	}

	return true;
}

objective_status_type CCaptureStructure::Status() 
{
	Building *m_pBuilding = (Building *)ObjectManager->getByWatchID(m_pBuildingWID);
	if (m_pBuilding->isDestroyed()) 
	{
		return OS_FAILED;
	} 
	else if ( m_pBuilding->getTeamId() == Alignment() ) 
	{
		return OS_SUCCESSFUL;
	} 
	else 
	{
		return OS_UNDETERMINED;
	}
}

bool CGuardSpecificUnit::Read( FitIniFile* missionFile )
{
	long result = 0;

	result = missionFile->readIdInt("MoverWID", m_pUnitWID);
	if (result != NO_ERR)
	{
		float positionX, positionY;
		result = sReadIdFloat(missionFile, "PositionX", positionX);
		if (NO_ERR != result) { return false; }
		result = sReadIdFloat(missionFile, "PositionY", positionY);
		if (NO_ERR != result) { return false; }
		MoverPtr pMover = 0;
		if (ObjectManager) {
			long numMovers = ObjectManager->getNumMovers();
			long i;
			for (i = 0; i < numMovers; i+= 1) {
				pMover = ObjectManager->getMover(i);
				if (pMover && (pMover->getTeamId() == Alignment()) && areCloseEnough(pMover->getPosition().x, positionX)  && areCloseEnough(pMover->getPosition().y, positionY)) {
					break;
				}
			}
			if (!(i < numMovers)) { assert(false); return false; }
		}
		if (pMover)
			m_pUnitWID = pMover->getWatchID();
		else
			m_pUnitWID = 0;
	}

	return true;
}

objective_status_type CGuardSpecificUnit::Status() 
{
	Mover *m_pUnit = (Mover *)ObjectManager->getByWatchID(m_pUnitWID);

	if (m_pUnit->isDestroyed()) {
		return OS_FAILED;
	} else if (  m_pUnit->getTeamId() != Alignment() ) {
		return OS_FAILED;
	} else {
		return OS_UNDETERMINED;
	}
}

objective_status_type CGuardSpecificStructure::Status() 
{
	Building *m_pBuilding = (Building *)ObjectManager->getByWatchID(m_pBuildingWID);
	if (m_pBuilding->isDestroyed()) 
	{
		return OS_FAILED;
	}
	else if ( m_pBuilding->getCaptured() && (m_pBuilding->getTeamId() != Alignment())) 
	{
		return OS_FAILED;
	} 
	else if ((mission->m_timeLimit != -1.0f) && (mission->actualTime > mission->m_timeLimit)) 
	{
		return OS_SUCCESSFUL;
	} 
	else 
	{
		return OS_UNDETERMINED;
	}
}

bool CAreaObjectiveCondition::Read( FitIniFile* missionFile )
{
	long result = 0;
	result = sReadIdFloat(missionFile, "TargetCenterX", m_targetCenterX);
	if (NO_ERR != result) { return false; }
	result = sReadIdFloat(missionFile, "TargetCenterY", m_targetCenterY);
	if (NO_ERR != result) { return false; }
	result = sReadIdFloat(missionFile, "TargetRadius", m_targetRadius);
	if (NO_ERR != result) { return false; }
	return true;
}

EString CAreaObjectiveCondition::InstanceDescription() {
	EString tmpEStr;
	tmpEStr.Format("x: %.3f, y: %.3f, radius: %.3f", m_targetCenterX, m_targetCenterY, m_targetRadius);
	return tmpEStr;
}

objective_status_type CMoveAnyUnitToArea::Status() 
{
	long numMovers = ObjectManager->getNumMovers();
	long i;
	if (MPlayer) {
		for (i = 0; i < numMovers; i+= 1) 
		{
			MoverPtr pMover = ObjectManager->getMover(i);
			assert(pMover);

			if (pMover->getTeamId() == Alignment())
			{
				float dx = pMover->getPosition().x - m_targetCenterX;
				float dy = pMover->getPosition().y - m_targetCenterY;
				if (dx*dx + dy*dy < m_targetRadius*m_targetRadius) 
				{
					return OS_SUCCESSFUL;
				}
			}
		}
		}
	else {
		for (i = 0; i < numMovers; i+= 1) 
		{
			MoverPtr pMover = ObjectManager->getMover(i);
			assert(pMover);

			if (pMover->isOnGUI() && (pMover->getCommanderId() == Commander::home->getId()))
			{
				float dx = pMover->getPosition().x - m_targetCenterX;
				float dy = pMover->getPosition().y - m_targetCenterY;
				if (dx*dx + dy*dy < m_targetRadius*m_targetRadius) 
				{
					return OS_SUCCESSFUL;
				}
			}
		}
	}
	return OS_UNDETERMINED;
}

objective_status_type CMoveAllUnitsToArea::Status() 
{
	long numMovers = ObjectManager->getNumMovers();
	long i;
	if (MPlayer) {
		for (i = 0; i < numMovers; i+= 1) 
		{
			MoverPtr pMover = ObjectManager->getMover(i);
			assert(pMover);

			if (pMover->isRecover()) 
				continue;

			//Seems like any allied or unlinked up unit would keep this from being true?
			if (pMover->getTeamId() == Alignment())
			{
				float dx = pMover->getPosition().x - m_targetCenterX;
				float dy = pMover->getPosition().y - m_targetCenterY;
				if (dx*dx + dy*dy > m_targetRadius*m_targetRadius) 
				{
					return OS_UNDETERMINED;
				}
			}
		}
		}
	else {
		for (i = 0; i < numMovers; i+= 1) 
		{
			MoverPtr pMover = ObjectManager->getMover(i);
			assert(pMover);

			if (pMover->isRecover()) 
				continue;

			//Seems like any allied or unlinked up unit would keep this from being true?
			if (pMover->isOnGUI() && (pMover->getCommanderId() == Commander::home->getId()))
			{
				float dx = pMover->getPosition().x - m_targetCenterX;
				float dy = pMover->getPosition().y - m_targetCenterY;
				if (dx*dx + dy*dy > m_targetRadius*m_targetRadius) 
				{
					return OS_UNDETERMINED;
				}
			}
		}
	}

	return OS_SUCCESSFUL;
}

objective_status_type CMoveAllSurvivingUnitsToArea::Status() 
{
	long numMovers = ObjectManager->getNumMovers();
	long i;
	if (MPlayer) {
		for (i = 0; i < numMovers; i+= 1) 
		{
			MoverPtr pMover = ObjectManager->getMover(i);
			assert(pMover);
			if (pMover->isRecover()) 
				continue;

			//Seems like any allied or unlinked up unit would keep this from being true?
			if (pMover->getTeamId() == Alignment())
			{
				if (!MoverIsDeadOrDisabledOrFled(pMover)) 
				{
					float dx = pMover->getPosition().x - m_targetCenterX;
					float dy = pMover->getPosition().y - m_targetCenterY;
					if (dx*dx + dy*dy > m_targetRadius*m_targetRadius) 
					{
						return OS_UNDETERMINED;
					}
				}
			}
		}
		}
	else {
		for (i = 0; i < numMovers; i+= 1) 
		{
			MoverPtr pMover = ObjectManager->getMover(i);
			assert(pMover);
			if (pMover->isRecover()) 
				continue;

			//Seems like any allied or unlinked up unit would keep this from being true?
			if (pMover->isOnGUI() && (pMover->getCommanderId() == Commander::home->getId()))
			{
				if (!MoverIsDeadOrDisabledOrFled(pMover)) 
				{
					float dx = pMover->getPosition().x - m_targetCenterX;
					float dy = pMover->getPosition().y - m_targetCenterY;
					if (dx*dx + dy*dy > m_targetRadius*m_targetRadius) 
					{
						return OS_UNDETERMINED;
					}
				}
			}
		}
	}

	return OS_SUCCESSFUL;
}

objective_status_type CMoveAllSurvivingMechsToArea::Status() 
{
	long numMovers = ObjectManager->getNumMovers();
	long i;
	if (MPlayer) 
	{
		for (i = 0; i < numMovers; i+= 1) 
		{
			MoverPtr pMover = ObjectManager->getMover(i);
			assert(pMover);
			if (!pMover->isMech() || (pMover->isMech() && (pMover->getMoveType() == MOVETYPE_AIR))) 
				continue;

			//Seems like any allied or unlinked up unit would keep this from being true?
			if (pMover->getTeamId() == Alignment())
			{
				if (!MoverIsDeadOrDisabledOrFled(pMover)) 
				{
					float dx = pMover->getPosition().x - m_targetCenterX;
					float dy = pMover->getPosition().y - m_targetCenterY;
					if (dx*dx + dy*dy > m_targetRadius*m_targetRadius) 
					{
						return OS_UNDETERMINED;
					}
				}
			}
		}		
	}
	else 
	{
		for (i = 0; i < numMovers; i+= 1) 
		{
			MoverPtr pMover = ObjectManager->getMover(i);
			assert(pMover);
			if (!pMover->isMech() || (pMover->isMech() && (pMover->getMoveType() == MOVETYPE_AIR))) 
				continue;

			//Seems like any allied or unlinked up unit would keep this from being true?
			if (pMover->isOnGUI() && (pMover->getCommanderId() == Commander::home->getId()))
			{
				if (!MoverIsDeadOrDisabledOrFled(pMover)) 
				{
					float dx = pMover->getPosition().x - m_targetCenterX;
					float dy = pMover->getPosition().y - m_targetCenterY;
					if (dx*dx + dy*dy > m_targetRadius*m_targetRadius) 
					{
						return OS_UNDETERMINED;
					}
				}
			}
		}

		//All of our units are in the area.  NOW check to make no ENEMY units are within x Distance of the area.
		float MaxUnitDistanceSquared = (MaxExtractUnitDistance * MaxExtractUnitDistance);
		for (i = 0; i < numMovers; i+= 1) 
		{
			MoverPtr pMover = ObjectManager->getMover(i);
			assert(pMover);

			//Seems like any allied or unlinked up unit would keep this from being true?
			if (pMover->getCommanderId() != Commander::home->getId())
			{
				if (!MoverIsDeadOrDisabledOrFled(pMover)) 
				{
					float dx = pMover->getPosition().x - m_targetCenterX;
					float dy = pMover->getPosition().y - m_targetCenterY;
					if (dx*dx + dy*dy < MaxUnitDistanceSquared)
					{
						return OS_UNDETERMINED;
					}
				}
			}
		}
	}

	return OS_SUCCESSFUL;
}

bool CBooleanFlagIsSet::Read( FitIniFile* missionFile )
{
	long result = 0;
	ECharString tmpECStr;
	result = sReadIdString(missionFile, "FlagID", tmpECStr);
	if (NO_ERR != result) {
		/*for backward compatibility*/
		int flagIndex = 0;
		result = sReadIdWholeNum(missionFile, "FlagIndex", flagIndex);
		if (NO_ERR != result) { return false; }
		m_flagID.Format("%d", flagIndex);
	} else {
		m_flagID = tmpECStr.Data();
	}
	result = sReadIdBoolean(missionFile, "Value", m_value);
	return true;
}

objective_status_type CBooleanFlagIsSet::Status() 
{
	//if (mission->missionObjectives.boolFlags.getElementValue(m_flagID) == m_value) {
	if (Team::teams[Alignment()]->objectives.boolFlags.getElementValue(m_flagID) == m_value) {
		return OS_SUCCESSFUL;
	}
	return OS_UNDETERMINED;
}

EString CBooleanFlagIsSet::InstanceDescription() {
	EString tmpEStr;
	tmpEStr.Format("flag name: %s, value: %d", m_flagID.Data(), m_value);
	return tmpEStr;
}

bool CElapsedMissionTime::Read( FitIniFile* missionFile )
{
	long result = 0;
	result = sReadIdFloat(missionFile, "Time", m_time);
	if (NO_ERR != result) { return false; }
	return true;
}

objective_status_type CElapsedMissionTime::Status() {
	if (mission->actualTime >= m_time) {
		return OS_SUCCESSFUL;
	}
	return OS_UNDETERMINED;
}

EString CElapsedMissionTime::InstanceDescription() {
	EString tmpEStr;
	tmpEStr.Format("%f seconds", m_time);
	return tmpEStr;
}

bool CPlayBIK::Read( FitIniFile* missionFile )
{
	long result = 0;
	ECharString tmpECStr;
	result = sReadIdString(missionFile, "AVIFileName", tmpECStr);
	if (NO_ERR != result) { return false; }

	ECharString tmpECStr2 = "./";
	tmpECStr2 += tmpECStr;
	m_pathname = tmpECStr2.Data();

	return true;
}

EString CPlayBIK::InstanceDescription() {
	EString tmpEStr;
	tmpEStr.Format("%s", m_pathname.Data());
	return tmpEStr;
}

int CPlayBIK::Execute()
{
	ControlGui::instance->playMovie( m_pathname );
	return true;
}

bool CPlayWAV::Read( FitIniFile* missionFile )
{
	long result = 0;
	ECharString tmpECStr;
	result = sReadIdString(missionFile, "WAVFileName", tmpECStr);
	if (NO_ERR != result) { return false; }

	ECharString tmpECStr2 = "./";
	tmpECStr2 += tmpECStr;
	m_pathname = tmpECStr2.Data();

	return true;
}

EString CPlayWAV::InstanceDescription() {
	EString tmpEStr;
	tmpEStr.Format("%s", m_pathname.Data());
	return tmpEStr;
}

int CPlayWAV::Execute()
{
	soundSystem->playDigitalStream(m_pathname.Data());
	return true;
}

bool CDisplayTextMessage::Read( FitIniFile* missionFile )
{
	long result = 0;
	ECharString tmpECStr;
	result = sReadIdString(missionFile, "Message", tmpECStr);
	if (NO_ERR != result) { return false; }
	m_message = tmpECStr.Data();
	return true;
}

EString CDisplayTextMessage::InstanceDescription() {
	EString tmpEStr;
	tmpEStr.Format("%s", m_message.Data());
	return tmpEStr;
}

int CDisplayTextMessage::Execute()
{
	ControlGui::instance->setChatText("", m_message.Data(), 0xf0000000, 0);
	return true;
}

bool CDisplayResourceTextMessage::Read( FitIniFile* missionFile )
{
	long result = 0;
	result = sReadIdWholeNum(missionFile, "ResourceStringID", m_resourceStringID);
	if (NO_ERR != result) { return false; }
	return true;
}

static bool ESLoadString(int resourceID, EString &targetStr) {
	char szTmp[16384/*max string length*/];
	cLoadString( resourceID, szTmp, 16384/*max string length*/ );
	targetStr = szTmp;
	EString tmpStr;
	tmpStr.Format("mc2res.dll:%d Not defined", resourceID);
	if (0 == strcmp(tmpStr.Data(), szTmp)) {
		return (0);
	}
	return (!0);
}

EString CDisplayResourceTextMessage::InstanceDescription() {
	EString resourceText;
	ESLoadString(m_resourceStringID, resourceText);
	EString tmpEStr;
	tmpEStr.Format("%d: %s", m_resourceStringID, resourceText.Data());
	return tmpEStr;
}

int CDisplayResourceTextMessage::Execute()
{
	EString resourceText;
	ESLoadString(m_resourceStringID, resourceText);
	ControlGui::instance->setChatText("", resourceText.Data(), 0xf0000000, 0);
	return true;
}

bool CSetBooleanFlag::Read( FitIniFile* missionFile )
{
	long result = 0;
	ECharString tmpECStr;
	result = sReadIdString(missionFile, "FlagID", tmpECStr);
	if (NO_ERR != result) {
		/*for backward compatibility*/
		int flagIndex = 0;
		result = sReadIdWholeNum(missionFile, "FlagIndex", flagIndex);
		if (NO_ERR != result) { return false; }
		m_flagID.Format("%d", flagIndex);
	} else {
		m_flagID = tmpECStr.Data();
	}
	result = sReadIdBoolean(missionFile, "Value", m_value);
	return true;
}

EString CSetBooleanFlag::InstanceDescription() {
	EString tmpEStr;
	tmpEStr.Format("flag name: %s, value: %d", m_flagID.Data(), m_value);
	return tmpEStr;
}

int CSetBooleanFlag::Execute()
{
	//mission->missionObjectives.boolFlags.setElementValue(m_flagID, m_value);
	Team::teams[Alignment()]->objectives.boolFlags.setElementValue(m_flagID, m_value);
	return true;
}

bool CMakeNewTechnologyAvailable::Read( FitIniFile* missionFile )
{
	long result = 0;
	ECharString tmpECStr;
	result = sReadIdString(missionFile, "PurchaseFileName", tmpECStr);
	if (NO_ERR != result) { return false; }
	m_purchaseFilePathname = tmpECStr.Data();
	return true;
}

EString CMakeNewTechnologyAvailable::InstanceDescription() {
	EString tmpEStr;
	tmpEStr.Format("%s", m_purchaseFilePathname.Data());
	return tmpEStr;
}

int CMakeNewTechnologyAvailable::Execute()
{
	LogisticsData::instance->addNewBonusPurchaseFile( m_purchaseFilePathname );
	return true;
}

bool C_RemoveStructure::SetParams(float positionX, float positionY) {

	BuildingPtr pBuilding = 0;
	if (ObjectManager) 
	{
		pBuilding = ObjectManager->getBuilding(0);
		long i;
		for (i = 0; (0 != pBuilding); i+= 1) {
			pBuilding = ObjectManager->getBuilding(i);
			if ((0 != pBuilding) && (pBuilding->getPosition().x == positionX) && (pBuilding->getPosition().y == positionY)) {
				break;
			}
		}
		if (!pBuilding) { assert(false); return false; }
	}
	if (pBuilding)
		m_pBuildingWID = pBuilding->getWatchID();
	else
		m_pBuildingWID = 0;

	return true;
}

bool C_RemoveStructure::Read( FitIniFile* missionFile )
{
	long result = 0;

	result = missionFile->readIdLong("BuildingWID",m_pBuildingWID);
	if (result != NO_ERR)
	{
		float positionX, positionY;
		result = sReadIdFloat(missionFile, "PositionX", positionX);
		if (NO_ERR != result) { return false; }
		result = sReadIdFloat(missionFile, "PositionY", positionY);
		if (NO_ERR != result) { return false; }
		BuildingPtr pBuilding = 0;
		if (ObjectManager) {
			pBuilding = ObjectManager->getBuilding(0);
			long i;
			for (i = 0; (0 != pBuilding); i+= 1) {
				pBuilding = ObjectManager->getBuilding(i);
				if ((0 != pBuilding) && (pBuilding->getPosition().x == positionX) && (pBuilding->getPosition().y == positionY)) {
					break;
				}
			}
			if (!pBuilding) { assert(false); return false; }
		}
		if (pBuilding)
			m_pBuildingWID = pBuilding->getWatchID();
		else
			m_pBuildingWID = 0;
	}

	return true;
}

EString C_RemoveStructure::InstanceDescription() {
	EString tmpEStr;
	Building * m_pBuilding = (Building *)ObjectManager->getByWatchID(m_pBuildingWID);
	long partId = m_pBuilding->getPartId();

	tmpEStr.Format("partId: %ld", partId);
	return tmpEStr;
}

CObjectiveCondition *CObjective::new_CObjectiveCondition(condition_species_type conditionSpecies, int alignment) {
	CObjectiveCondition *retval = 0;
	switch (conditionSpecies) {
	case DESTROY_ALL_ENEMY_UNITS: retval = new CDestroyAllEnemyUnits(alignment); break;
	case DESTROY_NUMBER_OF_ENEMY_UNITS: retval = new CDestroyNumberOfEnemyUnits(alignment); break;
	case DESTROY_SPECIFIC_ENEMY_UNIT: retval = new CDestroySpecificEnemyUnit(alignment); break;
	case DESTROY_SPECIFIC_STRUCTURE: retval = new CDestroySpecificStructure(alignment); break;
	case CAPTURE_OR_DESTROY_ALL_ENEMY_UNITS: retval = new CCaptureOrDestroyAllEnemyUnits(alignment); break;
	case CAPTURE_OR_DESTROY_NUMBER_OF_ENEMY_UNITS: retval = new CCaptureOrDestroyNumberOfEnemyUnits(alignment); break;
	case CAPTURE_OR_DESTROY_SPECIFIC_ENEMY_UNIT: retval = new CCaptureOrDestroySpecificEnemyUnit(alignment); break;
	case CAPTURE_OR_DESTROY_SPECIFIC_STRUCTURE: retval = new CCaptureOrDestroySpecificStructure(alignment); break;
	case DEAD_OR_FLED_ALL_ENEMY_UNITS: retval = new CDeadOrFledAllEnemyUnits(alignment); break;
	case DEAD_OR_FLED_NUMBER_OF_ENEMY_UNITS: retval = new CDeadOrFledNumberOfEnemyUnits(alignment); break;
	case DEAD_OR_FLED_SPECIFIC_ENEMY_UNIT: retval = new CDeadOrFledSpecificEnemyUnit(alignment); break;
	case CAPTURE_UNIT: retval = new CCaptureUnit(alignment); break;
	case CAPTURE_STRUCTURE: retval = new CCaptureStructure(alignment); break;
	case GUARD_SPECIFIC_UNIT: retval = new CGuardSpecificUnit(alignment); break;
	case GUARD_SPECIFIC_STRUCTURE: retval = new CGuardSpecificStructure(alignment); break;
	case MOVE_ANY_UNIT_TO_AREA: retval = new CMoveAnyUnitToArea(alignment); break;
	case MOVE_ALL_UNITS_TO_AREA: retval = new CMoveAllUnitsToArea(alignment); break;
	case MOVE_ALL_SURVIVING_UNITS_TO_AREA: retval = new CMoveAllSurvivingUnitsToArea(alignment); break;
	case MOVE_ALL_SURVIVING_MECHS_TO_AREA: retval = new CMoveAllSurvivingMechsToArea(alignment); break;
	case BOOLEAN_FLAG_IS_SET: retval = new CBooleanFlagIsSet(alignment); break;
	case ELAPSED_MISSION_TIME: retval = new CElapsedMissionTime(alignment); break;
	default:
		assert(false);
		retval = 0;
		break;
	}
	return retval;
}

EString CObjective::DescriptionOfConditionSpecies(condition_species_type conditionSpecies) {
	EString retval = "<unimplemented condition>" /* needs to be put somewhere localizable */;
	CObjectiveCondition *tmpCondition = new_CObjectiveCondition(conditionSpecies, 0/*dummy alignment*/);
	if (0 != tmpCondition) {
		retval = tmpCondition->Description();
	}
	delete tmpCondition;
	return retval;
}

CObjectiveAction *CObjective::new_CObjectiveAction(action_species_type actionSpecies, int alignment) {
	CObjectiveAction *retval = 0;
	switch (actionSpecies) {
	case PLAY_BIK: retval = new CPlayBIK(alignment); break;
	case PLAY_WAV: retval = new CPlayWAV(alignment); break;
	case DISPLAY_TEXT_MESSAGE: retval = new CDisplayTextMessage(alignment); break;
	case DISPLAY_RESOURCE_TEXT_MESSAGE: retval = new CDisplayResourceTextMessage(alignment); break;
	case SET_BOOLEAN_FLAG: retval = new CSetBooleanFlag(alignment); break;
	case MAKE_NEW_TECHNOLOGY_AVAILABLE: retval = new CMakeNewTechnologyAvailable(alignment); break;

	//ONLY used for in-mission saveLoad to avoid calling ReadNavMarkers
	case _REMOVE_STRUCTURE: retval = new C_RemoveStructure(alignment); break;
	default:
		assert(false);
		retval = 0;
		break;
	}
	return retval;
}

EString CObjective::DescriptionOfActionSpecies(action_species_type actionSpecies) {
	EString retval = "<unimplemented action>" /* needs to be put somewhere localizable */;
	CObjectiveAction *tmpAction = new_CObjectiveAction(actionSpecies, 0/*dummy alignment*/);
	if (0 != tmpAction) {
		retval = tmpAction->Description();
	}
	delete tmpAction;
	return retval;
}

CObjective::CObjective(int alignment) {
	m_alignment = alignment;
	Title(_TEXT(""));
	m_titleUseResourceString = false;
	m_titleResourceStringID = 0;
	Description(_TEXT(""));
	m_descriptionUseResourceString = false;
	m_descriptionResourceStringID = 0;
	m_priority = 1;
	m_resourcePoints = 0;
	m_previousPrimaryObjectiveMustBeComplete = false;
	m_allPreviousPrimaryObjectivesMustBeComplete = false;
	m_displayMarker = false;
	m_markerX = 0.0;
	m_markerY = 0.0;
	m_isHiddenTrigger = false;
	m_isActive = true;
	m_activateOnFlag = false;
	ActivateFlagID(_TEXT(""));
	m_activationTime = -1.0;
	m_resetStatusOnFlag = false;
	ResetStatusFlagID(_TEXT(""));
	m_resolved = false;
	m_changedStatus = false;
	m_resolvedStatus = OS_UNDETERMINED;
	m_modelScale = 1.0f;
}

CObjective &CObjective::operator=(const CObjective &master) {
	Clear();
	m_alignment = master.m_alignment;
	Title(master.Title());
	m_titleUseResourceString = master.m_titleUseResourceString;
	m_titleResourceStringID = master.m_titleResourceStringID;
	Description(master.Description());
	m_descriptionUseResourceString = master.m_descriptionUseResourceString;
	m_descriptionResourceStringID = master.m_descriptionResourceStringID;
	m_priority = master.m_priority;
	m_resourcePoints = master.m_resourcePoints;
	m_previousPrimaryObjectiveMustBeComplete = master.m_previousPrimaryObjectiveMustBeComplete;
	m_allPreviousPrimaryObjectivesMustBeComplete = master.m_allPreviousPrimaryObjectivesMustBeComplete;
	m_displayMarker = master.m_displayMarker;
	m_markerX = master.m_markerX;
	m_markerY = master.m_markerY;
	m_isHiddenTrigger = master.m_isHiddenTrigger;
	m_isActive = master.m_isActive;
	m_activateOnFlag = master.m_activateOnFlag;
	m_activateFlagID = master.m_activateFlagID;
	m_activationTime = master.m_activationTime;
	m_resetStatusOnFlag = master.m_resetStatusOnFlag;
	m_resetStatusFlagID = master.m_resetStatusFlagID;
	m_modelName = master.m_modelName;
	m_modelType = master.m_modelType;
	m_modelBaseColor = master.m_modelBaseColor;
	m_modelHighlightColor = master.m_modelHighlightColor;
	m_modelHighlightColor2 = master.m_modelHighlightColor2;
	m_modelScale = master.m_modelScale;
	m_resolved = master.m_resolved;
	m_changedStatus = master.m_changedStatus;
	if (m_resolved) {
		m_resolvedStatus = master.m_resolvedStatus;
	}

	{
		CObjective::EConstIterator it = master.Begin();
		while (!it.IsDone()) {
			condition_species_type conditionSpecies = (*it)->Species();
			CObjectiveCondition *pTmpCondition = new_CObjectiveCondition(conditionSpecies, m_alignment);
			(*pTmpCondition).CastAndCopy(*it);
			Append(pTmpCondition);
			it++;
		}
	}

	{
		action_list_type::EConstIterator it = master.m_actionList.Begin();
		while (!it.IsDone()) {
			action_species_type actionSpecies = (*it)->Species();
			CObjectiveAction *pTmpAction = new_CObjectiveAction(actionSpecies, m_alignment);
			(*pTmpAction).CastAndCopy(*it);
			m_actionList.Append(pTmpAction);
			it++;
		}
	}

	{
		condition_list_type::EConstIterator it = master.m_failureConditionList.Begin();
		while (!it.IsDone()) {
			condition_species_type conditionSpecies = (*it)->Species();
			CObjectiveCondition *pTmpCondition = new_CObjectiveCondition(conditionSpecies, m_alignment);
			(*pTmpCondition).CastAndCopy(*it);
			m_failureConditionList.Append(pTmpCondition);
			it++;
		}
	}

	{
		action_list_type::EConstIterator it = master.m_failureActionList.Begin();
		while (!it.IsDone()) {
			action_species_type actionSpecies = (*it)->Species();
			CObjectiveAction *pTmpAction = new_CObjectiveAction(actionSpecies, m_alignment);
			(*pTmpAction).CastAndCopy(*it);
			m_failureActionList.Append(pTmpAction);
			it++;
		}
	}

	return (*this);
}

void CObjective::Clear() {
	m_resolved = false;
	m_changedStatus = false;

	{
		EIterator it = Begin();
		while (!it.IsDone()) {
			delete (*it);
			it++;
		}
		inherited::Clear();
	}

	{
		action_list_type::EIterator it = m_actionList.Begin();
		while (!it.IsDone()) {
			delete (*it);
			it++;
		}
		m_actionList.Clear();
	}

	{
		condition_list_type::EIterator it = m_failureConditionList.Begin();
		while (!it.IsDone()) {
			delete (*it);
			it++;
		}
		m_failureConditionList.Clear();
	}

	{
		action_list_type::EIterator it = m_failureActionList.Begin();
		while (!it.IsDone()) {
			delete (*it);
			it++;
		}
		m_failureActionList.Clear();
	}
}

void CObjective::Alignment(int alignment) {
	m_alignment = alignment;
	{
		EIterator it = Begin();
		while (!it.IsDone()) {
			(*it)->Alignment(m_alignment);
			it++;
		}
	}

	{
		action_list_type::EIterator it = m_actionList.Begin();
		while (!it.IsDone()) {
			(*it)->Alignment(m_alignment);
			it++;
		}
	}

	{
		condition_list_type::EIterator it = m_failureConditionList.Begin();
		while (!it.IsDone()) {
			(*it)->Alignment(m_alignment);
			it++;
		}
	}

	{
		action_list_type::EIterator it = m_failureActionList.Begin();
		while (!it.IsDone()) {
			(*it)->Alignment(m_alignment);
			it++;
		}
	}
}

static condition_species_type ConditionSpeciesMap(const char *speciesString) {
	condition_species_type retval = DESTROY_ALL_ENEMY_UNITS;
	int i;
	for (i = 0; i < (int)NUM_CONDITION_SPECIES; i += 1) {
		if (0 == strcmp(speciesString, g_conditionSpeciesStringArray[i])) {
			retval = (condition_species_type)i;
			break;
		}
	}
	assert(i < (int)NUM_CONDITION_SPECIES);
	return retval;
}

static action_species_type ActionSpeciesMap(const char *speciesString) {
	action_species_type retval = PLAY_BIK;
	int i;
	for (i = 0; i < (int)NUM_ACTION_SPECIES; i += 1) {
		if (0 == strcmp(speciesString, g_actionSpeciesStringArray[i])) {
			retval = (action_species_type)i;
			break;
		}
	}
	assert(i < (int)NUM_ACTION_SPECIES);
	return retval;
}

bool CObjective::Read( FitIniFile* missionFile, int objectiveNum, int version, int MarkerNum, char secondary )
{
	//Load this up here to make assert at program start go away!

	if ( !s_markerFont )
	{
		s_markerFont = new aFont;
		// need to use this one arialBlack9.d3f
		s_markerFont->init( IDS_OBJECTIVE_MARKER_FONT );
	}

	long result = 0;
	ECharString tmpECStr;
	result = sReadIdString(missionFile, "Title", tmpECStr);
	Title(_T(tmpECStr.Data()));
	result = sReadIdBoolean(missionFile, "TitleUseResourceString", m_titleUseResourceString);
	result = sReadIdWholeNum(missionFile, "TitleResourceStringID", m_titleResourceStringID);
	result = sReadIdString(missionFile, "Description", tmpECStr);
	Description(_T(tmpECStr.Data()));
	result = sReadIdBoolean(missionFile, "DescriptionUseResourceString", m_descriptionUseResourceString);
	result = sReadIdWholeNum(missionFile, "DescriptionResourceStringID", m_descriptionResourceStringID);

	result = sReadIdWholeNum(missionFile, "Priority", m_priority);
	result = sReadIdWholeNum(missionFile, "ResourcePoints", m_resourcePoints);
	result = sReadIdBoolean(missionFile, "PreviousPrimaryObjectiveMustBeComplete", m_previousPrimaryObjectiveMustBeComplete);
	if (NO_ERR != result) {
		result = sReadIdBoolean(missionFile, "PreviousObjectiveMustBeComplete", m_previousPrimaryObjectiveMustBeComplete);
	}
	result = sReadIdBoolean(missionFile, "AllPreviousPrimaryObjectivesMustBeComplete", m_allPreviousPrimaryObjectivesMustBeComplete);

	m_markerText[0] = 0;

	bool tmp;
	result = sReadIdBoolean(missionFile, "DisplayMarker", tmp);
	if ( tmp )
	{
		m_displayMarker = NUMERIC;
		if ( m_priority == 1)
			sprintf( m_markerText, "%ld", MarkerNum );
		else
			sprintf( m_markerText, "%c", secondary );
	}
	result = sReadIdFloat(missionFile, "MarkerX", m_markerX);
	result = sReadIdFloat(missionFile, "MarkerY", m_markerY);
	result = sReadIdBoolean(missionFile, "HiddenTrigger", m_isHiddenTrigger);
	result = sReadIdBoolean(missionFile, "ActivateOnFlag", m_activateOnFlag);
	if (m_activateOnFlag) { m_isActive = false; }
	result = sReadIdString(missionFile, "ActivateFlagID", tmpECStr);
	if (NO_ERR != result) {
		/*for backward compatibility*/
		int flagIndex = 0;
		result = sReadIdWholeNum(missionFile, "ActivateFlagIndex", flagIndex);
		if (NO_ERR == result) {
			m_activateFlagID.Format("%d", flagIndex);
		}
	} else {
		m_activateFlagID = tmpECStr.Data();
	}
	result = sReadIdBoolean(missionFile, "ResetStatusOnFlag", m_resetStatusOnFlag);
	result = sReadIdString(missionFile, "ResetStatusFlagID", tmpECStr);
	if (NO_ERR == result) {
		m_resetStatusFlagID = tmpECStr.Data();
	}

	result = sReadIdString(missionFile, "ObjectiveModel", tmpECStr);
	ModelName(_T(tmpECStr.Data()));
	result = sReadIdLongInt(missionFile, "ModelType", m_modelType);
	result = sReadIdLongInt(missionFile, "BaseColor", m_modelBaseColor);
	result = sReadIdLongInt(missionFile, "HighlightColor", m_modelHighlightColor);
	result = sReadIdLongInt(missionFile, "HighlightColor2", m_modelHighlightColor2);
	result = sReadIdFloat(missionFile, "ModelScale", m_modelScale);

	{
		/*these items should only be present in "in-mission" saved games*/
		/*state data*/
		result = sReadIdBoolean(missionFile, "IsActive", m_isActive);
		float tempFloat;
		result = sReadIdFloat(missionFile, "ActivationTime", tempFloat);
		if (NO_ERR == result) {
			m_activationTime = tempFloat;
		}
		result = sReadIdBoolean(missionFile, "IsResolved", m_resolved);
		int tempInt;
		result = sReadIdLongInt(missionFile, "ResolvedStatus", tempInt);
		if (NO_ERR == result) {
			m_resolvedStatus = (objective_status_type)tempInt;
		}
		result = sReadIdBoolean(missionFile, "ChangedStatus", m_changedStatus);
		/*end state data*/
	}

	int numConditions = 0;
	result = sReadIdWholeNum(missionFile, "NumConditions", numConditions);
	if (NO_ERR != result) { return false; }
	int numActions = 0;
	result = sReadIdWholeNum(missionFile, "NumActions", numActions);
	int numFailureConditions = 0;
	result = sReadIdWholeNum(missionFile, "NumFailureConditions", numFailureConditions);
	int numFailureActions = 0;
	result = sReadIdWholeNum(missionFile, "NumFailureActions", numFailureActions);

	{
		int i;
		for (i = 0; i < numConditions; i += 1) {
			ECharString tmpStr;
			if (2 == version) {
				tmpStr.Format("Objective%dCondition%d", objectiveNum, i);
			} else {
				tmpStr.Format("Team%dObjective%dCondition%d", Alignment(), objectiveNum, i);
			}
			result = missionFile->seekBlock(tmpStr.Data());
			if (NO_ERR != result) { assert(false); continue; }
			condition_species_type species;
			result = sReadIdString(missionFile, "ConditionSpeciesString", tmpECStr);
			if (NO_ERR != result) {
				/* this is just for backward compatibility, this entry is depricated */
				int ispecies = 0;
				result = sReadIdWholeNum(missionFile, "ConditionSpecies", ispecies);
				if (NO_ERR != result) { continue; }
				species = (condition_species_type)ispecies;
			} else {
				species = ConditionSpeciesMap(tmpECStr.Data());
			}

			CObjectiveCondition *pNewObjectiveCondition = new_CObjectiveCondition(species, m_alignment);
			if (!pNewObjectiveCondition) { assert(false); delete pNewObjectiveCondition; continue; }
			bool bresult = pNewObjectiveCondition->Read(missionFile);
			if (true != bresult) { assert(false); delete pNewObjectiveCondition; continue; }
			Append(pNewObjectiveCondition);
		}
	}

	{
		int i;
		for (i = 0; i < numActions; i += 1) {
			ECharString tmpStr;
			tmpStr.Format("Team%dObjective%dAction%d", Alignment(), objectiveNum, i);
			result = missionFile->seekBlock(tmpStr.Data());
			if (NO_ERR != result) { assert(false); continue; }
			action_species_type species;
			result = sReadIdString(missionFile, "ActionSpeciesString", tmpECStr);
			if (NO_ERR != result) { assert(false); continue; }
			species = ActionSpeciesMap(tmpECStr.Data());

			CObjectiveAction *pNewObjectiveAction = new_CObjectiveAction(species, m_alignment);
			if (!pNewObjectiveAction) { assert(false); delete pNewObjectiveAction; continue; }
			bool bresult = pNewObjectiveAction->Read(missionFile);
			if (true != bresult) { assert(false); delete pNewObjectiveAction; continue; }
			m_actionList.Append(pNewObjectiveAction);
		}
	}

	{
		int i;
		for (i = 0; i < numFailureConditions; i += 1) {
			ECharString tmpStr;
			tmpStr.Format("Team%dObjective%dFailureCondition%d", Alignment(), objectiveNum, i);
			result = missionFile->seekBlock(tmpStr.Data());
			if (NO_ERR != result) { assert(false); continue; }
			condition_species_type species;
			result = sReadIdString(missionFile, "FailureConditionSpeciesString", tmpECStr);
			if (NO_ERR != result) { assert(false); continue; }
			species = ConditionSpeciesMap(tmpECStr.Data());

			CObjectiveCondition *pNewObjectiveFailureCondition = new_CObjectiveCondition(species, m_alignment);
			if (!pNewObjectiveFailureCondition) { assert(false); delete pNewObjectiveFailureCondition; continue; }
			bool bresult = pNewObjectiveFailureCondition->Read(missionFile);
			if (true != bresult) { assert(false); delete pNewObjectiveFailureCondition; continue; }
			m_failureConditionList.Append(pNewObjectiveFailureCondition);
		}
	}

	{
		int i;
		for (i = 0; i < numFailureActions; i += 1) {
			ECharString tmpStr;
			tmpStr.Format("Team%dObjective%dFailureAction%d", Alignment(), objectiveNum, i);
			result = missionFile->seekBlock(tmpStr.Data());
			if (NO_ERR != result) { assert(false); continue; }
			action_species_type species;
			result = sReadIdString(missionFile, "FailureActionSpeciesString", tmpECStr);
			if (NO_ERR != result) { assert(false); continue; }
			species = ActionSpeciesMap(tmpECStr.Data());

			CObjectiveAction *pNewObjectiveFailureAction = new_CObjectiveAction(species, m_alignment);
			if (!pNewObjectiveFailureAction) { assert(false); delete pNewObjectiveFailureAction; continue; }
			bool bresult = pNewObjectiveFailureAction->Read(missionFile);
			if (true != bresult) { assert(false); delete pNewObjectiveFailureAction; continue; }
			m_failureActionList.Append(pNewObjectiveFailureAction);
		}
	}
	return true;
}

objective_status_type CObjective::Status() {
	if (!IsActive()) {
		//assert(false);
		return OS_UNDETERMINED;
	}
	if (m_resolved) {
		return m_resolvedStatus;
	} else { 
		objective_status_type objectiveStatus = OS_SUCCESSFUL;
		{
			/*evaluate the status of the success conditions*/
			EIterator it = Begin();
			while (!it.IsDone()) {
				objective_status_type conditionStatus = (*it)->Status();
				if (OS_FAILED == conditionStatus) {
					objectiveStatus = OS_FAILED;
					break;
				} else if (OS_UNDETERMINED == conditionStatus) {
					objectiveStatus = OS_UNDETERMINED;
				}
				it++;
			}
		}
		if ((OS_FAILED != objectiveStatus) && (0 < m_failureConditionList.Count())) {
			/*evaluate the status of the failure conditions*/
			objective_status_type failureStatus = OS_SUCCESSFUL;
			EIterator it = m_failureConditionList.Begin();
			while (!it.IsDone()) {
				objective_status_type conditionStatus = (*it)->Status();
				if (OS_FAILED == conditionStatus) {
					failureStatus = OS_FAILED;
					break;
				} else if (OS_UNDETERMINED == conditionStatus) {
					failureStatus = OS_UNDETERMINED;
					break;
				}
				it++;
			}
			if (OS_SUCCESSFUL == failureStatus) {
				/*the failure conditions are satisfied*/
				objectiveStatus = OS_FAILED;
			}
		}
		if (OS_UNDETERMINED == objectiveStatus) {
			return OS_UNDETERMINED;
		} else if (OS_FAILED == objectiveStatus) {
			m_resolved = true;
			m_changedStatus = true;
			m_resolvedStatus = OS_FAILED;
			if (!IsHiddenTrigger()) {
				soundSystem->playBettySample(BETTY_CANNOT_COMP_OBJ);
			}
			action_list_type::EIterator it = m_failureActionList.Begin();
			while (!it.IsDone()) {
				(*it)->Execute();
				it++;
			}
			return m_resolvedStatus;
		} else {
			m_resolved = true;
			m_changedStatus = true;
			m_resolvedStatus = OS_SUCCESSFUL;
			if (!IsHiddenTrigger()) {
				soundSystem->playBettySample(BETTY_OBJECTIVE_COMPLETE);
			}
			action_list_type::EIterator it = m_actionList.Begin();
			while (!it.IsDone()) {
				(*it)->Execute();
				it++;
			}
			return m_resolvedStatus;
		}
	}
}

objective_status_type CObjective::Status(CObjectives &objectives) {
	if (!IsActive()) {
		//assert(false);
		return OS_UNDETERMINED;
	}

	if ((!PreviousPrimaryObjectiveMustBeComplete()) && !(AllPreviousPrimaryObjectivesMustBeComplete())) {
		return Status();
	}

	objective_status_type retval = OS_SUCCESSFUL;
	objective_status_type statusOfPreviousPrimaryObjective = OS_SUCCESSFUL;
	bool allPreviousPrimaryObjectivesHaveBeenCompleted = true;
	CObjectives::EIterator it = objectives.Begin();
	while (!it.IsDone()) {
		objective_status_type status;
		if (((*it)->AllPreviousPrimaryObjectivesMustBeComplete()) && (!allPreviousPrimaryObjectivesHaveBeenCompleted)) {
			status = OS_UNDETERMINED;
		} else if (((*it)->PreviousPrimaryObjectiveMustBeComplete()) && (OS_SUCCESSFUL != statusOfPreviousPrimaryObjective)) {
			assert(!(*it)->AllPreviousPrimaryObjectivesMustBeComplete());
			assert(OS_UNDETERMINED == statusOfPreviousPrimaryObjective);
			status = statusOfPreviousPrimaryObjective;
		} else {
			status = (*it)->Status();
		}
		if (this == (*it)) {
			return status;
		}
		assert (OS_FAILED != retval);
		if (1 == (*it)->Priority()) {
			if (OS_FAILED ==  status) {
				allPreviousPrimaryObjectivesHaveBeenCompleted = false;
				retval = OS_FAILED;
				/* if one primary objective is failed, it's over */
				break;
			} else if (OS_UNDETERMINED == status) {
				allPreviousPrimaryObjectivesHaveBeenCompleted = false;
				retval = OS_UNDETERMINED;
			}
			statusOfPreviousPrimaryObjective = status;
		}
		it++;
	}
	return retval;

}

void CObjective::Status(objective_status_type newStatus) {
	if (OS_UNDETERMINED == newStatus) {
		m_resolved = false;
	} else {
		m_resolved = true;
	}
	m_resolvedStatus = newStatus;
	m_changedStatus = true;
}

bool CObjective::StatusChangedSuccess (void)
{
	if (!IsActive())
	{
		return false;
	}
	
	if (m_changedStatus)
	{
		m_changedStatus = false;
        //sebi:
		//m_resolvedStatus;
		if (m_resolvedStatus == OS_SUCCESSFUL)
			return true;
	}
	
	return false;
}
									  
bool CObjective::StatusChangedFailed (void)
{
	if (!IsActive())
	{
		return false;
	}
	
	if (m_changedStatus)
	{
		m_changedStatus = false;
		if (m_resolvedStatus == OS_FAILED)
			return true;
	}
	
	return false;
}

EString CObjective::LocalizedTitle() const {
	EString retval;
	if (TitleUseResourceString()) {
		EString EStr;
		ESLoadString(TitleResourceStringID(), EStr);
		retval = EStr.Data();
	} else {
		retval = Title();
	}
	return retval;
}

EString CObjective::LocalizedDescription() const {
	EString retval;
	if (DescriptionUseResourceString()) {
		EString EStr;
		ESLoadString(DescriptionResourceStringID(), EStr);
		retval = EStr.Data();
	} else {
		retval = Description();
	}
	return retval;
}

CObjectives &CObjectives::operator=(const CObjectives &master) {
	m_alignment = master.m_alignment;

	Clear();
	CObjectives::EConstIterator it = master.Begin();
	while (!it.IsDone()) {
		//CObjective *tmpObjective = new CObjective(*(*it));
		CObjective *tmpObjective = new CObjective(m_alignment);
		(*tmpObjective) = (*(*it));
		Append(tmpObjective);
		it++;
	}
	return (*this);
}

void CObjectives::Clear() {
	EIterator it = Begin();
	while (!it.IsDone()) {
		delete (*it);
		it++;
	}
	inherited::Clear();
	boolFlags.Clear();
}

void CObjectives::Alignment(int alignment) {
	m_alignment = alignment;
	EIterator it = Begin();
	while (!it.IsDone()) {
		(*it)->Alignment(m_alignment);
		it++;
	}
}

bool CObjectives::Read( FitIniFile* missionFile )
{
	long result = 0;
	result = missionFile->seekBlock("Objectives Version");
	if (NO_ERR != result) { assert(false); }
	int objectivesVersion = 0;
	result = sReadIdWholeNum(missionFile, "Version", objectivesVersion);
	if (result != NO_ERR) 
	{
		assert(false); return false;
	} 
	else 
	{
		if (2 == objectivesVersion) 
		{
			if (0 != Alignment()) 
			{
				// assign version 2 objectives to team1 only
				return true;
			}
			result = missionFile->seekBlock("Objectives");
		} 
		else 
		{
			ECharString tmpStr;
			tmpStr.Format("Team%dObjectives", Alignment());
			result = missionFile->seekBlock(tmpStr.Data());
		}

		if (NO_ERR != result) 
		{ 
			assert(false); 
		}

		int numObjectives = 0;
		result = sReadIdWholeNum(missionFile, "NumObjectives", numObjectives);
		if (NO_ERR != result) 
		{ 
			return false; 
		}

		boolFlags.load(Alignment(),missionFile);

		int i;
		int markerI = 1;
		char secondaryMarkers = 'a';
		for (i = 0; i < numObjectives; i += 1) {
			ECharString tmpStr;
			if (2 == objectivesVersion) {
				tmpStr.Format("Objective%d", i);
			} else {
				tmpStr.Format("Team%dObjective%d", Alignment(), i);
			}
			result = missionFile->seekBlock(tmpStr.Data());
			if (NO_ERR != result) { assert(false); continue; }
			CObjective *pNewObjective = new CObjective(m_alignment);
			bool bresult = pNewObjective->Read(missionFile, i, objectivesVersion, markerI, secondaryMarkers);
			
			if ( 1 == pNewObjective->Priority() )
				markerI++;
			else
				secondaryMarkers++;
			
			if (true != bresult) { assert(false); delete pNewObjective; continue; }
			Append(pNewObjective);
		}
	}
	return true;
}

objective_status_type CObjectives::Status() {
	objective_status_type retval = OS_SUCCESSFUL;
	objective_status_type statusOfPreviousPrimaryObjective = OS_SUCCESSFUL;
	bool allPreviousPrimaryObjectivesHaveBeenCompleted = true;
	EIterator it = Begin();
	while (!it.IsDone()) {
		if ((*it)->IsActive()) {
			/* check if objective status needs to be reset */
			if ((*it)->ResetStatusOnFlag()) {
				if (Team::teams[Alignment()]->objectives.boolFlags.getElementValue((*it)->ResetStatusFlagID())) {
					Team::teams[Alignment()]->objectives.boolFlags.setElementValue((*it)->ResetStatusFlagID(), false);
					(*it)->Status(OS_UNDETERMINED);
					//mission->missionInterface->showObjectives(true);
				}
			}
			objective_status_type status;
			if (((*it)->AllPreviousPrimaryObjectivesMustBeComplete()) && (!allPreviousPrimaryObjectivesHaveBeenCompleted)) {
				status = OS_UNDETERMINED;
			} else if (((*it)->PreviousPrimaryObjectiveMustBeComplete()) && (OS_SUCCESSFUL != statusOfPreviousPrimaryObjective)) {
				assert(!(*it)->AllPreviousPrimaryObjectivesMustBeComplete());
				assert(OS_UNDETERMINED == statusOfPreviousPrimaryObjective);
				status = statusOfPreviousPrimaryObjective;
			} else {
				status = (*it)->Status();
			}
			assert (OS_FAILED != retval);
			if (1 == (*it)->Priority()) {
				if (OS_FAILED ==  status) {
					allPreviousPrimaryObjectivesHaveBeenCompleted = false;
					retval = OS_FAILED;
					/* if one primary objective is failed, it's over */
					break;
				} else if (OS_UNDETERMINED == status) {
					allPreviousPrimaryObjectivesHaveBeenCompleted = false;
					retval = OS_UNDETERMINED;
				}
				statusOfPreviousPrimaryObjective = status;
			}
		} else {
			/* check if inactive objective needs to become active */
			if ((*it)->ActivateOnFlag()) {
				if (1 == (*it)->Priority()) {
					/* do not end the mission if a primary objective is still inactive */
					retval = OS_UNDETERMINED;
				}
				//if (mission->missionObjectives.boolFlags.getElementValue((*it)->ActivateFlagID())) {
				if (Team::teams[Alignment()]->objectives.boolFlags.getElementValue((*it)->ActivateFlagID())) {
					(*it)->IsActive(true);
					(*it)->ActivationTime(mission->actualTime);
					mission->missionInterface->showObjectives(true);
					//mission->missionInterface->controlGui.handleClick(OBJECTIVES_COMMAND);
				}
			}
		}
		it++;
	}

	return retval;
}

/*void CObjective::Render( unsigned long xPos, unsigned long yPos, HGOSFONT3D guiFont )
{
	unsigned long fontWidth, fontHeight;
	gos_TextStringLength( &fontWidth, &fontHeight, "ABC" );
	
	// draw little box first
	GUI_RECT rect = { xPos, yPos, xPos + fontHeight, yPos + fontHeight };
	drawEmptyRect( rect, 0xffffffff, 0xffffffff );

	unsigned long color = 0xffffffff;

	if ( m_resolved )
	{
		color = m_resolvedStatus == OS_FAILED ?  0xffff0000 : 0xff00ff00;
		gos_TextSetAttributes( guiFont, color, 1.0, false, true, false, false, 0 );	
		gos_TextSetPosition(xPos + (fontWidth >> 1), yPos );

		if ( m_resolvedStatus == OS_FAILED )
			gos_TextDraw( "X" );
		else if ( m_resolvedStatus == OS_SUCCESSFUL )
		{
			gos_TextDraw( "V" );
		}	
	}
	
	gos_TextSetAttributes( guiFont, color, 1.0, false, true, false, false, 0 );	
	gos_TextSetPosition(xPos + 2 * fontWidth, yPos );

	gos_TextDraw( (const char*)m_description );

}*/

bool CObjective::RenderMarkers(GameTacMap *tacMap, bool blink ) 
{
	bool bRetVal = 0;
	if (m_resolved || !m_displayMarker || !m_isActive)
	{
		//Draw Nothing.  Objective is resolved or hidden
	} 
	else 
	{
			Stuff::Vector3D objectivePos;
			objectivePos.x = m_markerX;
			objectivePos.y = m_markerY;
			long color = SD_YELLOW;

			if ( m_displayMarker == NAV )
			{
				bRetVal = 1; // true if we drew something

				if ( blink )
				{
					s_lastBlinkTime += frameLength;
					if ( s_lastBlinkTime > s_blinkLength )
					{
						s_lastBlinkTime = 0.f;
						if ( !s_blinkColor )
							s_blinkColor = SD_YELLOW;
						else
							s_blinkColor = 0;
					}
					color = s_blinkColor;

				}

			}
			else
			{
				color = 0xffff0000;
				gos_VERTEX pos;
				tacMap->worldToTacMap(objectivePos, pos);

				unsigned long width, height;
				height = s_markerFont->height();
				width = s_markerFont->width( m_markerText );
				drawShadowText( 0xffffffff, 0xff000000, s_markerFont->getTempHandle(), pos.x - width/2, pos.y - height/2, true, m_markerText, 0, s_markerFont->getSize(), -2, 2 );

			}

	}

	return bRetVal;
}


/* Reads Nav Marker info and adds appropriate "hidden trigger" objectives. */
bool ReadNavMarkers( FitIniFile* missionFile, CObjectives &objectives )
{
	long result = 0;
	result = missionFile->seekBlock("NavMarkers");
	if (NO_ERR != result) { /*assert(false);*/ return false; }
	int numNavMarkers = 0;
	result = sReadIdWholeNum(missionFile, "NumNavMarkers", numNavMarkers);
	if (NO_ERR != result) { return false; }
	int i;
	for (i = 0; i < numNavMarkers; i += 1) {
		ECharString tmpStr;
		tmpStr.Format("NavMarker%d", i);
		result = missionFile->seekBlock(tmpStr.Data());
		if (NO_ERR != result) { assert(false); continue; }
		CObjective *pNewObjective = new CObjective(objectives.Alignment());

		ECharString tmpECStr;
		tmpECStr.Format("Nav Marker %d", i);
		pNewObjective->Title(tmpECStr.Data());
		pNewObjective->Priority(2);
		pNewObjective->IsHiddenTrigger(true);
		pNewObjective->DisplayMarker( 0 );

		float targetCenterX, targetCenterY, targetRadius;
		result = sReadIdFloat(missionFile, "xPos", targetCenterX);
		if (NO_ERR != result) { assert(false); delete pNewObjective; continue; }
		result = sReadIdFloat(missionFile, "yPos", targetCenterY);
		if (NO_ERR != result) { assert(false); delete pNewObjective; continue; }
		result = sReadIdFloat(missionFile, "radius", targetRadius);
		if (NO_ERR != result) { assert(false); delete pNewObjective; continue; }
		pNewObjective->MarkerX( targetCenterX );
		pNewObjective->MarkerY( targetCenterY );

		CMoveAnyUnitToArea *pCondition = new CMoveAnyUnitToArea(pNewObjective->Alignment());
		pCondition->SetParams(targetCenterX, targetCenterY, targetRadius);
		pNewObjective->Append(pCondition);

		C_RemoveStructure *pAction = new C_RemoveStructure(pNewObjective->Alignment());
		bool bresult = pAction->SetParams(targetCenterX, targetCenterY);
		if (true != bresult) { assert(false); delete pAction; delete pNewObjective; continue; }
		pNewObjective->m_actionList.Append(pAction);

		objectives.Append(pNewObjective);
	}
	return true;
}

int C_RemoveStructure::Execute()
{
	Stuff::Vector3D ludicrousPos;
	ludicrousPos.x = 100000.f;
	ludicrousPos.y = 100000.f;
	ludicrousPos.z = 100000.f;

	Building * m_pBuilding = (Building *)ObjectManager->getByWatchID(m_pBuildingWID);
	m_pBuilding->position = ludicrousPos;
	m_pBuilding->update();
	return true;
}

//*************************************************************************************************
bool CNumberOfUnitsObjectiveCondition::Save( FitIniFile* file )
{
	file->writeIdULong( "Num", m_num );
	return true;
}

bool CSpecificUnitObjectiveCondition::Save( FitIniFile* file )
{
	if (ObjectManager->getByWatchID(m_pUnitWID) == NULL)
		STOP(("Tried to save specific unit objective with NULL unit"));

	file->writeIdLong( "MoverWID", m_pUnitWID );

	return true;
}

bool C_RemoveStructure::Save( FitIniFile* file )
{
	if (ObjectManager->getByWatchID(m_pBuildingWID) == NULL)
		STOP(("Tried to save specific Building objective with NULL Building"));

	file->writeIdLong( "BuildingWID", m_pBuildingWID );

	return true;
}

bool CSpecificStructureObjectiveCondition::Save( FitIniFile* file )
{
	if (ObjectManager->getByWatchID(m_pBuildingWID) == NULL)
		STOP(("Tried to save specific Bduiling objective with NULL Building"));

	file->writeIdLong( "BuildingWID", m_pBuildingWID );

	return true;
}

bool CAreaObjectiveCondition::Save( FitIniFile* file )
{
	file->writeIdFloat( "TargetCenterX", m_targetCenterX );
	file->writeIdFloat( "TargetCenterY", m_targetCenterY );
	file->writeIdFloat( "TargetRadius", m_targetRadius );
	return true;
}

bool CBooleanFlagIsSet::Save( FitIniFile* file )
{
	file->writeIdString("FlagID", m_flagID.Data());
	file->writeIdBoolean("Value", m_value);
	return true;
}

bool CElapsedMissionTime::Save( FitIniFile* file )
{
	file->writeIdFloat("Time", m_time);
	return true;
}

bool CPlayBIK::Save( FitIniFile* file )
{
	file->writeIdString( "AVIFileName", m_pathname.Data());
	return true;
}

bool CPlayWAV::Save( FitIniFile* file )
{
	file->writeIdString( "WAVFileName", m_pathname.Data());
	return true;
}

bool CDisplayTextMessage::Save( FitIniFile* file )
{
	file->writeIdString( "Message", m_message.Data());
	return true;
}

bool CDisplayResourceTextMessage::Save( FitIniFile* file )
{
	file->writeIdULong( "ResourceStringID", m_resourceStringID);
	return true;
}

bool CSetBooleanFlag::Save( FitIniFile* file )
{
	file->writeIdString("FlagID", m_flagID.Data());
	file->writeIdBoolean("Value", m_value);
	return true;
}

bool CMakeNewTechnologyAvailable::Save( FitIniFile* file )
{
	file->writeIdString( "PurchaseFileName", m_purchaseFilePathname.Data());
	return true;
}

bool CObjective::Save( FitIniFile* file, int objectiveNum )
{
	file->writeIdString( "Title", Title().Data());
	file->writeIdBoolean( "TitleUseResourceString", m_titleUseResourceString );
	file->writeIdULong( "TitleResourceStringID", m_titleResourceStringID );
	file->writeIdString( "Description", Description().Data());
	file->writeIdBoolean( "DescriptionUseResourceString", m_descriptionUseResourceString );
	file->writeIdULong( "DescriptionResourceStringID", m_descriptionResourceStringID );
	file->writeIdULong( "Priority", m_priority );
	file->writeIdULong( "ResourcePoints", m_resourcePoints );
	file->writeIdBoolean( "PreviousPrimaryObjectiveMustBeComplete", m_previousPrimaryObjectiveMustBeComplete );
	file->writeIdBoolean( "AllPreviousPrimaryObjectivesMustBeComplete", m_allPreviousPrimaryObjectivesMustBeComplete );
	file->writeIdBoolean( "DisplayMarker", m_displayMarker );
	file->writeIdFloat( "MarkerX", m_markerX );
	file->writeIdFloat( "MarkerY", m_markerY );
	file->writeIdBoolean( "HiddenTrigger", m_isHiddenTrigger );
	file->writeIdBoolean( "ActivateOnFlag", m_activateOnFlag );
	file->writeIdString( "ActivateFlagID", m_activateFlagID.Data() );
	file->writeIdBoolean( "ResetStatusOnFlag", m_resetStatusOnFlag );
	file->writeIdString( "ResetStatusFlagID", m_resetStatusFlagID.Data() );

	file->writeIdULong( "NumConditions", Count() );
	file->writeIdULong( "NumActions", m_actionList.Count() );
	file->writeIdULong( "NumFailureConditions", m_failureConditionList.Count() );
	file->writeIdULong( "NumFailureActions", m_failureActionList.Count() );

	if (m_modelName.Data())
		file->writeIdString( "ObjectiveModel", m_modelName.Data() );

	file->writeIdLong( "ModelType", m_modelType );
	file->writeIdLong( "BaseColor", m_modelBaseColor );
	file->writeIdLong( "HighlightColor", m_modelHighlightColor );
	file->writeIdLong( "HighlightColor2", m_modelHighlightColor2 );
//	file->writeIdLong( "ModelID", m_modelID );
	file->writeIdFloat( "ModelScale", m_modelScale );

	/*state data*/
	file->writeIdBoolean( "IsActive", m_isActive );
	file->writeIdFloat( "ActivationTime", m_activationTime );
	file->writeIdBoolean( "IsResolved", m_resolved );
	file->writeIdLong( "ResolvedStatus", (int)m_resolvedStatus );
	file->writeIdBoolean( "ChangedStatus", m_changedStatus );
	/*end state data*/

	{
		int i = 0;
		EIterator it = Begin();
		while (!it.IsDone()) {
			ECharString tmpStr;
			tmpStr.Format("Team%dObjective%dCondition%d", Alignment(), objectiveNum, i);
			file->writeBlock( tmpStr.Data() );
			file->writeIdULong( "ConditionSpecies", (unsigned long)(*it)->Species());
			file->writeIdString( "ConditionSpeciesString", g_conditionSpeciesStringArray[(int)(*it)->Species()]);
			(*it)->Save(file);
			i += 1;
			it++;
		}
	}

	{
		int i = 0;
		action_list_type::EIterator it = m_actionList.Begin();
		while (!it.IsDone()) {
			ECharString tmpStr;
			tmpStr.Format("Team%dObjective%dAction%d", Alignment(), objectiveNum, i);
			file->writeBlock( tmpStr.Data() );
			file->writeIdULong( "ActionSpecies", (unsigned long)(*it)->Species());
			file->writeIdString( "ActionSpeciesString", g_actionSpeciesStringArray[(int)(*it)->Species()]);
			(*it)->Save(file);
			i += 1;
			it++;
		}
	}

	{
		int i = 0;
		condition_list_type::EIterator it = m_failureConditionList.Begin();
		while (!it.IsDone()) {
			ECharString tmpStr;
			tmpStr.Format("Team%dObjective%dFailureCondition%d", Alignment(), objectiveNum, i);
			file->writeBlock( tmpStr.Data() );
			file->writeIdULong( "FailureConditionSpecies", (unsigned long)(*it)->Species());
			file->writeIdString( "FailureConditionSpeciesString", g_conditionSpeciesStringArray[(int)(*it)->Species()]);
			(*it)->Save(file);
			i += 1;
			it++;
		}
	}

	{
		int i = 0;
		action_list_type::EIterator it = m_failureActionList.Begin();
		while (!it.IsDone()) {
			ECharString tmpStr;
			tmpStr.Format("Team%dObjective%dFailureAction%d", Alignment(), objectiveNum, i);
			file->writeBlock( tmpStr.Data() );
			file->writeIdULong( "FailureActionSpecies", (unsigned long)(*it)->Species());
			file->writeIdString( "FailureActionSpeciesString", g_actionSpeciesStringArray[(int)(*it)->Species()]);
			(*it)->Save(file);
			i += 1;
			it++;
		}
	}

	return true;
}

bool CObjectives::Save( FitIniFile* file )
{
 	file->writeBlock("Objectives Version");
	file->writeIdULong("Version", 3);

	ECharString tmpStr;
	tmpStr.Format("Team%dObjectives", Alignment());
	file->writeBlock( tmpStr.Data() );
	file->writeIdULong( "NumObjectives", Count() );
	int i = 0;

	boolFlags.save(Alignment(),file);

	EIterator it = Begin();
	while (!it.IsDone()) {
		ECharString tmpStr;
		tmpStr.Format("Team%dObjective%d", Alignment(), i);
		file->writeBlock( tmpStr.Data() );
		(*it)->Save(file, i);
		i += 1;
		it++;
	}

	return true;
}

//*************************************************************************************************
// end of file ( Objective.cpp )
