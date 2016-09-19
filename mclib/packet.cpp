//---------------------------------------------------------------------------
//
// Packet.cpp -- File contains the actual code for the Packet File class
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//---------------------------------------------------------------------------
// Include Files
#ifndef PACKET_H
#include"packet.h"
#endif

#ifndef HEAP_H
#include"heap.h"
#endif

#ifndef LZ_H
#include"lz.h"
#endif

#include"zlib.h"

#ifndef _MBCS
#include<gameos.hpp>
#else
#include<assert.h>
#define gosASSERT assert
#define gos_Malloc malloc
#define gos_Free free
#endif

#include<string.h>
//---------------------------------------------------------------------------
extern MemoryPtr 	LZPacketBuffer;
extern unsigned long LZPacketBufferSize;
//---------------------------------------------------------------------------
// class PacketFile
void PacketFile::clear (void)
{
	currentPacket	= -1;

	packetSize		=
	packetBase		=
	numPackets		= 0;

	if (seekTable)
		systemHeap->Free(seekTable);

	seekTable 		= NULL;
}

//---------------------------------------------------------------------------
void PacketFile::atClose (void)
{
	if (isOpen() && fileMode != READ)								// update filesize
	{
		long endPtr = getLength();
		
		//seek(sizeof(long));								//Move Past Version Marker
		//writeLong(endPtr);								//Write File length
	
		long tableEntry;
		currentPacket = numPackets;

		if (!seekTable)
		{
			while (--currentPacket >= 0)
			{
				seek(TABLE_ENTRY(currentPacket));
				tableEntry = readLong();
					
				if (GetPacketType(tableEntry) == STORAGE_TYPE_NUL)
				{
					seek(TABLE_ENTRY(currentPacket));
					writeLong(SetPacketType(endPtr,STORAGE_TYPE_NUL));
				}
				else
				{
					endPtr = GetPacketOffset(tableEntry);
				}
			}
		}
		else
		{
			while (--currentPacket >= 0)
			{
				tableEntry = seekTable[currentPacket];
					
				if (GetPacketType(tableEntry) == STORAGE_TYPE_NUL)
				{
					seekTable[currentPacket] = SetPacketType(endPtr,STORAGE_TYPE_NUL);
				}
				else
				{
					endPtr = GetPacketOffset(tableEntry);
				}
			}
		}

		//-----------------------------------------------------
		// If seekTable was being used, write it back to file
		if (seekTable)
		{
			seek(sizeof(long)*2);							//File Version & File Length
			write(MemoryPtr(seekTable),(numPackets*sizeof(long)));
		}

		//------------------------------------------------------
		// Is we were using a checkSum, calc it and write it to
		// the beginning of the file.
		if (usesCheckSum)
		{
			long checkSum = checkSumFile();
			seek(0);
			writeLong(checkSum);
		}
	}
	
	clear();
}

//---------------------------------------------------------------------------
long PacketFile::checkSumFile (void)
{
	//-----------------------------------------
	long currentPosition  = logicalPosition;
	seek(4);

	MemoryPtr fileMap = (MemoryPtr)malloc(fileSize());
	read(fileMap,fileSize());

	long sum = 0;
	MemoryPtr curFileByte = fileMap;
	for (unsigned long i=4;i<fileSize();i++,curFileByte++)
	{
		sum += *curFileByte;
	}

	free(fileMap);

	seek (currentPosition);

	return sum;
}

//---------------------------------------------------------------------------
long PacketFile::afterOpen (void)
{
	if (!numPackets && getLength() >= 12)
	{
		long firstPacketOffset;
		long firstCheck = readLong();

		if (firstCheck == PACKET_FILE_VERSION && !usesCheckSum)
		{
			
		}
		else
		{
			//---------------------------------------
			// This is probably a checksum.  Check it
			long checkSum = checkSumFile();
			if (checkSum != firstCheck)
				return PACKET_OUT_OF_RANGE;
		}

		firstPacketOffset = readLong();
		
		numPackets = (firstPacketOffset/sizeof(long))-2;
	}

	currentPacket = -1;

	if (fileMode == READ || fileMode == RDWRITE)
	{
		if (numPackets && !seekTable)
		{
			seekTable = (long *)systemHeap->Malloc(numPackets * sizeof(long));
			gosASSERT(seekTable != NULL);
				
			seek(sizeof(long)*2);												//File Version & File Length
			read(MemoryPtr(seekTable),(numPackets*sizeof(long)));
		}
	}
	
	return(NO_ERR);
}

