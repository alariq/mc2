//===========================================================================//
// File:     color.hpp                                                       //
// Title:    Declaration of Color classes.                                   //
// Purpose:  Stores color information.                                       //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#include"stuff.hpp"
#include"memorystream.hpp"
#include"scalar.hpp"
#include"vector3d.hpp"

namespace Stuff {
	class RGBColor;
	class RGBAColor;
	class HSVColor;
	class HSVAColor;
}

#if !defined(Spew)
	void
		Spew(
			const char* group,
			const Stuff::RGBColor &color
		);
	void
		Spew(
			const char* group,
			const Stuff::RGBAColor &color
		);
	void
		Spew(
			const char* group,
			const Stuff::HSVColor &color
		);
	void
		Spew(
			const char* group,
			const Stuff::HSVAColor &color
		);
#endif

namespace Stuff {

	//##########################################################################
	//##############    RGBColor    ############################################
	//##########################################################################

	class RGBColor
	{
		friend class RGBAColor;

	public:
		static const RGBColor
			Unassigned;

		Scalar
			red,
			green,
			blue;

		RGBColor()
			{red = green = blue = -1.0f;}

		RGBColor(
			Scalar r,
			Scalar g,
			Scalar b
		)
			{red = r; green = g; blue = b;}

		friend bool
			Close_Enough(
				const RGBColor &c1,
				const RGBColor &c2,
				Scalar e/* = SMALL*/
			);
		bool
			operator==(const RGBColor &color) const
				{Check_Object(this); return Close_Enough(*this, color, SMALL);}
		bool
			operator!=(const RGBColor &color) const
				{Check_Object(this); return !Close_Enough(*this, color, SMALL);}

		RGBColor&
			operator=(const RGBColor &color)
				{
					Check_Object(this); Check_Object(&color);
					red = color.red; green = color.green; blue = color.blue;
					return *this;
				}
		RGBColor&
			operator=(const HSVColor &color);

		void
			TestInstance() const
				{}

		Scalar
			Infrared() const
				{Check_Object(this); return 0.3f*red + 0.5f*green + 0.2f*blue;}

		RGBColor&
			Combine(
				const RGBColor& c1,
				Scalar t1,
				const RGBColor& c2,
				Scalar t2
			)
				{
					Check_Pointer(this); Check_Object(&c1); Check_Object(&c2);
					red = c1.red*t1 + c2.red*t2; green = c1.green*t1 + c2.green*t2;
					blue = c1.blue*t1 + c2.blue*t2; return *this;
				}

		RGBColor&
			Lerp(
				const RGBColor& c1,
				const RGBColor& c2,
				Scalar t
			)
				{
					Check_Pointer(this); Check_Object(&c1); Check_Object(&c2);
					red = c1.red + t*(c2.red-c1.red);
					green = c1.green + t*(c2.green-c1.green);
					blue = c1.blue + t*(c2.blue-c1.blue);
					return *this;
				}

	//
	// Support functions
	//
		#if !defined(Spew)
			friend void
				::Spew(
					const char* group,
					const RGBColor &color
				);
		#endif
	};

	//~~~~~~~~~~~~~~~~~~~~~~~~~~ RGBColor functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

   void
   	Convert_From_Ascii(
			const char *str,
			RGBColor *color
      );

	//##########################################################################
	//##############    RGBAColor    ###########################################
	//##########################################################################

