//===========================================================================//
// File:	memstrm.cpp                                                      //
// Contents: Implementation Details of resource management                   //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"

//#############################################################################
//##########################    MemoryStream    ###############################
//#############################################################################

MemoryStream::ClassData*
	MemoryStream::DefaultData = NULL;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MemoryStream::InitializeClass()
{
	Verify(!DefaultData);
	DefaultData =
		new ClassData(
			MemoryStreamClassID,
			"Stuff::MemoryStream",
			RegisteredClass::DefaultData
		);
	Check_Object(DefaultData);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MemoryStream::TerminateClass()
{
	Check_Object(DefaultData);
	delete DefaultData;
	DefaultData = NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MemoryStream::MemoryStream(
	void *stream_start,
	DWORD stream_size,
	DWORD initial_offset
):
	RegisteredClass(DefaultData)
{
	Check_Pointer(this);

	streamStart = static_cast<BYTE*>(stream_start);
	streamSize = stream_size;
	Verify(initial_offset <= stream_size);
	currentPosition = streamStart + initial_offset;
	currentBit = Empty_Bit_Buffer;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MemoryStream::MemoryStream(
	ClassData *class_data,
	void *stream_start,
	DWORD stream_size,
	DWORD initial_offset
):
	RegisteredClass(class_data)
{
	Check_Pointer(this);

	streamStart = static_cast<BYTE*>(stream_start);
	streamSize = stream_size;
	Verify(initial_offset <= stream_size);
	currentPosition = streamStart + initial_offset;
	currentBit = 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MemoryStream::~MemoryStream()
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MemoryStream&
	MemoryStream::ReadBytes(
		void *ptr,
		DWORD number_of_bytes
	)
{
	Check_Object(this);
	Check_Pointer(ptr);
	Verify(number_of_bytes > 0);
	#if defined(_ARMOR)
		DWORD remaining = GetBytesRemaining();
		Verify(remaining >= number_of_bytes);
	#endif

	Mem_Copy(ptr, GetPointer(), number_of_bytes, number_of_bytes);
	AdvancePointer(number_of_bytes);
	return *this;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
int
	MemoryStream::ReadChar()
{
	Check_Object(this);
	DWORD bytes_remaining = GetBytesRemaining();
	if (bytes_remaining > 0)
	{
		BYTE byte;
		ReadBytes(&byte, 1);
		return byte;				
	}
	return End_Of_Stream;
}	

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
bool
	MemoryStream::ReadLine(
		char *buffer,
		int max_number_of_bytes,
		char continuator
	)
{
	Check_Object(this);
	Check_Pointer(buffer);
	Verify(max_number_of_bytes > 0);

	//
	// while the character is not end of file &&
	// there is room left in the buffer,
	// copy the character into the buffer
	//
	int c = ReadChar();
	if (c == End_Of_Stream)
		return false;

	char *p = buffer;
	int i = 0;

	Check_Object(FileStreamManager::Instance);
	const char *eol = "\r\n";
	DWORD eol_length = strlen(eol);

	while (c != End_Of_Stream && i < max_number_of_bytes)
	{
		Check_Pointer(p); 
		Verify(i < max_number_of_bytes); 
		p[i] = (char)c;

		//
		// If we have met the end of line condition, break
		//
		switch (eol_length)
		{
		case 1:
			if (p[i] == *eol)
			{
				c = End_Of_Stream;
			}
			else
			{
				++i;
				c = ReadChar();			
			}
			break;

		case 2:
			if (p[i] == eol[1] && i > 0 && p[i-1] == eol[0])
			{
				i -= 1;
				c = End_Of_Stream;
			}
			else
			{
				++i;
				c = ReadChar();			
			}
			break;

		default:
			STOP(("Unhandled eol style"));
		}
	}
	if (i < max_number_of_bytes)
	{
		p[i] = '\0';
		return true;
	}

	Warn(i >= max_number_of_bytes);
	Verify(i == max_number_of_bytes);
	p[i-1] = '\0';

	return false;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MString
	MemoryStream::ReadString(
		DWORD size_of_buffer,
		char continuator
	)
{
	Check_Object(this);
	Verify(size_of_buffer > 0);

	static char buffer[1024];
	Verify(size_of_buffer < sizeof(buffer));

	MString temp_string;
	if (ReadLine(buffer, size_of_buffer, continuator))
	{
		temp_string = buffer;
	}
	return temp_string;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MemoryStream&
	MemoryStream::ReadSwappedBytes(
		void *ptr,
		DWORD number_of_bytes
	)
{
	Check_Object(this);
	Check_Pointer(ptr);
	Verify(number_of_bytes > 0);
	Verify(GetBytesRemaining() >= number_of_bytes);

	BYTE *buffer = new BYTE[number_of_bytes];
	Check_Pointer(buffer);
	ReadBytes(buffer, number_of_bytes);
	for (DWORD i=0; i<number_of_bytes; ++i)
	{
		static_cast<BYTE*>(ptr)[i] = buffer[number_of_bytes-1 - i];
	}
	Check_Pointer(buffer);
	delete[] buffer;
	return *this;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MemoryStream::WriteLine(char *buffer)
{
	Check_Object(this);
	Check_Pointer(buffer);

	WriteBytes(buffer, strlen(buffer));
	WriteBytes("\r\n", 2);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MemoryStream&
	MemoryStream::WriteBytes(
		const void *ptr,
		size_t number_of_bytes
	)
{
	Check_Object(this);
	Check_Pointer(ptr);
	Verify(number_of_bytes > 0);

	Mem_Copy(GetPointer(), ptr, number_of_bytes, GetBytesRemaining());
	AdvancePointer(number_of_bytes);
	return *this;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MemoryStream&
	MemoryStream::WriteSwappedBytes(
		const void *ptr,
		DWORD number_of_bytes
	)
{
	Check_Object(this);
	Check_Pointer(ptr);
	Verify(number_of_bytes > 0);
	Verify(GetBytesRemaining() >= number_of_bytes);

	BYTE *buffer = new BYTE[number_of_bytes];
	Check_Pointer(buffer);
	for (DWORD i=0; i<number_of_bytes; ++i)
	{
		buffer[number_of_bytes-1 - i] = static_cast<const BYTE*>(ptr)[i];
	}
	WriteBytes(buffer, number_of_bytes);
	Check_Pointer(buffer);
	delete[] buffer;
	return *this;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MemoryStream&
	MemoryStreamIO::Write(
		MemoryStream *stream,
		const MemoryStream *input_stream
	)
{
	Check_Object(stream);
	Check_Object(input_stream);

	return
		stream->WriteBytes(
			input_stream->GetPointer(),
			input_stream->GetBytesRemaining()
		);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  Bit packing methods...
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


#if 0
		
		// quick reference for the boneheads...
		
		// >> shift right
		// << shift left	
		// ~ complement
		// ^ exclusive or - 1 if bits match
		// | and		  - 1 if either bit is one
		// & or			  - 1 if both bits are one
		
		// 0000|1111 << 4 = 1111|0000
		// 1111|0000 >> 2 = 0011|1100
		// ~1111|0000 = 0000|1111
		
		//
		//    0000|1111
		//	^ 1010|1010
		//---------------
		//	  0101|1010
		
		//
		//	  0000|0101
		//	| 1100|1100
		//---------------
		//	  1100|1101
		
		//
		//	  0000|0101
		//	& 1100|1100
		//---------------
		//	  0000|0100
						
		
		
		// set a number to a specific value
		
		
		
		// bitmasks
		
		//0x01 // 0000|0001
		//0x03 // 0000|0011
		//0x07 // 0000|0111
		//0x0f // 0000|1111
		//0x1f // 0001|1111
		//0x3f // 0011|1111
		//0x7f // 0111|1111
		//0xff // 1111|1111
		
		// inverted bitmasks
		
		//0x80 // 1000|0000
		//0xc0 // 1100|0000
		//0xe0 // 1110|0000
		//0xf0 // 1111|0000
		//0xf8 // 1111|1000
		//0xfc // 1111|1100
		//0xfe // 1111|1110
		//0xff // 1111|1111

	

		Verify(bit_count > 0);
		Verify(bit_count < 32);

		unsigned int bit_count = 0;
		bit_count = 0x1 << i;

		SPEW(("jerryeds", "%u %u", i,bit_count));
	
#endif


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//

void
	MemoryStream::ByteAlign()
{
	if (currentBit == 0)
	{
		return;
	}
	Check_Object(this);
	//SPEW(("jerryeds", "WRITE : %x", workingBitBuffer));
	WriteBytes(&workingBitBuffer, 1);
	workingBitBuffer = 0x00;
	currentBit = 0;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MemoryStream&
	MemoryStream::ReadBit(bool &bit_value)
{
	Check_Object(this);

	int result = 0;
	
	if (currentBit == Empty_Bit_Buffer)
	{
		ReadBytes(&workingBitBuffer, 1);
		//SPEW(("jerryeds", "READ : %x", workingBitBuffer));
		currentBit = 0;
	}	


	Verify(currentBit >= 0);
	Verify(currentBit <  8);

	result = (0x01 & (workingBitBuffer >> currentBit));
	bit_value = (result)?true:false;

	currentBit++;
	if (currentBit > 7 && GetBytesRemaining())
	{

		ReadBytes(&workingBitBuffer, 1);
		//SPEW(("jerryeds", "READ : %x", workingBitBuffer));
		currentBit = 0;
	}

	return *this;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
 
MemoryStream&
	MemoryStream::WriteBit(const bool &bit_value)
{
	Check_Object(this);

	int int_bit_value = (bit_value)?1:0;
	
	if (currentBit == Empty_Bit_Buffer)
	{
		currentBit = 0;
		workingBitBuffer = 0x0;
	}

	Verify(currentBit >= 0);
	Verify(currentBit <  8);

	workingBitBuffer |= (int_bit_value << currentBit);
	
	
	currentBit++;
	if (currentBit > 7)
	{
		WriteBytes(&workingBitBuffer, 1);
		//SPEW(("jerryeds", "WRITE : %x", workingBitBuffer));
		workingBitBuffer = 0x00;
		currentBit = 0;
	}



	return *this;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MemoryStream&
	MemoryStream::ReadUnsafeBits(
		void *ptr, 
		DWORD number_of_bits
	)
{
	Check_Object(this);
	Check_Pointer(ptr);

	if (currentBit == Empty_Bit_Buffer)
	{
		ReadBytes(&workingBitBuffer, 1);
		//SPEW(("jerryeds", "READ : %x", workingBitBuffer));
		currentBit = 0;
	}

	Verify(currentBit >= 0);
	Verify(currentBit < 8);

	int total_number_of_bytes = (int)(number_of_bits/8.0f);
	int total_remainder_bits = number_of_bits - (total_number_of_bytes*8);

	if ( total_remainder_bits != 0)
	{
		total_number_of_bytes += 1;
	}

	BYTE *dest_array = Cast_Pointer(BYTE *, ptr);
	int dest_byte_counter = 0;

	int bits_remaining = number_of_bits;

	while (bits_remaining > 0)
	{
		BYTE *dest = &dest_array[dest_byte_counter];
		// empy out destination
		*dest = 0x00;


		int total_bits_to_be_read = 8;
		Max_Clamp(total_bits_to_be_read, bits_remaining);
		
		int bits_in_first_source_byte = total_bits_to_be_read;
		Max_Clamp(bits_in_first_source_byte, 8-currentBit);

		// make a mask of bits to be used
		int bit_mask = 0xff >> (8-bits_in_first_source_byte);

		// shift past the used bits in the buffer
		*dest = (BYTE)(workingBitBuffer >> currentBit);
		
		// remove any bits from the next set that tagged along
		*dest &= bit_mask;
		

		currentBit += bits_in_first_source_byte;


		if (currentBit == 8)
		{
			int bits_in_second_source_byte = total_bits_to_be_read - bits_in_first_source_byte;

			if (GetBytesRemaining())
			{
				ReadBytes(&workingBitBuffer, 1);
				//SPEW(("jerryeds", "READ : %x", workingBitBuffer));
			}

			++dest_byte_counter;
			currentBit = 0;

			if (bits_in_second_source_byte)
			{
				// make a bitmask of the used bits
				bit_mask = 0xff >> (8-bits_in_second_source_byte);
				// mask the used bits out of the buffer and then shift the buffer to the correct 
				// bit location.  Then combine it with the destination
				*dest |= ((workingBitBuffer & bit_mask) << bits_in_first_source_byte);


				currentBit = bits_in_second_source_byte;
			}
		}


		bits_remaining -= total_bits_to_be_read;


	}

	Verify(currentBit >= 0);
	Verify(currentBit < 8);


	return *this;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//

MemoryStream&
	MemoryStream::WriteBits(
		const void *ptr, 
		DWORD number_of_bits
	)
{
	Check_Object(this);
	Check_Pointer(ptr);


	if (currentBit == Empty_Bit_Buffer)
	{
		workingBitBuffer = 0x00;
		currentBit = 0;
	}
	
	Verify(currentBit >= 0);
	Verify(currentBit < 8);


	

	int total_number_of_bytes = (int)(number_of_bits/8.0f);
	int total_remainder_bits = number_of_bits  - (total_number_of_bytes*8);

	if ( total_remainder_bits != 0)
	{
		total_number_of_bytes += 1;
	}

	
	const BYTE *source_array = Cast_Pointer(const BYTE *, ptr);
	int source_byte_counter = 0;
	
	int bits_remaining = number_of_bits;

	while (bits_remaining > 0)
	{
	
		
		// if we are writing the full byte
		const BYTE *source = &source_array[source_byte_counter];
	
		int total_bits_to_be_written = 8;
		Max_Clamp(total_bits_to_be_written, bits_remaining);
		
		int bits_in_first_destination_byte = total_bits_to_be_written;
		Max_Clamp(bits_in_first_destination_byte, 8-currentBit);


		
		// make sure our destination is empty
		workingBitBuffer &= (0xff >> (8-currentBit));
		// move the source data to the correct bit location
		BYTE new_source = (BYTE)(*source << currentBit);
		// put the adjusted source into the destination
		workingBitBuffer |= new_source;



		currentBit += bits_in_first_destination_byte;


		if (currentBit == 8)
		{
			int bits_in_second_destination_byte = total_bits_to_be_written - bits_in_first_destination_byte;

			//SPEW(("jerryeds", "WRITE : %x", workingBitBuffer));
			WriteBytes(&workingBitBuffer, 1);
			++source_byte_counter;
			workingBitBuffer = 0x00;
			currentBit = 0;

			if (bits_in_second_destination_byte)
			{
				// remove bits we have already used
				workingBitBuffer = (BYTE)(*source >> bits_in_first_destination_byte);
				currentBit = bits_in_second_destination_byte;
			}

		}


		bits_remaining -= total_bits_to_be_written;
	
	}
	return *this;
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//

MemoryStream&
	MemoryStream::ReadBitsToScaledInt(int &number, int min, int max,  DWORD number_of_bits)
{

	DWORD buffer = 0x0;

	ReadUnsafeBits(&buffer, number_of_bits);
	
	number = Scaled_Int_From_Bits(buffer, min, max, number_of_bits);

	return *this;
}	

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//

MemoryStream&
	MemoryStream::WriteScaledIntToBits(const int &number, int min, int max,  DWORD number_of_bits)
{
	Check_Object(this);
	
	DWORD buffer;

	buffer = Scaled_Int_To_Bits(number, min, max, number_of_bits);

	WriteBits(&buffer, number_of_bits);
	
	return *this;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//

MemoryStream&
	MemoryStream::ReadBitsToScaledFloat(float &number, float min, float max,  DWORD number_of_bits)
{
	Check_Object(this);
	
	DWORD buffer = 0x0;

	ReadBits(&buffer, number_of_bits);
	
	number = Scaled_Float_From_Bits(buffer, min, max, number_of_bits);

	return *this;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//

MemoryStream&
	MemoryStream::WriteScaledFloatToBits(const float &number, float min, float max,  DWORD number_of_bits)
{
	Check_Object(this);
	
	DWORD buffer;

	buffer = Scaled_Float_To_Bits(number, min, max, number_of_bits);

	WriteBits(&buffer, number_of_bits);
	
	return *this;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
void
	MemoryStream::TestInstance() const
{
	Verify(IsDerivedFrom(DefaultData));
	Verify(DWORD(currentPosition - streamStart) <= streamSize);
}

//#############################################################################
//######################    DynamicMemoryStream    ############################
//#############################################################################

//
// calculate the allocation size for stream
//
static inline DWORD
	Calculate_Buffer_Size(DWORD needed)
{
	return (needed+0x80)&~0x7F;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
DynamicMemoryStream::DynamicMemoryStream(DWORD stream_size):
	MemoryStream(NULL, stream_size)
{
	Check_Pointer(this);

	bufferSize = Calculate_Buffer_Size(stream_size);
	streamStart = new BYTE[bufferSize];
	Check_Pointer(streamStart);

	currentPosition = streamStart;
	streamSize = stream_size;
	ownsStream = true;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
DynamicMemoryStream::DynamicMemoryStream(const DynamicMemoryStream &otherStream):
	MemoryStream(NULL, otherStream.GetSize())
{
	Check_Pointer(this);
	Check_Pointer(&otherStream);

	bufferSize = otherStream.bufferSize;	
	streamStart = new BYTE[bufferSize];
	Check_Pointer(streamStart);

	streamSize = otherStream.GetSize();
	Mem_Copy(streamStart, otherStream.streamStart, streamSize, bufferSize);
	currentPosition = streamStart + otherStream.GetBytesUsed();
	ownsStream = true;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
DynamicMemoryStream::DynamicMemoryStream(
	void *stream_start,
	DWORD stream_size,
	DWORD initial_offset
):
	MemoryStream(stream_start, stream_size, initial_offset)
{
	Check_Pointer(this);
	ownsStream = false;
	bufferSize = stream_size;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
DynamicMemoryStream::~DynamicMemoryStream()
{
	Check_Object(this);
	if (ownsStream)
	{
		Check_Pointer(streamStart);
		delete[] streamStart;
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
bool
	DynamicMemoryStream::AllocateBytes(DWORD count)
{
	Check_Object(this);

	DWORD current_offset = currentPosition - streamStart;
	DWORD new_stream_size = current_offset + count;
	if (ownsStream && bufferSize - current_offset < count)
	{
		bufferSize = Calculate_Buffer_Size(new_stream_size);
		BYTE *new_stream_start = new BYTE[bufferSize];
		Check_Pointer(new_stream_start);

		Mem_Copy(new_stream_start, streamStart, streamSize, bufferSize);
		Check_Pointer(streamStart);
		delete[] streamStart;
		streamStart = new_stream_start;
		streamSize = new_stream_size;
		currentPosition = streamStart + current_offset;
	}
	else if (new_stream_size > streamSize)
		streamSize = new_stream_size;
	return true;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
MemoryStream&
	DynamicMemoryStream::WriteBytes(
		const void *ptr,
		DWORD number_of_bytes
	)
{
	Check_Object(this);
	Check_Pointer(ptr);
	Verify(number_of_bytes > 0);

	//
	//--------------------------------------------------------------------
	// If we own the stream, and we don't have enough memory to accept the
	// write, copy the buffer and make a new stream that is bigger
	//--------------------------------------------------------------------
	//
	AllocateBytes(number_of_bytes);
	Mem_Copy(
		GetPointer(),
		ptr,
		number_of_bytes,
		MemoryStream::GetBytesRemaining()
	);
	AdvancePointer(number_of_bytes);

	bufferSize = Max(bufferSize, GetBytesUsed());
	return *this;
}

