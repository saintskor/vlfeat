/** @internal
 ** @file    vl_imintegral.c
 ** @author  Andrea Vedaldi
 ** @brief   vl_imintegral MEX definition
 **/

/* AUTORIGHTS
 Copyright (C) 2007-09 Andrea Vedaldi and Brian Fulkerson

 This file is part of VLFeat, available under the terms of the
 GNU GPLv2, or (at your option) any later version.
 */

#include <mexutils.h>

#include <vl/generic.h>
#include <vl/mathop.h>


void
vl_image_distance_transform (double const * im,
                             vl_size numColumns,
                             vl_size numRows,
                             vl_size columnStride,
                             vl_size rowStride,
                             double * dt,
                             vl_uindex * indexes,
                             double coeff,
                             double offset)
{
  /* Each image pixel corresponds to a parabola. The algorithm scans
     such parabola from the left to the right, keeping track of all
     the parabolas in the lower envelop and their intersection points.
     There are NUM active parabolas, the intersections are in FROM
     and WHICH stores the seed (point x) of that parabola.
  */
  vl_uindex x, y ;
  double * from = vl_malloc (sizeof(double) * (numColumns + 1)) ;
  double * base = vl_malloc (sizeof(double) * numColumns) ;
  vl_uindex * baseIndexes = vl_malloc (sizeof(vl_uindex) * numColumns) ;
  vl_uindex * which = vl_malloc (sizeof(vl_uindex) * numColumns) ;
  vl_uindex num = 0 ;

  for (y = 0 ; y < numRows ; ++y) {
    num = 0 ;
    for (x = 0 ; x < numColumns ; ++x) {
      double r = im[x  * columnStride + y * rowStride] ;
      double x2 = x * x ;
      double from_ = - VL_INFINITY_D ;

      /*
         Add next parabola (there are NUM so far).
         The algorithm finds intersection INTERS with the previously
         added parabola. If the intersection is on the right of
         the "starting point" of this parabola, then the previous parabola
         is kept, and the new one is added to its right. Otherwise
         the new parabola "eats" the old one, that gets deleted and the
         next parabola is checked.
       */

      while (num >= 1) {
        vl_uindex x_ = which[num - 1] ;
        double x2_ = x_ * x_ ;
        double r_ = im[x_ * columnStride + y * rowStride] ;
        double inters = ((r - r_) + coeff * (x2 - x2_)) / (x - x_) / (2*coeff) - offset;

        if (inters <= from [num - 1]) {
          /* delete a previous parabola */
          -- num ;
        } else {
          /* accept intersection */
          from_ = inters ;
          break ;
        }
      }

      /* add a new parabola */
      which[num] = x ;
      from[num] = from_ ;
      base[num] = r ;
      if (indexes) baseIndexes[num] = indexes[x  * columnStride + y * rowStride] ;
      num ++ ;
    } /* next column */

    from[num] = VL_INFINITY_D ;

    /* fill in */
    num = 0 ;
    for (x = 0 ; x < numColumns ; ++x) {
      double delta ;
      while (x >= from[num + 1]) ++ num ;
      delta = (double) x - (double) which[num] + offset ;
      dt[x  * columnStride + y * rowStride]
      = base[num] + coeff * delta * delta ;
      if (indexes) {
        indexes[x  * columnStride + y * rowStride]
        = baseIndexes[num] ;
      }
    }
  } /* next row */

  vl_free (from) ;
  vl_free (which) ;
  vl_free (base) ;
}

void
mexFunction(int nout, mxArray *out[],
            int nin, const mxArray *in[])
{
  vl_size M, N ;
  enum {IN_I = 0, IN_PARAM, IN_END} ;
  enum {OUT_DT = 0, OUT_INDEXES} ;
  vl_uindex * indexes = NULL ;
  double const defaultParam [] = {1.0, 0.0, 1.0, 0.0} ;
  double const * param = defaultParam ;

  /* -----------------------------------------------------------------
   *                                               Check the arguments
   * -------------------------------------------------------------- */

  if (nin < 1) {
    mxuError(vlmxErrInvalidArgument,
             "At least an argument is needed.") ;
  }
  if (nin > 2) {
    mxuError(vlmxErrInvalidArgument,
             "At most two arguments are allowed.") ;
  }
  if (nout > 2) {
    mxuError(vlmxErrInvalidArgument,
             "Too many outout arguments.") ;
  }
  if (! vlmxIsPlainMatrix(IN(I), -1, -1)) {
    mxuError(vlmxErrInvalidArgument,
             "I must be a plain matrix.") ;
  }
  if (nin == 2) {
    if (! vlmxIsPlainVector(IN(PARAM), 4)) {
      mxuError(vlmxErrInvalidArgument,
               "PARAM must be a 4-dimensional vector.") ;
    }
    param = mxGetPr (IN(PARAM)) ;
  }

  M = mxGetM (IN(I)) ;
  N = mxGetN (IN(I)) ;

  OUT(DT) = mxCreateDoubleMatrix (M, N, mxREAL) ;
  if (nout > 1) {
    int i ;
    OUT(INDEXES) = mxCreateDoubleMatrix (M, N, mxREAL) ;
    indexes = mxMalloc(sizeof(vl_uindex) * M * N) ;
    for (i = 0 ; i < M * N ; ++i) indexes[i] = i + 1 ;
  }

  /* -----------------------------------------------------------------
   *                                                        Do the job
   * -------------------------------------------------------------- */

  vl_image_distance_transform(mxGetPr(IN(I)),
                              M, N,
                              1, M,
                              mxGetPr(OUT(DT)),
                              indexes,
                              param[2],
                              param[3]) ;

  vl_image_distance_transform(mxGetPr(OUT(DT)),
                              N, M,
                              M, 1,
                              mxGetPr(OUT(DT)),
                              indexes,
                              param[0],
                              param[1]) ;

  if (indexes) {
    int i ;
    double * pt = mxGetPr(OUT(INDEXES)) ;
    for (i = 0 ; i < M * N ; ++i) pt[i] = indexes[i] ;
    mxFree(indexes) ;
  }
}
