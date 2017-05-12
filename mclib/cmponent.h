//***************************************************************************
//
//	cmponent.h -- File contains the Component definition
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef CMPONENT_H
#define CMPONENT_H

//---------------------------------------------------------------------------
#ifndef INIFILE_H
#include"inifile.h"
#endif

#ifndef ERR_H
#include"err.h"
#endif

#include<string.h>

// sebi: uncommented on Windows as well, not sure why it is commented, probabaly has something to do with mutibyte stuff
//#ifndef _MBCS
#include<gameos.hpp>
//#else
//#include<assert.h>
//#define gosASSERT assert
//#define gos_Malloc malloc
//#define gos_Free free
//#endif

//***************************************************************************

#define	MAXLEN_COMPONENT_NAME		256
#define	MAXLEN_COMPONENT_ABBREV		256

#define	NUM_BODY_LOCATIONS			8

#define	TECH_BASE_CLAN				1
#define	TECH_BASE_INNERSPHERE		2

#define	COMPONENT_USE_MECH			1
#define	COMPONENT_USE_VEHICLE		2
#define	COMPONENT_USE_CLAN			16
#define	COMPONENT_USE_INNERSPHERE	32

#define	WEAPON_AMMO_OFFSET			65		// Master Component Index Offset from weapon

typedef enum {
	COMPONENT_FORM_SIMPLE = 0,
	COMPONENT_FORM_COCKPIT,
	COMPONENT_FORM_SENSOR,
	COMPONENT_FORM_ACTUATOR,
	COMPONENT_FORM_ENGINE,
	COMPONENT_FORM_HEATSINK,
	COMPONENT_FORM_WEAPON,
	COMPONENT_FORM_WEAPON_ENERGY,
	COMPONENT_FORM_WEAPON_BALLISTIC,
	COMPONENT_FORM_WEAPON_MISSILE,
	COMPONENT_FORM_AMMO,
	COMPONENT_FORM_JUMPJET,
	COMPONENT_FORM_CASE,
	COMPONENT_FORM_LIFESUPPORT,
	COMPONENT_FORM_GYROSCOPE,
	COMPONENT_FORM_POWER_AMPLIFIER,
	COMPONENT_FORM_ECM,
	COMPONENT_FORM_PROBE,
	COMPONENT_FORM_JAMMER,
	COMPONENT_FORM_BULK,
	NUM_COMPONENT_FORMS
} ComponentFormType;

typedef enum {
	WEAPON_AMMO_NONE,		// unlimited ammo (energy weapons)
	WEAPON_AMMO_SRM,
	WEAPON_AMMO_LRM,
	WEAPON_AMMO_ST,
	WEAPON_AMMO_LIMITED,	// not SRM, LRM or ST, but still finite ammo amount
	NUM_WEAPON_AMMO_TYPES
} WeaponAmmoType;

typedef enum {
	WEAPON_STATE_READY,
	WEAPON_STATE_RECYCLING,
	WEAPON_STATE_DAMAGED,
	WEAPON_STATE_DESTROYED
} WeaponStateType;

typedef enum {
	WEAPON_RANGE_SHORT,
	WEAPON_RANGE_MEDIUM,
	WEAPON_RANGE_LONG,
	WEAPON_RANGE_SHORT_MEDIUM,
	WEAPON_RANGE_MEDIUM_LONG,
	NUM_WEAPON_RANGE_TYPES
} WeaponRangeType;

//******************************************************************************************

#define	WEAPON_FLAG_STREAK		1
#define	WEAPON_FLAG_INFERNO		2
#define WEAPON_FLAG_LBX			4
#define WEAPON_FLAG_ARTILLERY	8

typedef union {
	struct {
		float				range;			// in meters
	} sensor;
	struct {
		float				range;
		float				effect;
	} ecm;
	struct {
		float				effect;
	} jammer;
	struct {
		float				effect;
	} probe;
	struct {
		short				rating;
	} engine;
	struct {
		short				dissipation;
	} heatsink;
	struct {
		float				heat;			// how much heat does it generate?
		float				damage;			// amount of damage
		float				recycleTime;	// in seconds
		long				ammoAmount;		// amount of ammo per shot
		unsigned char		ammoType;		// 0 = unlimited, 1 = finite amount (e.g. bullets)
		unsigned char		ammoMasterId;	// ammo used by this weapon
		unsigned char		range;			// short, med or long
		unsigned char		flags;
		short				type;			// which weapon type is this
		char				specialEffect;	// used to cue whatever visual/sound effects this needs
	} weapon;
	struct {
		long				ammoPerTon;		// ammo per ton
		float				explosiveDamage;// damage done (per missile) if it explodes
	} ammo;
	struct {
		long				rangeMod;		// range modifier
	} jumpjet;
} ComponentStats;

