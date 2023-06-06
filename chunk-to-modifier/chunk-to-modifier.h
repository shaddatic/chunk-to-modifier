#pragma once

#define _CRT_SECURE_NO_WARNINGS

/*
*	Integer types
*/
typedef unsigned __int8		uint8;		/*  unsigned 1 byte integer	*/
typedef signed	 __int8		sint8;		/*  signed 1 byte integer	*/
typedef unsigned __int16	uint16;		/*  unsigned 2 byte integer	*/
typedef signed	 __int16	sint16;		/*  signed 2 byte integer	*/
typedef unsigned __int32	uint32;		/*  unsigned 4 byte integer	*/
typedef signed	 __int32	sint32;		/*  signed 4 byte integer	*/
typedef unsigned __int64	uint64;		/*  unsigned 8 byte integer	*/
typedef signed	 __int64	sint64;		/*  signed 8 byte integer	*/

typedef sint32				Angle;

/*
*	Floating Point types
*/
typedef float				float32;	/*  4 byte real number		*/
typedef double				float64;	/*  8 byte real number		*/

/*
*	Defines
*/

/* Chunk type offset */
#define NJD_NULLOFF             0 /* null chunk (16 bits size)            */
#define NJD_BITSOFF             1 /* chunk bits offset (16 bits size)     */
#define NJD_TINYOFF             8 /* chunk tiny offset (32 bits size)     */
#define NJD_MATOFF             16 /* chunk material offset (32 bits size) */
#define NJD_VERTOFF            32 /* chunk vertex offset (32 bits size)   */
#define NJD_VOLOFF             56 /* chunk volume offset (32 bits size)   */
#define NJD_STRIPOFF           64 /* chunk strip offset                   */
#define NJD_ENDOFF            255 /* end chunk offset (16 bits size)      */

#define NJD_CV_SH     (NJD_VERTOFF+0)  /* x,y,z,1.0F, ...                 */
#define NJD_CV_VN_SH  (NJD_VERTOFF+1)  /* x,y,z,1.0F,nx,ny,nz,0.0F,...    */

/* chunk vertex */
#define NJD_CV        (NJD_VERTOFF+2)  /* x,y,z, ...                      */
#define NJD_CV_D8     (NJD_VERTOFF+3)  /* x,y,z,D8888,...                 */  
#define NJD_CV_UF     (NJD_VERTOFF+4)  /* x,y,z,UserFlags32, ...          */
#define NJD_CV_NF     (NJD_VERTOFF+5)  /* x,y,z,NinjaFlags32,...          */
#define NJD_CV_S5     (NJD_VERTOFF+6)  /* x,y,z,D565|S565,...             */
#define NJD_CV_S4     (NJD_VERTOFF+7)  /* x,y,z,D4444|S565,...            */
#define NJD_CV_IN     (NJD_VERTOFF+8)  /* x,y,z,D16|S16,...               */

#define NJD_CV_VN     (NJD_VERTOFF+9)  /* x,y,z,nx,ny,nz, ...             */
#define NJD_CV_VN_D8  (NJD_VERTOFF+10) /* x,y,z,nx,ny,nz,D8888,...        */
#define NJD_CV_VN_UF  (NJD_VERTOFF+11) /* x,y,z,nx,ny,nz,UserFlags32,...  */
#define NJD_CV_VN_NF  (NJD_VERTOFF+12) /* x,y,z,nx,ny,nz,NinjaFlags32,... */
#define NJD_CV_VN_S5  (NJD_VERTOFF+13) /* x,y,z,nx,ny,nz,D565|S565,...    */
#define NJD_CV_VN_S4  (NJD_VERTOFF+14) /* x,y,z,nx,ny,nz,D4444|S565,...   */
#define NJD_CV_VN_IN  (NJD_VERTOFF+15) /* x,y,z,nx,ny,nz,D16|S16,...      */

#define NJD_CV_VNX    (NJD_VERTOFF+16) /* x,y,z,nxyz32, ...               */
#define NJD_CV_VNX_D8 (NJD_VERTOFF+17) /* x,y,z,nxyz32,D8888,...          */
#define NJD_CV_VNX_UF (NJD_VERTOFF+18) /* x,y,z,nxyz32,UserFlags32,...    */

#define NJD_CM_D    (NJD_MATOFF+1)  /* [CHead][4(Size)][ARGB]              */
#define NJD_CM_A    (NJD_MATOFF+2)  /* [CHead][4(Size)][NRGB] N: NOOP(255) */
#define NJD_CM_DA   (NJD_MATOFF+3)  /* [CHead][8(Size)][ARGB][NRGB]        */
#define NJD_CM_S    (NJD_MATOFF+4)  /* [CHead][4(Size)][ERGB] E: Exponent  */
#define NJD_CM_DS   (NJD_MATOFF+5)  /* [CHead][8(Size)][ARGB][ERGB]        */
#define NJD_CM_AS   (NJD_MATOFF+6)  /* [CHead][8(Size)][NRGB][ERGB]        */
#define NJD_CM_DAS  (NJD_MATOFF+7)  /* [CHead][12(Size)][ARGB][NRGB][ERGB] */

/*
*	Structs
*/

typedef struct {
	float32 x, y, z;
} NJS_POINT3, NJS_VECTOR;

typedef struct {
	sint32* vlist;   /* vertex list                  */
	sint16* plist;   /* polygon list                 */
	NJS_POINT3        center;   /* model center                 */
	float32                  r;   /* radius                       */
} NJS_CNK_MODEL;

/*
 * NJS_CNK_OBJECT
 */
typedef struct cnkobj {
	uint32          evalflags;  /* evalation flags              */
	NJS_CNK_MODEL* model;     /* model data pointer           */
	float32           pos[3];     /* translation                  */
	Angle           ang[3];     /* rotation                     */
	float32           scl[3];     /* scaling                      */
	struct cnkobj* child;     /* child object                 */
	struct cnkobj* sibling;   /* sibling object               */
} NJS_CNK_OBJECT;

/*
*	Includes
*/

#include <wchar.h>
#include <stdio.h>

/*
*	Functions
*/

NJS_CNK_OBJECT* LoadChunkObject(const wchar_t* fn);

void	Pause();
char*	GetFileName(const wchar_t* wPath, char* pBuf);