//---------------------------------------------------------------------------
PacketFile::PacketFile (void)
{
	seekTable = NULL;
	usesCheckSum = FALSE;
	clear();
}

//---------------------------------------------------------------------------
PacketFile::~PacketFile (void)
{
	close();
}

//---------------------------------------------------------------------------
long PacketFile::open (const char* fName, FileMode _mode, long numChild)
{
	long openResult = File::open(fName,_mode,numChild);
	
	if (openResult != NO_ERR)
	{
		return(openResult);
	}

	openResult = afterOpen();
	return(openResult);
}

//---------------------------------------------------------------------------
long PacketFile::open (FilePtr _parent, unsigned long fileSize, long numChild)
{
	long result = File::open(_parent,fileSize,numChild);
	
	if (result != NO_ERR)
		return(result);
		
	result = afterOpen();	
	return(result);
}
		
//---------------------------------------------------------------------------
long PacketFile::create (const char* fName)
{
	long openResult = File::create(fName);
	
	if (openResult != NO_ERR)
	{
		return(openResult);
	}
	
	openResult = afterOpen();
	return(openResult);
}

long PacketFile::createWithCase(const char* fName )
{
	long openResult = File::createWithCase(fName);
	
	if (openResult != NO_ERR)
	{
		return(openResult);
	}
	
	openResult = afterOpen();
	return(openResult);
}

//---------------------------------------------------------------------------
void PacketFile::close (void)
{
	atClose();
	File::close();
}

//---------------------------------------------------------------------------
long PacketFile::readPacketOffset (long packet, long *lastType)
{
	long offset = -1;

	if (packet < numPackets)
	{
		if (seekTable)
			offset = seekTable[packet];

		if (lastType)
			*lastType = GetPacketType(offset);

		offset = GetPacketOffset(offset);
	}

	return offset;
}

//---------------------------------------------------------------------------
long PacketFile::readPacket (long packet, unsigned char *buffer)
{
	long result = 0;

	if ((packet==-1) || (packet == currentPacket) || (seekPacket(packet) == NO_ERR))
	{
		if ((getStorageType() == STORAGE_TYPE_RAW) || (getStorageType() == STORAGE_TYPE_FWF))
		{
			seek(packetBase);
			result = read(buffer, packetSize);
		}
		else
		{
			switch (getStorageType())
			{
				case STORAGE_TYPE_LZD:
				{
					seek(packetBase+sizeof(long));

					if (!LZPacketBuffer)
					{
						LZPacketBuffer = (MemoryPtr)malloc(LZPacketBufferSize);
						gosASSERT(LZPacketBuffer);
					}
						
					if ((long)LZPacketBufferSize < packetSize)
					{
						LZPacketBufferSize = packetSize;
						
						free(LZPacketBuffer);
						LZPacketBuffer = (MemoryPtr)malloc(LZPacketBufferSize);
						gosASSERT(LZPacketBuffer);
					}
					
					if (LZPacketBuffer)
					{
						read(LZPacketBuffer,(packetSize-sizeof(long)));
						long decompLength = LZDecomp(buffer,LZPacketBuffer,packetSize-sizeof(long));
						if (decompLength != packetUnpackedSize)
							result = 0;
						else
							result = decompLength;
					}
				}
				break;

				case STORAGE_TYPE_ZLIB:
				{
					seek(packetBase+sizeof(long));

					if (!LZPacketBuffer)
					{
						LZPacketBuffer = (MemoryPtr)malloc(LZPacketBufferSize);
						gosASSERT(LZPacketBuffer);
					}
						
					if ((long)LZPacketBufferSize < packetSize)
					{
						LZPacketBufferSize = packetSize;
						
						free(LZPacketBuffer);
						LZPacketBuffer = (MemoryPtr)malloc(LZPacketBufferSize);
						gosASSERT(LZPacketBuffer);
					}
					
					if (LZPacketBuffer)
					{
						read(LZPacketBuffer,(packetSize-sizeof(long)));
						unsigned long decompLength = LZPacketBufferSize;
						long decompResult = uncompress(buffer,&decompLength,LZPacketBuffer,packetSize-sizeof(long));
						if ((decompResult != Z_OK) || ((long)decompLength != packetUnpackedSize))
							result = 0;
						else
							result = decompLength;
					}
				}
				break;

				case STORAGE_TYPE_HF:
					STOP(("Tried to read a Huffman Compressed Packet.  No Longer Supported!!"));
					break;
			}
		}
	}

	return result;
}

