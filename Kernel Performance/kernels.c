/*******************************************
 * Solutions for the CS:APP Performance Lab
 ********************************************/

#include <stdio.h>
#include <stdlib.h>
#include "defs.h"

#define TILE 8  // Tile size for loop tiling.

 /*
  * Please fill in the following student struct
  */
student_t student = {
  "Scott Crowley",     /* Full name */
  "scott@utah.edu",  /* Email address */
};

/***************
 * COMPLEX KERNEL
 ***************/

 /******************************************************
  * Your different versions of the complex kernel go here
  ******************************************************/

  /*
   * my_complex_v1 - Ver. 1 of optimizing complex kernel.
   * Reduced the number of calls to src.
   * 1.2 Speedup
   */
//char my_complex_v1_descr[] = "my_complex_v1: 1st attempt at optimizing complex";
//void my_complex_v1(int dim, pixel* src, pixel* dest)
//{
//    int i, j;
//
//    for (i = 0; i < dim; i++)
//        for (j = 0; j < dim; j++)
//        {
//            pixel next = src[RIDX(i, j, dim)];
//            unsigned int val = (unsigned int)next.red + (unsigned int)next.green + (unsigned int)next.blue;
//            val = val / 3;
//            int index = RIDX(dim - j - 1, dim - i - 1, dim);
//            dest[index].red = (unsigned short)val;
//            dest[index].green = (unsigned short)val;
//            dest[index].blue = (unsigned short)val;
//        }
//}


/*
 * my_complex_v2 - Ver. 2 of optimizing complex kernel.
 * No calls to RIDX and moved partial index calculations out of the inner for-loop.
 * 1.2 Speedup (Why no gain? RIDX is a macro that's inlined and the compiler is
 *                                    likely already making these optimizations.)
 */
//char my_complex_v2_descr[] = "my_complex_v2: 2nd attempt at optimizing complex";
//void my_complex_v2(int dim, pixel* src, pixel* dest)
//{
//    int i, j;
//    int lastIdx = dim * dim - 1;
//
//    for (i = 0; i < dim; i++)
//    {
//        int srcIdx = i * dim;
//        int intIdx = lastIdx - i;
//        for (j = 0; j < dim; j++)
//        {   
//            pixel next = src[srcIdx + j];
//            unsigned int val = (unsigned int)next.red + (unsigned int)next.green + (unsigned int)next.blue;
//            val = val / 3;
//            int destIdx = intIdx - (j * dim);
//            dest[destIdx].red = val;
//            dest[destIdx].green = val;
//            dest[destIdx].blue = val;
//        }
//    }
//}


/*
 * my_complex_v3 - Ver. 3 of optimizing complex kernel.
 * Loop tiling attempt.
 * 1.6 Speedup (w/ tile size of 8)
 */
//char my_complex_v3_descr[] = "my_complex_v3: 3rd attempt at optimizing complex";
//void my_complex_v3(int dim, pixel* src, pixel* dest)
//{
//    int i, j, ii, jj;
//
//    for (i = 0; i < dim; i += TILE)
//        for (j = 0; j < dim; j += TILE)
//            for (ii = i; ii < i + TILE; ii++)
//                for (jj = j; jj < j + TILE; jj++)
//                {
//                    pixel next = src[RIDX(ii, jj, dim)];
//                    unsigned int val = (unsigned int)next.red + (unsigned int)next.green + (unsigned int)next.blue;
//                    val = val / 3;
//                    int index = RIDX(dim - jj - 1, dim - ii - 1, dim);
//                    dest[index].red = (unsigned short)val;
//                    dest[index].green = (unsigned short)val;
//                    dest[index].blue = (unsigned short)val;
//                }
//}


/*
 * naive_complex - The naive baseline version of complex
 */
char naive_complex_descr[] = "naive_complex: Naive baseline implementation";
void naive_complex(int dim, pixel* src, pixel* dest)
{
    int i, j;

    for (i = 0; i < dim; i++)
        for (j = 0; j < dim; j++)
        {

            dest[RIDX(dim - j - 1, dim - i - 1, dim)].red = ((int)src[RIDX(i, j, dim)].red +
                (int)src[RIDX(i, j, dim)].green +
                (int)src[RIDX(i, j, dim)].blue) / 3;

            dest[RIDX(dim - j - 1, dim - i - 1, dim)].green = ((int)src[RIDX(i, j, dim)].red +
                (int)src[RIDX(i, j, dim)].green +
                (int)src[RIDX(i, j, dim)].blue) / 3;

            dest[RIDX(dim - j - 1, dim - i - 1, dim)].blue = ((int)src[RIDX(i, j, dim)].red +
                (int)src[RIDX(i, j, dim)].green +
                (int)src[RIDX(i, j, dim)].blue) / 3;

        }
}


/*
 * complex - Your current working version of complex
 * IMPORTANT: This is the version you will be graded on
 */
char complex_descr[] = "complex: Current working version";
void complex(int dim, pixel* src, pixel* dest)
{
    int i, j, ii, jj;

    for (i = 0; i < dim; i += TILE)
        for (j = 0; j < dim; j += TILE)
            for (ii = i; ii < i + TILE; ii++)
                for (jj = j; jj < j + TILE; jj++)
                {
                    pixel next = src[RIDX(ii, jj, dim)];
                    unsigned int val = (unsigned int)next.red + (unsigned int)next.green + (unsigned int)next.blue;
                    val = val / 3;
                    int index = RIDX(dim - jj - 1, dim - ii - 1, dim);
                    dest[index].red = (unsigned short)val;
                    dest[index].green = (unsigned short)val;
                    dest[index].blue = (unsigned short)val;
                }
}

