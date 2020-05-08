//лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
//лл                                                                        лл
//лл  VFX.H: C type definitions & API function prototypes                   лл
//лл                                                                        лл
//лл  Source compatible with 32-bit 80386 C/C++                             лл
//лл                                                                        лл
//лл  V0.10 of 10-Dec-92: Initial version                                   лл
//лл  V1.01 of 12-May-93: Added VFX_shape_lookaside(), new polygon calls    лл
//лл                      PCX_draw() returns void                           лл
//лл  V1.02 of 18-Jun-93: Added rotation/scaling calls                      лл
//лл                      intervals parameter added to window_fade()        лл
//лл                      Added VFX_RECT structure                              лл
//лл  V1.03 of 28-Jul-93: VERTEX2D structure changed to SCRNVERTEX          лл
//лл                      Fixed-VFX_POINT data types renamed                    лл
//лл                      Added VFX_bank_reset() call                       лл
//лл  V1.04 of  4-Sep-93: Indirect function prototypes changed for C++ use  лл
//лл  V1.05 of 26-Sep-93: Added FLOAT typedef                               лл
//лл                      VFX_RGB BYTE members changed to UBYTEs                лл
//лл  V1.06 of 13-Oct-93: Added VFX_pane_refresh(), other new calls         лл
//лл  V1.07 of 17-Nov-93: Added MetaWare High C support                     лл
//лл  V1.10 of  3-Dec-93: Modified VFX_pane_refresh(), WINDOW structure     лл
//лл  V1.15 of 13-Mar-94: Added new VFX.C function prototypes               лл
//лл                      Added new WINDOW members, PANE_LIST structure     лл
//лл                                                                        лл
//лл  Project: 386FX Sound & Light(TM)                                      лл
//лл   Author: Ken Arnold, John Miles, John Lemberger                       лл
//лл                                                                        лл
//лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
//лл                                                                        лл
//лл  Copyright (C) 1992-1994 Non-Linear Arts, Inc.                         лл
//лл                                                                        лл
//лл  Non-Linear Arts, Inc.                                                 лл
//лл  3415 Greystone #200                                                   лл
//лл  Austin, TX 78731                                                      лл
//лл                                                                        лл
//лл  (512) 346-9595 / FAX (512) 346-9596 / BBS (512) 454-9990              лл
//лл                                                                        лл
//лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

#ifndef VFX_H
#define VFX_H

#ifndef TRUE
#define TRUE -1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define SHAPE_FILE_VERSION '01.1' // 1.10 backwards for big-endian compare


//
// MetaWare support
//

#ifdef __HIGHC__
#define  _CC(_REVERSE_PARMS | _NEAR_CALL)
#pragma Global_aliasing_convention("_%r");
#pragma Align_members(1)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
#pragma pack(1)
#endif

#ifndef TYPEDEFS1
#define TYPEDEFS1

typedef unsigned char  UBYTE;
typedef unsigned short UWORD;
typedef unsigned long  ULONG;
typedef unsigned char  BYTE;

// sebi
//#ifndef WIN32
//typedef unsigned		  WORD;
//#endif

typedef signed   long  LONG;
typedef          float FLOAT;
typedef signed   long  FIXED16;   // 16:16 fixed-VFX_POINT type [-32K,+32K]
typedef signed   long  FIXED30;   // 2:30 fixed-VFX_POINT type [-1.999,+1.999]

#endif

#define GIF_SCRATCH_SIZE 20526L   // Temp memory req'd for GIF decompression

#define S_BLACK			264
//
// VFX_map_polygon() flags
//

#define MP_XLAT      0x0001       // Use lookaside table (speed loss = ~9%)
#define MP_XP        0x0002       // Enable transparency (speed loss = ~6%)

//
// VFX_shape_transform() flags
//

#define ST_XLAT      0x0001       // Use shape_lookaside() table
#define ST_REUSE     0x0002       // Use buffer contents from prior call

//
// VFX_line_draw() modes
//  

#define LD_DRAW      0
#define LD_TRANSLATE 1
#define LD_EXECUTE   2

//
// VFX_pane_scroll() modes
//

#define PS_NOWRAP    0
#define PS_WRAP      1

#define NO_COLOR -1

//
// VFX_shape_visible_rectangle() mirror values
//

