//===========================================================================//
// File:	memstrm.hpp                                                      //
// Contents: Implementation Details of resource management                   //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"stuff.hpp"

namespace Stuff {
    class MemoryStream;
    class RGBAColor;
    class RGBColor;

    class HSVAColor;
    class HSVColor;

	typedef float Scalar; // scalar.hpp

    class Point3D;

    class LinearMatrix4D;

    class MStringRepresentation;

    class Radian;
}

// sebi [
// added declarations here bacuse of crose includes of memorystream from those files.
// maybe better create additional color.h and scalar.h files with those definitions
//

// angle.hpp
namespace MemoryStreamIO {

	inline Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream* stream,
			Stuff::Radian *output
		);
	inline Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const Stuff::Radian *input
		);
}

// mstring
namespace MemoryStreamIO {

	Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream* stream,
			Stuff::MStringRepresentation *str
		);
	Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const Stuff::MStringRepresentation& str
		);

	Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream* stream,
			Stuff::MString *str
		);
	Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const Stuff::MString* str
		);
}

// linearmatrix
namespace MemoryStreamIO {

	inline Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream *stream,
			Stuff::LinearMatrix4D *output
		);
	inline Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream *stream,
			const Stuff::LinearMatrix4D *input
		);
}

// point3d
namespace MemoryStreamIO {

	inline Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream* stream,
			Stuff::Point3D *output
		);
	inline Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const Stuff::Point3D *input
        );

}

// from scalar.hpp
namespace MemoryStreamIO {

	inline Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream* stream,
			Stuff::Scalar *output
		);
	inline Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const Stuff::Scalar *input
		);
}

// from color.hpp
namespace MemoryStreamIO {

	inline Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream* stream,
			Stuff::RGBColor *output
		);
	inline Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const Stuff::RGBColor *input
		);

	inline Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream* stream,
			Stuff::RGBAColor *output
		);
	inline Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const Stuff::RGBAColor *input
		);

	inline Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream* stream,
			Stuff::HSVColor *output
		);
	inline Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const Stuff::HSVColor *input
		);

	inline Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream* stream,
			Stuff::HSVAColor *output
		);
	inline Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const Stuff::HSVAColor *input
		);
}
// ] sebi

namespace MemoryStreamIO {

	inline Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream* stream,
			char *output
		);
	inline Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream* stream,
			BYTE *output
		);
	inline Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream* stream,
			short *output
		);
	inline Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream* stream,
			WORD *output
		);
	inline Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream* stream,
			bool *output
		);
	inline Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream* stream,
			uint32_t *output
		);
	inline Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream* stream,
			int32_t *output
		);
	inline Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream* stream,
			uint64_t*output
		);
	inline Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream* stream,
			int64_t *output
		);

////////////////////////////////////////////////////////////////////////////////
	inline Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const char *input
		);
	inline Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const BYTE *input
		);
	inline Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const short *input
		);
	inline Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const WORD *input
		);
	inline Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const bool *input
		);
	inline Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const uint32_t *input
		);
	inline Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const int32_t *input
		);
	inline Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const uint64_t* input
		);
	inline Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const int64_t *input
		);
	inline Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const char* const* input
		);
	Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const Stuff::MemoryStream* input_stream
		);
}

namespace Stuff {

	class MemoryStream;
	class DynamicMemoryStream;

	//##########################################################################
	//#######################    MemoryStream    ###############################
	//##########################################################################

	const int End_Of_Stream=-1;

	const int Empty_Bit_Buffer=-20;


