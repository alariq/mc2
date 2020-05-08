//===========================================================================//
// File:	MArray.hpp                                                       //
// Contents: 																 //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"stuff.hpp"

namespace Stuff {

	template <class T, size_t N> class StaticArrayOf;

	template <class T, size_t N> MemoryStream&
		MemoryStreamIO_Write(
			MemoryStream *stream,
			const StaticArrayOf<T, N> *array
       );

	template <class T, size_t N> MemoryStream&
		MemoryStreamIO_Read(
			MemoryStream *stream,
			StaticArrayOf<T, N> *array
		);

	//##########################################################################
	//##########################    StaticArrayOf    ###########################
	//##########################################################################

	template <class T, size_t N> class StaticArrayOf
		#if defined(_ARMOR)
			: public Stuff::Signature
		#endif
	{
	public:
		StaticArrayOf();
		StaticArrayOf(const StaticArrayOf<T, N>&);
		StaticArrayOf(const T &value);
		StaticArrayOf(T *data, size_t length);
		~StaticArrayOf();

		void
			TestInstance() const
				{}

		StaticArrayOf<T, N>&
			operator=(const StaticArrayOf<T, N>&);
		T&
			operator[](size_t i);
		const T&
			operator[](size_t i) const;

		void
			AssignValue(const T &value);
		void
			AssignData(const T *data, size_t length);

		size_t 
			GetLength() const
				{return N;}
		size_t 
			GetSize() const
				{return N * sizeof(T);}
		T*
			GetData()
				{return data;}
		const T*
			GetData() const
				{return data;}

#if 0
		bool
			Compare(const StaticArrayOf<T, N>&);
#endif

		friend MemoryStream&
			Stuff::MemoryStreamIO_Write<T, N>(
				Stuff::MemoryStream *stream,
				const StaticArrayOf<T, N> *array
			);

		friend MemoryStream&
			MemoryStreamIO_Read<T, N>(
				Stuff::MemoryStream *stream,
				StaticArrayOf<T, N> *array
			);

	private:
		T 
			data[N];
	};

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	template<class T, size_t N> inline
		StaticArrayOf<T, N>::StaticArrayOf() 
	{ 
	}

	template<class T, size_t N> inline
		StaticArrayOf<T, N>::StaticArrayOf(const StaticArrayOf<T, N> &array) 
	{
		AssignData(array.data, N);
	}

	template<class T, size_t N> inline
		StaticArrayOf<T, N>::StaticArrayOf(const T &value) 
	{
		AssignValue(value);
	}

	template<class T, size_t N> inline
		StaticArrayOf<T, N>::StaticArrayOf(T *data_to_copy, size_t length) 
	{
		AssignData(data_to_copy, length);
	}

	template<class T, size_t N> inline
		StaticArrayOf<T, N>::~StaticArrayOf() 
	{ 
	}

	template <class T, size_t N> StaticArrayOf<T, N>& 
		StaticArrayOf<T, N>::operator=(const StaticArrayOf<T, N> &array)
	{ 
		if (this != &array)
		{
			AssignData(&array.data[0], N);
		} 
		return *this; 
	}

	template <class T, size_t N> inline T&
		StaticArrayOf<T, N>::operator[](size_t i) 
	{
		Verify(i < N);  
		return data[i]; 
	}

	template <class T, size_t N> inline const T&
		StaticArrayOf<T, N>::operator[](size_t i) const
	{
		Verify(i < N);  
		return data[i]; 
	}

	template <class T, size_t N> void
		StaticArrayOf<T, N>::AssignValue(const T &value) 
	{
		// Do not memcopy, object semantics may be required
		for (int i = 0; i < N; i++)
			data[i] = value;
	}

	template <class T, size_t N> void
		StaticArrayOf<T, N>::AssignData(const T *data_to_copy, size_t length) 
	{
		Check_Pointer(data_to_copy);
		Verify(length <= N);
		// Do not memcopy, object semantics may be required
		for (int i = 0; i < length; i++)
			data[i] = data_to_copy[i];
	}

#if 0
	template <class T, size_t N> bool
		StaticArrayOf<T, N>::Compare(const StaticArrayOf<T, N> &array) 
	{
		if (N != array.GetLength())
			return false;
		for (int i = 0; i < N; i++)
		{
			if (data[i] != array.data[i])
				return false;
		}
		return true;
	}
#endif