#define VR_NO_MIRROR 0
#define VR_X_MIRROR  1
#define VR_Y_MIRROR  2
#define VR_XY_MIRROR 3

//
// PANE_LIST.flags values
//

#define PL_FREE      0            // Free and available for assignment
#define PL_VALID     1            // Assigned; to be refreshed
#define PL_CONTAINED 2            // Contained within another pane; don't refresh

//
// VFX data structures
//

typedef unsigned char STENCIL;

typedef struct _window
{
	UBYTE   *buffer;
	LONG     x_max;
	LONG     y_max;

   STENCIL *stencil;
   UBYTE   *shadow;
} 
WINDOW;

typedef struct _pane
{
	WINDOW *window;
	LONG x0;
	LONG y0;
	LONG x1;
	LONG y1;
} 
PANE;

typedef struct _pane_list
{
   PANE  *array;
   ULONG *flags;
   LONG   size;
}
PANE_LIST;

typedef union
{
   void  *v;
   UBYTE *b;
   UWORD *w;
   ULONG *d;
} 
FLEX_PTR;

typedef struct
{
   LONG scrn_width;
   LONG scrn_height;
   LONG bytes_per_pixel;
   LONG ncolors;
   LONG npages;
   ULONG flags;
}
VFX_DESC;

typedef struct
{
   UBYTE r;
   UBYTE g;
   UBYTE b;
}
VFX_RGB;

typedef struct
{
   UBYTE color;
   VFX_RGB   VFX_RGB;
}
VFX_CRGB;

typedef struct
{
   LONG x;
   LONG y;
}
VFX_POINT;

typedef struct
{
   LONG version;
   LONG char_count;
   LONG char_height;
   LONG font_background;
}
FONT;

typedef struct            // Vertex structure used by polygon primitives
{
   LONG x;                // Screen X
   LONG y;                // Screen Y

   FIXED16 c;             // Color/addition value used by some primitives

   FIXED16 u;             // Texture source X
   FIXED16 v;             // Texture source Y
   FIXED30 w;             // Homogeneous perspective divisor (unused by VFX3D)
}
SCRNVERTEX;               

typedef struct
{
   LONG x0;
   LONG y0;
   LONG x1;
   LONG y1;
}
VFX_RECT;

#define INT_TO_FIXED16(x)       (((long)(int)(x)) << 16)
#define DOUBLE_TO_FIXED16(x)    ((long) ((x) * 65536.0 + 0.5))
#define FIXED16_TO_DOUBLE(x)    (((double)(x)) / 65536.0)
#define FIXED16_TO_INT(x)       ((int) ((x)<0 ? -(-(x) >> 16) : (x) >> 16))
#define ROUND_FIXED16_TO_INT(x) ((int) ((x)<0 ? -((32768-(x)) >> 16) : ((x)+32768) >> 16))

#define FIXED16_TO_FIXED30(x)   ((x) << 14)
#define FIXED30_TO_FIXED16(x)   ((x) >> 14)
#define FIXED30_TO_DOUBLE(x)    (((double)x) / 1073741824.0)
#define DOUBLE_TO_FIXED30(x)    ((long) (x * 1073741824.0 + 0.5))

#define PIXELS_IN_PANE(pane)    (((pane).x1-(pane).x0+1)*((pane).y1-(pane).y0+1))
#define PIXELS_IN_PANEP(pane)   (((pane)->x1-(pane)->x0+1)*((pane)->y1-(pane)0>y0+1))

//
// Hardware-specific VFX DLL functions
//

/*
extern VFX_DESC *  (*VFX_describe_driver)(void);
extern void  (*VFX_init_driver)(void);
extern void  (*VFX_shutdown_driver)(void);
extern void  (*VFX_wait_vblank)(void);
extern void  (*VFX_wait_vblank_leading)(void);
extern void  (*VFX_area_wipe)(LONG x0, LONG y0, LONG x1, LONG y1, 
   LONG color);
extern void  (*VFX_window_refresh)(WINDOW *target, LONG x0, LONG y0,
   LONG x1, LONG y1);
extern void  (*VFX_window_read)(WINDOW *destination, LONG x0, LONG y0,
   LONG x1, LONG y1);
extern void  (*VFX_pane_refresh)(PANE *target, LONG x0, LONG y0,
   LONG x1, LONG y1);
extern void  (*VFX_DAC_read)(LONG color_number, VFX_RGB *triplet);
extern void  (*VFX_DAC_write)(LONG color_number, VFX_RGB *triplet);
extern void  (*VFX_bank_reset)(void);
extern void  (*VFX_line_address)(LONG x, LONG y, UBYTE **addr, ULONG *nbytes);
*/