	class RGBAColor:
		public RGBColor
	{
	public:
		static const RGBAColor
			Unassigned;

		Scalar
			alpha;
		
		RGBAColor():
			RGBColor()
				{alpha = -1.0f;}

		RGBAColor(
			Scalar r,
			Scalar g,
			Scalar b,
			Scalar a
		):
			RGBColor(r, g, b)
				{alpha = a;}

		friend bool
			Close_Enough(
				const RGBAColor &c1,
				const RGBAColor &c2,
				Scalar e/* = SMALL*/
			);
		bool
			operator==(const RGBAColor &color) const
				{return Close_Enough(*this, color, SMALL);}
		bool
			operator!=(const RGBAColor &color) const
				{return !Close_Enough(*this, color, SMALL);}

		RGBAColor&
			operator=(const RGBAColor &color)
				{
					Check_Object(this); Check_Object(&color);
					red = color.red; green = color.green; blue = color.blue;
					alpha = color.alpha;
					return *this;
				}
		RGBAColor&
			operator=(const HSVAColor &color);

		RGBAColor&
			Combine(
				const RGBAColor& c1,
				Scalar t1,
				const RGBAColor& c2,
				Scalar t2
			)
				{
					Check_Pointer(this); Check_Object(&c1); Check_Object(&c2);
					red = c1.red*t1 + c2.red*t2; green = c1.green*t1 + c2.green*t2;
					blue = c1.blue*t1 + c2.blue*t2;
					alpha = c1.alpha*t1 + c2.alpha*t2; return *this;
				}

		RGBAColor&
			Lerp(
				const RGBAColor& c1,
				const RGBAColor& c2,
				Scalar t
			)
				{
					Check_Pointer(this); Check_Object(&c1); Check_Object(&c2);
					red = c1.red + t*(c2.red-c1.red);
					green = c1.green + t*(c2.green-c1.green);
					blue = c1.blue + t*(c2.blue-c1.blue);
					alpha = c1.alpha + t*(c2.alpha-c1.alpha);
					return *this;
				}

	//
	// Support functions
	//
		#if !defined(Spew)
			friend void
				::Spew(
					const char* group,
					const RGBAColor &color
				);
		#endif
	};

	//~~~~~~~~~~~~~~~~~~~~~~~~~~ RGBAColor functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	void
		Convert_From_Ascii(
			const char *str,
		  RGBAColor *color
		);

	//##########################################################################
	//##############    HSVColor    ############################################
	//##########################################################################

	class HSVColor
	{
		friend class HSVAColor;

	public:
		static const HSVColor
			Unassigned;

		Scalar
			hue,
			saturation,
			value;

		HSVColor()
			{hue = saturation = value = -1.0f;}

		HSVColor(
			Scalar h,
			Scalar s,
			Scalar v
		)
			{hue = h; saturation = s; value = v;}

		friend bool
			Close_Enough(
				const HSVColor &c1,
				const HSVColor &c2,
				Scalar e/* = SMALL*/
			);
		bool
			operator==(const HSVColor &color) const
				{Check_Object(this); return Close_Enough(*this, color, SMALL);}
		bool
			operator!=(const HSVColor &color) const
				{Check_Object(this); return !Close_Enough(*this, color, SMALL);}

		HSVColor&
			operator=(const RGBColor &color);
		HSVColor&
			operator=(const HSVColor &color)
				{
					Check_Object(this); Check_Object(&color);
					hue = color.hue; saturation = color.saturation;
					value = color.value; return *this;
				}

		void
			TestInstance() const
				{}

		HSVColor&
			Combine(
				const HSVColor& c1,
				Scalar t1,
				const HSVColor& c2,
				Scalar t2
			)
				{
					Check_Pointer(this); Check_Object(&c1); Check_Object(&c2);
					hue = c1.hue*t1 + c2.hue*t2;
					saturation = c1.saturation*t1 + c2.saturation*t2;
					value = c1.value*t1 + c2.value*t2;
					return *this;
				}

		HSVColor&
			Lerp(
				const HSVColor& c1,
				const HSVColor& c2,
				Scalar t
			)
				{
					Check_Pointer(this); Check_Object(&c1); Check_Object(&c2);
					hue = c1.hue + t*(c2.hue-c1.hue);
					saturation = c1.saturation + t*(c2.saturation-c1.saturation);
					value = c1.value + t*(c2.value-c1.value);
					return *this;
				}

	//
	// Support functions
	//
		#if !defined(Spew)
			friend void
				::Spew(
					const char* group,
					const HSVColor &color
				);
		#endif
	};

	//~~~~~~~~~~~~~~~~~~~~~~~~~~ HSVColor functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

   void
   	Convert_From_Ascii(
			const char *str,
			HSVColor *color
      );

