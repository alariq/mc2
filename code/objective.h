#ifndef OBJECTIVE_H
#define OBJECTIVE_H
/*************************************************************************************************\
Objective.h			: Interface for the Objective component.
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
\*************************************************************************************************/

#include"elist.h"
#include"estring.h"
#include"platform_tchar.h"
#include"mover.h"

#ifndef BLDNG_H
#include"bldng.h"
#endif

#ifndef GAMETACMAP_H
#include"gametacmap.h"
#endif

class FitIniFile;

class aFont;

//*************************************************************************************************

/**************************************************************************************************
CLASS DESCRIPTION
CObjectiveCondition:
**************************************************************************************************/
enum condition_species_type {
	DESTROY_ALL_ENEMY_UNITS,
	DESTROY_NUMBER_OF_ENEMY_UNITS,
	DESTROY_ENEMY_UNIT_GROUP,
	DESTROY_SPECIFIC_ENEMY_UNIT,
	DESTROY_SPECIFIC_STRUCTURE,

	CAPTURE_OR_DESTROY_ALL_ENEMY_UNITS,
	CAPTURE_OR_DESTROY_NUMBER_OF_ENEMY_UNITS,
	CAPTURE_OR_DESTROY_ENEMY_UNIT_GROUP,
	CAPTURE_OR_DESTROY_SPECIFIC_ENEMY_UNIT,
	CAPTURE_OR_DESTROY_SPECIFIC_STRUCTURE,

	DEAD_OR_FLED_ALL_ENEMY_UNITS,
	DEAD_OR_FLED_NUMBER_OF_ENEMY_UNITS,
	DEAD_OR_FLED_ENEMY_UNIT_GROUP,
	DEAD_OR_FLED_SPECIFIC_ENEMY_UNIT,

	CAPTURE_UNIT,
	CAPTURE_STRUCTURE,

	GUARD_SPECIFIC_UNIT,
	GUARD_SPECIFIC_STRUCTURE,

	MOVE_ANY_UNIT_TO_AREA,
	MOVE_ALL_UNITS_TO_AREA,
	MOVE_ALL_SURVIVING_UNITS_TO_AREA,
	MOVE_ALL_SURVIVING_MECHS_TO_AREA,

	BOOLEAN_FLAG_IS_SET,
	ELAPSED_MISSION_TIME,

	NUM_CONDITION_SPECIES
};

enum MARKER_TYPES
{
	NO_MARKER = 0,
	NAV = 1,
	NUMERIC = 2
};

extern const char *g_conditionSpeciesStringArray[];

enum objective_status_type {
	OS_UNDETERMINED = 0,		//Make sure first one is zero just to be safe
	OS_SUCCESSFUL,
	OS_FAILED
};

class CEStringList : public EList<EString, EString> {};
class CBoolList : public EList<bool, bool> {};

class CBooleanArray {
private:
	CEStringList m_FlagIDList;
	CBoolList m_valueList;
public:
	CBooleanArray() {}
	~CBooleanArray() {}
	void Clear() {
		m_FlagIDList.Clear();
		m_valueList.Clear();
	}
	int elementPos(EString element) {
		bool elementFound = false;
		int pos = 0;
		CEStringList::EIterator flagIDListIter;
		for (flagIDListIter = m_FlagIDList.Begin(); !flagIDListIter.IsDone(); flagIDListIter++) {
			if ((*flagIDListIter) == element) {
				elementFound = true;
				break;
			}
			pos += 1;
		}
		if (elementFound) {
			return pos;
		} else {
			return -1;
		}
	}
	void setElementValue(EString element, bool value) {
		int pos = elementPos(element);
		if (-1 != pos) {
			m_valueList.Replace(value, pos);
		} else {
			m_FlagIDList.Append(element);
			m_valueList.Append(value);
		}
	}
	bool getElementValue(EString element) {
		int pos = elementPos(element);
		if (-1 != pos) {
			return m_valueList[pos];
		} else {
			/*assert(false);*/
			return false;
		}
	}

	void save (long alignment, FitIniFile *file);

	void load (long alignment, FitIniFile *file);
};

