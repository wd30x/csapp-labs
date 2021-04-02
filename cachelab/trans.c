/*
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */
#include <stdio.h>

#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N]) {
  int i, k, j, l, a0, a1, a2, a3, a4, a5, a6, a7;

  if (M == 32 && N == 32) {
    for (k = 0; k < 4; k++) {
      for (l = 0; l < 4; l++) {
        for (i = 8 * k; i < 8 * k + 8; i++) {
          a0 = A[i][8 * l];
          a1 = A[i][8 * l + 1];
          a2 = A[i][8 * l + 2];
          a3 = A[i][8 * l + 3];
          a4 = A[i][8 * l + 4];
          a5 = A[i][8 * l + 5];
          a6 = A[i][8 * l + 6];
          a7 = A[i][8 * l + 7];
          B[8 * l][i] = a0;
          B[8 * l + 1][i] = a1;
          B[8 * l + 2][i] = a2;
          B[8 * l + 3][i] = a3;
          B[8 * l + 4][i] = a4;
          B[8 * l + 5][i] = a5;
          B[8 * l + 6][i] = a6;
          B[8 * l + 7][i] = a7;
        }
      }
    }
  } else if (M == 64 && N == 64) {
    //这题有点难，参考了网上的大佬做法^^
    //
    for (i = 0; i < 64; i += 8) {
      for (j = 0; j < 64; j += 8) {
        // transpose top A (4x8) to top B (4x8)
        for (k = i; k < i + 4; ++k) {
          a1 = A[k][j];
          a2 = A[k][j + 1];
          a3 = A[k][j + 2];
          a4 = A[k][j + 3];
          a5 = A[k][j + 4];
          a6 = A[k][j + 5];
          a7 = A[k][j + 6];
          a0 = A[k][j + 7];

          B[j][k] = a1;
          B[j][k + 4] = a5;
          B[j + 1][k] = a2;
          B[j + 1][k + 4] = a6;
          B[j + 2][k] = a3;
          B[j + 2][k + 4] = a7;
          B[j + 3][k] = a4;
          B[j + 3][k + 4] = a0;
        }
        // transpose bottom left A to top right B, move top right B to bottom
        // left B
        for (k = j; k < j + 4; ++k) {
          a1 = B[k][i + 4];
          a2 = B[k][i + 5];
          a3 = B[k][i + 6];
          a4 = B[k][i + 7];
          a5 = A[i + 4][k];
          a6 = A[i + 5][k];
          a7 = A[i + 6][k];
          a0 = A[i + 7][k];

          B[k][i + 4] = a5;
          B[k][i + 5] = a6;
          B[k][i + 6] = a7;
          B[k][i + 7] = a0;
          B[k + 4][i] = a1;
          B[k + 4][i + 1] = a2;
          B[k + 4][i + 2] = a3;
          B[k + 4][i + 3] = a4;
        }
        // transpose bottom right part from A to B
        for (k = j + 4; k < j + 8; ++k) {
          a1 = A[i + 4][k];
          a2 = A[i + 5][k];
          a3 = A[i + 6][k];
          a4 = A[i + 7][k];

          B[k][i + 4] = a1;
          B[k][i + 5] = a2;
          B[k][i + 6] = a3;
          B[k][i + 7] = a4;
        }
      }
    }
  } else if (M == 61 && N == 67) {
    for (k = 0; k < 4; k++) {
      for (l = 0; l < 4; l++) {
        for (i = 17 * k; (i < 17 * k + 17) && (i < N); i++) {
          for (j = 17 * l; (j < 17 * l + 17) && (j < M); j++) {
            B[j][i] = A[i][j];
          }
        }
      }
    }
  }
}

/*
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started.
 */

/*
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N]) {
  int i, j, tmp;

  for (i = 0; i < N; i++) {
    for (j = 0; j < M; j++) {
      tmp = A[i][j];
      B[j][i] = tmp;
    }
  }
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions() {
  /* Register your solution function */
  registerTransFunction(transpose_submit, transpose_submit_desc);

  /* Register any additional transpose functions */
  registerTransFunction(trans, trans_desc);
}

/*
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N]) {
  int i, j;

  for (i = 0; i < N; i++) {
    for (j = 0; j < M; ++j) {
      if (A[i][j] != B[j][i]) {
        return 0;
      }
    }
  }
  return 1;
}