//******************************************************************************************

class MasterComponent;
typedef MasterComponent* MasterComponentPtr;

class MasterComponent {

	private:

		long							masterID;									// unique ID for component
		long							resourcePoints;								// resource cost of object
		ComponentFormType				form;										// form of component
		char							name[MAXLEN_COMPONENT_NAME];				// name string/description
		char							abbreviation[MAXLEN_COMPONENT_ABBREV];		// abbreviated name
		float							tonnage;									// in tons
		char							size;										// # of total spaces used
		char							health;										// # of hits before destroyed
		char							criticalSpacesReq[NUM_BODY_LOCATIONS];		// # of critical spaces required in specific location
		char							disableLevel;								// # of critical spaces to disable
		unsigned char					uses;
		unsigned char					techBase;
		float							CV;											// CV for this component
		ComponentStats					stats;
		unsigned long					art;

	public:

		static MasterComponentPtr		masterList;
		static long						numComponents;
		static long						armActuatorID;
		static long						legActuatorID;
		static long						clanAntiMissileSystemID;
		static long						innerSphereAntiMissileSystemID;

	public:

		void* operator new (size_t mySize);

		void operator delete (void* us);

		void destroy (void);

		~MasterComponent (void) {
			destroy();
		}
		
		void init (void) {
			masterID = -1;
			name[0] = '\0';
			abbreviation[0] = '\0';
		}

		MasterComponent (void) {
			init();
		}
		
		long init (FitIniFile* componentFile);

		long initEXCEL (char* dataLine, float baseSensorRange);

		long saveEXCEL (FilePtr file, unsigned char masterId,float baseSensorRange);

		char* getName (void) {
			return(&name[0]);
		}

		long getMasterID (void) {
			return(masterID);
		}

		long getResourcePoints (void) {
			return(resourcePoints);
		}
		
		ComponentFormType getForm (void) {
			return(form);
		}

		long getSize (void) {
			return(size);
		}

		void setSize (long sz)
		{
			size = sz;
		}
		
		long getHealth (void) {
			return(health);
		}

		char getCriticalSpacesReq (long location) 
		{
			// sebi: ORIG BUG FIX
			if ((location < 0) || (location > NUM_BODY_LOCATIONS))
				return -1;

			return(criticalSpacesReq[location]);
		}

		void setCriticalSpacesReq (long location, char value) 
		{
			// sebi: ORIG BUG FIX
			if ((location < 0) || (location > NUM_BODY_LOCATIONS))
				return;

			criticalSpacesReq[location] = value;
		}

		char getDisableLevel (void) {
			return(disableLevel);
		}

		float getCV (void) {
			return(CV);
		}

		void setCV (float cv)
		{
			CV = cv;
		}

		float getTonnage (void) {
			return(tonnage);
		}
		
		void setTonnage (float tons)
		{
			tonnage = tons;
		}

		float getHeatDissipation (void)
		{
			return (stats.heatsink.dissipation);
		}

		void setHeatDissipation (float heatD)
		{
			stats.heatsink.dissipation = heatD;
		}

		float getWeaponHeat (void) {
			return(stats.weapon.heat);
		}

		void setWeaponHeat (float weaponHeat)
		{
			stats.weapon.heat = weaponHeat;
		}

		float getWeaponDamage (void) {
			return(stats.weapon.damage);
		}

		void setWeaponDamage (float weapDmg)
		{
			stats.weapon.damage = weapDmg;
		}

		float getWeaponRecycleTime (void) {
			return(stats.weapon.recycleTime);
		}

		void setWeaponRecycleTime (float recycleTime)
		{
			stats.weapon.recycleTime = recycleTime;
		}

		long getWeaponAmmoAmount (void) {
			return(stats.weapon.ammoAmount);
		}

		void setWeaponAmmoAmount (long weaponAmmo)
		{
			stats.weapon.ammoAmount = weaponAmmo;
		}

		unsigned long getWeaponAmmoType (void) {
			return(stats.weapon.ammoType);
		}

		void setWeaponAmmoType (long ammoType)
		{
			stats.weapon.ammoType = ammoType;
		}

		unsigned long getWeaponAmmoMasterId (void) {
			return(stats.weapon.ammoMasterId);
		}

		void setWeaponAmmoMasterId (long ammoId)
		{
			stats.weapon.ammoMasterId = ammoId;
		}

		long getWeaponRange (void) {
			return(stats.weapon.range);
		}

		void setWeaponRange (long weaponRange) {
			stats.weapon.range = weaponRange;
		}

		void clearWeaponFlags (void)
		{
			stats.weapon.flags = 0;
		}

		bool getWeaponInferno (void) {
			return((stats.weapon.flags & WEAPON_FLAG_INFERNO) != 0);
		}

