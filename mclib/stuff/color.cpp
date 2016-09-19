//===========================================================================//
// File:     color.cpp                                                       //
// Title:    Definition of Color classes.                                    //
// Contents: Implementation details for color classes                        //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"

//~~~~~~~~~~~~~~~~~~~~~~~~~~ RGBColor functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
const RGBColor
	RGBColor::Unassigned(-1.0f, -1.0f, -1.0f);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// (friend)
bool
	Stuff::Close_Enough(
		const RGBColor &c1,
		const RGBColor &c2,
		Scalar e	// = SMALL
	)
{
	Check_Object(&c1);
	Check_Object(&c2);
	return
		Close_Enough(c1.red, c2.red, e)
		 && Close_Enough(c1.green, c2.green, e)
		 && Close_Enough(c1.blue, c2.blue, e);
}

//
//###########################################################################
//###########################################################################
//
RGBColor&
	RGBColor::operator=(const HSVColor &color)
{
	Check_Object(this);
	Check_Object(&color);
	Verify(color.saturation >= 0.0 && color.saturation <= 1.0f);

	//
	//----------------
	// Check for black
	//----------------
	//
	if (color.saturation <= SMALL)
	{
		red = green = blue = 0.0f;
		return *this;
	}

	//
	//-----------------------------
	// find the sextant for the hue
	//-----------------------------
	//
	Verify(color.hue >= 0.0 && color.hue <= 1.0f);
	Scalar hue = (color.hue == 1.0f) ? 0.0f : color.hue;
	hue *= 6.0f;
	int sextant = Truncate_Float_To_Byte(hue);
	Verify(static_cast<unsigned>(sextant) < 6);
	Scalar remainder = hue - static_cast<Scalar>(sextant);

	//
	//--------------------
	// Build the RGB color
	//--------------------
	//
	Verify(color.value >= 0.0f && color.value <= 1.0f);
	Scalar a = color.value * (1.0f - color.saturation);
	Verify(a >= 0.0f && a < 1.0f);
	switch (sextant)
	{
	case 0:
		red = color.value;
		green = color.value * (1.0f - color.saturation * (1.0f - remainder));
		Verify(green >= 0.0f && green <= 1.0f);
		blue = a;
		break;

	case 1:
		red = color.value * (1.0f - color.saturation * remainder);
		Verify(red >= 0.0f && red <= 1.0f);
		green = color.value;
		blue = a;
		break;

	case 2:
		red = a;
		green = color.value;
		blue = color.value * (1.0f - color.saturation * (1.0f - remainder));
		Verify(blue >= 0.0f && blue <= 1.0f);
		break;

	case 3:
		red = a;
		green = color.value * (1.0f - color.saturation * remainder);
		Verify(green >= 0.0f && green <= 1.0f);
		blue = color.value;
		break;

	case 4:
		red = color.value * (1.0f - color.saturation * (1.0f - remainder));
		Verify(red >= 0.0f && red <= 1.0f);
		green = a;
		blue = color.value;
		break;

	case 5:
		red = color.value;
		green = a;
		blue = color.value * (1.0f - color.saturation * remainder);
		Verify(blue >= 0.0f && blue <= 1.0f);
		break;
	}
	return *this;
}