class CObjectiveCondition {
private:
	int m_alignment;
public:
	CObjectiveCondition(int alignment) { m_alignment = alignment; }
	virtual ~CObjectiveCondition() {}
	int Alignment() { return m_alignment; }
	void Alignment(int alignment) { m_alignment = alignment; }
	virtual condition_species_type Species() = 0;
	virtual bool Init() { return true; }
	virtual bool Read( FitIniFile* missionFile ) { return true; }
	virtual bool Save( FitIniFile* file ) { return true; }
	virtual objective_status_type Status() = 0;
	virtual EString Description() = 0;
	virtual EString InstanceDescription() { EString retval; return retval; }
	virtual void CastAndCopy(const CObjectiveCondition *pMaster) { (*this) = (*pMaster); }
	virtual Stuff::Vector3D GetObjectivePosition()		//Used to draw on tacmap
	{
		return Stuff::Vector3D(-999999.0f,-999999.0f,-999999.0f);
	}
};

class CDestroyAllEnemyUnits: public CObjectiveCondition {
public:
	CDestroyAllEnemyUnits(int alignment) : CObjectiveCondition(alignment) {}
	condition_species_type Species() { return DESTROY_ALL_ENEMY_UNITS; }
	objective_status_type Status();
	EString Description() {
		EString retval = "DestroyAllEnemyUnits"; /* needs to be put somewhere localizable */
		return retval;
	}
};

class CNumberOfUnitsObjectiveCondition: public CObjectiveCondition { /*abstract class*/
protected:
	int m_num;
public:
	CNumberOfUnitsObjectiveCondition(int alignment) : CObjectiveCondition(alignment) { m_num = 0; }
	virtual bool Read( FitIniFile* missionFile );
	virtual bool Save( FitIniFile* file );
	virtual EString InstanceDescription();
	virtual void CastAndCopy(const CObjectiveCondition *pMaster) { (*this) = (*(dynamic_cast<const CNumberOfUnitsObjectiveCondition *>(pMaster))); }
};

class CDestroyNumberOfEnemyUnits: public CNumberOfUnitsObjectiveCondition {
public:
	CDestroyNumberOfEnemyUnits(int alignment) : CNumberOfUnitsObjectiveCondition(alignment) {}
	condition_species_type Species() { return DESTROY_NUMBER_OF_ENEMY_UNITS; }
	objective_status_type Status();
	EString Description() {
		EString retval = "DestroyNumberOfEnemyUnits"; /* needs to be put somewhere localizable */
		return retval;
	}
};

class CSpecificUnitObjectiveCondition: public CObjectiveCondition { /*abstract class*/
protected:
	GameObjectWatchID       m_pUnitWID;

public:
	CSpecificUnitObjectiveCondition(int alignment) : CObjectiveCondition(alignment) { m_pUnitWID = 0; }
	virtual EString InstanceDescription();
	virtual void CastAndCopy(const CObjectiveCondition *pMaster) { (*this) = (*(dynamic_cast<const CSpecificUnitObjectiveCondition *>(pMaster))); }
	virtual bool Save( FitIniFile* file );
};

class CSpecificEnemyUnitObjectiveCondition: public CSpecificUnitObjectiveCondition { /*abstract class*/
public:
	CSpecificEnemyUnitObjectiveCondition(int alignment) : CSpecificUnitObjectiveCondition(alignment) {}
	bool Read( FitIniFile* missionFile );
};

class CDestroySpecificEnemyUnit: public CSpecificEnemyUnitObjectiveCondition {
public:
	CDestroySpecificEnemyUnit(int alignment) : CSpecificEnemyUnitObjectiveCondition(alignment) {}
	condition_species_type Species() { return DESTROY_SPECIFIC_ENEMY_UNIT; }
	objective_status_type Status();
	EString Description() {
		EString retval = "DestroySpecificEnemyUnit"; /* needs to be put somewhere localizable */
		return retval;
	}
};

class CSpecificStructureObjectiveCondition: public CObjectiveCondition { /*abstract class*/
protected:
	long m_pBuildingWID;
public:
	CSpecificStructureObjectiveCondition(int alignment) : CObjectiveCondition(alignment) { m_pBuildingWID = 0; }
	virtual bool Read( FitIniFile* missionFile );
	virtual bool Save( FitIniFile* file );
	virtual EString InstanceDescription();
	virtual void CastAndCopy(const CObjectiveCondition *pMaster) { (*this) = (*(dynamic_cast<const CSpecificStructureObjectiveCondition *>(pMaster))); }
	