//
// Device-independent VFX API functions (VFXC.C)
//

extern ULONG VFX_stencil_size(WINDOW *source, ULONG transparent_color);

extern STENCIL *VFX_stencil_construct(WINDOW *source, void *dest, ULONG transparent_color);
extern void VFX_stencil_destroy(STENCIL *stencil);

extern WINDOW *VFX_window_construct(LONG width, LONG height);
extern void VFX_window_destroy(WINDOW *window);

extern PANE *VFX_pane_construct(WINDOW *window, LONG x0, LONG y0, LONG x1, LONG y1);
extern void VFX_pane_destroy(PANE *pane);

extern PANE_LIST *VFX_pane_list_construct(LONG n_entries);
extern void VFX_pane_list_destroy(PANE_LIST *list);

extern void VFX_pane_list_clear(PANE_LIST *list);

extern LONG VFX_pane_list_add(PANE_LIST *list, PANE *target);
extern LONG VFX_pane_list_add_area(PANE_LIST *list, WINDOW *window, LONG x0, LONG y0,
       LONG x1, LONG y1);

extern void VFX_pane_list_delete_entry(PANE_LIST *list, LONG entry_num);

extern void VFX_pane_list_refresh(PANE_LIST *list);

//
// Device-independent VFX API functions (VFXA.ASM)
//

extern BYTE * VFX_driver_name(void *VFXScanDLL);

extern LONG  VFX_register_driver(void *DLLbase);

extern LONG  VFX_line_draw (PANE *pane, LONG x0, LONG y0, 
                                 LONG x1, LONG y1, LONG mode, void* parm);

extern long  VFX_shape_draw (PANE *pane, void *shape_table,
                                  LONG shape_number, LONG hotX, LONG hotY);

extern long VFX_nTile_draw (PANE* pane, void *tile, LONG hotX, LONG hotY, unsigned char *fadeTable = 0);

extern long VFX_newShape_count (void *shape);

extern void  VFX_shape_lookaside (UBYTE *table);
extern long  VFX_shape_translate_draw (PANE *pane, void *shape_table,
                                       LONG shape_number, 
                                       LONG hotX, LONG hotY);

extern void  VFX_shape_remap_colors(void *shape_table,
                                         ULONG shape_number);

void  VFX_shape_visible_rectangle(void *shape_table,
                          LONG shape_number, LONG hotX, LONG hotY,
                          LONG mirror, LONG *rectangle);

extern LONG  VFX_shape_scan (PANE *pane, ULONG transparentColor,
                                  LONG hotX, LONG hotY, void *buffer);
								  
extern long VFX_newShape_scan (PANE *shapePane, UBYTE xparentColor, UBYTE Hsx, UBYTE Hsy, unsigned char* *shapeBuffer);
extern long VFX_deltaShape_scan (PANE *shapePane, UBYTE xparentColor, UBYTE Hsx, UBYTE Hsy, unsigned char* *shapeBuffer);

extern long VFX_tile_scan (PANE *shapePane, UBYTE xparentColor, UBYTE Hsx, UBYTE Hsy, unsigned char* *shapeBuffer);
								  
extern int VFX_shape_bounds(void *shape_table, LONG shape_num);
extern LONG  VFX_shape_origin(void *shape_table, LONG shape_num);
extern LONG  VFX_shape_resolution(void *shape_table, LONG shape_num);
extern LONG  VFX_shape_minxy(void *shape_table, LONG shape_num);
extern void  VFX_shape_palette(void *shape_table, LONG shape_num,
                                    VFX_RGB *palette);
extern LONG  VFX_shape_colors(void *shape_table, LONG shape_num,
                                   VFX_CRGB *colors);
extern LONG  VFX_shape_set_colors(void *shape_table, LONG shape_number,
                                       VFX_CRGB *colors);
extern LONG  VFX_shape_count(void *shape_table);
extern LONG  VFX_shape_list(void *shape_table, ULONG *index_list);
extern LONG  VFX_shape_palette_list(void *shape_table, ULONG *index_list);