	//##########################################################################
	//##############    HSVAColor    ###########################################
	//##########################################################################

	class HSVAColor:
		public HSVColor
	{
	public:
		static const HSVAColor
			Unassigned;

		Scalar
			alpha;
		
		HSVAColor():
			HSVColor()
				{alpha = -1.0f;}

		HSVAColor(
			Scalar h,
			Scalar s,
			Scalar v,
			Scalar a
		):
			HSVColor(h, s, v)
				{alpha = a;}

		friend bool
			Close_Enough(
				const HSVAColor &c1,
				const HSVAColor &c2,
				Scalar e/* = SMALL*/
			);
		bool
			operator==(const HSVAColor &color) const
				{return Close_Enough(*this, color, SMALL);}
		bool
			operator!=(const HSVAColor &color) const
				{return !Close_Enough(*this, color, SMALL);}

		HSVAColor&
			operator=(const RGBAColor &color)
				{
					Check_Object(this); Check_Object(&color);
					HSVColor::operator=(color); alpha = color.alpha;
					return *this;
				}
		HSVAColor&
			operator=(const HSVAColor &color)
				{
					Check_Object(this); Check_Object(&color);
					hue = color.hue; saturation = color.saturation;
					value = color.value; alpha = color.alpha; return *this;
				}

		HSVAColor&
			Combine(
				const HSVAColor& c1,
				Scalar t1,
				const HSVAColor& c2,
				Scalar t2
			)
				{
					Check_Pointer(this); Check_Object(&c1); Check_Object(&c2);
					hue = c1.hue*t1 + c2.hue*t2;
					saturation = c1.saturation*t1 + c2.saturation*t2;
					value = c1.value*t1 + c2.value*t2;
					alpha = c1.alpha*t1 + c2.alpha*t2;
					return *this;
				}

		HSVAColor&
			Lerp(
				const HSVAColor& c1,
				const HSVAColor& c2,
				Scalar t
			)
				{
					Check_Pointer(this); Check_Object(&c1); Check_Object(&c2);
					hue = c1.hue + t*(c2.hue-c1.hue);
					saturation = c1.saturation + t*(c2.saturation-c1.saturation);
					value = c1.value + t*(c2.value-c1.value);
					alpha = c1.alpha + t*(c2.alpha-c1.alpha);
					return *this;
				}

	//
	// Support functions
	//
		#if !defined(Spew)
			friend void
				::Spew(
					const char* group,
					const HSVAColor &color
				);
		#endif
	};

	//~~~~~~~~~~~~~~~~~~~~~~~~~~ HSVAColor functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	void
		Convert_From_Ascii(
			const char *str,
		  HSVAColor *color
		);

	inline RGBAColor&
		RGBAColor::operator=(const HSVAColor &color)
	{
		Check_Object(this);
		Check_Object(&color);

		RGBColor::operator=(color);
		alpha = color.alpha;
		return *this;
	}

}

namespace MemoryStreamIO {

	inline Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream* stream,
			Stuff::RGBColor *output
		)
			{return stream->ReadBytes(output, sizeof(*output));}
	inline Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const Stuff::RGBColor *input
		)
			{return stream->WriteBytes(input, sizeof(*input));}

	inline Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream* stream,
			Stuff::RGBAColor *output
		)
			{return stream->ReadBytes(output, sizeof(*output));}
	inline Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const Stuff::RGBAColor *input
		)
			{return stream->WriteBytes(input, sizeof(*input));}

	inline Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream* stream,
			Stuff::HSVColor *output
		)
			{return stream->ReadBytes(output, sizeof(*output));}
	inline Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const Stuff::HSVColor *input
		)
			{return stream->WriteBytes(input, sizeof(*input));}

	inline Stuff::MemoryStream&
		Read(
			Stuff::MemoryStream* stream,
			Stuff::HSVAColor *output
		)
			{return stream->ReadBytes(output, sizeof(*output));}
	inline Stuff::MemoryStream&
		Write(
			Stuff::MemoryStream* stream,
			const Stuff::HSVAColor *input
		)
			{return stream->WriteBytes(input, sizeof(*input));}
}
