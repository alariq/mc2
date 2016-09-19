#ifndef LOGISTICSPILOT_H
#define LOGISTICSPILOT_H

//===========================================================================//
//LogisticsPilot.h			: Interface for the LogisticsPilot component.    //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

class FitIniFile;
class MechWarrior;
class ForceGroupIcon;
/**************************************************************************************************
CLASS DESCRIPTION
LogisticsPilot:
**************************************************************************************************/
#include"estring.h"
#include"elist.h"
#include"warrior.h"

#define MAX_MISSIONS		50

enum Medals
{

	CAMPAIGN_RIBBON1 = 0,
	CAMPAIGN_RIBBON2 = 1,
	CAMPAIGN_RIBBON3 = 2,
	CAMPAIGN_FULLRIBBON1 = 3,
	CAMPAIGN_FULLRIBBON2 = 4,
	CAMPAIGN_FULLRIBBON3 = 5,
	PURPLE_HEART = 6,
	VALOR = 7,
	UNCOMMON_VALOR = 8,
	MISSION_SPEC1 = 9,
	MISSION_SPEC2 = 10,
	MISSION_SPEC3 = 11,
	MISSION_SPEC4 = 12,
	LIAO_MEDAL = 13,
	DAVION_MEDAL = 14,
	STEINER_MEDAL = 15,
	MAX_MEDAL

};


class LogisticsPilot
{
	public:

		int init( char* fileName );
		LogisticsPilot();
		~LogisticsPilot();


		const EString&  getName() const { return name; }
		const EString&	 getAudio() const { return audio; }
		const EString& getVideo() const { return video; }
		int			getRank() const { return rank; }
		const EString& getIconFile() const { return iconFile; }
		float			getGunnery() const { return gunnery; }
		float			getPiloting() const { return piloting; }
		int			getMechKills() const { return mechKills; }
		int			getVehicleKills() const { return vehicleKills; }
		int			getInfantryKills() const { return infantryKills; }
		int			getID() const { return id; }
		int			getFlavorTextID() const { return flavorTextID; }
		int			getNumberMissions() const;
		bool		getMissionPlayed (long missionNum)
		{
			if ((missionNum >= 0) && (missionNum < MAX_MISSIONS))
				return (missionsPlayed[missionNum] == 1);

			return false;
		}

		bool		isUsed() const{ return bIsUsed; }
		void		setUsed( bool bUsed ){ bIsUsed = bUsed; }
		int			getNewGunnery() const { return newGunnery; }
		int			getNewPiloting() const { return newPiloting; }
		
		void		setDead();
		bool		isAvailable(){ return (bAvailable&&!bDead); } // depends on purchasing file .... maybe should put dead checks and stuff
		void		setAvailable( bool available ) { bAvailable = available; }
		bool		isDead() const { return bDead; }
		bool		justDied() const { return bJustDied; }
		void		setJustDied( bool bdied){ bJustDied = bdied; }
		long		getPhotoIndex() const { return photoIndex; }

		bool		promotePilot();
		int			turnAverageIntoRank( float avg);



		static		const char* getSkillText( int skillID );
		const EString&	getFileName(){ return fileName; }

		long		save( FitIniFile& file, long count );
		long		load( FitIniFile& file );

		long		update( MechWarrior* pWarrior );

		int			getSpecialtySkillCount() const;
		int			getSpecialtySkills( const char** array, int& count );
		int			getSpecialtySkills( int* array, int& count );
		bool		*getSpecialtySkills() { return specialtySkills;}
		void		setSpecialtySkill( int skill, bool set );

		const bool*			getNewMedalArray() const{ return medalsLastMission;}
		const bool*			getMedalArray() const{ return medals;}


		void		clearIcons();

		
		unsigned long	id;
		EList< ForceGroupIcon*, ForceGroupIcon* >	killedIcons;



	private:
		EString		name;
		EString		audio;
		EString		video;
		long			rank;
		EString		iconFile; // or file name?
		float			gunnery;
		float			piloting;
		long			mechKills;
		long			vehicleKills;
		long			infantryKills;
		long			missionsCompleted;
		unsigned char	missionsPlayed[MAX_MISSIONS];
		long			flavorTextID;
		bool			bIsUsed;
		bool			bDead;
		bool			bAvailable;
		bool			bJustDied;
		long			photoIndex;
		float			newPiloting;
		float			newGunnery;
		bool			specialtySkills[NUM_SPECIALTY_SKILLS];

		static char skillTexts[][255];
		EString		fileName;

		bool		medals[MAX_MEDAL];
		bool		medalsLastMission[MAX_MEDAL];


		// last second hack
		friend class LogisticsData;


};


//*************************************************************************************************
#endif  // end of file ( LogisticsPilot.h )