//---------------------------------------------------------------------------
long PacketFile::readPackedPacket (long packet, unsigned char *buffer)
{
	long result = 0;

	if ((packet==-1) || (packet == currentPacket) || (seekPacket(packet) == NO_ERR))
	{
		if ((getStorageType() == STORAGE_TYPE_RAW) || (getStorageType() == STORAGE_TYPE_FWF))
		{
			seek(packetBase);
			result = read(buffer, packetSize);
		}
		else
		{
			switch (getStorageType())
			{
				case STORAGE_TYPE_LZD:
				{
					seek(packetBase+sizeof(long));
					read(buffer,packetSize);
				}
				break;

				case STORAGE_TYPE_ZLIB:
				{
					seek(packetBase+sizeof(long));
					read(buffer,packetSize);
				}
				break;

			}
		}
	}

	return result;
}

//---------------------------------------------------------------------------
long PacketFile::seekPacket (long packet)
{
	long offset, next;

	if (packet < 0)
	{
		return(PACKET_OUT_OF_RANGE);
	}
	
	offset = readPacketOffset(packet, &packetType);

	currentPacket = packet++;

	if (packet == numPackets)
		next = getLength();
	else
		next = readPacketOffset(packet);

	packetSize = next-offset;

	packetBase = offset;  // seek to beginning of packet
	seek(packetBase);
	
	switch (getStorageType())
	{
		case STORAGE_TYPE_LZD:
			// the first DWORD of a compressed packet is the unpacked length
			packetUnpackedSize = readLong();
			break;

		case STORAGE_TYPE_ZLIB:
			// the first DWORD of a compressed packet is the unpacked length
			packetUnpackedSize = readLong();
			break;

		case STORAGE_TYPE_RAW:
			packetUnpackedSize = packetSize;
		break;

		case STORAGE_TYPE_NUL:
			packetUnpackedSize = 0;
		break;	
			
		default:
			return(BAD_PACKET_VERSION);
	}

	if (offset > 0)
		return(NO_ERR);
		
	return (PACKET_OUT_OF_RANGE);
}

//---------------------------------------------------------------------------
void PacketFile::operator ++ (void)
{
	if (++currentPacket >= numPackets)
	{
		currentPacket = numPackets-1;
	}

	seekPacket(currentPacket);
}

//---------------------------------------------------------------------------
void PacketFile::operator -- (void)
{
	if (currentPacket-- <= 0)
	{
		currentPacket = 0;
	}

	seekPacket(currentPacket);
}

//---------------------------------------------------------------------------
long PacketFile::getNumPackets (void) 
{ 
	return numPackets;
}

//---------------------------------------------------------------------------
long PacketFile::getCurrentPacket (void) 
{ 
	return currentPacket; 
}

//---------------------------------------------------------------------------
inline long PacketFile::getPacketOffset(void) 
{ 
	return packetBase;
}

//---------------------------------------------------------------------------
long PacketFile::getPackedPacketSize (void)
{
	return packetSize;
}

//---------------------------------------------------------------------------
long PacketFile::getStorageType (void)
{
	return packetType;
}

