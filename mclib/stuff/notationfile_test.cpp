//===========================================================================//
// File:     notation.tcp                                                    //
// Title:    Definition of NotationFile TestClass methods.                   //
// Purpose:  Provide general purpose access to data stored in a formatted    //
//           text file.                                                      //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"
#include<toolos.hpp>

//#############################################################################
//##############    NotationFile::TestClass    ################################
//#############################################################################

bool
	NotationFile::TestClass()
{
	SPEW((GROUP_STUFF_TEST, "Starting NotationFile test..."));

	Check_Object(FileStreamManager::Instance);

	const char
		*notation_snapshot = "notation.snapshot",
		*page_1_name = "Page 1",
		*page_2_name = "Page 2",
		*page_3_name = "Page 3",
		*page_5_name = "Page 5",
		*page_8_name = "Page 8",
		*page_9_name = "Page9",
		*sub_page_name = "SubPage1",

		*note_1_name = "Entry 1",
		*note_2_name = "Entry 2",
		*note_3_name = "Entry 3",
		*note_4_name = "Entry 4",
		*note_5_name = "Entry 5",
		*note_6_name = "Entry 6",
		*note_9_name = "Entry 9",

		*text_a = "Text Line A",
		*text_b = "Text Line B",
		*text_c = "Text Line C",
		*text_i = "-35",
		*text_s = "3.1415",
		*text_d = "1.99d+199",
		*text_t = "yes",
		*text_f = "no",
		*string_value;

	Page
		*page,
		*sub_page=NULL,
		*page_1=NULL,
		*page_2=NULL,
		*page_3=NULL,
		*page_5=NULL,
		*page_8=NULL,
		*page_9=NULL;

	PageIterator *pages;

	//
	//----------------------------------------
	// test NotationFile part 1 - create file
	//----------------------------------------
	//
	{
		NotationFile file;
		Test_Assumption(file.IsEmpty());
		Test_Assumption(!file.IsChanged());

		pages = file.MakePageIterator();
		Test_Assumption(pages);
		Test_Assumption(pages->GetSize() == 0);
		Check_Object(pages);
		delete pages;

		file.SaveAs(notation_snapshot);

		page_2 = file.AddPage(page_2_name);
		Test_Assumption(page_2);
		page_2->SetEntry(note_4_name, text_a);

		Test_Assumption(!file.FindPage(page_1_name));
		page = file.FindPage(page_2_name);
		Test_Assumption(page);
		Test_Assumption(!page->GetEntry(note_2_name, &string_value));
		Test_Assumption(page->GetEntry(note_4_name, &string_value));
		Test_Assumption(!strcmp(string_value, text_a));

		page_2->SetEntry(note_2_name, text_s);
		page_1 = file.AddPage(page_1_name);
		page_1->SetEntry(note_5_name, text_i);
		page_9 = file.AddPage(page_9_name);
		page_9->SetEntry(note_6_name, text_b);
		page_8 = file.AddPage(page_8_name);
		page_2->SetEntry(note_3_name, text_d);
		page_1->SetEntry(note_4_name, text_b);
		page_9->SetEntry(note_5_name, true);
		page_8->SetEntry(note_3_name, text_c);
		page_5 = file.AddPage(page_5_name);
		page_5->SetEntry(note_5_name, text_a);
		page_9->SetEntry(note_2_name, false);

		NotationFile subfile;
		sub_page = subfile.AddPage(sub_page_name);
		Check_Object(sub_page);
		sub_page->SetEntry(note_1_name, text_a);
		page_1->SetEntry(note_2_name, &subfile);

		page_8->DeleteNote(note_1_name);
		page_1->DeleteNote(note_5_name);
		page_9->SetEntry(note_1_name, text_t);
		page_2->SetEntry(note_3_name, text_c);
		page_1->SetEntry(note_1_name, text_i);
		page_5->AppendEntry(note_5_name, text_b);
		page_9->SetEntry(note_3_name, text_f);
		page_8->SetEntry(note_1_name, text_d);
		page_1->SetEntry(note_9_name, text_s);
		page_9->AppendEntry(note_9_name, true);
		page_8->SetEntry(note_2_name, text_i);
		page_9->SetEntry(note_4_name, false);

		file.SaveAs(notation_snapshot);

		// notation file should look like this:
		//			 (page_2_name, note_4_name, text_a)
		//			 (page_2_name, note_2_name, text_s)
		//			 (page_2_name, note_3_name, text_c)
		//			 (page_1_name, note_4_name, text_b)
		//			 (page_1_name, note_2_name, <<subfile>>
		//			 (page_1_name, note_1_name, text_i)
		//			 (page_1_name, note_9_name, text_s)
		//			 (page_9_name, note_6_name, text_b)
		//			 (page_9_name, note_5_name, 1)
		//			 (page_9_name, note_2_name, false)
		//			 (page_9_name, note_1_name, text_t)
		//			 (page_9_name, note_3_name, text_f)
		//			 (page_9_name, note_9_name, true)
		//			 (page_9_name, note_4_name, 0)
		//			 (page_8_name, note_3_name, text_c)
		//			 (page_8_name, note_1_name, text_d)
		//			 (page_8_name, note_2_name, text_i)
		//			 (page_5_name, note_5_name, text_a)
		//			 (page_5_name, note_5_name, text_b)

		Test_Assumption(file.DoesPageExist("pAgE9"));
		pages = file.MakePageIterator();
		Test_Assumption(pages);
		Test_Assumption(pages->GetSize() == 5);
		page = pages->ReadAndNext();
		Test_Assumption(page_2 == page);
		page = pages->ReadAndNext();
		Test_Assumption(page_1 == page);
		page = pages->ReadAndNext();;
		Test_Assumption(page_9 == page);
		page = pages->ReadAndNext();
		Test_Assumption(page_8 == page);
		page = pages->ReadAndNext();
		Test_Assumption(page_5 == page);
		Test_Assumption(!strcmp(page->GetName(), page_5_name));
		Check_Object(pages);
		delete pages;
		Test_Assumption(!file.IsChanged());

		pages = file.MakePageIterator();
		Test_Assumption(pages);
		Test_Assumption(pages->GetSize() == 5);
		Check_Object(pages);
		delete pages;
//	}

	//
	//--------------------------------------
	// Now we open up a second notation file
	//--------------------------------------
	//
//	{
		NotationFile file2(notation_snapshot);
		Test_Assumption(!file2.IsChanged());

		page_5 = file2.FindPage(page_5_name);
		Test_Assumption(page_5);
		Page::NoteIterator *notes = page_5->MakeNoteIterator();
		Test_Assumption(notes);
		Test_Assumption(notes->GetSize() == 2);
		Note *note = notes->ReadAndNext();
		Test_Assumption(!strcmp(note->GetName(), note_5_name));
		note = notes->ReadAndNext();
		Test_Assumption(!strcmp(note->GetName(), note_5_name));
		Check_Object(notes);
		delete notes;

		//
		//-------------------------
		// Test the integer readers
		//-------------------------
		//
		int integer_value;
		Scalar scalar_value;
		bool logical_value;
		page_3 = file2.FindPage(page_3_name);
		Test_Assumption(!page_3);
		page_2 = file2.FindPage(page_2_name);
		Test_Assumption(page_2);
		Test_Assumption(!page_2->GetEntry(note_5_name, &scalar_value));
		page_1 = file2.FindPage(page_1_name);
		Test_Assumption(page_1);
		Test_Assumption(page_1->GetEntry(note_1_name, &integer_value));
		Test_Assumption(integer_value == atoi(text_i));
		Test_Assumption(page_2->GetEntry(note_2_name, &scalar_value));
		Test_Assumption(Close_Enough(scalar_value, (Scalar)AtoF(text_s)));
		Test_Assumption(page_1->GetEntry(note_4_name, &string_value));
		Test_Assumption(!strcmp(string_value, text_b));
		Test_Assumption(page_5->GetEntry(note_5_name, &string_value));
		Test_Assumption(!strcmp(string_value, text_a));
		page_9 = file2.FindPage(page_9_name);
		Test_Assumption(page_9);
		Test_Assumption(page_9->GetEntry(note_9_name, &logical_value));
		Test_Assumption(logical_value);
		Test_Assumption(page_9->GetEntry(note_1_name, &logical_value));
		Test_Assumption(logical_value);
		Test_Assumption(page_9->GetEntry(note_5_name, &logical_value));
		Test_Assumption(logical_value);
		Test_Assumption(page_9->GetEntry(note_3_name, &logical_value));
		Test_Assumption(!logical_value);
		Test_Assumption(page_9->GetEntry(note_2_name, &logical_value));
		Test_Assumption(!logical_value);
		Test_Assumption(page_9->GetEntry(note_4_name, &logical_value));
		Test_Assumption(!logical_value);
		Test_Assumption(!file2.IsChanged());

		NotationFile subfile2;
		Test_Assumption(page_1->GetEntry(note_2_name, &subfile2));
		sub_page = subfile2.FindPage(sub_page_name);
		Test_Assumption(sub_page);
		Test_Assumption(sub_page->GetEntry(note_1_name, &string_value));
		Test_Assumption(!strcmp(string_value, text_a));

		file2.DeletePage(page_2_name);
		Test_Assumption(file2.IsChanged());
		Test_Assumption(!file2.DoesPageExist(page_2_name));

		page_1->DeleteNote(note_1_name);
		notes = page_1->MakeNoteIterator();
		Test_Assumption(notes);
		Test_Assumption(notes->GetSize() == 3);
		note = notes->GetCurrent();
		Test_Assumption(note);
		Test_Assumption(!strcmp(note->GetName(), note_4_name));
		Check_Object(notes);
		delete note;
		file2.IgnoreChanges();
	}

	//
	// TESTING PRE-PROCESSOR STYLE COMMANDS
	//
	const char* include_temp;
	const char* base_name = "base.ini";
	const char* inc_name = "inc.ini";
	{
		NotationFile basefile;
		page = basefile.AddPage("Page 1");
		Test_Assumption(page);
		page->SetEntry("Pro_wrestling", "GoldFishShoes");
		page->SetEntry("!Erica", "HotBabe");
		page->AddNote("!include inc.ini");
		page->SetEntry("OurHeroes", "$(GOTHAM_HEROES)");
		page->SetEntry("Notes", notation_snapshot);
		basefile.SaveAs(base_name);
	}

	{
		NotationFile incfile;
		page = incfile.AddPage("Page 22");
		Test_Assumption(page);
		page->SetEntry("!Batman", "BruceWayne");
		page->SetEntry("!Robin", "DickGrayson");
		page->SetEntry("$(Batman)", "not $(Erica)");
		page->SetEntry("!GOTHAM_HEROES", "$(Batman) + $(Robin)");
		incfile.SaveAs(inc_name);
	}

	{
		NotationFile rawfile(base_name, Raw);
		page = rawfile.FindPage("Page 22");
		Test_Assumption(!page);
		page = rawfile.FindPage("Page 1");
		Test_Assumption(page);
		Test_Assumption(page->GetEntry("OurHeroes", &include_temp));
		Test_Assumption(!strcmp(include_temp, "$(GOTHAM_HEROES)"));
	}

	{
		NotationFile processed(base_name);
		processed.SaveAs(notation_snapshot);
		page = processed.FindPage("Page 22");
		Test_Assumption(page);
		Test_Assumption(page->GetEntry("BruceWayne", &include_temp));
		Test_Assumption(!strcmp(include_temp, "not HotBabe"));
		Test_Assumption(page->GetEntry("OurHeroes", &include_temp));
		Test_Assumption(!strcmp(include_temp, "BruceWayne + DickGrayson"));
	}

	gos_DeleteFile(base_name);
	gos_DeleteFile(inc_name);
	gos_DeleteFile(notation_snapshot);

	return true;
}

//#############################################################################
//#############################################################################