	virtual Stuff::Vector3D GetObjectivePosition()		//Used to draw on tacmap
	{
		Building *m_pBuilding = (Building *)ObjectManager->getByWatchID(m_pBuildingWID);
		if (m_pBuilding)
			return m_pBuilding->getPosition();
			
		return Stuff::Vector3D(-999999.0f,-999999.0f,-999999.0f);
	}
};

class CDestroySpecificStructure: public CSpecificStructureObjectiveCondition {
public:
	CDestroySpecificStructure(int alignment) : CSpecificStructureObjectiveCondition(alignment) {}
	condition_species_type Species() { return DESTROY_SPECIFIC_STRUCTURE; }
	objective_status_type Status();
	EString Description() {
		EString retval = "DestroySpecificStructure"; /* needs to be put somewhere localizable */
		return retval;
	}
};

class CCaptureOrDestroyAllEnemyUnits: public CObjectiveCondition {
public:
	CCaptureOrDestroyAllEnemyUnits(int alignment) : CObjectiveCondition(alignment) {}
	condition_species_type Species() { return CAPTURE_OR_DESTROY_ALL_ENEMY_UNITS; }
	objective_status_type Status();
	EString Description() {
		EString retval = "CaptureOrDestroyAllEnemyUnits"; /* needs to be put somewhere localizable */
		return retval;
	}
};

class CCaptureOrDestroyNumberOfEnemyUnits: public CNumberOfUnitsObjectiveCondition {
public:
	CCaptureOrDestroyNumberOfEnemyUnits(int alignment) : CNumberOfUnitsObjectiveCondition(alignment) {}
	condition_species_type Species() { return CAPTURE_OR_DESTROY_NUMBER_OF_ENEMY_UNITS; }
	objective_status_type Status();
	EString Description() {
		EString retval = "CaptureOrDestroyNumberOfEnemyUnits"; /* needs to be put somewhere localizable */
		return retval;
	}
};

class CCaptureOrDestroySpecificEnemyUnit: public CSpecificEnemyUnitObjectiveCondition {
public:
	CCaptureOrDestroySpecificEnemyUnit(int alignment) : CSpecificEnemyUnitObjectiveCondition(alignment) {}
	condition_species_type Species() { return CAPTURE_OR_DESTROY_SPECIFIC_ENEMY_UNIT; }
	bool Read( FitIniFile* missionFile );
	objective_status_type Status();
	EString Description() {
		EString retval = "CaptureOrDestroySpecificEnemyUnit"; /* needs to be put somewhere localizable */
		return retval;
	}
};

class CCaptureOrDestroySpecificStructure: public CSpecificStructureObjectiveCondition {
public:
	CCaptureOrDestroySpecificStructure(int alignment) : CSpecificStructureObjectiveCondition(alignment) {}
	condition_species_type Species() { return CAPTURE_OR_DESTROY_SPECIFIC_STRUCTURE; }
	bool Read( FitIniFile* missionFile );
	objective_status_type Status();
	EString Description() {
		EString retval = "CaptureOrDestroySpecificStructure"; /* needs to be put somewhere localizable */
		return retval;
	}
};

class CDeadOrFledAllEnemyUnits: public CObjectiveCondition {
public:
	CDeadOrFledAllEnemyUnits(int alignment) : CObjectiveCondition(alignment) {}
	condition_species_type Species() { return DEAD_OR_FLED_ALL_ENEMY_UNITS; }
	objective_status_type Status();
	EString Description() {
		EString retval = "DeadOrFledAllEnemyUnits"; /* needs to be put somewhere localizable */
		return retval;
	}
};

class CDeadOrFledNumberOfEnemyUnits: public CNumberOfUnitsObjectiveCondition {
public:
	CDeadOrFledNumberOfEnemyUnits(int alignment) : CNumberOfUnitsObjectiveCondition(alignment) {}
	condition_species_type Species() { return DEAD_OR_FLED_NUMBER_OF_ENEMY_UNITS; }
	objective_status_type Status();
	EString Description() {
		EString retval = "DeadOrFledNumberOfEnemyUnits"; /* needs to be put somewhere localizable */
		return retval;
	}
};

class CDeadOrFledSpecificEnemyUnit: public CSpecificEnemyUnitObjectiveCondition {
public:
	CDeadOrFledSpecificEnemyUnit(int alignment) : CSpecificEnemyUnitObjectiveCondition(alignment) {}
	condition_species_type Species() { return DEAD_OR_FLED_SPECIFIC_ENEMY_UNIT; }
	objective_status_type Status();
	EString Description() {
		EString retval = "DeadOrFledSpecificEnemyUnit"; /* needs to be put somewhere localizable */
		return retval;
	}
};