	class MemoryStream :
		public RegisteredClass
	{
		friend MemoryStream&
			MemoryStreamIO::Write(
				MemoryStream *stream,
				const MemoryStream *input_stream
			);

	public:
		static void
			InitializeClass();
		static void
			TerminateClass();

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Construction and testing
	//
	public:
		MemoryStream(
			void *stream_start,
			DWORD stream_size,
			DWORD initial_offset=0
		);
		virtual ~MemoryStream();

		void
			TestInstance() const;

		static bool
			TestClass();

	protected:
		MemoryStream(
			ClassData *data,
			void *stream_start,
			DWORD stream_size,
			DWORD initial_offset=0
		);


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Class Data Support
	//
	public:
		static ClassData
			*DefaultData;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Accessors
	//
	public:
		virtual void*
			GetPointer() const
				{Check_Object(this); return currentPosition;}
		
		size_t GetIndex() const
				{Check_Object(this); return currentPosition - streamStart;}
		size_t
			GetSize() const
				{Check_Object(this); return streamSize;}

		size_t
			GetBytesUsed() const
				{Check_Object(this); return currentPosition - streamStart;}
		virtual size_t
			GetBytesRemaining() const
				{Check_Object(this); return streamSize - GetBytesUsed();}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Assignment methods
	//
	public:
		virtual void
			SetPointer(void *new_pointer)
				{
					Check_Pointer(this);
					currentPosition = Cast_Pointer(BYTE*, new_pointer);
					Check_Object(this);
				}
		void
			operator=(void *new_pointer)
				{SetPointer(new_pointer);}

		virtual void
			SetPointer(DWORD index)
				{
					Check_Pointer(this);
					currentPosition = streamStart + index;
					Check_Object(this);
				}
		void
			operator=(DWORD index)
				{SetPointer(index);}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Stream methods
	//
	public:
		void
			Rewind()
				{SetPointer((DWORD) 0U);currentBit = Empty_Bit_Buffer;}

		virtual bool
			AllocateBytes(DWORD count)
				{return GetBytesRemaining() <= count;}
		virtual MemoryStream&
			AdvancePointer(DWORD count)
				{
					Check_Object(this);
					currentPosition += count;

					Verify(currentPosition >= streamStart);
					Verify(currentPosition <= streamStart + streamSize);

					Check_Object(this);
					return *this;
				}
		MemoryStream&
			operator+=(DWORD count)
				{return AdvancePointer(count);}

		virtual MemoryStream&
			RewindPointer(DWORD count)
				{
					Check_Object(this);
					currentPosition -= count;

					Verify(currentPosition >= streamStart);
					Verify(currentPosition <= streamStart + streamSize);

					Check_Object(this);
					return *this;
				}
		MemoryStream&
			operator-=(DWORD count)
				{return RewindPointer(count);}

		virtual MemoryStream&
			ReadBytes(
				void *ptr,
				DWORD number_of_bytes
			);
		virtual MemoryStream&
			WriteBytes(
				const void *ptr,
				size_t number_of_bytes
			);

		MemoryStream&
			ReadSwappedBytes(
				void *ptr,
				DWORD number_of_bytes
			);
		MemoryStream&
			WriteSwappedBytes(
				const void *ptr,
				DWORD number_of_bytes
			);

		virtual int
			ReadChar();

		virtual bool
			ReadLine(
				char *buffer,
				int size_of_buffer,
				char continuator = '\0'
			);

		MString
			ReadString(
				DWORD size_of_buffer = 512,
				char continuator = '\0'
			);

		void
			WriteLine(char *buffer);


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Bit operators  
	//
	// WARNING - DO NOT MIX AND MATCH WITH BYTE METHODS!!!!!
	//
	
	private:
		int
			currentBit;

		BYTE
			workingBitBuffer;

		MemoryStream&
			ReadUnsafeBits(void *ptr, DWORD number_of_bits);

	public:
		MemoryStream&
			ReadBit(bool &bit_value);
 
		MemoryStream&
			WriteBit(const bool &bit_value);

		
		MemoryStream&
			ReadBits(int *ptr, DWORD number_of_bits)
				{
					Verify(number_of_bits <= 32);
					Check_Object(this); Check_Pointer(ptr);
					*ptr = 0; return ReadUnsafeBits(ptr, number_of_bits);
				}

		MemoryStream&
			ReadBits(BYTE *ptr, DWORD number_of_bits)
				{
					Verify(number_of_bits <= 8);
					Check_Object(this); Check_Pointer(ptr);
					*ptr = 0; return ReadUnsafeBits(ptr, number_of_bits);
				}


		MemoryStream&
			ReadBits(WORD *ptr, DWORD number_of_bits)
				{
					Verify(number_of_bits <= 16);
					Check_Object(this); Check_Pointer(ptr);
					*ptr = 0; return ReadUnsafeBits(ptr, number_of_bits);
				}

		MemoryStream&
			ReadBits(DWORD *ptr, DWORD number_of_bits)
				{
					Verify(number_of_bits <= 32);
					Check_Object(this); Check_Pointer(ptr);
					*ptr = 0; return ReadUnsafeBits(ptr, number_of_bits);
				}

		MemoryStream&
			ReadBits(float *ptr, DWORD number_of_bits)
				{
					Verify(number_of_bits == 32);
					Check_Object(this); Check_Pointer(ptr);
					*ptr = 0.0; return ReadUnsafeBits(ptr, number_of_bits);
				}


		MemoryStream&
			ReadBits(double *ptr, DWORD number_of_bits)
				{
					Verify(number_of_bits == 64);
					Check_Object(this); Check_Pointer(ptr);
					*ptr = 0.0; return ReadUnsafeBits(ptr, number_of_bits);
				}


		MemoryStream&
			WriteBits(const void *ptr, DWORD number_of_bits);
		
		
		
	
		
		MemoryStream&
			ReadBitsToScaledInt(int &number, int min, int max,  DWORD number_of_bits);
		
		MemoryStream&
			WriteScaledIntToBits(const int &number, int min, int max,  DWORD number_of_bits);
	
		MemoryStream&
			ReadBitsToScaledFloat(float &number, float min, float max,  DWORD number_of_bits);
		
		MemoryStream&
			WriteScaledFloatToBits(const float &number, float min, float max,  DWORD number_of_bits);


		void
			ByteAlign();

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Protected data
	//
	protected:
		BYTE
			*streamStart,
			*currentPosition;
		size_t
			streamSize;
	};

