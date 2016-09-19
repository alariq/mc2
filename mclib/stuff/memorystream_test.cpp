//===========================================================================//
// File:	MemoryStream_Test.cpp                                            //
// Contents: Test stuff for MMIO class                                       //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"
#include<toolos.hpp>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ MMIOstream ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool SingeBitStreamTest(int total_sections_to_write);
bool MultipleBitStreamTest(int total_sections_to_write);
bool FloatIntBitStreamTest(int total_sections_to_write);

//
//#############################################################################
//#############################################################################
//

bool
	MemoryStream::TestClass()
{


	SPEW((GROUP_STUFF_TEST, "Starting MemoryStream Single BitPacking Test..."));
	Test_Assumption(SingeBitStreamTest(100));
	SPEW((GROUP_STUFF_TEST, "Complete MemoryStream Single BitPacking Test!"));


	SPEW((GROUP_STUFF_TEST, "Starting MemoryStream MultipleBitPacking Test..."));
	//Test_Assumption(MultipleBitStreamTest(100));
	SPEW((GROUP_STUFF_TEST, "Complete MemoryStream MultipleBitPacking Test!"));


	SPEW((GROUP_STUFF_TEST, "Starting MemoryStream Int/Float BitPacking Test..."));
	Test_Assumption(FloatIntBitStreamTest(1000));
	SPEW((GROUP_STUFF_TEST, "Complete MemoryStream Int/Float BitPacking Test!"));




	return true;
};

//
//#############################################################################

bool SingeBitStreamTest(int total_sections_to_write)
{
	bool *source_array_of_bools = new bool[total_sections_to_write];
	Register_Pointer(source_array_of_bools);

	bool *comp_array_of_bools = new bool[total_sections_to_write];
	Register_Pointer(comp_array_of_bools);

	
	for (int i = 0; i < total_sections_to_write; ++i)
	{
		int random = Random::GetLessThan(10);

		if (random > 4)
		{
			source_array_of_bools[i] = true;
		}
		else
		{
			source_array_of_bools[i] = false;
		}

	}


	// calculate total byte depth.
	int total_number_of_bytes = (int)(total_sections_to_write/8.0f);
	int total_remainder_bits = total_sections_to_write - (total_number_of_bytes*8);
	if ( total_remainder_bits != 0)
	{
		total_number_of_bytes += 1;
	}

	
	BYTE *big_byte_array = new BYTE[total_number_of_bytes];
	Register_Pointer(big_byte_array);
	
	MemoryStream bit_stream(big_byte_array, total_number_of_bytes);


	for (int i = 0; i < total_sections_to_write; ++i)
	{
		bit_stream.WriteBit(source_array_of_bools[i]);
	}


	bit_stream.ByteAlign();
	bit_stream.Rewind();

	for (int i = 0; i < total_sections_to_write; ++i)
	{
		bit_stream.ReadBit(comp_array_of_bools[i]);
	}


	for (int i = 0; i < total_sections_to_write; ++i)
	{
		Verify(source_array_of_bools[i] == comp_array_of_bools[i]);
		Test_Assumption(source_array_of_bools[i] == comp_array_of_bools[i]);
		
	}

	Unregister_Pointer(big_byte_array);
	delete[] big_byte_array;

	Unregister_Pointer(source_array_of_bools);
	delete[] source_array_of_bools;

	Unregister_Pointer(comp_array_of_bools);
	delete[] comp_array_of_bools;


	return true;
}
//
//#############################################################################


template <class T> class MinMaxHolderOf
	{
	 public:
		 T 
			 minValue;
		 T 
			 maxValue;
		 T
			 value;

		MinMaxHolderOf()
		{
		
		}
	};

template<>
MinMaxHolderOf<int>::MinMaxHolderOf()
{
	minValue = -Random::GetInt();
	maxValue = Random::GetInt();


	int ramp = Random::GetLessThan(10)+1;
	ramp *= ramp;

	minValue *= ramp;
	maxValue *= ramp;

	if (minValue >= maxValue)
	{
		SPEW((GROUP_STUFF_TEST, "Should never reach here"));
		minValue -= 10;
	}

	value = (int)((Random::GetFraction() * (maxValue - minValue))+minValue);

	
}

//#############################################################################
template<>
MinMaxHolderOf<float>::MinMaxHolderOf()
{

	minValue = -Random::GetFraction();
	maxValue = Random::GetFraction();

	value = (Random::GetFraction() * (maxValue - minValue))+minValue;

	


}
//#############################################################################