class CCaptureUnit: public CSpecificEnemyUnitObjectiveCondition {
public:
	CCaptureUnit(int alignment) : CSpecificEnemyUnitObjectiveCondition(alignment) {}
	condition_species_type Species() { return CAPTURE_UNIT; }
	bool Read( FitIniFile* missionFile );
	objective_status_type Status();
	EString Description() {
		EString retval = "CaptureSpecificUnit"; /* needs to be put somewhere localizable */
		return retval;
	}
};

class CCaptureStructure: public CSpecificStructureObjectiveCondition {
public:
	CCaptureStructure(int alignment) : CSpecificStructureObjectiveCondition(alignment) {}
	condition_species_type Species() { return CAPTURE_STRUCTURE; }
	bool Read( FitIniFile* missionFile );
	objective_status_type Status();
	EString Description() {
		EString retval = "CaptureStructure"; /* needs to be put somewhere localizable */
		return retval;
	}
};

class CGuardSpecificUnit: public CSpecificUnitObjectiveCondition {
public:
	CGuardSpecificUnit(int alignment) : CSpecificUnitObjectiveCondition(alignment) {}
	condition_species_type Species() { return GUARD_SPECIFIC_UNIT; }
	bool Read( FitIniFile* missionFile );
	objective_status_type Status();
	EString Description() {
		EString retval = "GuardSpecificUnit"; /* needs to be put somewhere localizable */
		return retval;
	}
};

class CGuardSpecificStructure: public CSpecificStructureObjectiveCondition {
public:
	CGuardSpecificStructure(int alignment) : CSpecificStructureObjectiveCondition(alignment) {}
	condition_species_type Species() { return GUARD_SPECIFIC_STRUCTURE; }
	objective_status_type Status();
	EString Description() {
		EString retval = "GuardSpecificStructure"; /* needs to be put somewhere localizable */
		return retval;
	}
};

class CAreaObjectiveCondition: public CObjectiveCondition { /*abstract class*/
protected:
	float m_targetCenterX;
	float m_targetCenterY;
	float m_targetRadius;
public:
	CAreaObjectiveCondition(int alignment) : CObjectiveCondition(alignment) { m_targetCenterX = 0.0; m_targetCenterY = 0.0; m_targetRadius = 0.0; }
	virtual bool SetParams(float targetCenterX, float targetCenterY, float targetRadius) {
		m_targetCenterX = targetCenterX; m_targetCenterY = targetCenterY; m_targetRadius = targetRadius; return true;
	}
	virtual bool Read( FitIniFile* missionFile );
	virtual bool Save( FitIniFile* file );
	virtual EString InstanceDescription();
	virtual void CastAndCopy(const CObjectiveCondition *pMaster) { (*this) = (*(dynamic_cast<const CAreaObjectiveCondition *>(pMaster))); }
	
	virtual Stuff::Vector3D GetObjectivePosition()		//Used to draw on tacmap
	{
 		return Stuff::Vector3D(m_targetCenterX,m_targetCenterY,0.0f);
	}
};

class CMoveAnyUnitToArea: public CAreaObjectiveCondition {
public:
	CMoveAnyUnitToArea(int alignment) : CAreaObjectiveCondition(alignment) {}
	condition_species_type Species() { return MOVE_ANY_UNIT_TO_AREA; }
	objective_status_type Status();
	EString Description() {
		EString retval = "MoveAnyUnitToArea"; /* needs to be put somewhere localizable */
		return retval;
	}
};

class CMoveAllUnitsToArea: public CAreaObjectiveCondition {
public:
	CMoveAllUnitsToArea(int alignment) : CAreaObjectiveCondition(alignment) {}
	condition_species_type Species() { return MOVE_ALL_UNITS_TO_AREA; }
	objective_status_type Status();
	EString Description() {
		EString retval = "MoveAllUnitsToArea"; /* needs to be put somewhere localizable */
		return retval;
	}
};

class CMoveAllSurvivingUnitsToArea: public CAreaObjectiveCondition {
public:
	CMoveAllSurvivingUnitsToArea(int alignment) : CAreaObjectiveCondition(alignment) {}
	condition_species_type Species() { return MOVE_ALL_SURVIVING_UNITS_TO_AREA; }
	objective_status_type Status();
	EString Description() {
		EString retval = "MoveAllSurvivingUnitsToArea"; /* needs to be put somewhere localizable */
		return retval;
	}
};