//---------------------------------------------------------------------------
void PacketFile::reserve (long count, bool useCheckSum)
{
	//---------------------------------------------------
	// If we already have packets, reserve does nothing.
	// Otherwise, reserve sets up the file.  Must be
	// called before any writing to a newly created file.
	if (numPackets)
	{
		return;
	}

	usesCheckSum = useCheckSum;

	numPackets = count;
	long firstPacketOffset = TABLE_ENTRY(numPackets);
	writeLong(PACKET_FILE_VERSION);
	writeLong(firstPacketOffset);

	//----------------------------
	// initialize the seek table
	while (count-- > 0)
		writeLong(SetPacketType(firstPacketOffset,STORAGE_TYPE_NUL));

	//-------------------------------------------------------------
	// If we called this, chances are we are writing a packet file
	// from start to finish.  It is MUCH faster if this table is
	// updated in memory and flushed when the file is closed.
	if (!seekTable)
	{
		seekTable = (long *)systemHeap->Malloc(numPackets * sizeof(long));
   			
   		if (seekTable != NULL)
   		{
   			seek(sizeof(long)*2);							//File Version & File Length
   			read(MemoryPtr(seekTable),(numPackets*sizeof(long)));
   		}
	}
}

//---------------------------------------------------------------------------
long PacketFile::writePacket (long packet, MemoryPtr buffer, long nbytes, unsigned char pType)
{
	//--------------------------------------------------------
	// This function writes the packet to the current end
	// of file and stores the packet address in the seek
	// table.  NOTE that this cannot be used to replace
	// a packet.  That function is writePacket which takes
	// a packet number and a buffer.  The size cannot change
	// and, as such, is irrelevant.  You must write the
	// same sized packet each time, if the packet already
	// exists.  In theory, it could be smaller but the check
	// right now doesn't allow anything but same size.
	long result = 0;

	MemoryPtr workBuffer = NULL;

	if (pType == ANY_PACKET_TYPE || pType == STORAGE_TYPE_LZD || pType == STORAGE_TYPE_ZLIB)
	{
		if ((nbytes<<1) < 4096)
			workBuffer = (MemoryPtr)malloc(4096);
		else
			workBuffer = (MemoryPtr)malloc(nbytes<<1);
		
		gosASSERT(workBuffer != NULL);
	}

	gosASSERT((packet > 0) || (packet < numPackets));

	packetBase = getLength();
	currentPacket = packet;
	packetSize = packetUnpackedSize = nbytes;

	//-----------------------------------------------
	// Code goes in here to pick the best compressed
	// version of the packet.  Otherwise, default
	// to RAW.
	if ((pType == ANY_PACKET_TYPE) || (pType == STORAGE_TYPE_LZD) || (pType == STORAGE_TYPE_ZLIB))
	{
		if (pType == ANY_PACKET_TYPE)
			pType = STORAGE_TYPE_RAW;
		
		//-----------------------------
		// Find best compression here.
		// This USED to use LZ.  Use ZLib from now on.
		// Game will ALWAYS be able to READ LZ Packets!!
		unsigned long actualSize = nbytes << 1;
		if (actualSize < 4096)
			actualSize = 4096;

		unsigned long workBufferSize = actualSize;
		unsigned long oldBufferSize = nbytes;
		long compressedResult = compress2(workBuffer,&workBufferSize,buffer,nbytes,Z_DEFAULT_COMPRESSION);
		if (compressedResult != Z_OK)
			STOP(("Unable to write packet %d to file %s.  Error %d",packet,fileName,compressedResult));
			
		compressedResult = uncompress(buffer,&oldBufferSize,workBuffer,nbytes);
		if ((long)oldBufferSize != nbytes)
			STOP(("Packet size changed after compression.  Was %d is now %d",nbytes,oldBufferSize));

		if ((pType == STORAGE_TYPE_LZD) || (pType == STORAGE_TYPE_ZLIB) || ((long)workBufferSize < nbytes))
		{
			pType = STORAGE_TYPE_ZLIB;
			packetSize = workBufferSize;
		}
	}
	
	packetType = pType;
	seek(packetBase);

	if (packetType == STORAGE_TYPE_ZLIB)
	{
		writeLong(packetUnpackedSize);
		result = write(workBuffer, packetSize);
	}
	else
	{
		result = write(buffer, packetSize);
	}

	if (!seekTable)
	{
		seek(TABLE_ENTRY(packet));
		writeLong(SetPacketType(packetBase,packetType));
	}
	else
	{
		seekTable[packet] = SetPacketType(packetBase,packetType);
	}

	long *currentEntry = NULL;
	if (seekTable)
	{
		packet++;
		currentEntry = &(seekTable[packet]);
	}

	long tableData = SetPacketType(getLength(),STORAGE_TYPE_NUL);
	while (packet < (numPackets - 1))
	{
		if (!seekTable)
		{
			writeLong(tableData);
		}
		else
		{
			*currentEntry = tableData;
			currentEntry++;
		}
		packet++;
	}
	
	if (workBuffer)
		free(workBuffer);
	
	return result;
}