		void setWeaponInferno (void)
		{
			stats.weapon.flags |= WEAPON_FLAG_INFERNO;
		}

		bool getWeaponStreak (void) {
			return((stats.weapon.flags & WEAPON_FLAG_STREAK) != 0);
		}

		void setWeaponStreak (void)
		{
			stats.weapon.flags |= WEAPON_FLAG_STREAK;
		}

		bool getWeaponLBX (void) {
			return((stats.weapon.flags & WEAPON_FLAG_LBX) != 0);
		}

		void setWeaponLBX (void)
		{
			stats.weapon.flags |= WEAPON_FLAG_LBX;
		}

		bool getWeaponArtillery (void) {
			return((stats.weapon.flags & WEAPON_FLAG_ARTILLERY) != 0);
		}

		void setWeaponArtillery (void)
		{
			stats.weapon.flags |= WEAPON_FLAG_ARTILLERY;
		}

		bool getCanClanUse (void)
		{
			return ((uses & COMPONENT_USE_CLAN) != 0);
		}

		bool getCanISUse (void)
		{
			return ((uses & COMPONENT_USE_INNERSPHERE) != 0);
		}

		bool getCanMechUse (void)
		{
			return ((uses & COMPONENT_USE_MECH) != 0);
		}

		bool getCanVehicleUse (void)
		{
			return ((uses & COMPONENT_USE_VEHICLE) != 0);
		}

		void clearUseFlags (void)
		{
			uses = 0;
		}

		void setCanClanUse (void)
		{
			uses |= COMPONENT_USE_CLAN;
		}

		void setCanISUse (void)
		{
			uses |= COMPONENT_USE_INNERSPHERE;
		}

		void setCanVehicleUse (void)
		{
			uses |= COMPONENT_USE_VEHICLE;
		}

		void setCanMechUse (void)
		{
			uses |= COMPONENT_USE_MECH;
		}

		bool getClanTechBase (void)
		{
			return ((techBase & TECH_BASE_CLAN) != 0);
		}

		bool getISTechBase (void)
		{
			return ((techBase & TECH_BASE_INNERSPHERE) != 0);
		}

		void setClanTechBase (void)
		{
			techBase = TECH_BASE_CLAN;
		}

		void setISTechBase (void)
		{
			techBase = TECH_BASE_INNERSPHERE;
		}

		void setBothTechBase (void)
		{
			techBase = TECH_BASE_INNERSPHERE + TECH_BASE_CLAN;
		}

//		long getWeaponType (void) {
//			return(stats.weapon.type);
//		}

		long getWeaponSpecialEffect (void) {
			return(stats.weapon.specialEffect);
		}

		long getAmmoPerTon (void) {
			return(stats.ammo.ammoPerTon);
		}

		void setAmmoPerTon (long ammoPerTon)
		{
			stats.ammo.ammoPerTon = ammoPerTon;
		}

		float getJumpJetRangeMod (void)
		{
			return (stats.jumpjet.rangeMod);
		}

		void setJumpJetRangeMod (float rangeMod)
		{
			stats.jumpjet.rangeMod = rangeMod;
		}

		float getAmmoExplosiveDamage (void) {
			return(stats.ammo.explosiveDamage);
		}
		
		void setAmmoExplosiveDamage (float ammoDamage)
		{
			stats.ammo.explosiveDamage = ammoDamage;
		}

		float getSensorRange (void) {
			return(stats.sensor.range);
		}

		unsigned char getTechBase (void) {
			return(techBase);
		}

		void setResourcePoints (long points) {
			resourcePoints = points;
		}
		
		void setSensorRange (float range) 
		{
			stats.sensor.range = (short)range;
		}

		float getEcmRange (void) {
			return(stats.ecm.range);
		}

		void setEcmRange (float range) {
			stats.ecm.range = range;
		}

		float getEcmEffect (void) {
			return(stats.ecm.effect);
		}

		void setEcmEffect (float effect) {
			stats.ecm.effect = effect;
		}

		float getJammerEffect (void) {
			return(stats.jammer.effect);
		}

		void setJammerEffect (float effect) {
			stats.jammer.effect = effect;
		}

		float getProbeEffect (void) {
			return(stats.probe.effect);
		}

		void setProbeEffect (float effect) {
			stats.probe.effect = effect;
		}

		char *getAbbreviation (void) {
			return (&abbreviation[0]);
		}
		
		bool isOffensiveWeapon (void);

		bool isDefensiveWeapon (void);

		void multiplyWeaponRanges (float factor);

		static long loadMasterList (const char* fileName, long numComponents, float baseSensorRange);
		static long saveMasterList (const char* fileName, long numComponents,float baseSensorRange);

		static long freeMasterList (void);

		static long multiplyMasterListWeaponRanges (float factor);
};

#endif

//******************************************************************************************