class CMoveAllSurvivingMechsToArea: public CAreaObjectiveCondition {
public:
	CMoveAllSurvivingMechsToArea(int alignment) : CAreaObjectiveCondition(alignment) {}
	condition_species_type Species() { return MOVE_ALL_SURVIVING_MECHS_TO_AREA; }
	objective_status_type Status();
	EString Description() {
		EString retval = "MoveAllSurvivingMechsToArea"; /* needs to be put somewhere localizable */
		return retval;
	}
};

class CBooleanFlagIsSet: public CObjectiveCondition {
protected:
	EString m_flagID;
	bool m_value;
public:
	CBooleanFlagIsSet(int alignment) : CObjectiveCondition(alignment) { m_flagID = _TEXT("flag0"); m_value = true; }
	condition_species_type Species() { return BOOLEAN_FLAG_IS_SET; }
	bool Read( FitIniFile* missionFile );
	bool Save( FitIniFile* file );
	objective_status_type Status();
	EString Description() {
		EString retval = "BooleanFlagIsSet"; /* needs to be put somewhere localizable */
		return retval;
	}
	EString InstanceDescription();
	void CastAndCopy(const CObjectiveCondition *pMaster) { (*this) = (*(dynamic_cast<const CBooleanFlagIsSet *>(pMaster))); }
};

class CElapsedMissionTime: public CObjectiveCondition {
protected:
	float m_time;
public:
	CElapsedMissionTime(int alignment) : CObjectiveCondition(alignment) { m_time = 0.0; }
	condition_species_type Species() { return ELAPSED_MISSION_TIME; }
	bool Read( FitIniFile* missionFile );
	bool Save( FitIniFile* file );
	objective_status_type Status();
	EString Description() {
		EString retval = "ElapsedMissionTime"; /* needs to be put somewhere localizable */
		return retval;
	}
	EString InstanceDescription();
	void CastAndCopy(const CObjectiveCondition *pMaster) { (*this) = (*(dynamic_cast<const CElapsedMissionTime *>(pMaster))); }
};

enum action_species_type {
	PLAY_BIK,
	PLAY_WAV,
	DISPLAY_TEXT_MESSAGE,
	DISPLAY_RESOURCE_TEXT_MESSAGE,
	SET_BOOLEAN_FLAG,
	MAKE_NEW_TECHNOLOGY_AVAILABLE,
	_REMOVE_STRUCTURE,

	NUM_ACTION_SPECIES
};

class CObjectiveAction {
private:
	int m_alignment;
public:
	CObjectiveAction(int alignment) { m_alignment = alignment; }
	virtual ~CObjectiveAction() {}
	int Alignment() { return m_alignment; }
	void Alignment(int alignment) { m_alignment = alignment; }
	bool DoCommonEditDialog() { return false; /* sebi: what else can I return here? */}
	virtual action_species_type Species() = 0;
	virtual bool Init() = 0;
	virtual bool Read( FitIniFile* missionFile ) = 0;
	virtual bool Save( FitIniFile* file ) = 0;
	virtual int Execute() = 0;
	virtual EString Description() = 0;
	virtual EString InstanceDescription() { EString retval; return retval; }
	virtual void CastAndCopy(const CObjectiveAction *pMaster) { (*this) = (*pMaster); }
};

class CPlayBIK: public CObjectiveAction {
private:
	EString m_pathname;
public:
	CPlayBIK(int alignment) : CObjectiveAction(alignment) {}
	action_species_type Species() { return PLAY_BIK; }
	bool Init() { return true; }
	bool Read( FitIniFile* missionFile );
	bool Save( FitIniFile* file );
	int Execute();
	EString Description() {
		EString retval = "PlayBIK"; /* needs to be put somewhere localizable */
		return retval;
	}
	EString InstanceDescription();
	void CastAndCopy(const CObjectiveAction *pMaster) { (*this) = (*(dynamic_cast<const CPlayBIK *>(pMaster))); }
};

