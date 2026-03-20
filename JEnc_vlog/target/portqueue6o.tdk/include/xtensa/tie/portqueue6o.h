/* Definitions for the portqueue6o TIE package */

/*
 * Copyright (c) 2004-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of Tensilica Inc.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part, without
 * the prior written consent of Tensilica Inc.
 */

/* Do not modify. This is automatically generated.*/

#ifndef _XTENSA_portqueue6o_HEADER
#define _XTENSA_portqueue6o_HEADER

#ifdef __XTENSA__
#ifdef __XCC__

#include <xtensa/tie/xt_core.h>

/*
 * The following prototypes describe intrinsic functions
 * corresponding to TIE instructions.  Some TIE instructions
 * may produce multiple results (designated as "out" operands
 * in the iclass section) or may have operands used as both
 * inputs and outputs (designated as "inout").  However, the C
 * and C++ languages do not provide syntax that can express
 * the in/out/inout constraints of TIE intrinsics.
 * Nevertheless, the compiler understands these constraints
 * and will check that the intrinsic functions are used
 * correctly.  To improve the readability of these prototypes,
 * the "out" and "inout" parameters are marked accordingly
 * with comments.
 */

extern void _TIE_portqueue6o_SEND1(unsigned mes);
extern void _TIE_portqueue6o_SEND2(unsigned mes);
extern void _TIE_portqueue6o_SEND3(unsigned mes);
extern void _TIE_portqueue6o_SEND4(unsigned mes);
extern void _TIE_portqueue6o_SEND5(unsigned mes);
extern void _TIE_portqueue6o_SEND6(unsigned mes);
extern void _TIE_portqueue6o_DUMMY(void);
#define SEND1 _TIE_portqueue6o_SEND1
#define SEND2 _TIE_portqueue6o_SEND2
#define SEND3 _TIE_portqueue6o_SEND3
#define SEND4 _TIE_portqueue6o_SEND4
#define SEND5 _TIE_portqueue6o_SEND5
#define SEND6 _TIE_portqueue6o_SEND6
#define DUMMY _TIE_portqueue6o_DUMMY

#endif /* __XCC__ */

#endif /* __XTENSA__ */
#endif /* !_XTENSA_portqueue6o_HEADER */