/*********************************************************************
 * register_complex_functions - Register all of your different versions
 *     of the complex kernel with the driver by calling the
 *     add_complex_function() for each test function. When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.
 *********************************************************************/

void register_complex_functions() {
    add_complex_function(&complex, complex_descr);
    add_complex_function(&naive_complex, naive_complex_descr);
    //add_complex_function(&my_complex_v1, my_complex_v1_descr);
    //add_complex_function(&my_complex_v2, my_complex_v2_descr);
    //add_complex_function(&my_complex_v3, my_complex_v3_descr);
}


/***************
 * MOTION KERNEL
 **************/

 /***************************************************************
  * Various helper functions for the motion kernel
  * You may modify these or add new ones any way you like.
  **************************************************************/


  /*
   * weighted_combo - Returns new pixel value at (i,j)
   */
static pixel weighted_combo(int dim, int i, int j, pixel* src)
{
    int ii, jj;
    pixel current_pixel;

    int red, green, blue;
    red = green = blue = 0;

    int num_neighbors = 0;
    for (ii = 0; ii < 3; ii++)
        for (jj = 0; jj < 3; jj++)
            if ((i + ii < dim) && (j + jj < dim))
            {
                num_neighbors++;
                red += (int)src[RIDX(i + ii, j + jj, dim)].red;
                green += (int)src[RIDX(i + ii, j + jj, dim)].green;
                blue += (int)src[RIDX(i + ii, j + jj, dim)].blue;
            }

    current_pixel.red = (unsigned short)(red / num_neighbors);
    current_pixel.green = (unsigned short)(green / num_neighbors);
    current_pixel.blue = (unsigned short)(blue / num_neighbors);

    return current_pixel;
}



/******************************************************
 * Your different versions of the motion kernel go here
 ******************************************************/

 /*
  * my_motion_v1 - Ver. 1 of optimizing blurring kernel.
  * The if-statement in the nested for-loop is costly. Let's try and deal with
  * the border pixels w/o any conditionals.
  * ERROR ON EVERY PIXEL. TOOK AVERAGE BASED ON CENTER PIXEL. :(
  */