class CPlayWAV: public CObjectiveAction {
private:
	EString m_pathname;
public:
	CPlayWAV(int alignment) : CObjectiveAction(alignment) {}
	action_species_type Species() { return PLAY_WAV; }
	bool Init() { return true; }
	bool Read( FitIniFile* missionFile );
	bool Save( FitIniFile* file );
	int Execute();
	EString Description() {
		EString retval = "PlayWAV"; /* needs to be put somewhere localizable */
		return retval;
	}
	EString InstanceDescription();
	void CastAndCopy(const CObjectiveAction *pMaster) { (*this) = (*(dynamic_cast<const CPlayWAV *>(pMaster))); }
};

class CDisplayTextMessage: public CObjectiveAction {
private:
	EString m_message;
public:
	CDisplayTextMessage(int alignment) : CObjectiveAction(alignment) {}
	action_species_type Species() { return DISPLAY_TEXT_MESSAGE; }
	bool Init() { return true; }
	bool Read( FitIniFile* missionFile );
	bool Save( FitIniFile* file );
	int Execute();
	EString Description() {
		EString retval = "DisplayTextMessage"; /* needs to be put somewhere localizable */
		return retval;
	}
	EString InstanceDescription();
	void CastAndCopy(const CObjectiveAction *pMaster) { (*this) = (*(dynamic_cast<const CDisplayTextMessage *>(pMaster))); }
};

class CDisplayResourceTextMessage: public CObjectiveAction {
private:
	int m_resourceStringID;
public:
	CDisplayResourceTextMessage(int alignment) : CObjectiveAction(alignment) {}
	action_species_type Species() { return DISPLAY_RESOURCE_TEXT_MESSAGE; }
	bool Init() { return true; }
	bool Read( FitIniFile* missionFile );
	bool Save( FitIniFile* file );
	int Execute();
	EString Description() {
		EString retval = "DisplayResourceTextMessage"; /* needs to be put somewhere localizable */
		return retval;
	}
	EString InstanceDescription();
	void CastAndCopy(const CObjectiveAction *pMaster) { (*this) = (*(dynamic_cast<const CDisplayResourceTextMessage *>(pMaster))); }
};

class CSetBooleanFlag: public CObjectiveAction {
private:
	EString m_flagID;
	bool m_value;
public:
	CSetBooleanFlag(int alignment) : CObjectiveAction(alignment) { m_flagID = _TEXT("flag0"); m_value = true; }
	action_species_type Species() { return SET_BOOLEAN_FLAG; }
	bool Init() { return true; }
	bool Read( FitIniFile* missionFile );
	bool Save( FitIniFile* file );
	int Execute();
	EString Description() {
		EString retval = "SetBooleanFlag"; /* needs to be put somewhere localizable */
		return retval;
	}
	EString InstanceDescription();
	void CastAndCopy(const CObjectiveAction *pMaster) { (*this) = (*(dynamic_cast<const CSetBooleanFlag *>(pMaster))); }
};

class CMakeNewTechnologyAvailable: public CObjectiveAction {
private:
	EString m_purchaseFilePathname;
public:
	CMakeNewTechnologyAvailable(int alignment) : CObjectiveAction(alignment) {}
	action_species_type Species() { return MAKE_NEW_TECHNOLOGY_AVAILABLE; }
	bool Init() { return true; }
	bool Read( FitIniFile* missionFile );
	bool Save( FitIniFile* file );
	int Execute();
	EString Description() {
		EString retval = "MakeNewTechnologyAvailable"; /* needs to be put somewhere localizable */
		return retval;
	}
	EString InstanceDescription();
	void CastAndCopy(const CObjectiveAction *pMaster) { (*this) = (*(dynamic_cast<const CMakeNewTechnologyAvailable *>(pMaster))); }
};

class C_RemoveStructure: public CObjectiveAction {
private:
	long m_pBuildingWID;
public:
	C_RemoveStructure(int alignment) : CObjectiveAction(alignment) {}
	bool SetParams(float positionX, float positionY);
	action_species_type Species() { return _REMOVE_STRUCTURE; }
	bool Init() { return true; }
	bool Read( FitIniFile* missionFile );
	bool Save( FitIniFile* file );
	int Execute();
	EString Description() {
		EString retval = "_RemoveStructure"; /* needs to be put somewhere localizable */
		return retval;
	}
	EString InstanceDescription();
	void CastAndCopy(const CObjectiveAction *pMaster) { (*this) = (*(dynamic_cast<const C_RemoveStructure *>(pMaster))); }
};

class CObjectives;
class  CObjectiveConditionList : public EList <CObjectiveCondition *, CObjectiveCondition *> {};
class  CObjectiveActionList : public EList <CObjectiveAction *, CObjectiveAction *> {};