bool FloatIntBitStreamTest(int total_sections_to_write)
{


	// just to make this test interesting I will misalign the stream by one byte every number
	int total_bits = 0;

	bool *source_array_of_bools = new bool[total_sections_to_write];
	Register_Pointer(source_array_of_bools);

	bool *comp_array_of_bools = new bool[total_sections_to_write];
	Register_Pointer(comp_array_of_bools);

	int *convert_int_array = new int[total_sections_to_write];
	Register_Pointer(convert_int_array);

	float *convert_float_array = new float[total_sections_to_write];
	Register_Pointer(convert_float_array);

	int *comp_int_array = new int[total_sections_to_write];
	Register_Pointer(comp_int_array);

	float *comp_float_array = new float[total_sections_to_write];
	Register_Pointer(comp_float_array);

	int *float_bit_depth = new int[total_sections_to_write];
	Register_Pointer(float_bit_depth);

	int *int_bit_depth = new int[total_sections_to_write];
	Register_Pointer(int_bit_depth);

	MinMaxHolderOf<int> *int_min_max = new MinMaxHolderOf<int>[total_sections_to_write];
	Register_Pointer(int_min_max);

	MinMaxHolderOf<float> *float_min_max = new MinMaxHolderOf<float>[total_sections_to_write];
	Register_Pointer(float_min_max);



	for (int i = 0; i < total_sections_to_write; ++i)
	{
		int random = Random::GetLessThan(10);

		if (random > 4)
		{
		
			source_array_of_bools[i] = true;
		}
		else
		{
			
			source_array_of_bools[i] = false;
		}

		float_bit_depth[i] = Random::GetLessThan(32) + 1;
		int_bit_depth[i] = Random::GetLessThan(32) + 1;
		
		Test_Assumption(float_bit_depth[i] > 0);
		Test_Assumption(int_bit_depth[i] > 0);
		Test_Assumption(float_bit_depth[i] <= 32);
		Test_Assumption(int_bit_depth[i] <= 32);


		total_bits += float_bit_depth[i];
		total_bits += int_bit_depth[i];
		

	}

	// add in the bools
	total_bits += total_sections_to_write;



	// calculate total byte depth.
	int total_number_of_bytes = (int)(total_bits/8.0f);
	int total_remainder_bits = total_bits - (total_number_of_bytes*8);
	if ( total_remainder_bits != 0)
	{
		total_number_of_bytes += 1;
	}

	
	BYTE *big_byte_array = new BYTE[total_number_of_bytes];
	Register_Pointer(big_byte_array);
	
	MemoryStream bit_stream(big_byte_array, total_number_of_bytes);


	for (int i = 0; i < total_sections_to_write; ++i)
	{
		//write int
		bit_stream.WriteScaledIntToBits(int_min_max[i].value, int_min_max[i].minValue, int_min_max[i].maxValue, int_bit_depth[i]);

		//write float
		bit_stream.WriteScaledFloatToBits(float_min_max[i].value, float_min_max[i].minValue, float_min_max[i].maxValue, float_bit_depth[i]);

		//write bit
		bit_stream.WriteBit(source_array_of_bools[i]);
	}


	bit_stream.ByteAlign();
	bit_stream.Rewind();

	for (int i = 0; i < total_sections_to_write; ++i)
	{

		//read int
		bit_stream.ReadBitsToScaledInt(comp_int_array[i], int_min_max[i].minValue, int_min_max[i].maxValue, int_bit_depth[i]);

		//read float
		bit_stream.ReadBitsToScaledFloat(comp_float_array[i], float_min_max[i].minValue, float_min_max[i].maxValue, float_bit_depth[i]);

		//read bit
		bit_stream.ReadBit(comp_array_of_bools[i]);
	}

	SPEW((GROUP_STUFF_TEST, "-------------------"));

	// convert the floats and bits and see if they match..
	for (int i = 0; i < total_sections_to_write; ++i)
	{

		DWORD buffer;


		buffer = 0x00;
		buffer = Scaled_Int_To_Bits(int_min_max[i].value, int_min_max[i].minValue, int_min_max[i].maxValue, int_bit_depth[i]);
		convert_int_array[i] = Scaled_Int_From_Bits(buffer, int_min_max[i].minValue, int_min_max[i].maxValue, int_bit_depth[i]);;
		
		
		buffer = 0x00;
		buffer = Scaled_Float_To_Bits(float_min_max[i].value, float_min_max[i].minValue, float_min_max[i].maxValue, float_bit_depth[i]);
		convert_float_array[i] = Scaled_Float_From_Bits(buffer, float_min_max[i].minValue, float_min_max[i].maxValue, float_bit_depth[i]);

	}



	for (int i = 0; i < total_sections_to_write; ++i)
	{
		Verify(convert_int_array[i] == comp_int_array[i]);
		Verify(convert_float_array[i] == comp_float_array[i]);

		

		Test_Assumption(convert_int_array[i] == comp_int_array[i]);
		Test_Assumption(convert_float_array[i] == comp_float_array[i]);

		Test_Assumption(source_array_of_bools[i] == comp_array_of_bools[i]);
	}


	Unregister_Pointer(int_min_max);
	delete[] int_min_max;

	Unregister_Pointer(float_min_max);
	delete[] float_min_max;

	Unregister_Pointer(float_bit_depth);
	delete[] float_bit_depth;

	Unregister_Pointer(int_bit_depth);
	delete[] int_bit_depth;

	Unregister_Pointer(big_byte_array);
	delete[] big_byte_array;

	Unregister_Pointer(comp_int_array);
	delete[] comp_int_array;

	Unregister_Pointer(comp_float_array);
	delete[] comp_float_array;

	Unregister_Pointer(convert_int_array);
	delete[] convert_int_array;

	Unregister_Pointer(convert_float_array);
	delete[] convert_float_array;

	Unregister_Pointer(source_array_of_bools);
	delete[] source_array_of_bools;

	Unregister_Pointer(comp_array_of_bools);
	delete[] comp_array_of_bools;


	return true;
}
//
//#############################################################################