extern LONG  VFX_pixel_write (PANE *pane, LONG x, LONG y, ULONG color);
extern LONG  VFX_pixel_read  (PANE *pane, LONG x, LONG y);

extern LONG  VFX_rectangle_hash (PANE *pane, LONG x0, LONG y0,
                                      LONG x1, LONG y1, ULONG color);

extern LONG  VFX_pane_wipe (PANE *pane, LONG color);
extern LONG  VFX_pane_copy (PANE *source, LONG sx, LONG sy,
                                 PANE *target, LONG tx, LONG ty, LONG fill);

extern LONG  VFX_pane_scroll (PANE *pane, LONG dx, LONG dy,
                                   LONG mode, LONG parm);

extern void  VFX_ellipse_draw(PANE *pane, LONG xc, LONG yc, 
   LONG width, LONG height, LONG color);
extern void  VFX_ellipse_fill(PANE *pane, LONG xc, LONG yc, 
   LONG width, LONG height, LONG color);

extern void  VFX_point_transform(VFX_POINT *in, VFX_POINT *out, VFX_POINT *origin,
   LONG rot, LONG x_scale, LONG y_scale);

extern void  VFX_Cos_Sin(LONG Angle, FIXED16 *Cos, FIXED16 *Sin);
extern void  VFX_fixed_mul(FIXED16 M1, FIXED16 M2,
   FIXED16 *result);

extern LONG  VFX_font_height(void *font);
extern LONG  VFX_character_width(void *font, LONG character);
extern LONG  VFX_character_draw(PANE *pane, LONG x, LONG y, void *font,
   LONG character, UBYTE *color_translate);
extern void  VFX_string_draw(PANE *pane, LONG x, LONG y, void *font,
   char *string, UBYTE *color_translate);

extern LONG  VFX_ILBM_draw(PANE *pane, UBYTE *ILBM_buffer);
extern void  VFX_ILBM_palette(UBYTE *ILBM_buffer, VFX_RGB *palette);
extern LONG  VFX_ILBM_resolution(UBYTE *ILBM_buffer);

extern void  VFX_PCX_draw(PANE *pane, UBYTE *PCX_buffer);
extern void  VFX_PCX_palette(UBYTE *PCX_buffer, LONG PCX_file_size,
                                      VFX_RGB *palette);
extern LONG  VFX_PCX_resolution(UBYTE *PCX_buffer);

extern LONG  VFX_GIF_draw(PANE *pane, UBYTE *GIF_buffer,
                               void *GIF_scratch);
extern void  VFX_GIF_palette(UBYTE *GIF_buffer, VFX_RGB *palette);
extern LONG  VFX_GIF_resolution(UBYTE *GIF_buffer);

extern LONG  VFX_pixel_fade(PANE *source, PANE *destination,
                                 LONG intervals, LONG rnd);

extern void  VFX_window_fade(WINDOW *buffer, VFX_RGB *palette, LONG intervals);
extern LONG  VFX_color_scan(PANE *pane, ULONG *colors);

extern void  VFX_shape_transform(PANE *pane,
   void *shape_table, LONG shape_number, LONG hotX, LONG hotY,
   void *buffer, LONG rot, LONG x_scale, LONG y_scale, LONG flags);

//
// VFX 3D polygon functions
//

extern void VFX_flat_polygon(PANE *pane, LONG vcnt, SCRNVERTEX *vlist);

extern void VFX_Gouraud_polygon(PANE *pane, LONG vcnt, 
   SCRNVERTEX *vlist);

extern void VFX_dithered_Gouraud_polygon(PANE *pane, FIXED16
   dither_amount, LONG vcnt, SCRNVERTEX *vlist);

extern void VFX_map_lookaside(UBYTE *table);

extern void VFX_map_polygon(PANE *pane, LONG vcnt, SCRNVERTEX *vlist,
   WINDOW *texture, ULONG flags);

extern void VFX_translate_polygon(PANE *pane, LONG vcnt, SCRNVERTEX *vlist,
   void *lookaside);

extern void VFX_illuminate_polygon(PANE *pane, FIXED16 dither_amount, 
   LONG vcnt, SCRNVERTEX *vlist);

#ifdef __cplusplus
}
#endif

//
// MetaWare support
//

#ifdef __HIGHC__
#pragma Global_aliasing_convention();
#pragma Align_members(4)
#endif

#ifdef WIN32
#pragma pack()
#endif

#endif