class CObjective : public/*maybe protected*/ CObjectiveConditionList {
private:
	typedef CObjectiveConditionList inherited;
	int m_alignment;
	EString m_title;
	bool m_titleUseResourceString;
	int m_titleResourceStringID;
	EString m_description;
	bool m_descriptionUseResourceString;
	int m_descriptionResourceStringID;
	int m_priority;
	int m_resourcePoints;
	bool m_previousPrimaryObjectiveMustBeComplete;
	bool m_allPreviousPrimaryObjectivesMustBeComplete;
	int m_displayMarker;
	float m_markerX;
	float m_markerY;
	char m_markerText[4];
	bool m_isHiddenTrigger;
	bool m_isActive;
	bool m_activateOnFlag;
	EString m_activateFlagID;
	bool m_resetStatusOnFlag;
	EString m_resetStatusFlagID;
	double m_activationTime;
	bool m_resolved;
	bool m_changedStatus;
	objective_status_type m_resolvedStatus;
	EString m_modelName;
	int m_modelType;
	int m_modelBaseColor;
	int m_modelHighlightColor;
	int m_modelHighlightColor2;
	float m_modelScale;
	static float s_blinkLength;
	static float s_lastBlinkTime;
	static unsigned long s_blinkColor;
	static aFont*	s_markerFont;
public: /* we could make this protected if only the editdialog is to access these functions */
	static CObjectiveCondition *new_CObjectiveCondition(condition_species_type conditionSpecies, int alignment);
	static EString DescriptionOfConditionSpecies(condition_species_type conditionSpecies);
	static CObjectiveAction *new_CObjectiveAction(action_species_type actionSpecies, int alignment);
	static EString DescriptionOfActionSpecies(action_species_type actionSpecies);
public:
	typedef CObjectiveConditionList condition_list_type;
	typedef CObjectiveActionList action_list_type;
	action_list_type m_actionList;
	condition_list_type m_failureConditionList;
	action_list_type m_failureActionList;