bool MultipleBitStreamTest(int total_sections_to_write)
{

	

	
	int *bit_depth = new int[total_sections_to_write];
	Register_Pointer(bit_depth);
	
	double *bits_to_write = new double[total_sections_to_write];
	Register_Pointer(bits_to_write);

	double *bits_to_read = new double[total_sections_to_write];
	Register_Pointer(bits_to_read);

	SPEW((GROUP_STUFF_TEST, "Testing %d random bit depth, random value numbers", total_sections_to_write));

	int total_bit_depth = 0;

	for (int i = 0; i < total_sections_to_write; ++i)
	{
		bit_depth[i] = Random::GetLessThan(63) + 1;

		total_bit_depth += bit_depth[i];

		Test_Assumption(bit_depth[i] < 65);
		Test_Assumption(bit_depth[i] > 0);


		//SPEW((GROUP_STUFF_TEST, "%d ---- Bit Depth : %d", i, bit_depth[i]));

		bits_to_write[i] = 0x00;
		bits_to_read[i] = 0x00;

		BYTE *byte_array = Cast_Pointer(BYTE *, &bits_to_write[i]);

		
		int number_of_bytes = (int)(bit_depth[i]/8.0f);
		int remainder_bits = bit_depth[i] - (number_of_bytes*8);

		if ( remainder_bits != 0)
		{
			number_of_bytes += 1;
		}


		for (int byte_count = 0; byte_count < number_of_bytes; ++byte_count)
		{
			byte_array[byte_count] = (BYTE)Random::GetLessThan(256);
			//byte_array[byte_count] = 0xff;
		}


		// mask off unused bits...
		if (remainder_bits != 0)
		{
			byte_array[number_of_bytes-1] = (BYTE)(byte_array[number_of_bytes-1] >> (8-remainder_bits));
		}
	}


	// calculate total byte depth.
	int total_number_of_bytes = (int)(total_bit_depth/8.0f);
	int total_remainder_bits = total_bit_depth - (total_number_of_bytes*8);
	if ( total_remainder_bits != 0)
	{
		total_number_of_bytes += 1;
	}


	BYTE *big_byte_array = new BYTE[total_number_of_bytes];
	Register_Pointer(big_byte_array);


	MemoryStream bit_stream(big_byte_array, total_number_of_bytes);

	int bit_depth_test = total_bit_depth;

	int total_bits_written = 0;
	for (int i = 0; i < total_sections_to_write; ++i)
	{
		Test_Assumption(bit_depth_test > 0);
		bit_stream.WriteBits(&bits_to_write[i], bit_depth[i]);
		bit_depth_test -= bit_depth[i];

		total_bits_written += bit_depth[i];
		int total_bytes_written = (int)(total_bits_written/8.0f);
		

		// one is in the pipe so it doesn't count
		int stream_bytes_written = bit_stream.GetBytesUsed();

		Test_Assumption(total_bytes_written == stream_bytes_written);

	}

	Test_Assumption(bit_depth_test == 0);

	bit_stream.ByteAlign();
	bit_stream.Rewind();

	int total_bits_read = 0;
	for (int i = 0; i < total_sections_to_write; ++i)
	{
		bit_stream.ReadBits(&bits_to_read[i], bit_depth[i]);
		total_bits_read += bit_depth[i];

		int total_bytes_read = (int)(total_bits_read/8.0f);
		//int total_remainder_bits_read = total_bits_read - (total_bytes_read*8);


		// one is in the pipe so it doesn't count
		int stream_bytes_read = bit_stream.GetBytesUsed() - 1;

		Test_Assumption(total_bytes_read == stream_bytes_read);

	}

	for (int i = 0; i < total_sections_to_write; ++i)
	{
		BYTE *source_byte_array = Cast_Pointer(BYTE *, &bits_to_write[i]);
		BYTE *copy_byte_array = Cast_Pointer(BYTE *, &bits_to_read[i]);

		SPEW((GROUP_STUFF_TEST, "%d\t---- Bit Depth : %d", i, bit_depth[i]));

		//SPEW((GROUP_STUFF_TEST, "%d ---- Src Bit Value : +", i));

		MString text = "\t---- Src Bit Value : "; 

		for (int byte_count = 0; byte_count < 8; ++byte_count)
		{
			//Test_Assumption(source_byte_array == copy_byte_array);

			

			for (int bit_count = 7; bit_count > -1; --bit_count)
			{
				BYTE bit_value = (BYTE)(source_byte_array[byte_count] >> bit_count);

				bit_value &= 0x01;

				//SPEW((GROUP_STUFF_TEST, "%d+", bit_value));

				if (bit_value)
				{
					MString value = "1";
					text += value;
				}
				else
				{
					MString value = "0";
					text += value;
				}

				if (bit_count == 4)
				{
					//SPEW((GROUP_STUFF_TEST, "|+"));
					MString value = "|";
					text += value;
				}
			}

			
			MString value = " ";
			text += value;
			

		}

		SPEW((GROUP_STUFF_TEST, "%s", (const char *)text));

		//SPEW((GROUP_STUFF_TEST, "%d ---- Dst Bit Value : +", i));

		text = "\t---- Dst Bit Value : "; 

		for (int byte_count = 0; byte_count < 8; ++byte_count)
		{
			for (int bit_count = 7; bit_count > -1; --bit_count)
			{
				BYTE bit_value = (BYTE)(copy_byte_array[byte_count] >> bit_count);

				bit_value &= 0x01;

				if (bit_value)
				{
					MString value = "1";
					text += value;
				}
				else
				{
					MString value = "0";
					text += value;
				}

				if (bit_count == 4)
				{
					//SPEW((GROUP_STUFF_TEST, "|+"));
					MString value = "|";
					text += value;
				}
			}

			
			MString value = " ";
			text += value;

		}


		SPEW((GROUP_STUFF_TEST, "%s", (const char *)text));

		SPEW((GROUP_STUFF_TEST, "\t---- Src Hex Value : +", i));
		for (int byte_count = 0; byte_count < 8; ++byte_count)
		{
		
			SPEW((GROUP_STUFF_TEST, "%02x +", source_byte_array[byte_count]));

		}

		SPEW((GROUP_STUFF_TEST, ""));

		SPEW((GROUP_STUFF_TEST, "\t---- Dst Hex Value : +", i));
		for (int byte_count = 0; byte_count < 8; ++byte_count)
		{
		
			SPEW((GROUP_STUFF_TEST, "%02x +", copy_byte_array[byte_count]));

		}

		for (int byte_count = 0; byte_count < 8; ++byte_count)
		{
			Test_Assumption(source_byte_array[byte_count] == copy_byte_array[byte_count]);
		}


		SPEW((GROUP_STUFF_TEST, ""));


	}

	
	Unregister_Pointer(big_byte_array);
	delete[] big_byte_array;

	Unregister_Pointer(bits_to_write);
	delete[] bits_to_write;

	Unregister_Pointer(bits_to_read);
	delete[] bits_to_read;
		
	Unregister_Pointer(bit_depth);
	delete[] bit_depth;

	return true;
}
