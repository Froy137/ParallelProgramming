/*
Fractal v1.3: This code computes a fractal from the Mandelbrot set.
 
Copyright (c) 2015, Texas State University. All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted for academic, research, experimental, or personal use provided
that the following conditions are met:

   * Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.
   * Neither the name of Texas State University nor the names of its
     contributors may be used to endorse or promote products derived from this
     software without specific prior written permission.

For all other uses, please contact the Office for Commercialization and Industry
Relations at Texas State University <http://www.txstate.edu/ocir/>.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Author: Martin Burtscher <burtscher@txstate.edu>
*/


#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <assert.h>
#include <sys/time.h>

#define delta 0.00578
#define xMin 0.74395
#define xMax (xMin + delta)
#define yMin 0.10461
#define yMax (yMin + delta)

static void WriteBMP(const int x, const int y, const unsigned char* const bmp, const char* const name)
{
  const unsigned char bmphdr[54] = {66, 77, 255, 255, 255, 255, 0, 0, 0, 0, 54, 4, 0, 0, 40, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 1, 0, 8, 0, 0, 0, 0, 0, 255, 255, 255, 255, 196, 14, 0, 0, 196, 14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  unsigned char hdr[1078];
  int i, j, c, xcorr, diff;
  FILE* f;

  xcorr = (x + 3) >> 2 << 2;  // BMPs have to be a multiple of 4 pixels wide.
  diff = xcorr - x;

  for (i = 0; i < 54; i++) hdr[i] = bmphdr[i];
  *((int*)(&hdr[18])) = xcorr;
  *((int*)(&hdr[22])) = y;
  *((int*)(&hdr[34])) = xcorr * y;
  *((int*)(&hdr[2])) = xcorr * y + 1078;
  for (i = 0; i < 256; i++) {
    j = i * 4 + 54;
    hdr[j+0] = i;  // blue
    hdr[j+1] = i;  // green
    hdr[j+2] = i;  // red
    hdr[j+3] = 0;  // dummy
  }

  f = fopen(name, "wb");
  assert(f != NULL);
  c = fwrite(hdr, 1, 1078, f);
  assert(c == 1078);
  if (diff == 0) {
    c = fwrite(bmp, 1, x * y, f);
    assert(c == x * y);
  } else {
    *((int*)(&hdr[0])) = 0;  // need up to three zero bytes
    for (j = 0; j < y; j++) {
      c = fwrite(&bmp[j * x], 1, x, f);
      assert(c == x);
      c = fwrite(hdr, 1, diff, f);
      assert(c == diff);
    }
  }
  fclose(f);
}

int main(int argc, char *argv[])
{
  int row, col, depth, width, maxdepth;
  double cx, cy, dx, dy, x, y, x2, y2;
  unsigned char *cnt;
  struct timeval start, end;

  printf("Fractal v1.3 [serial]\n");

  // check command line
  if (argc != 3) {fprintf(stderr, "usage: %s edge_length max_depth\n", argv[0]); exit(-1);}
  width = atoi(argv[1]);
  if (width < 10) {fprintf(stderr, "edge_length must be at least 10\n"); exit(-1);}
  maxdepth = atoi(argv[2]);
  if (maxdepth < 10) {fprintf(stderr, "max_depth must be at least 10\n"); exit(-1);}

  printf("computing %d by %d fractal with a maximum depth of %d\n", width, width, maxdepth);

  // allocate array
  cnt = (unsigned char *)malloc(width * width * sizeof(unsigned char));
  if (cnt == NULL) {fprintf(stderr, "could not allocate memory\n"); exit(-1);}

  // start time
  gettimeofday(&start, NULL);

  // compute fractal
  dx = (xMax - xMin) / width;
  dy = (yMax - yMin) / width;
  for (row = 0; row < width; row++) {
    cy = yMin + row * dy;
    for (col = 0; col < width; col++) {
      cx = xMin + col * dx;
      x = -cx;
      y = -cy;
      depth = maxdepth;
      do {
        x2 = x * x;
        y2 = y * y;
        y = 2 * x * y - cy;
        x = x2 - y2 - cx;
        depth--;
      } while ((depth > 0) && ((x2 + y2) <= 5.0));
      cnt[row * width + col] = depth & 255;
    }
  }

  // end time
  gettimeofday(&end, NULL);
  double runtime = end.tv_sec + end.tv_usec / 1000000.0 - start.tv_sec - start.tv_usec / 1000000.0;
  printf("compute time: %.4f s\n", runtime);

  // verify result by writing it to a file
  if (width <= 1024) {
    WriteBMP(width, width, cnt, "fractal.bmp");
  }

  free(cnt);
  return 0;
}