//
//###########################################################################
//###########################################################################
//
void
	Stuff::Convert_From_Ascii(
		const char *str,
		RGBColor *color
	)
{
	Check_Pointer(str);
	Check_Object(color);

	MString parse_string(str);

	const char* token = parse_string.GetNthToken(0);
	Check_Pointer(token);
	color->red = AtoF(token);

	token = parse_string.GetNthToken(1);
	Check_Pointer(token);
	color->green = AtoF(token);

	token = parse_string.GetNthToken(2);
	Check_Pointer(token);
	color->blue = AtoF(token);

	Check_Object(color);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~ RGBAColor functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

const RGBAColor
	RGBAColor::Unassigned(-1.0f, -1.0f, -1.0f, -1.0f);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// (friend)
bool
	Stuff::Close_Enough(
		const RGBAColor &c1,
		const RGBAColor &c2,
		Scalar e	// = SMALL
	)
{
	Check_Object(&c1);
	Check_Object(&c2);
	return
		Close_Enough(c1.red, c2.red, e)
		 && Close_Enough(c1.green, c2.green, e)
		 && Close_Enough(c1.blue, c2.blue, e)
		 && Close_Enough(c1.alpha, c2.alpha, e);
}

//
//###########################################################################
//###########################################################################
//
void
	Stuff::Convert_From_Ascii(
		const char *str,
		RGBAColor *color
	)
{
	Check_Pointer(str);
	Check_Object(color);

	MString parse_string(str);

	const char* token = parse_string.GetNthToken(0);
	Check_Pointer(token);
	color->red = AtoF(token);

	token = parse_string.GetNthToken(1);
	Check_Pointer(token);
	color->green = AtoF(token);

	token = parse_string.GetNthToken(2);
	Check_Pointer(token);
	color->blue = AtoF(token);

	token = parse_string.GetNthToken(3);
	Check_Pointer(token);
	color->alpha = AtoF(token);

	Check_Object(color);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~ HSVColor functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
const HSVColor
	HSVColor::Unassigned(-1.0f, -1.0f, -1.0f);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// (friend)
bool
	Stuff::Close_Enough(
		const HSVColor &c1,
		const HSVColor &c2,
		Scalar e	// = SMALL
	)
{
	Check_Object(&c1);
	Check_Object(&c2);
	return
		Close_Enough(c1.hue, c2.hue, e)
		 && Close_Enough(c1.saturation, c2.saturation, e)
		 && Close_Enough(c1.value, c2.value, e);
}

//
//###########################################################################
//###########################################################################
//
HSVColor&
	HSVColor::operator=(const RGBColor &color)
{
	Check_Object(this);
	Check_Object(&color);

	Verify(color.red >= 0.0f && color.red <= 1.0f);
	Verify(color.green >= 0.0f && color.green <= 1.0f);
	Verify(color.blue >= 0.0f && color.blue <= 1.0f);

	//
	//--------------------
	// Set the color value
	//--------------------
	//
	value = Max(color.red, Max(color.green, color.blue));

	//
	//-------------------------
	// Set the saturation value
	//-------------------------
	//
	Scalar delta = value - Min(color.red, Min(color.green, color.blue));
	if (value > SMALL)
	{
		saturation = delta / value;
		Verify(saturation > 0.0f && saturation <= 1.0f);
	}
	else
	{
		saturation = 0.0f;
	}

	//
	//------------
	// Set the hue
	//------------
	//
	if (saturation <= SMALL)
	{
		hue = 0.0f;
	}
	else
	{
		Verify(delta > SMALL);
		if (color.red == value)
		{
			hue = (color.green - color.blue) / delta;
		}
		else if (color.green == value)
		{
			hue = 2.0f + (color.blue - color.red) / delta;
		}
		else
		{
			hue = 4.0f + (color.red - color.green) / delta;
		}
		if (hue < 0.0f)
		{
			hue += 6.0f;
		}
		hue *= 1.0f/6.0f;
		Verify(hue >= 0.0f && hue <= 1.0f);
	}
	return *this;
}

//
//###########################################################################
//###########################################################################
//
void
	Stuff::Convert_From_Ascii(
		const char *str,
		HSVColor *color
	)
{
	Check_Pointer(str);
	Check_Object(color);

	MString parse_string(str);

	const char* token = parse_string.GetNthToken(0);
	Check_Pointer(token);
	color->hue = AtoF(token);

	token = parse_string.GetNthToken(1);
	Check_Pointer(token);
	color->saturation = AtoF(token);

	token = parse_string.GetNthToken(2);
	Check_Pointer(token);
	color->value = AtoF(token);

	Check_Object(color);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~ HSVAColor functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

const HSVAColor
	HSVAColor::Unassigned(-1.0f, -1.0f, -1.0f, -1.0f);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// (friend)
bool
	Stuff::Close_Enough(
		const HSVAColor &c1,
		const HSVAColor &c2,
		Scalar e	// = SMALL
	)
{
	Check_Object(&c1);
	Check_Object(&c2);
	return
		Close_Enough(c1.hue, c2.hue, e)
		 && Close_Enough(c1.saturation, c2.saturation, e)
		 && Close_Enough(c1.value, c2.value, e)
		 && Close_Enough(c1.alpha, c2.alpha, e);
}

//
//###########################################################################
//###########################################################################
//
void
	Stuff::Convert_From_Ascii(
		const char *str,
		HSVAColor *color
	)
{
	Check_Pointer(str);
	Check_Object(color);

	MString parse_string(str);

	const char* token = parse_string.GetNthToken(0);
	Check_Pointer(token);
	color->hue = AtoF(token);

	token = parse_string.GetNthToken(1);
	Check_Pointer(token);
	color->saturation = AtoF(token);

	token = parse_string.GetNthToken(2);
	Check_Pointer(token);
	color->value = AtoF(token);

	token = parse_string.GetNthToken(3);
	Check_Pointer(token);
	color->alpha = AtoF(token);

	Check_Object(color);
}

