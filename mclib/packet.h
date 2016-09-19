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

#define GetPacketType(offset) 		(((unsigned long)offset)>>TYPE_SHIFT)
#define GetPacketOffset(offset)		(offset&OFFSET_MASK)
#define SetPacketType(offset,type)	((offset)+(long(type)<<TYPE_SHIFT))

#define TABLE_ENTRY(p) ((2+p)<<2)	// ((1+p)*sizeof(long))

//---------------------------------------------------------------------------
// Structure and Class Definitions

//---------------------------------------------------------------------------
class PacketFile : public File
{
	//Data Members
	//-------------
	protected:
		long numPackets;
		long currentPacket;
		long packetSize;
		long packetBase;

		long packetType;		
		long packetUnpackedSize;

		long *seekTable;

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

		long readPacketOffset (long packet, long *lastType = 0);
		long readPacket (long packet, unsigned char *buffer);
		long readPackedPacket (long packet, unsigned char *buffer);

		long seekPacket (long packet);

		void operator ++ (void);
		void operator -- (void);
		
		long getNumPackets (void);
		long getCurrentPacket (void);
		long getPacketOffset(void);

		long getPacketSize (void)
		{
			return packetUnpackedSize;
		}

		long getPackedPacketSize (void);
		long getStorageType (void);

		virtual FileClass getFileClass (void)
		{
			return PACKETFILE;
		}
			
		long checkSumFile (void);

		//-------------------------------------------
		// Functions to Write Packet Files
		void reserve (long count, bool withCheckSum = FALSE);
		long writePacket (long packet, MemoryPtr buffer, long nbytes, unsigned char p_type = ANY_PACKET_TYPE);
		long insertPacket (long packet, MemoryPtr buffer, long nbytes, unsigned char p_type = ANY_PACKET_TYPE);
		long writePacket (long packet, MemoryPtr buffer);
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