//char my_motion_v1_descr[] = "my_motion_v1: 1st attempt at optimizing motion";
//void my_motion_v1(int dim, pixel* src, pixel* dst)
//{
//    pixel work;
//    pixel* workPtr = &work;
//
//    int i = 0;
//    int j = 0;         // Top-Left Pixel
//    workPtr -> red   = (unsigned short)(((unsigned int)src[RIDX(0, 0, dim)].red
//                                       + (unsigned int)src[RIDX(0, 1, dim)].red
//                                       + (unsigned int)src[RIDX(1, 0, dim)].red
//                                       + (unsigned int)src[RIDX(1, 1, dim)].red) / 4);
//	workPtr -> green = (unsigned short)(((unsigned int)src[RIDX(0, 0, dim)].green
//								       + (unsigned int)src[RIDX(0, 1, dim)].green
//									   + (unsigned int)src[RIDX(1, 0, dim)].green
//									   + (unsigned int)src[RIDX(1, 1, dim)].green) / 4);
//    workPtr -> blue  = (unsigned short)(((unsigned int)src[RIDX(0, 0, dim)].blue
//                                       + (unsigned int)src[RIDX(0, 1, dim)].blue
//                                       + (unsigned int)src[RIDX(1, 0, dim)].blue
//                                       + (unsigned int)src[RIDX(1, 1, dim)].blue) / 4);
//    dst[RIDX(i, j, dim)] = work;
//
//    i = 0;
//    j = dim - 1;       // Top-Right Pixel
//    workPtr -> red   = (unsigned short)(((unsigned int)src[RIDX(i  , j,   dim)].red
//                                       + (unsigned int)src[RIDX(i+1, j,   dim)].red
//                                       + (unsigned int)src[RIDX(i  , j-1, dim)].red
//                                       + (unsigned int)src[RIDX(i+1, j-1, dim)].red) / 4);
//    workPtr -> green = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].green
//                                       + (unsigned int)src[RIDX(i+1, j,   dim)].green
//                                       + (unsigned int)src[RIDX(i,   j-1, dim)].green
//                                       + (unsigned int)src[RIDX(i+1, j-1, dim)].green) / 4);
//    workPtr -> blue  = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].blue
//                                       + (unsigned int)src[RIDX(i+1, j,   dim)].blue
//                                       + (unsigned int)src[RIDX(i,   j-1, dim)].blue
//                                       + (unsigned int)src[RIDX(i+1, j-1, dim)].blue) / 4);
//    dst[RIDX(i, j, dim)] = work;
//
//    i = dim - 1;
//    j = 0;             // Bottom-Left Pixel
//    workPtr -> red   = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].red
//                                       + (unsigned int)src[RIDX(i-1, j,   dim)].red
//                                       + (unsigned int)src[RIDX(i,   j+1, dim)].red
//                                       + (unsigned int)src[RIDX(i-1, j+1, dim)].red) / 4);
//    workPtr -> green = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].green
//                                       + (unsigned int)src[RIDX(i-1, j,   dim)].green
//                                       + (unsigned int)src[RIDX(i,   j+1, dim)].green
//                                       + (unsigned int)src[RIDX(i-1, j+1, dim)].green) / 4);
//    workPtr -> blue  = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].blue
//                                       + (unsigned int)src[RIDX(i-1, j,   dim)].blue
//                                       + (unsigned int)src[RIDX(i,   j+1, dim)].blue
//                                       + (unsigned int)src[RIDX(i-1, j+1, dim)].blue) / 4);
//    dst[RIDX(i, j, dim)] = work;
//
//    i = dim - 1;
//    j = dim - 1;       // Bottom-Right Pixel
//    workPtr -> red   = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].red
//                                       + (unsigned int)src[RIDX(i-1, j,   dim)].red
//                                       + (unsigned int)src[RIDX(i,   j-1, dim)].red
//                                       + (unsigned int)src[RIDX(i-1, j-1, dim)].red) / 4);
//    workPtr -> green = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].green
//                                       + (unsigned int)src[RIDX(i-1, j,   dim)].green
//                                       + (unsigned int)src[RIDX(i,   j-1, dim)].green
//                                       + (unsigned int)src[RIDX(i-1, j-1, dim)].green) / 4);
//    workPtr -> blue  = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].blue
//                                       + (unsigned int)src[RIDX(i-1, j,   dim)].blue
//                                       + (unsigned int)src[RIDX(i,   j-1, dim)].blue
//                                       + (unsigned int)src[RIDX(i-1, j-1, dim)].blue) / 4);
//    dst[RIDX(i, j, dim)] = work;
//
//    i = 0;
//    for (j = 1; j < dim - 1; j++) // Top Row
//    {
//        workPtr -> red   = (unsigned short)(((unsigned int)src[RIDX(i,   j-1, dim)].red
//                                           + (unsigned int)src[RIDX(i,   j,   dim)].red
//                                           + (unsigned int)src[RIDX(i,   j+1, dim)].red
//                                           + (unsigned int)src[RIDX(i+1, j-1, dim)].red
//                                           + (unsigned int)src[RIDX(i+1, j,   dim)].red
//                                           + (unsigned int)src[RIDX(i+1, j+1, dim)].red) / 6);
//        workPtr -> green = (unsigned short)(((unsigned int)src[RIDX(i,   j-1, dim)].green
//                                           + (unsigned int)src[RIDX(i,   j,   dim)].green
//                                           + (unsigned int)src[RIDX(i,   j+1, dim)].green
//                                           + (unsigned int)src[RIDX(i+1, j-1, dim)].green
//                                           + (unsigned int)src[RIDX(i+1, j,   dim)].green
//                                           + (unsigned int)src[RIDX(i+1, j+1, dim)].green) / 6);
//        workPtr -> blue  = (unsigned short)(((unsigned int)src[RIDX(i,   j-1, dim)].blue
//                                           + (unsigned int)src[RIDX(i,   j,   dim)].blue
//                                           + (unsigned int)src[RIDX(i,   j+1, dim)].blue
//                                           + (unsigned int)src[RIDX(i+1, j-1, dim)].blue
//                                           + (unsigned int)src[RIDX(i+1, j,   dim)].blue
//                                           + (unsigned int)src[RIDX(i+1, j+1, dim)].blue) / 6);
//        dst[RIDX(i, j, dim)] = work;
//    }
//
//    j = 0;
//    for (i = 1; i < dim - 1; i++) // Left Column
//    {
//        workPtr -> red   = (unsigned short)(((unsigned int)src[RIDX(i-1, j,   dim)].red
//                                           + (unsigned int)src[RIDX(i-1, j+1, dim)].red
//                                           + (unsigned int)src[RIDX(i,   j,   dim)].red
//                                           + (unsigned int)src[RIDX(i,   j+1, dim)].red
//                                           + (unsigned int)src[RIDX(i+1, j,   dim)].red
//                                           + (unsigned int)src[RIDX(i+1, j+1, dim)].red) / 6);
//        workPtr -> green = (unsigned short)(((unsigned int)src[RIDX(i-1, j,   dim)].green
//                                           + (unsigned int)src[RIDX(i-1, j+1, dim)].green
//                                           + (unsigned int)src[RIDX(i,   j,   dim)].green
//                                           + (unsigned int)src[RIDX(i,   j+1, dim)].green
//                                           + (unsigned int)src[RIDX(i+1, j,   dim)].green
//                                           + (unsigned int)src[RIDX(i+1, j+1, dim)].green) / 6);
//        workPtr -> blue  = (unsigned short)(((unsigned int)src[RIDX(i-1, j,   dim)].blue
//                                           + (unsigned int)src[RIDX(i-1, j+1, dim)].blue
//                                           + (unsigned int)src[RIDX(i,   j,   dim)].blue
//                                           + (unsigned int)src[RIDX(i,   j+1, dim)].blue
//                                           + (unsigned int)src[RIDX(i+1, j,   dim)].blue
//                                           + (unsigned int)src[RIDX(i+1, j+1, dim)].blue) / 6);
//        dst[RIDX(i, j, dim)] = work;
//    }
//
//    i = dim - 1;
//    for (j = 1; j < dim - 1; j++) // Bottom Row
//    {
//        workPtr -> red   = (unsigned short)(((unsigned int)src[RIDX(i-1, j-1, dim)].red
//                                           + (unsigned int)src[RIDX(i-1, j,   dim)].red
//                                           + (unsigned int)src[RIDX(i-1, j+1, dim)].red
//                                           + (unsigned int)src[RIDX(i,   j-1, dim)].red
//                                           + (unsigned int)src[RIDX(i,   j,   dim)].red
//                                           + (unsigned int)src[RIDX(i,   j+1, dim)].red) / 6);
//        workPtr -> green = (unsigned short)(((unsigned int)src[RIDX(i-1, j-1, dim)].green
//                                           + (unsigned int)src[RIDX(i-1, j,   dim)].green
//                                           + (unsigned int)src[RIDX(i-1, j+1, dim)].green
//                                           + (unsigned int)src[RIDX(i,   j-1, dim)].green
//                                           + (unsigned int)src[RIDX(i,   j,   dim)].green
//                                           + (unsigned int)src[RIDX(i,   j+1, dim)].green) / 6);
//        workPtr -> blue  = (unsigned short)(((unsigned int)src[RIDX(i-1, j-1, dim)].blue
//                                           + (unsigned int)src[RIDX(i-1, j,   dim)].blue
//                                           + (unsigned int)src[RIDX(i-1, j+1, dim)].blue
//                                           + (unsigned int)src[RIDX(i,   j-1, dim)].blue
//                                           + (unsigned int)src[RIDX(i,   j,   dim)].blue
//                                           + (unsigned int)src[RIDX(i,   j+1, dim)].blue) / 6);
//        dst[RIDX(i, j, dim)] = work;
//    }
//
//    j = dim - 1;
//    for (i = 1; i < dim - 1; i++) // Right Column
//    {
//        workPtr -> red   = (unsigned short)(((unsigned int)src[RIDX(i-1, j-1, dim)].red
//                                           + (unsigned int)src[RIDX(i-1, j,   dim)].red
//                                           + (unsigned int)src[RIDX(i,   j-1, dim)].red
//                                           + (unsigned int)src[RIDX(i,   j,   dim)].red
//                                           + (unsigned int)src[RIDX(i+1, j-1, dim)].red
//                                           + (unsigned int)src[RIDX(i+1, j,   dim)].red) / 6);
//        workPtr -> green = (unsigned short)(((unsigned int)src[RIDX(i-1, j-1, dim)].green
//                                           + (unsigned int)src[RIDX(i-1, j,   dim)].green
//                                           + (unsigned int)src[RIDX(i,   j-1, dim)].green
//                                           + (unsigned int)src[RIDX(i,   j,   dim)].green
//                                           + (unsigned int)src[RIDX(i+1, j-1, dim)].green
//                                           + (unsigned int)src[RIDX(i+1, j,   dim)].green) / 6);
//        workPtr -> blue  = (unsigned short)(((unsigned int)src[RIDX(i-1, j-1, dim)].blue
//                                           + (unsigned int)src[RIDX(i-1, j,   dim)].blue
//                                           + (unsigned int)src[RIDX(i,   j-1, dim)].blue
//                                           + (unsigned int)src[RIDX(i,   j,   dim)].blue
//                                           + (unsigned int)src[RIDX(i+1, j-1, dim)].blue
//                                           + (unsigned int)src[RIDX(i+1, j,   dim)].blue) / 6);
//        dst[RIDX(i, j, dim)] = work;
//    }
//
//    for (i = 1; i < dim - 1; i++) // Interior Pixels
//        for (j = 1; j < dim - 1; j++)
//        {
//            workPtr -> red   = (unsigned short)(((unsigned int)src[RIDX(i-1, j-1, dim)].red
//                                               + (unsigned int)src[RIDX(i-1, j,   dim)].red
//                                               + (unsigned int)src[RIDX(i-1, j+1, dim)].red
//                                               + (unsigned int)src[RIDX(i,   j-1, dim)].red
//                                               + (unsigned int)src[RIDX(i,   j,   dim)].red
//                                               + (unsigned int)src[RIDX(i,   j+1, dim)].red
//                                               + (unsigned int)src[RIDX(i+1, j-1, dim)].red
//                                               + (unsigned int)src[RIDX(i+1, j,   dim)].red
//                                               + (unsigned int)src[RIDX(i+1, j+1, dim)].red) / 9);
//
//            workPtr -> green = (unsigned short)(((unsigned int)src[RIDX(i-1, j-1, dim)].green
//                                               + (unsigned int)src[RIDX(i-1, j,   dim)].green
//                                               + (unsigned int)src[RIDX(i-1, j+1, dim)].green
//                                               + (unsigned int)src[RIDX(i,   j-1, dim)].green
//                                               + (unsigned int)src[RIDX(i,   j,   dim)].green
//                                               + (unsigned int)src[RIDX(i,   j+1, dim)].green
//                                               + (unsigned int)src[RIDX(i+1, j-1, dim)].green
//                                               + (unsigned int)src[RIDX(i+1, j,   dim)].green
//                                               + (unsigned int)src[RIDX(i+1, j+1, dim)].green) / 9);
//
//            workPtr -> blue  = (unsigned short)(((unsigned int)src[RIDX(i-1, j-1, dim)].blue
//                                               + (unsigned int)src[RIDX(i-1, j,   dim)].blue
//                                               + (unsigned int)src[RIDX(i-1, j+1, dim)].blue
//                                               + (unsigned int)src[RIDX(i,   j-1, dim)].blue
//                                               + (unsigned int)src[RIDX(i,   j,   dim)].blue
//                                               + (unsigned int)src[RIDX(i,   j+1, dim)].blue
//                                               + (unsigned int)src[RIDX(i+1, j-1, dim)].blue
//                                               + (unsigned int)src[RIDX(i+1, j,   dim)].blue
//                                               + (unsigned int)src[RIDX(i+1, j+1, dim)].blue) / 9);
//
//            dst[RIDX(i, j, dim)] = work;
//        }
//}


 /*
  * my_motion_v2 - Ver. 2 of optimizing blurring kernel.
  * The if-statement in the nested for-loop is costly. Let's try and deal with
  * the border pixels w/o any conditionals. (Using top-left pixel for 3x3 block.)
  * 3.4 Speedup
  */
