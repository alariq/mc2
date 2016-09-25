//---------------------------------------------------------------------------
//
// Packet.h -- File contains the header for the Packet File class
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef PACKET_H
#define PACKET_H
//---------------------------------------------------------------------------
// Include Files

#ifndef DPACKET_H
#include"dpacket.h"
#endif

#ifndef FILE_H
#include"file.h"
#endif

#ifndef LZ_H
#include"lz.h"
#endif

//---------------------------------------------------------------------------
// Macro Definitions

#define PACKET_FILE_VERSION	0xFEEDFACE
#define NEW_PACKET_FLAG			0xFEEDFACE

// 3 bits of type info

#define STORAGE_TYPE_RAW		0x00L
#define STORAGE_TYPE_FWF		0x01L		// file within file
#define STORAGE_TYPE_LZD		0x02L		// LZ Compressed Packet
#define STORAGE_TYPE_HF			0x03L		// Huffman Compressed Packet
#define STORAGE_TYPE_ZLIB		0x04L		// zLib Compressed Packet
#define STORAGE_TYPE_NUL		0x07L		// NULL packet.

#define TYPE_SHIFT					29	// Bit position of masked type
#define DEFAULT_PACKET_TYPE			STORAGE_TYPE_RAW
#define ANY_PACKET_TYPE				0x07
#define OFFSET_MASK         		((1L << TYPE_SHIFT) - 1L)

#define GetPacketType(offset) 		(((unsigned int)offset)>>TYPE_SHIFT)
#define GetPacketOffset(offset)		(offset&OFFSET_MASK)
#define SetPacketType(offset,type)	((offset)+((unsigned int)(type)<<TYPE_SHIFT))

#define TABLE_ENTRY(p) ((2+p)<<2)	// ((1+p)*sizeof(long))

//---------------------------------------------------------------------------
// Structure and Class Definitions

//---------------------------------------------------------------------------
class PacketFile : public File
{
	//Data Members
	//-------------
	protected:
		unsigned int numPackets;
		unsigned int currentPacket;
		unsigned int packetSize;
		unsigned int packetBase;

		unsigned int packetType;		
		unsigned int packetUnpackedSize;

		unsigned int* seekTable;

		bool usesCheckSum;

	//Member Functions
	//-----------------
	protected:
		void clear (void);
		void atClose (void);
		long afterOpen (void);

	public:

		PacketFile (void);
		~PacketFile (void);
		
		virtual long open (const char* fName, FileMode _mode = READ, long numChildren = 50);
		virtual long open (FilePtr _parent, unsigned long fileSize, long numChildren = 50);
		
		virtual long create (const char* fName);
		virtual long createWithCase(const char* fName ); // don't strlwr for me please!
		virtual void close (void);

		void forceUseCheckSum (void)
		{
			usesCheckSum = TRUE;
		}

		unsigned int readPacketOffset (unsigned int packet, unsigned int *lastType = 0);
		unsigned int readPacket (unsigned int packet, unsigned char *buffer);
		unsigned int readPackedPacket (unsigned int packet, unsigned char *buffer);

		unsigned int seekPacket (unsigned int packet);

		void operator ++ (void);
		void operator -- (void);
		
		unsigned int getNumPackets (void);
		unsigned int getCurrentPacket (void);
		unsigned int getPacketOffset(void);

		unsigned int getPacketSize (void)
		{
			return packetUnpackedSize;
		}

		unsigned int getPackedPacketSize (void);
		unsigned int getStorageType (void);

		virtual FileClass getFileClass (void)
		{
			return PACKETFILE;
		}
			
		unsigned int checkSumFile (void);

		//-------------------------------------------
		// Functions to Write Packet Files
		void reserve (unsigned int count, bool withCheckSum = FALSE);
		unsigned int writePacket (unsigned int packet, MemoryPtr buffer, unsigned int nbytes, unsigned char p_type = ANY_PACKET_TYPE);
		unsigned int insertPacket (unsigned int packet, MemoryPtr buffer, unsigned int nbytes, unsigned char p_type = ANY_PACKET_TYPE);
		unsigned int writePacket (unsigned int packet, MemoryPtr buffer);
};


//---------------------------------------------------------------------------
#endif

//---------------------------------------------------------------------------
//
// Edit Log
//
// $Log: packetfile.h $
// Revision 1.1  1995/08/09 17:38:22  fsavage
// Initial revision
//
//
//---------------------------------------------------------------------------