	template <class T, size_t N> MemoryStream&
		MemoryStreamIO_Write(
			MemoryStream *stream,
			const StaticArrayOf<T, N> *array
		)
	{
		Check_Object(stream);
		Check_Pointer(array);
		return stream->WriteBytes(&array->data[0], N * sizeof(T));
	}

	template <class T, size_t N> MemoryStream&
		MemoryStreamIO_Read(
			MemoryStream *stream,
			StaticArrayOf<T, N> *array
		)
	{
		Check_Object(stream);
		Check_Pointer(array);
		return stream->ReadBytes(&array->data[0], N * sizeof(T));
	}

	template <class T> class DynamicArrayOf;

	template <class T> MemoryStream&
		MemoryStreamIO_Write(
			MemoryStream *stream,
			const DynamicArrayOf<T> *array
		);

	template <class T> MemoryStream&
		MemoryStreamIO_Read(
			MemoryStream *stream,
			DynamicArrayOf<T> *array
		);
	
	//##########################################################################
	//#########################    DynamicArrayOf    ###########################
	//##########################################################################

	template <class T> class DynamicArrayOf
		#if defined(_ARMOR)
			: public Stuff::Signature
		#endif
	{
	public:
		DynamicArrayOf();
		DynamicArrayOf(const DynamicArrayOf<T>&);
		DynamicArrayOf(size_t length);
		DynamicArrayOf(const T &value, size_t length);
		DynamicArrayOf(const T *data, size_t length);
		~DynamicArrayOf();

		void
			TestInstance() const
				{}

		void
			TestInstance()
				{}

		DynamicArrayOf<T>&
			operator=(const DynamicArrayOf<T>&);
		T&
			operator[](size_t i);
		const T&
			operator[](size_t i) const;

		void
			AssignValue(const T &value, size_t length);
		void
			AssignData(const T *data, size_t length);

		size_t 
			GetLength() const
				{return length;}
		void
			SetLength(size_t length);
		size_t 
			GetSize() const
				{return length * sizeof(T);}
		T*
			GetData()
				{return data;}
		const T*
			GetData() const
				{return data;}

#if 0
		bool
			Compare(const DynamicArrayOf<T>&);
#endif

		friend MemoryStream&
			MemoryStreamIO_Write<T>(
				MemoryStream *stream,
				const DynamicArrayOf<T> *array
			);

		friend MemoryStream&
			MemoryStreamIO_Read<T>(
				MemoryStream *stream,
				DynamicArrayOf<T> *array
			);

	private:
		void
			SetStorageLength(size_t length);
		void
			CopyArray(const DynamicArrayOf<T>&);

		T 
			*data;
		size_t
			length;
	};

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	template<class T>
		DynamicArrayOf<T>::DynamicArrayOf() 
	{ 
		data = NULL;
		length = 0;
	}

	template<class T>
		DynamicArrayOf<T>::DynamicArrayOf(const DynamicArrayOf<T> &array) 
	{
		data = NULL;
		length = 0;
		CopyArray(array);
	}

	template<class T>
		DynamicArrayOf<T>::DynamicArrayOf(size_t length_to_set) 
	{
		data = NULL;
		length = 0;
		SetStorageLength(length_to_set);
		Verify(length == length_to_set);
		#if defined(_ARMOR)
			if (length_to_set > 0)
				Check_Pointer(data);
			else
				Verify(!data);
		#endif
	}

	template<class T>
		DynamicArrayOf<T>::DynamicArrayOf(const T &value, size_t length_to_set) 
	{
		data = NULL;
		length = 0;
		AssignValue(value, length_to_set);
	}

	template<class T>
		DynamicArrayOf<T>::DynamicArrayOf(const T *data_to_copy, size_t length_to_set) 
	{
		data = NULL;
		length = 0;
		AssignData(data_to_copy, length_to_set);
	}

	template<class T>
		DynamicArrayOf<T>::~DynamicArrayOf() 
	{ 
		if (data != NULL)
		{
			Unregister_Pointer(data);
			delete[] data;
		}
	}