	template <class T> inline MemoryStream&
		Read_Swapped(
			MemoryStream *stream,
			T data
		)
	{
		return stream->ReadSwappedBytes(data, sizeof(*data));
	}

	template <class T> inline MemoryStream&
		Write_Swapped(
			MemoryStream *stream,
			const T data
		)
	{
		return stream->WriteSwappedBytes(&data, sizeof(data));
	}

	//--------------------------------------------------------------------------
	// Extraction operators
	//
	template <class T> inline MemoryStream&
		operator>>(
			MemoryStream &stream,
			T &output
		)
			{return MemoryStreamIO::Read(&stream, &output);}

	//--------------------------------------------------------------------------
	// Insertion operators
    //
    template <class T> inline MemoryStream&
        operator<<(
                MemoryStream &stream,
                const T &input
                )
        {return MemoryStreamIO::Write(&stream, &input);}

    inline MemoryStream&
        operator<<(
                MemoryStream& stream,
                const char* input
                )
        {
            Check_Pointer(input);
            return stream.WriteBytes(input, strlen(input));
        }

    //##########################################################################
	//###################    DynamicMemoryStream    ############################
	//##########################################################################

	class DynamicMemoryStream:
		public MemoryStream
	{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Construction, destruction, and testing
	//
	public:
		DynamicMemoryStream(DWORD stream_size=0);
		DynamicMemoryStream(
			void *stream_start,
			DWORD stream_size,
			DWORD initial_offset=0
		);
		DynamicMemoryStream(const DynamicMemoryStream& otherStream);
		~DynamicMemoryStream();

		void
			TestInstance() const
				{
					Verify((DWORD)(currentPosition - streamStart) <= streamSize);
					Verify(streamSize <= bufferSize);
				}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Accessors
	//
	public:
		DWORD
			GetBufferSize()
				{
					Check_Object(this);
					return bufferSize;
				}

		void
			SetSize(DWORD stream_length)
				{
					Check_Object(this);
					Verify(stream_length <= bufferSize);
					streamSize = stream_length;
				}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Stream methods
	//
	public:
		MemoryStream&
			WriteBytes(
				const void *ptr,
				DWORD number_of_bytes
			);
		
		bool
			AllocateBytes(DWORD count);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Private data
	//
	private:
		DWORD
			bufferSize;
		bool
			ownsStream;
	};

}

namespace MemoryStreamIO {

	inline Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream* stream,
			char *output
		)
			{return stream->ReadBytes(output, sizeof(*output));}
	inline Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream* stream,
			BYTE *output
		)
			{return stream->ReadBytes(output, sizeof(*output));}

	inline Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream* stream,
			short *output
		)
			{return stream->ReadBytes(output, sizeof(*output));}
	inline Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream* stream,
			WORD *output
		)
			{return stream->ReadBytes(output, sizeof(*output));}
	inline Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream* stream,
			bool *output
		)
			{return stream->ReadBytes(output, sizeof(*output));}
	inline Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream* stream,
			int32_t*output
		)
			{return stream->ReadBytes(output, sizeof(*output));}
	inline Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream* stream,
			uint32_t *output
		)
			{return stream->ReadBytes(output, sizeof(*output));}
	inline Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream* stream,
			uint64_t *output
		)
			{return stream->ReadBytes(output, sizeof(*output));}
	inline Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream* stream,
			int64_t *output
		)
			{return stream->ReadBytes(output, sizeof(*output));}

////////////////////////////////////////////////////////////////////////////////
	inline Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const char *input
		)
			{return stream->WriteBytes(input, sizeof(*input));}
	inline Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const BYTE *input
		)
			{return stream->WriteBytes(input, sizeof(*input));}

	inline Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const short *input
		)
			{return stream->WriteBytes(input, sizeof(*input));}
	inline Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const WORD *input
		)
			{return stream->WriteBytes(input, sizeof(*input));}
	inline Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const bool *input
		)
			{return stream->WriteBytes(input, sizeof(*input));}
	inline Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const uint32_t *input
		)
			{return stream->WriteBytes(input, sizeof(*input));}
	inline Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const int32_t *input
		)
			{return stream->WriteBytes(input, sizeof(*input));}
	inline Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const uint64_t*input
		)
			{return stream->WriteBytes(input, sizeof(*input));}
	inline Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const int64_t* input
		)
			{return stream->WriteBytes(input, sizeof(*input));}

	inline Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const char* const* input
		)
			{
				Check_Pointer(*input);
				return stream->WriteBytes(*input, strlen(*input));
			}

	Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const Stuff::MemoryStream* input_stream
		);

}
