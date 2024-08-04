/*******************************************************************************
*																			   *
*	Define size independent data types and operations.						   *
*																			   *
*   The following types must be supported by all platforms:					   *
*																			   *
*	UINT8  - Unsigned 8-bit Integer		INT8  - Signed 8-bit integer           *
*	UINT16 - Unsigned 16-bit Integer	INT16 - Signed 16-bit integer          *
*	UINT32 - Unsigned 32-bit Integer	INT32 - Signed 32-bit integer          *
*	UINT64 - Unsigned 64-bit Integer	INT64 - Signed 64-bit integer          *
*																			   *
*																			   *
*   The macro names for the artithmatic operations are composed as follows:    *
*																			   *
*   XXX_R_A_B, where XXX - 3 letter operation code (ADD, SUB, etc.)			   *
*					 R   - The type	of the result							   *
*					 A   - The type of operand 1							   *
*			         B   - The type of operand 2 (if binary operation)		   *
*																			   *
*				     Each type is one of: U8,8,U16,16,U32,32,U64,64			   *
*																			   *
*******************************************************************************/


/***************************** DOS Section ************************************/

#ifndef OSD_CPU_H
#define OSD_CPU_H

typedef unsigned char		UINT8;
typedef unsigned short		UINT16;
typedef unsigned int		UINT32;
typedef unsigned __int64	UINT64;
typedef signed char 		INT8;
typedef signed short		INT16;
typedef signed int			INT32;
typedef signed __int64	    INT64;

/* Combine to 32-bit integers into a 64-bit integer */
#define COMBINE_64_32_32(A,B)     ((((UINT64)(A))<<32) | (B))
#define COMBINE_U64_U32_U32(A,B)  COMBINE_64_32_32(A,B)

/* Return upper 32 bits of a 64-bit integer */
#define HI32_32_64(A)		  (((UINT64)(A)) >> 32)
#define HI32_U32_U64(A)		  HI32_32_64(A)

/* Return lower 32 bits of a 64-bit integer */
#define LO32_32_64(A)		  ((A) & 0xffffffff)
#define LO32_U32_U64(A)		  LO32_32_64(A)

#define DIV_64_64_32(A,B)	  ((A)/(B))
#define DIV_U64_U64_U32(A,B)  ((A)/(UINT32)(B))

#define MOD_32_64_32(A,B)	  ((A)%(B))
#define MOD_U32_U64_U32(A,B)  ((A)%(UINT32)(B))

#define MUL_64_32_32(A,B)	  ((A)*(INT64)(B))
#define MUL_U64_U32_U32(A,B)  ((A)*(UINT64)(UINT32)(B))

/***************************** Common types ***********************************/

/******************************************************************************
 * Union of UINT8, UINT16 and UINT32 in native endianess of the target
 * This is used to access bytes and words in a machine independent manner.
 * The upper bytes h2 and h3 normally contain zero (16 bit CPU cores)
 * thus PAIR.d can be used to pass arguments to the memory system
 * which expects 'int' really.
 ******************************************************************************/
typedef union {
#ifdef LSB_FIRST
	struct { UINT8 l,h,h2,h3; } b;
	struct { UINT16 l,h; } w;
#else
	struct { UINT8 h3,h2,h,l; } b;
	struct { UINT16 h,l; } w;
#endif
	UINT32 d;
}	PAIR;

/* Turn off type mismatch warnings */
#pragma warning (disable:4244)
#pragma warning (disable:4761)
#pragma warning (disable:4018)
#pragma warning (disable:4146)
#pragma warning (disable:4068)
#pragma warning (disable:4005)
#pragma warning (disable:4305)

/* Unused variables */
/* #pragma warning (disable:4101) */


#ifndef HAS_CPUS
#define HAS_CPUS

#ifdef  NEOMAME

#define HAS_Z80         1
#define HAS_M68000      1

#else

/* CPUs available */
#define HAS_GENSYNC     1
#define HAS_Z80         1
#define HAS_8080        1
#define HAS_8085A       1
#define HAS_M6502       1
#define HAS_M65C02      1
#define HAS_M6510       1
#define HAS_N2A03       1
#define HAS_H6280       1
#define HAS_I86         1
#define HAS_V20         1
#define HAS_V30         1
#define HAS_V33         1
#define HAS_I8035       1
#define HAS_I8039       1
#define HAS_I8048       1
#define HAS_N7751       1
#define HAS_M6800       1
#define HAS_M6801       1
#define HAS_M6802       1
#define HAS_M6803       1
#define HAS_M6805       1
#define HAS_M6808       1
#define HAS_HD63701     1
#define HAS_NSC8105     1
#define HAS_M68705      1
#define HAS_HD63705     1
#define HAS_HD6309      1
#define HAS_M6309       1
#define HAS_M6809       1
#define HAS_KONAMI      1
#define HAS_M68000      1
#define HAS_M68010      1
#define HAS_M68020      1
#define HAS_T11         1
#define HAS_S2650       1
#define HAS_TMS34010    1
#define HAS_TMS9900     1
#define HAS_Z8000       1
#define HAS_TMS320C10   1
#define HAS_CCPU        1
#define HAS_PDP1        0

#endif  /* !NEOMAME */
#endif  /* !HAS_CPUS */

#ifndef HAS_SOUND
#define HAS_SOUND

#ifdef  NEOMAME

#define HAS_YM2610      1

#else

/* Sound systems */
#define HAS_CUSTOM      1
#define HAS_SAMPLES     1
#define HAS_DAC         1
#define HAS_AY8910      1
#define HAS_YM2203      1
#define HAS_YM2151      1
#define HAS_YM2151_ALT  1
#define HAS_YM2608      1
#define HAS_YM2610      1
#define HAS_YM2610B     1
#define HAS_YM2612      1
#define HAS_YM3438      1
#define HAS_YM2413      1
#define HAS_YM3812      1
#define HAS_YM3526      1
#define HAS_Y8950       1
#define HAS_SN76496     1
#define HAS_POKEY       1
#define HAS_TIA         0
#define HAS_NES         1
#define HAS_ASTROCADE   1
#define HAS_NAMCO       1
#define HAS_TMS5220     1
#define HAS_VLM5030     1
#define HAS_ADPCM       1
#define HAS_OKIM6295    1
#define HAS_MSM5205     1
#define HAS_UPD7759     1
#define HAS_HC55516     1
#define HAS_K005289     1
#define HAS_K007232     1
#define HAS_K051649     1
#define HAS_K053260     1
#define HAS_SEGAPCM     1
#define HAS_RF5C68      1
#define HAS_CEM3394     1
#define HAS_C140        1
#define HAS_QSOUND      1


#endif  /* !NEOMAME */
#endif  /* !HAS_SOUND */

#endif	/* defined OSD_CPU_H */