	template <class T> DynamicArrayOf<T>& 
		DynamicArrayOf<T>::operator=(const DynamicArrayOf<T> &array)
	{ 
		if (this != &array)
			CopyArray(array);
		return *this; 
	}

	template <class T> inline T&
		DynamicArrayOf<T>::operator[](size_t i) 
	{
		Check_Pointer(data);
		Verify(i < length);  
		return data[i]; 
	}

	template <class T> inline const T&
		DynamicArrayOf<T>::operator[](size_t i) const
	{
		Verify(i < length);
		return data[i]; 
	}

	template <class T> void
		DynamicArrayOf<T>::AssignValue(const T &value, size_t length_to_set) 
	{
		SetStorageLength(length_to_set);
		// Do not memcopy, object semantics may be required
		for (int i = 0; i < length; i++)
			data[i] = value;
	}

	template <class T> void
		DynamicArrayOf<T>::AssignData(const T *data_to_copy, size_t length_to_set) 
	{
		Check_Pointer(data_to_copy);
		SetStorageLength(length_to_set);
		// Do not memcopy, object semantics may be required
		for (int i = 0; i < length; i++)
			data[i] = data_to_copy[i];
	}

	template <class T> void
		DynamicArrayOf<T>::SetLength(size_t length_to_set)
	{
		if (length_to_set != length)
		{
			if (length_to_set > 0)
			{
				T* new_data = new T[length_to_set];
				Register_Pointer(new_data);
				if (data != NULL)
				{
					// Do not memcopy, object semantics may be required
					size_t i = Min(length_to_set, length);
					while (i--)
						new_data[i] = data[i];
					Unregister_Pointer(data);
					delete[] data;
				}
				data = new_data;
			}
			else
			{
				Unregister_Pointer(data);
				delete[] data;
				data = NULL;
			}
			length = length_to_set;
		}
	}

#if 0
	template <class T> void
		DynamicArrayOf<T>::Compare(const DynamicArrayOf<T> &array) 
	{
		if (length != array.length)
			return false;
		for (int i = 0; i < length; i++)
		{
			if (data[i] != array.data[i])
				return false;
		}
		return true;
	}
#endif

	template <class T> void
		DynamicArrayOf<T>::SetStorageLength(size_t length_to_set)
	{
		if (length_to_set != length)
		{
			if (length_to_set > 0)
			{
				if (data != NULL)
				{
					Unregister_Pointer(data);
					delete[] data;
				}
				length = length_to_set;
				data = new T[length];
				Register_Pointer(data);
			}
			else
			{
				Verify(length_to_set == 0);
				Unregister_Pointer(data);
				delete[] data;
				data = NULL;
			}
		}
	}

	template <class T> void
		DynamicArrayOf<T>::CopyArray(const DynamicArrayOf<T> &array)
	{ 
		if (array.length > 0)
		{
			Check_Pointer(array.data);
			AssignData(&array.data[0], array.length);
		}
		else
		{
			Verify(array.length == 0);
			Verify(array.data == NULL);
			if (data != NULL)
			{
				Unregister_Pointer(data);
				delete[] data;
				length = 0;
			}
		}
	}

	template <class T> MemoryStream&
		MemoryStreamIO_Write(
			MemoryStream *stream,
			const DynamicArrayOf<T> *array
		)
	{
		Check_Object(stream);
		Check_Pointer(array);
		uint32_t length = (uint32_t)array->length;
		MemoryStreamIO::Write(stream, &length);
		if (length > 0)
			stream->WriteBytes(&array->data[0], length * sizeof(T));
		return *stream;
	}

	template <class T> MemoryStream&
		MemoryStreamIO_Read(
			MemoryStream *stream,
			DynamicArrayOf<T> *array
		)
	{
		Check_Object(stream);
		Check_Pointer(array);
		uint32_t length;
		MemoryStreamIO::Read(stream, &length);
		array->SetLength(length);
		Verify(length == (uint32_t)array->length);
		if (length > 0)
			stream->ReadBytes(&array->data[0], length * sizeof(T));
		return *stream;
	}

}