#define DEFAULT_MAX_PACKET		65535
//---------------------------------------------------------------------------
long PacketFile::insertPacket (long packet, MemoryPtr buffer, long nbytes, unsigned char pType)
{
	//--------------------------------------------------------
	// This function writes the packet to the current end
	// of file and stores the packet address in the seek
	// table.  Originally, replace was a NONO.  No, we check
	// for the size and, if it is different, insert the new
	// packet into a new file and basically spend many timeparts doing it.
	// Necessary for the teditor.
	// I Love it.	
	long result = 0;

	if (packet < 0)
	{
		return result;
	}

	//---------------------------------------------------------------
	// Only used here, so OK if regular WINDOWS(tm) malloc!
	MemoryPtr workBuffer = (MemoryPtr)malloc(DEFAULT_MAX_PACKET);
	
	//-------------------------------------------------------------
	// All new code here.  Basically, open a new packet file,
	// write each of the old packets and this new one.  Close all
	// and copy the new one over the old one.  Open the new one and 
	// set pointers accordingly.
	PacketFile tmpFile;
	result = tmpFile.create("AF3456AF.788");
	if (packet >= numPackets)
	{
		numPackets++;
	}
	
	tmpFile.reserve(numPackets);
		
	for (long i=0;i<numPackets;i++)
	{
		if (i == packet)
		{
			if (nbytes >= DEFAULT_MAX_PACKET)
			{
				//----------------------------------------------------
				// Not sure what to do here.  We'll try reallocating
				::free(workBuffer);
				workBuffer = (MemoryPtr)malloc(packetSize);
			}
			
			tmpFile.writePacket(i,buffer,nbytes,pType);
		}
		else
		{
			seekPacket(i);
			long storageType = getStorageType();
			long packetSize = getPacketSize();
			if (packetSize >= DEFAULT_MAX_PACKET)
			{
				//----------------------------------------------------
				// Not sure what to do here.  We'll try reallocating
				::free(workBuffer);
				workBuffer = (MemoryPtr)malloc(packetSize);
			}
			
			readPacket(i,workBuffer);
			tmpFile.writePacket(i,workBuffer,packetSize,storageType);
		}
	}

	//------------------------------------
	// Now close and reassign everything.
	char ourFileName[250];
	long ourFileMode = 0;
	
	strcpy(ourFileName,fileName);
	ourFileMode = fileMode;
	
	tmpFile.close();
	close();
	
	remove(ourFileName);
	rename("AF3456AF.788",ourFileName);
	remove("AF3456AF.788");
	
	open(ourFileName,(FileMode)ourFileMode);
	
	seekPacket(packet);
		
	return result;
}

//---------------------------------------------------------------------------
long PacketFile::writePacket (long packet, MemoryPtr buffer)
{
	//--------------------------------------------------------
	// This function replaces the packet with the contents
	// of buffer.  There are two restrictions.  The first is
	// that the packet must be the same length as the existing
	// packet.  If not, buffer over/under run will occur.
	// The second is that the packet cannot be compressed since
	// there is no gaurantee that the new data will compress
	// to exactly the same length.  Returns NO_ERR if packet
	// written successfully.  Otherwise returns error.
	
	long result = 0;

	if ((packet < 0) || (packet >= numPackets))
	{
		return 0;
	}

	seekPacket(packet);

	if (packetType == STORAGE_TYPE_LZD || packetType == STORAGE_TYPE_HF || packetType == STORAGE_TYPE_ZLIB)
	{
		return (PACKET_WRONG_SIZE);
	}
	else
	{
		result = write(buffer, packetSize);
	}

	if (result == packetUnpackedSize)
	{
		return(NO_ERR);
	}
	
	return BAD_WRITE_ERR;
}