//char my_motion_v2_descr[] = "my_motion_v1: 2nd attempt at optimizing motion";
//void my_motion_v2(int dim, pixel* src, pixel* dst)
//{
//    pixel work;
//    pixel* workPtr = &work;
//
//    int i, j;
//    for (i = 0; i < dim - 2; i++) // Full 3x3 Pixels
//    {
//        for (j = 0; j < dim - 2; j++)
//        {
//            workPtr -> red   = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].red
//                                               + (unsigned int)src[RIDX(i,   j+1, dim)].red
//                                               + (unsigned int)src[RIDX(i,   j+2, dim)].red
//                                               + (unsigned int)src[RIDX(i+1, j,   dim)].red
//                                               + (unsigned int)src[RIDX(i+1, j+1, dim)].red
//                                               + (unsigned int)src[RIDX(i+1, j+2, dim)].red
//                                               + (unsigned int)src[RIDX(i+2, j,   dim)].red
//                                               + (unsigned int)src[RIDX(i+2, j+1, dim)].red
//                                               + (unsigned int)src[RIDX(i+2, j+2, dim)].red) / 9);
//
//            workPtr -> green = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].green
//                                               + (unsigned int)src[RIDX(i,   j+1, dim)].green
//                                               + (unsigned int)src[RIDX(i,   j+2, dim)].green
//                                               + (unsigned int)src[RIDX(i+1, j,   dim)].green
//                                               + (unsigned int)src[RIDX(i+1, j+1, dim)].green
//                                               + (unsigned int)src[RIDX(i+1, j+2, dim)].green
//                                               + (unsigned int)src[RIDX(i+2, j,   dim)].green
//                                               + (unsigned int)src[RIDX(i+2, j+1, dim)].green
//                                               + (unsigned int)src[RIDX(i+2, j+2, dim)].green) / 9);
//
//            workPtr -> blue  = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].blue
//                                               + (unsigned int)src[RIDX(i,   j+1, dim)].blue
//                                               + (unsigned int)src[RIDX(i,   j+2, dim)].blue
//                                               + (unsigned int)src[RIDX(i+1, j,   dim)].blue
//                                               + (unsigned int)src[RIDX(i+1, j+1, dim)].blue
//                                               + (unsigned int)src[RIDX(i+1, j+2, dim)].blue
//                                               + (unsigned int)src[RIDX(i+2, j,   dim)].blue
//                                               + (unsigned int)src[RIDX(i+2, j+1, dim)].blue
//                                               + (unsigned int)src[RIDX(i+2, j+2, dim)].blue) / 9);
//
//            dst[RIDX(i, j, dim)] = work;
//        }
//    }
//
//    i = dim - 2;
//    for (j = 0; j < dim - 2; j++) // 2nd From Bottom Row
//    {
//        workPtr -> red   = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].red
//                                           + (unsigned int)src[RIDX(i,   j+1, dim)].red
//                                           + (unsigned int)src[RIDX(i,   j+2, dim)].red
//                                           + (unsigned int)src[RIDX(i+1, j,   dim)].red
//                                           + (unsigned int)src[RIDX(i+1, j+1, dim)].red
//                                           + (unsigned int)src[RIDX(i+1, j+2, dim)].red) / 6);
//
//        workPtr -> green = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].green
//                                           + (unsigned int)src[RIDX(i,   j+1, dim)].green
//                                           + (unsigned int)src[RIDX(i,   j+2, dim)].green
//                                           + (unsigned int)src[RIDX(i+1, j,   dim)].green
//                                           + (unsigned int)src[RIDX(i+1, j+1, dim)].green
//                                           + (unsigned int)src[RIDX(i+1, j+2, dim)].green) / 6);
//
//        workPtr -> blue  = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].blue
//                                           + (unsigned int)src[RIDX(i,   j+1, dim)].blue
//                                           + (unsigned int)src[RIDX(i,   j+2, dim)].blue
//                                           + (unsigned int)src[RIDX(i+1, j,   dim)].blue
//                                           + (unsigned int)src[RIDX(i+1, j+1, dim)].blue
//                                           + (unsigned int)src[RIDX(i+1, j+2, dim)].blue) / 6);
//        dst[RIDX(i, j, dim)] = work;
//    }
//
//    i = dim - 1;
//    for (j = 0; j < dim - 2; j++) // Bottom Row
//    {
//        workPtr -> red   = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].red
//                                           + (unsigned int)src[RIDX(i,   j+1, dim)].red
//                                           + (unsigned int)src[RIDX(i,   j+2, dim)].red) / 3);
//
//        workPtr -> green = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].green
//                                           + (unsigned int)src[RIDX(i,   j+1, dim)].green
//                                           + (unsigned int)src[RIDX(i,   j+2, dim)].green) / 3);
//
//        workPtr -> blue  = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].blue
//                                           + (unsigned int)src[RIDX(i,   j+1, dim)].blue
//                                           + (unsigned int)src[RIDX(i,   j+2, dim)].blue) / 3);
//        dst[RIDX(i, j, dim)] = work;
//    }
//
//    j = dim - 2;
//    for (i = 0; i < dim - 2; i++) // 2nd From Right Column
//    {
//        workPtr -> red   = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].red
//                                           + (unsigned int)src[RIDX(i,   j+1, dim)].red
//                                           + (unsigned int)src[RIDX(i+1, j,   dim)].red
//                                           + (unsigned int)src[RIDX(i+1, j+1, dim)].red
//                                           + (unsigned int)src[RIDX(i+2, j,   dim)].red
//                                           + (unsigned int)src[RIDX(i+2, j+1, dim)].red) / 6);
//
//        workPtr -> green = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].green
//                                           + (unsigned int)src[RIDX(i,   j+1, dim)].green
//                                           + (unsigned int)src[RIDX(i+1, j,   dim)].green
//                                           + (unsigned int)src[RIDX(i+1, j+1, dim)].green
//                                           + (unsigned int)src[RIDX(i+2, j,   dim)].green
//                                           + (unsigned int)src[RIDX(i+2, j+1, dim)].green) / 6);
//
//        workPtr -> blue  = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].blue
//                                           + (unsigned int)src[RIDX(i,   j+1, dim)].blue
//                                           + (unsigned int)src[RIDX(i+1, j,   dim)].blue
//                                           + (unsigned int)src[RIDX(i+1, j+1, dim)].blue
//                                           + (unsigned int)src[RIDX(i+2, j,   dim)].blue
//                                           + (unsigned int)src[RIDX(i+2, j+1, dim)].blue) / 6);
//        dst[RIDX(i, j, dim)] = work;
//    }
//
//    j = dim - 1;
//    for (i = 0; i < dim - 2; i++) // Right Column
//    {
//        workPtr -> red   = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].red
//                                           + (unsigned int)src[RIDX(i+1, j,   dim)].red
//                                           + (unsigned int)src[RIDX(i+2, j,   dim)].red) / 3);
//
//        workPtr -> green = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].green
//                                           + (unsigned int)src[RIDX(i+1, j,   dim)].green
//                                           + (unsigned int)src[RIDX(i+2, j,   dim)].green) / 3);
//
//        workPtr -> blue  = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].blue
//                                           + (unsigned int)src[RIDX(i+1, j,   dim)].blue
//                                           + (unsigned int)src[RIDX(i+2, j,   dim)].blue) / 3);
//        dst[RIDX(i, j, dim)] = work;
//    }
//
//    i = dim - 2;
//    j = dim - 2;  // One Up & Left of Bottom-Right Pixel
//    workPtr -> red   = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].red
//                                       + (unsigned int)src[RIDX(i,   j+1, dim)].red
//                                       + (unsigned int)src[RIDX(i+1, j,   dim)].red
//                                       + (unsigned int)src[RIDX(i+1, j+1, dim)].red) / 4);
//    workPtr -> green = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].green
//                                       + (unsigned int)src[RIDX(i,   j+1, dim)].green
//                                       + (unsigned int)src[RIDX(i+1, j,   dim)].green
//                                       + (unsigned int)src[RIDX(i+1, j+1, dim)].green) / 4);
//    workPtr -> blue  = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].blue
//                                       + (unsigned int)src[RIDX(i,   j+1, dim)].blue
//                                       + (unsigned int)src[RIDX(i+1, j,   dim)].blue
//                                       + (unsigned int)src[RIDX(i+1, j+1, dim)].blue) / 4);
//    dst[RIDX(i, j, dim)] = work;
//
//    i = dim - 2;
//    j = dim - 1;  // One Above Bottom-Right Pixel
//    workPtr -> red   = (unsigned short)(((unsigned int)src[RIDX(i,   j, dim)].red
//                                       + (unsigned int)src[RIDX(i+1, j, dim)].red) / 2);
//    workPtr -> green = (unsigned short)(((unsigned int)src[RIDX(i,   j, dim)].green
//                                       + (unsigned int)src[RIDX(i+1, j, dim)].green) / 2);
//    workPtr -> blue  = (unsigned short)(((unsigned int)src[RIDX(i,   j, dim)].blue
//                                       + (unsigned int)src[RIDX(i+1, j, dim)].blue) / 2);
//    dst[RIDX(i, j, dim)] = work;
//
//    i = dim - 1;
//    j = dim - 2;  // One Left of Bottom-Right Pixel
//    workPtr -> red   = (unsigned short)(((unsigned int)src[RIDX(i, j,   dim)].red
//                                       + (unsigned int)src[RIDX(i, j+1, dim)].red) / 2);
//    workPtr -> green = (unsigned short)(((unsigned int)src[RIDX(i, j,   dim)].green
//                                       + (unsigned int)src[RIDX(i, j+1, dim)].green) / 2);
//    workPtr -> blue  = (unsigned short)(((unsigned int)src[RIDX(i, j,   dim)].blue
//                                       + (unsigned int)src[RIDX(i, j+1, dim)].blue) / 2);
//    dst[RIDX(i, j, dim)] = work;
//
//    i = dim - 1;
//    j = dim - 1;  // Bottom-Right Pixel
//    dst[RIDX(i, j, dim)] = src[RIDX(i, j, dim)];
//}

 /*
  * naive_motion - The naive baseline version of motion
  */
