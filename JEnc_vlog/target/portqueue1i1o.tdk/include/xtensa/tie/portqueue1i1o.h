/* Definitions for the portqueue1i1o TIE package */

/*
 * Copyright (c) 2004-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of Tensilica Inc.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part, without
 * the prior written consent of Tensilica Inc.
 */

/* Do not modify. This is automatically generated.*/

#ifndef _XTENSA_portqueue1i1o_HEADER
#define _XTENSA_portqueue1i1o_HEADER

#ifdef __XTENSA__
#ifdef __XCC__

#include <xtensa/tie/xt_core.h>
#include <xtensa/tie/xt_booleans.h>

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

extern unsigned _TIE_portqueue1i1o_RECV(void);
extern void _TIE_portqueue1i1o_SEND(unsigned mes);
extern xtbool _TIE_portqueue1i1o_IS_FIFOEMPTY(void);
extern xtbool _TIE_portqueue1i1o_IS_FIFOFULL(void);
extern void _TIE_portqueue1i1o_DUMMY(void);
#define RECV _TIE_portqueue1i1o_RECV
#define SEND _TIE_portqueue1i1o_SEND
#define IS_FIFOEMPTY _TIE_portqueue1i1o_IS_FIFOEMPTY
#define IS_FIFOFULL _TIE_portqueue1i1o_IS_FIFOFULL
#define DUMMY _TIE_portqueue1i1o_DUMMY

#endif /* __XCC__ */

#endif /* __XTENSA__ */
#endif /* !_XTENSA_portqueue1i1o_HEADER */
