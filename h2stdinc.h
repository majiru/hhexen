/*
	h2stdinc.h
	includes the minimum necessary stdc headers,
	defines common and / or missing types.

	$Id: h2stdinc.h 577 2011-06-11 13:30:45Z sezero $
*/

#ifndef __H2STDINC_H
#define __H2STDINC_H

#include <u.h>
#include <libc.h>
#include <stdio.h>

#define uint32_t u32int
#define int32_t s32int
#define int16_t s16int
#define uint16_t u16int
#define uint64_t u64int
#define int64_t s64int

#define intptr_t vlong
#define uintptr_t uvlong
#define ptrdiff_t vlong

#define size_t uvlong

#undef PI


/*==========================================================================*/

#ifndef NULL
#if defined(__cplusplus)
#define	NULL		0
#else
#define	NULL		((void *)0)
#endif
#endif

#define	H2MAXCHAR	((char)0x7f)
#define	H2MAXSHORT	((short)0x7fff)
#define	H2MAXINT	((int)0x7fffffff)	/* max positive 32-bit integer */
#define	H2MINCHAR	((char)0x80)
#define	H2MINSHORT	((short)0x8000)
#define	H2MININT	((int)0x80000000)	/* max negative 32-bit integer */

/* Make sure the types really have the right
 * sizes: These macros are from SDL headers.
 */
#define	COMPILE_TIME_ASSERT(name, x)	\
	typedef int dummy_ ## name[(x) * 2 - 1]

COMPILE_TIME_ASSERT(char, sizeof(char) == 1);
COMPILE_TIME_ASSERT(float, sizeof(float) == 4);
COMPILE_TIME_ASSERT(long, sizeof(long) >= 4);
COMPILE_TIME_ASSERT(int, sizeof(int) == 4);
COMPILE_TIME_ASSERT(short, sizeof(short) == 2);

/* make sure enums are the size of ints for structure packing */
typedef enum {
	THE_DUMMY_VALUE
} THE_DUMMY_ENUM;
COMPILE_TIME_ASSERT(enum, sizeof(THE_DUMMY_ENUM) == sizeof(int));


/*==========================================================================*/

typedef unsigned char		byte;

#undef true
#undef false
#if defined(__cplusplus)
/* some structures have boolean members and the x86 asm code expect
 * those members to be 4 bytes long. therefore, boolean must be 32
 * bits and it can NOT be binary compatible with the 8 bit C++ bool.  */
typedef int	boolean;
COMPILE_TIME_ASSERT(falsehood, (0 == false));
COMPILE_TIME_ASSERT(truth, (1  == true));
#else
typedef enum {
	false = 0,
	true  = 1
} boolean;
COMPILE_TIME_ASSERT(falsehood, ((1 != 1) == false));
COMPILE_TIME_ASSERT(truth, ((1 == 1) == true));
#endif
COMPILE_TIME_ASSERT(boolean, sizeof(boolean) == 4);

/*==========================================================================*/

/* math */
#define	FRACBITS		16
#define	FRACUNIT		(1 << FRACBITS)

typedef int	fixed_t;


/*==========================================================================*/

/* compatibility with DOS/Windows */
#ifndef O_BINARY
# if defined(_O_BINARY)
#  define O_BINARY	_O_BINARY
# else
#  define O_BINARY		0
# endif
#endif

/* compatibility with M$ types */
#if !defined(_WIN32)
#define	PASCAL
#define	FAR
#define	APIENTRY
#endif	/* ! WINDOWS */

/*==========================================================================*/

/* compiler specific definitions */

#if !defined(__GNUC__)
#define	__attribute__(x)
#endif	/* __GNUC__ */

/* argument format attributes for function
 * pointers are supported for gcc >= 3.1
 */
#if defined(__GNUC__) && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ > 0))
#define	__fp_attribute__	__attribute__
#else
#define	__fp_attribute__(x)
#endif

/* function optimize attribute is added
 * starting with gcc 4.4.0
 */
#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 3))
#define	__no_optimize		__attribute__((__optimize__("0")))
#else
#define	__no_optimize
#endif

/*==========================================================================*/

/* Some compilers, such as OpenWatcom, and possibly other compilers
 * from the DOS universe, define __386__ instead of __i386__
 */
#if defined(__386__) && !defined(__i386__)
#define __i386__	1
#endif

/*==========================================================================*/

/* Provide a substitute for offsetof() if we don't have one.
 * This variant works on most (but not *all*) systems...
 */
#ifndef offsetof
#define offsetof(t,m) ((size_t)&(((t *)0)->m))
#endif

/*==========================================================================*/


#endif	/* __H2STDINC_H */