char naive_motion_descr[] = "naive_motion: Naive baseline implementation";
void naive_motion(int dim, pixel* src, pixel* dst)
{
    int i, j;

    for (i = 0; i < dim; i++)
        for (j = 0; j < dim; j++)
            dst[RIDX(i, j, dim)] = weighted_combo(dim, i, j, src);
}


/*
 * motion - Your current working version of motion.
 * IMPORTANT: This is the version you will be graded on
 */
char motion_descr[] = "motion: Current working version";
void motion(int dim, pixel* src, pixel* dst)
{
    pixel work;
    pixel* workPtr = &work;

    int i, j;
    for (i = 0; i < dim - 2; i++) // Full 3x3 Pixels
    {
        for (j = 0; j < dim - 2; j++)
        {
            workPtr -> red   = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].red
                                               + (unsigned int)src[RIDX(i,   j+1, dim)].red
                                               + (unsigned int)src[RIDX(i,   j+2, dim)].red
                                               + (unsigned int)src[RIDX(i+1, j,   dim)].red
                                               + (unsigned int)src[RIDX(i+1, j+1, dim)].red
                                               + (unsigned int)src[RIDX(i+1, j+2, dim)].red
                                               + (unsigned int)src[RIDX(i+2, j,   dim)].red
                                               + (unsigned int)src[RIDX(i+2, j+1, dim)].red
                                               + (unsigned int)src[RIDX(i+2, j+2, dim)].red) / 9);

            workPtr -> green = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].green
                                               + (unsigned int)src[RIDX(i,   j+1, dim)].green
                                               + (unsigned int)src[RIDX(i,   j+2, dim)].green
                                               + (unsigned int)src[RIDX(i+1, j,   dim)].green
                                               + (unsigned int)src[RIDX(i+1, j+1, dim)].green
                                               + (unsigned int)src[RIDX(i+1, j+2, dim)].green
                                               + (unsigned int)src[RIDX(i+2, j,   dim)].green
                                               + (unsigned int)src[RIDX(i+2, j+1, dim)].green
                                               + (unsigned int)src[RIDX(i+2, j+2, dim)].green) / 9);

            workPtr -> blue  = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].blue
                                               + (unsigned int)src[RIDX(i,   j+1, dim)].blue
                                               + (unsigned int)src[RIDX(i,   j+2, dim)].blue
                                               + (unsigned int)src[RIDX(i+1, j,   dim)].blue
                                               + (unsigned int)src[RIDX(i+1, j+1, dim)].blue
                                               + (unsigned int)src[RIDX(i+1, j+2, dim)].blue
                                               + (unsigned int)src[RIDX(i+2, j,   dim)].blue
                                               + (unsigned int)src[RIDX(i+2, j+1, dim)].blue
                                               + (unsigned int)src[RIDX(i+2, j+2, dim)].blue) / 9);

            dst[RIDX(i, j, dim)] = work;
        }
    }

    i = dim - 2;
    for (j = 0; j < dim - 2; j++) // 2nd From Bottom Row
    {
        workPtr -> red   = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].red
                                           + (unsigned int)src[RIDX(i,   j+1, dim)].red
                                           + (unsigned int)src[RIDX(i,   j+2, dim)].red
                                           + (unsigned int)src[RIDX(i+1, j,   dim)].red
                                           + (unsigned int)src[RIDX(i+1, j+1, dim)].red
                                           + (unsigned int)src[RIDX(i+1, j+2, dim)].red) / 6);

        workPtr -> green = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].green
                                           + (unsigned int)src[RIDX(i,   j+1, dim)].green
                                           + (unsigned int)src[RIDX(i,   j+2, dim)].green
                                           + (unsigned int)src[RIDX(i+1, j,   dim)].green
                                           + (unsigned int)src[RIDX(i+1, j+1, dim)].green
                                           + (unsigned int)src[RIDX(i+1, j+2, dim)].green) / 6);

        workPtr -> blue  = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].blue
                                           + (unsigned int)src[RIDX(i,   j+1, dim)].blue
                                           + (unsigned int)src[RIDX(i,   j+2, dim)].blue
                                           + (unsigned int)src[RIDX(i+1, j,   dim)].blue
                                           + (unsigned int)src[RIDX(i+1, j+1, dim)].blue
                                           + (unsigned int)src[RIDX(i+1, j+2, dim)].blue) / 6);
        dst[RIDX(i, j, dim)] = work;
    }

    i = dim - 1;
    for (j = 0; j < dim - 2; j++) // Bottom Row
    {
        workPtr -> red   = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].red
                                           + (unsigned int)src[RIDX(i,   j+1, dim)].red
                                           + (unsigned int)src[RIDX(i,   j+2, dim)].red) / 3);

        workPtr -> green = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].green
                                           + (unsigned int)src[RIDX(i,   j+1, dim)].green
                                           + (unsigned int)src[RIDX(i,   j+2, dim)].green) / 3);

        workPtr -> blue  = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].blue
                                           + (unsigned int)src[RIDX(i,   j+1, dim)].blue
                                           + (unsigned int)src[RIDX(i,   j+2, dim)].blue) / 3);
        dst[RIDX(i, j, dim)] = work;
    }

    j = dim - 2;
    for (i = 0; i < dim - 2; i++) // 2nd From Right Column
    {
        workPtr -> red   = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].red
                                           + (unsigned int)src[RIDX(i,   j+1, dim)].red
                                           + (unsigned int)src[RIDX(i+1, j,   dim)].red
                                           + (unsigned int)src[RIDX(i+1, j+1, dim)].red
                                           + (unsigned int)src[RIDX(i+2, j,   dim)].red
                                           + (unsigned int)src[RIDX(i+2, j+1, dim)].red) / 6);

        workPtr -> green = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].green
                                           + (unsigned int)src[RIDX(i,   j+1, dim)].green
                                           + (unsigned int)src[RIDX(i+1, j,   dim)].green
                                           + (unsigned int)src[RIDX(i+1, j+1, dim)].green
                                           + (unsigned int)src[RIDX(i+2, j,   dim)].green
                                           + (unsigned int)src[RIDX(i+2, j+1, dim)].green) / 6);

        workPtr -> blue  = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].blue
                                           + (unsigned int)src[RIDX(i,   j+1, dim)].blue
                                           + (unsigned int)src[RIDX(i+1, j,   dim)].blue
                                           + (unsigned int)src[RIDX(i+1, j+1, dim)].blue
                                           + (unsigned int)src[RIDX(i+2, j,   dim)].blue
                                           + (unsigned int)src[RIDX(i+2, j+1, dim)].blue) / 6);
        dst[RIDX(i, j, dim)] = work;
    }

    j = dim - 1;
    for (i = 0; i < dim - 2; i++) // Right Column
    {
        workPtr -> red   = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].red
                                           + (unsigned int)src[RIDX(i+1, j,   dim)].red
                                           + (unsigned int)src[RIDX(i+2, j,   dim)].red) / 3);

        workPtr -> green = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].green
                                           + (unsigned int)src[RIDX(i+1, j,   dim)].green
                                           + (unsigned int)src[RIDX(i+2, j,   dim)].green) / 3);

        workPtr -> blue  = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].blue
                                           + (unsigned int)src[RIDX(i+1, j,   dim)].blue
                                           + (unsigned int)src[RIDX(i+2, j,   dim)].blue) / 3);
        dst[RIDX(i, j, dim)] = work;
    }

    i = dim - 2;
    j = dim - 2;  // One Up & Left of Bottom-Right Pixel
    workPtr -> red   = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].red
                                       + (unsigned int)src[RIDX(i,   j+1, dim)].red
                                       + (unsigned int)src[RIDX(i+1, j,   dim)].red
                                       + (unsigned int)src[RIDX(i+1, j+1, dim)].red) / 4);
    workPtr -> green = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].green
                                       + (unsigned int)src[RIDX(i,   j+1, dim)].green
                                       + (unsigned int)src[RIDX(i+1, j,   dim)].green
                                       + (unsigned int)src[RIDX(i+1, j+1, dim)].green) / 4);
    workPtr -> blue  = (unsigned short)(((unsigned int)src[RIDX(i,   j,   dim)].blue
                                       + (unsigned int)src[RIDX(i,   j+1, dim)].blue
                                       + (unsigned int)src[RIDX(i+1, j,   dim)].blue
                                       + (unsigned int)src[RIDX(i+1, j+1, dim)].blue) / 4);
    dst[RIDX(i, j, dim)] = work;

    i = dim - 2;
    j = dim - 1;  // One Above Bottom-Right Pixel
    workPtr -> red   = (unsigned short)(((unsigned int)src[RIDX(i,   j, dim)].red
                                       + (unsigned int)src[RIDX(i+1, j, dim)].red) / 2);
    workPtr -> green = (unsigned short)(((unsigned int)src[RIDX(i,   j, dim)].green
                                       + (unsigned int)src[RIDX(i+1, j, dim)].green) / 2);
    workPtr -> blue  = (unsigned short)(((unsigned int)src[RIDX(i,   j, dim)].blue
                                       + (unsigned int)src[RIDX(i+1, j, dim)].blue) / 2);
    dst[RIDX(i, j, dim)] = work;

    i = dim - 1;
    j = dim - 2;  // One Left of Bottom-Right Pixel
    workPtr -> red   = (unsigned short)(((unsigned int)src[RIDX(i, j,   dim)].red
                                       + (unsigned int)src[RIDX(i, j+1, dim)].red) / 2);
    workPtr -> green = (unsigned short)(((unsigned int)src[RIDX(i, j,   dim)].green
                                       + (unsigned int)src[RIDX(i, j+1, dim)].green) / 2);
    workPtr -> blue  = (unsigned short)(((unsigned int)src[RIDX(i, j,   dim)].blue
                                       + (unsigned int)src[RIDX(i, j+1, dim)].blue) / 2);
    dst[RIDX(i, j, dim)] = work;

    i = dim - 1;
    j = dim - 1;  // Bottom-Right Pixel
    dst[RIDX(i, j, dim)] = src[RIDX(i, j, dim)];
}

/*********************************************************************
 * register_motion_functions - Register all of your different versions
 *     of the motion kernel with the driver by calling the
 *     add_motion_function() for each test function.  When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.
 *********************************************************************/

void register_motion_functions() {
    add_motion_function(&motion, motion_descr);
    add_motion_function(&naive_motion, naive_motion_descr);
    //add_motion_function(&my_motion_v1, my_motion_v1_descr);
    //add_motion_function(&my_motion_v2, my_motion_v2_descr);
}
