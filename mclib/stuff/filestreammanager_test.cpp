//===========================================================================//
// File:	filestrmmgr_test.cpp                                             //
// Contents: Implementation Details of resource management                   //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include<munga.hpp>
#pragma hdrstop

#if !defined(FILESTREAMMANAGER_HPP)
#include<filestreammanager.hpp>
#endif

#if !defined(APPLICATION_HPP)
#include<application.hpp>
#endif

//#############################################################################
//######################    TestFileManagerReceiver    ########################
//#############################################################################

#define FILE_BUFFERSIZE (1024*1024)
#define RandInt(x) (rand() % x)

class TestFileManagerReceiver:
	public Receiver
{
public:	
	TestFileManagerReceiver();
	~TestFileManagerReceiver();

	void
		Request(FileStream *file_stream);
	Logical
		IsDone()
			{return data == NULL;}
	
	static void
		InitializeClass();
	static void
		TerminateClass();

	static ClassID
		testFileManagerReceiverClassID;

	static const MessageEntry
		MessageEntries[];

	void
		FileReadFinishedMessageHandler(FileReadFinishedMessage *message);

	static ClassData
		*DefaultData;
		
	FileStream
		*fileStream;	
	size_t
		offset,
		size;
	void
		*data;		
	int
		tag;
};

const Receiver::MessageEntry
	TestFileManagerReceiver::MessageEntries[]=
{
	MESSAGE_ENTRY(TestFileManagerReceiver, FileReadFinished)
};

TestFileManagerReceiver::ClassData*
	TestFileManagerReceiver::DefaultData = NULL;

RegisteredClass::ClassID
	TestFileManagerReceiver::testFileManagerReceiverClassID;

void
	TestFileManagerReceiver::InitializeClass()
{
	testFileManagerReceiverClassID = RegisteredClass::AllocateTemporaryClassID();

	Verify(!DefaultData);
	DefaultData =
		new ClassData(
			testFileManagerReceiverClassID,
			"TestFileManagerReceiver",
			Receiver::DefaultData,
			ELEMENTS(MessageEntries),
			MessageEntries
		);
	Register_Object(DefaultData);
}

void
	TestFileManagerReceiver::TerminateClass()
{
	Unregister_Object(DefaultData);
	delete DefaultData;
	DefaultData = NULL;
}

TestFileManagerReceiver::TestFileManagerReceiver():
	Receiver(DefaultData)
{		
	data = NULL;
}

TestFileManagerReceiver::~TestFileManagerReceiver()
{
}

void
	TestFileManagerReceiver::Request(FileStream *file_stream)
{
	Check_Object(file_stream);
	fileStream = file_stream;
	tag = RandInt(256);
	
	//
	// Choose offset and size
	//
	offset = RandInt(FILE_BUFFERSIZE-1);
	size = RandInt(FILE_BUFFERSIZE-1) + 1;
	size = Min(size, (FILE_BUFFERSIZE - offset));	

	//
	// Allocate memory for read
	//
	data = new char[size];
	Register_Pointer(data);
	
	//
	// Request read
	//
	Check_Object(Munga_Application);
	Check_Object(Munga_Application->GetFileStreamManager());
	Munga_Application->GetFileStreamManager()->ReadRequest(
		data,
		file_stream,
		offset,
		size,
		this,
		&tag
	);	
}

void
	TestFileManagerReceiver::FileReadFinishedMessageHandler(
		FileReadFinishedMessage *message
	)
{
	Check_Object(this);
	Check_Object(message);
	Verify(message->memoryAddress == data);
	Verify(message->clientField == &tag);
	
	//
	// Load the segment directly from file
	//
	char *confirm_data = new char[size];
	Register_Pointer(confirm_data);
	Check_Object(fileStream);
	fileStream->SetPointer(offset);
	fileStream->ReadBytes(confirm_data, size);
	
	//
	// Verify that the data is the same as that received
	//
	Verify(memcmp(data, confirm_data, size) == 0);

	Unregister_Pointer(confirm_data);
	delete[] confirm_data;
	Unregister_Pointer(data);
	delete[] data;
	data = NULL;
}	

//
//#############################################################################
//#############################################################################
//
void
	FileStreamManager::TestClass()
{	
	DEBUG_STREAM << "Starting FileStreamManager Test...\n";

	TestFileManagerReceiver::InitializeClass();

	//
	// Create a file for testing.  Fill with a sequence numbers.
	//
	unsigned char *buffer;
	size_t i;
	
	buffer = new unsigned char[FILE_BUFFERSIZE];
	Register_Pointer(buffer);
	for (i = 0; i < FILE_BUFFERSIZE; i++)
	{
		buffer[i] = (unsigned char)(i % 255);
	}	

	Check_Object(Munga_Application);
	FileStreamManager *file_mgr = Munga_Application->GetFileStreamManager();
	Check_Object(file_mgr);
	FileStream *output_file_stream = file_mgr->MakeFileStream();
	Register_Object(output_file_stream);

	output_file_stream->Open("filestrmtest.dat", FileStream::WriteOnly);
	output_file_stream->WriteBytes(buffer, FILE_BUFFERSIZE);

	output_file_stream->Close();
	Unregister_Object(output_file_stream);
	delete output_file_stream;
	Unregister_Pointer(buffer);
	delete[] buffer;

	FileStream *file_stream = file_mgr->MakeFileStream();
	Register_Object(file_stream);
	file_stream->Open("filestrmtest.dat", FileStream::ReadOnly);

	//
	// Perform test
	//	
	const int iterations = 1000;	
	TestFileManagerReceiver *receiver;
	ChainOf<TestFileManagerReceiver*> socket(NULL);
	
	for (i = 0; i < iterations; i++)
	{
		//
		// Request reads
		//
		receiver = new TestFileManagerReceiver;
		Register_Object(receiver);
		socket.Add(receiver);
		receiver->Request(file_stream);

		//
		// Process events and the file stream manager
		//
		Check_Object(Munga_Application);
		Munga_Application->ProcessOneEvent();
		Check_Object(Munga_Application->GetFileStreamManager());
		Munga_Application->GetFileStreamManager()->Execute();
	}
	
	ChainIteratorOf<TestFileManagerReceiver*> iterator(&socket);
	
	while ((receiver = iterator.GetCurrent()) != NULL)
	{
		//
		// Delete receivers as they finish
		//
		Check_Object(receiver);
		if (receiver->IsDone())
		{
			Unregister_Object(receiver);
			delete receiver;
		}
		iterator.First();
		
		//
		// Process events and the file stream manager
		//
		Check_Object(Munga_Application);
		Munga_Application->ProcessOneEvent();
		Check_Object(Munga_Application->GetFileStreamManager());
		Munga_Application->GetFileStreamManager()->Execute();
	}

	file_stream->Close();	
	Unregister_Object(file_stream);
	delete file_stream;

	TestFileManagerReceiver::TerminateClass();	
}	

