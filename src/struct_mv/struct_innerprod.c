/*BHEADER**********************************************************************
 * Copyright (c) 2008,  Lawrence Livermore National Security, LLC.
 * Produced at the Lawrence Livermore National Laboratory.
 * This file is part of HYPRE.  See file COPYRIGHT for details.
 *
 * HYPRE is free software; you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License (as published by the Free
 * Software Foundation) version 2.1 dated February 1999.
 *
 * $Revision$
 ***********************************************************************EHEADER*/

/******************************************************************************
 *
 * Structured inner product routine
 *
 *****************************************************************************/

#include "_hypre_struct_mv.h"

/*--------------------------------------------------------------------------
 * hypre_StructInnerProd
 * *compute only the real part*; for full complex, use ComplexInnerProd
 *--------------------------------------------------------------------------*/

HYPRE_Real
hypre_StructInnerProd( hypre_StructVector *x,
                       hypre_StructVector *y )
{
   HYPRE_Real       final_innerprod_result;
   HYPRE_Real       local_result;
   HYPRE_Real       process_result;
                   
   hypre_Box       *x_data_box;
   hypre_Box       *y_data_box;
                   
   HYPRE_Int        xi;
   HYPRE_Int        yi;
                   
   HYPRE_Complex   *xp;
   HYPRE_Complex   *yp;
                   
   hypre_BoxArray  *boxes;
   hypre_Box       *box;
   hypre_Index      loop_size;
   hypre_IndexRef   start;
   hypre_Index      unit_stride;
                   
   HYPRE_Int        i;

   local_result = 0.0;
   process_result = 0.0;

   hypre_SetIndex(unit_stride, 1);

   boxes = hypre_StructGridBoxes(hypre_StructVectorGrid(y));
   hypre_ForBoxI(i, boxes)
   {
      box   = hypre_BoxArrayBox(boxes, i);
      start = hypre_BoxIMin(box);

      x_data_box = hypre_BoxArrayBox(hypre_StructVectorDataSpace(x), i);
      y_data_box = hypre_BoxArrayBox(hypre_StructVectorDataSpace(y), i);

      xp = hypre_StructVectorBoxData(x, i);
      yp = hypre_StructVectorBoxData(y, i);

      hypre_BoxGetSize(box, loop_size);

      hypre_BoxLoop2Begin(hypre_StructVectorNDim(x), loop_size,
                          x_data_box, start, unit_stride, xi,
                          y_data_box, start, unit_stride, yi);
#ifdef HYPRE_USING_OPENMP
#pragma omp parallel for private(HYPRE_BOX_PRIVATE,xi,yi) reduction(+:local_result) HYPRE_SMP_SCHEDULE
#endif
      hypre_BoxLoop2For(xi, yi)
      {
#ifdef HYPRE_COMPLEX      
         local_result += hypre_creal(xp[xi]) * hypre_creal(yp[yi])
                       + hypre_cimag(xp[xi]) * hypre_cimag(yp[yi]);
#else
         local_result += xp[xi] * yp[yi];
#endif
      }
      hypre_BoxLoop2End(xi, yi);
   }
   process_result = local_result;

   // XXX HYPRE_MPI_COMPLEX = MPI_C_DOUBLE_COMPLEX doesn't work here, so changed
   // HYPRE_MPI_COMPLEX to MPI_LONG_DOUBLE
   hypre_MPI_Allreduce(&process_result, &final_innerprod_result, 1,
                       HYPRE_MPI_COMPLEX, hypre_MPI_SUM, hypre_StructVectorComm(x));

#ifdef HYPRE_COMPLEX
   hypre_IncFLOPCount(4*hypre_StructVectorGlobalSize(x));
#else
   hypre_IncFLOPCount(2*hypre_StructVectorGlobalSize(x));
#endif

   return final_innerprod_result;
}

/*--------------------------------------------------------------------------
 * hypre_StructComplexInnerProd
 *--------------------------------------------------------------------------*/

HYPRE_Complex
hypre_StructComplexInnerProd( hypre_StructVector *x,
                              hypre_StructVector *y )
{
   HYPRE_Complex   final_innerprod_result;
   HYPRE_Complex   local_result;
   HYPRE_Complex   process_result;
                   
   hypre_Box       *x_data_box;
   hypre_Box       *y_data_box;
                   
   HYPRE_Int        xi;
   HYPRE_Int        yi;
                   
   HYPRE_Complex   *xp;
   HYPRE_Complex   *yp;
                   
   hypre_BoxArray  *boxes;
   hypre_Box       *box;
   hypre_Index      loop_size;
   hypre_IndexRef   start;
   hypre_Index      unit_stride;
                   
   HYPRE_Int        i;

   local_result = 0.0;
   process_result = 0.0;

   hypre_SetIndex(unit_stride, 1);

   boxes = hypre_StructGridBoxes(hypre_StructVectorGrid(y));
   hypre_ForBoxI(i, boxes)
   {
      box   = hypre_BoxArrayBox(boxes, i);
      start = hypre_BoxIMin(box);

      x_data_box = hypre_BoxArrayBox(hypre_StructVectorDataSpace(x), i);
      y_data_box = hypre_BoxArrayBox(hypre_StructVectorDataSpace(y), i);

      xp = hypre_StructVectorBoxData(x, i);
      yp = hypre_StructVectorBoxData(y, i);

      hypre_BoxGetSize(box, loop_size);

      hypre_BoxLoop2Begin(hypre_StructVectorNDim(x), loop_size,
                          x_data_box, start, unit_stride, xi,
                          y_data_box, start, unit_stride, yi);
#ifdef HYPRE_USING_OPENMP
#pragma omp parallel for private(HYPRE_BOX_PRIVATE,xi,yi) reduction(+:local_result) HYPRE_SMP_SCHEDULE
#endif
      hypre_BoxLoop2For(xi, yi)
      {
         local_result += xp[xi] * hypre_conj(yp[yi]);
      }
      hypre_BoxLoop2End(xi, yi);
   }
   process_result = local_result;

   // XXX HYPRE_MPI_COMPLEX = MPI_C_DOUBLE_COMPLEX doesn't work here, so changed
   // HYPRE_MPI_COMPLEX to MPI_LONG_DOUBLE
   hypre_MPI_Allreduce(&process_result, &final_innerprod_result, 1,
                       HYPRE_MPI_COMPLEX, hypre_MPI_SUM, hypre_StructVectorComm(x));

   hypre_IncFLOPCount(8*hypre_StructVectorGlobalSize(x));

   return final_innerprod_result;
}
