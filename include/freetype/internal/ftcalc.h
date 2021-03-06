/***************************************************************************/
/*                                                                         */
/*  ftcalc.h                                                               */
/*                                                                         */
/*    Arithmetic computations (specification).                             */
/*                                                                         */
/*  Copyright 1996-2001, 2002 by                                           */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __FTCALC_H__
#define __FTCALC_H__



#include <freetype/freetype.h>


#ifdef __cplusplus
extern "C" {
#endif


  FT_EXPORT( FT_Int32 )  FT_SqrtFixed( FT_Int32  x );


#define SQRT_32( x )  FT_Sqrt32( x )


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Sqrt32                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Computes the square root of an Int32 integer (which will be        */
  /*    handled as an unsigned long value).                                */
  /*                                                                       */
  /* <Input>                                                               */
  /*    x :: The value to compute the root for.                            */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The result of `sqrt(x)'.                                           */
  /*                                                                       */
  FT_EXPORT( FT_Int32 )
  FT_Sqrt32( FT_Int32  x );


  /*************************************************************************/
  /*                                                                       */
  /* FT_MulDiv() and FT_MulFix() are declared in freetype.h.               */
  /*                                                                       */
  /*************************************************************************/


#define INT_TO_F26DOT6( x )    ( (FT_Long)(x) << 6  )
#define INT_TO_F2DOT14( x )    ( (FT_Long)(x) << 14 )
#define INT_TO_FIXED( x )      ( (FT_Long)(x) << 16 )
#define F2DOT14_TO_FIXED( x )  ( (FT_Long)(x) << 2  )
#define FLOAT_TO_FIXED( x )    ( (FT_Long)( x * 65536.0 ) )

#define ROUND_F26DOT6( x )     ( x >= 0 ? (    ( (x) + 32 ) & -64 )     \
                                        : ( -( ( 32 - (x) ) & -64 ) ) )


#ifdef __cplusplus
}
#endif

#endif /* __FTCALC_H__ */


/* END */