	CObjective(int alignment);
	CObjective(const CObjective &master) { (*this) = master; }
	~CObjective() { Clear(); }
	CObjective &operator=(const CObjective &master);
	void Init() {}
	void Clear();
	int Alignment() { return m_alignment; }
	void Alignment(int alignment);
	bool Read( FitIniFile* missionFile, int objectiveNum, int version, int markerNum, char secondaryMarkerNum );
	bool Save( FitIniFile* file, int objectiveNum );
	/* The following function evaluates the status of the objective irrespective of the other
	objectives (i.e. it disregards qualifiers like "PreviousPrimaryObjectiveMustbeComplete"). */
	objective_status_type Status();
	/* The following function evaluates the status of the objective in the context of the given
	objectives. */
	objective_status_type Status(CObjectives &objectives);
	void Status(objective_status_type newStatus);
	/* The following function evaluates the status of the objective in the context of the given
	objectives. It returns a bool telling me if the objective has failed or been completed since I last checked!*/
	bool StatusChangedSuccess (void);
	bool StatusChangedFailed (void);
	EString Title() const { return m_title; }
	void Title(EString title) { m_title = title; }
	bool TitleUseResourceString() const { return m_titleUseResourceString; }
	void TitleUseResourceString(bool titleUseResourceString) { m_titleUseResourceString = titleUseResourceString; }
	int TitleResourceStringID() const { return m_titleResourceStringID; }
	void TitleResourceStringID(int titleResourceStringID) { m_titleResourceStringID = titleResourceStringID; }
	EString LocalizedTitle() const;
	EString Description() const { return m_description; }
	void Description(EString description) { m_description = description; }
	bool DescriptionUseResourceString() const { return m_descriptionUseResourceString; }
	void DescriptionUseResourceString(bool descriptionUseResourceString) { m_descriptionUseResourceString = descriptionUseResourceString; }
	int DescriptionResourceStringID() const { return m_descriptionResourceStringID; }
	void DescriptionResourceStringID(int descriptionResourceStringID) { m_descriptionResourceStringID = descriptionResourceStringID; }
	EString LocalizedDescription() const;
	int Priority() { return m_priority; }
	void Priority(int priority) { m_priority = priority; }
	int ResourcePoints() { return m_resourcePoints; }
	void ResourcePoints(int resourcePoints) { m_resourcePoints = resourcePoints; }
	bool PreviousPrimaryObjectiveMustBeComplete() { return m_previousPrimaryObjectiveMustBeComplete; }
	void PreviousPrimaryObjectiveMustBeComplete(bool previousPrimaryObjectiveMustBeComplete) { m_previousPrimaryObjectiveMustBeComplete = previousPrimaryObjectiveMustBeComplete; }
	bool AllPreviousPrimaryObjectivesMustBeComplete() { return m_allPreviousPrimaryObjectivesMustBeComplete; }
	void AllPreviousPrimaryObjectivesMustBeComplete(bool allPreviousPrimaryObjectivesMustBeComplete) { m_allPreviousPrimaryObjectivesMustBeComplete = allPreviousPrimaryObjectivesMustBeComplete; }
	int DisplayMarker() { return m_displayMarker; }
	void DisplayMarker(int displayMarker) { m_displayMarker = displayMarker; }
	float MarkerX() { return m_markerX; }
	void MarkerX(float markerX) { m_markerX = markerX; }
	float MarkerY() { return m_markerY; }
	void MarkerY(float markerY) { m_markerY = markerY; }
	void IsHiddenTrigger(bool isHiddenTrigger) { m_isHiddenTrigger = isHiddenTrigger; }
	bool IsHiddenTrigger() { return m_isHiddenTrigger; }
	void IsActive(bool isActive) { m_isActive = isActive; }
	bool IsActive() { return m_isActive; }
	void ActivateOnFlag(bool activateOnFlag) { m_activateOnFlag = activateOnFlag; }
	bool ActivateOnFlag() { return m_activateOnFlag; }
	void ActivateFlagID(EString activateFlagId) { m_activateFlagID = activateFlagId; }
	EString ActivateFlagID() { return m_activateFlagID; }
	void ResetStatusOnFlag(bool resetStatusOnFlag) { m_resetStatusOnFlag = resetStatusOnFlag; }
	bool ResetStatusOnFlag() { return m_resetStatusOnFlag; }
	void ResetStatusFlagID(EString resetStatusFlagID) { m_resetStatusFlagID = resetStatusFlagID; }
	EString ResetStatusFlagID() { return m_resetStatusFlagID; }
	EString ModelName() { return m_modelName; }
	void ModelName(EString modelName) { m_modelName = modelName; }
	int ModelType() { return m_modelType; }
	void ModelType(int modelType) { m_modelType = modelType; }
	int ModelBaseColor() { return m_modelBaseColor; }
	void ModelBaseColor(int modelBaseColor) { m_modelBaseColor = modelBaseColor; }
	int ModelHighlightColor() { return m_modelHighlightColor; }
	void ModelHighlightColor(int modelHighlightColor) { m_modelHighlightColor = modelHighlightColor; }
	int ModelHighlightColor2() { return m_modelHighlightColor2; }
	void ModelHighlightColor2(int modelHighlightColor2) { m_modelHighlightColor2 = modelHighlightColor2; }
	float ModelScale() { return m_modelScale; }
	void ModelScale(float modelScale) { m_modelScale = modelScale; }
	void ActivationTime(double activationTime) { m_activationTime = activationTime; }
	double ActivationTime() { return m_activationTime; }
	void Render( unsigned long xPos, unsigned long yPos, HGOSFONT3D );
	bool RenderMarkers (GameTacMap *tacMap, bool blink);		//TacMap calls this to draw objective markers on tacMap.

	friend class Mission;
};

class CObjectives : public/*maybe protected*/ EList <CObjective *, CObjective *> {
public:
	CObjectives(int alignment = 0) { m_alignment = alignment; }
	CObjectives(const CObjectives &master) { (*this) = master; }
	~CObjectives() { Clear(); }
	CObjectives &operator=(const CObjectives &master);
	void Init();
	void Clear();
	int Alignment() { return m_alignment; }
	void Alignment(int alignment);
	bool Read( FitIniFile* missionFile );
	bool Save( FitIniFile* file );
	objective_status_type Status();
	CBooleanArray boolFlags;
private:
	typedef EList <CObjective *, CObjective *> inherited;
	int m_alignment;
};

/* Reads Nav Marker info and adds appropriate "hidden trigger" objectives. */
bool ReadNavMarkers( FitIniFile* missionFile, CObjectives &objectives );

//*************************************************************************************************
#endif  // end of file ( Objective.h )
