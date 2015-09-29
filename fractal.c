#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <assert.h>
#include <sys/time.h>
#include <mpi.h>

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

				int comm_sz,my_rank;

                                  MPI_Init(NULL, NULL);
                                  MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

                                  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

					MPI_Barrier(MPI_COMM_WORLD);

				  int row, col, depth, width, maxdepth;
				  double cx, cy, dx, dy, x, y, x2, y2;
				 // struct timeval start, end;
				  unsigned char *cnt;

				  
					
				  width = atoi(argv[1]);
				 
				  maxdepth = atoi(argv[2]);

					MPI_Barrier(MPI_COMM_WORLD);				 
				 if(my_rank==0)
				 {
				




				// check command line

                                  if (argc != 3)
                                        {fprintf(stderr, "usage: %s edge_length max_depth\n", argv[0]); exit(-1);}
			

                                 if (maxdepth < 10)
                                        {fprintf(stderr, "max_depth must be at least 10\n"); exit(-1);}

						printf("Total number of processes: %d\n",comm_sz);


						printf("Fractal v1.3 [Parallel]\n");
						//checking to see that work is evenly distributed
						if(width%comm_sz!=0)
						{
							printf("The number of processes is not equally divisable by width :(" );
							exit(-1);
						}


						if (width < 10) 
						{fprintf(stderr, "edge_length must be at least 10\n"); exit(-1);}

						printf("Here before\n");
						printf("computing %d by %d fractal with a maximum depth of %d\n", width, width, maxdepth);
						printf("Here 1\n");


						// allocate array

						cnt = (unsigned char *)malloc(width * width * sizeof(unsigned char));

						if (cnt == NULL) {fprintf(stderr, "could not allocate memory\n"); exit(-1);}



			     }// end of Rank 0 duty
				  

					double start, finish;
					int sentinel=0;

				  	 MPI_Barrier(MPI_COMM_WORLD);
					 start = MPI_Wtime();
					 
				  // compute fractal
				  dx = (xMax - xMin) / width;
				  dy = (yMax - yMin) / width;
				  
				  int my_start = my_rank * (width / comm_sz);
				  
				  int my_end = (my_rank + 1) * width / comm_sz;
				 
				  unsigned char segArray[(width*width)/comm_sz];
				  //unsigned char * segArray= ((width * width/comm_sz )* sizeof(unsigned char));


				
				  				  
				  for (row = my_start; row < my_end ; row++) 
				  {

					cy = yMin + row * dy;
					
					for (col = 0; col < width; col++) 
					{
				
							  sentinel++;
							  cx = xMin + col * dx;
							  x = -cx;
							  y = -cy;
							  depth = maxdepth;
							  do 
							  {
								x2 = x * x;
								y2 = y * y;
								y = 2 * x * y - cy;
								x = x2 - y2 - cx;
								depth--;
							  } while ((depth > 0) && ((x2 + y2) <= 5.0));
							  
							  segArray[sentinel] = depth & 255;
					}

				  }
	 
//				if(my_rank==0) {
				  MPI_Gather(
				  &segArray,
				  (width*width)/comm_sz,
				  MPI_UNSIGNED_CHAR,
				  cnt,
				  (width*width)/comm_sz,
				  MPI_UNSIGNED_CHAR,
				  0,
				  MPI_COMM_WORLD);
//				  }
//				  else{
/*
                                  MPI_Gather(
                                  &segArray,
                                  width/comm_sz,
                                  MPI_UNSIGNED_CHAR,
                                  NULL,
                                  width/comm_sz,
                                  MPI_UNSIGNED_CHAR,
                                  0,
                                  MPI_COMM_WORLD);

					}				  
*/
				  finish = MPI_Wtime();





				  /*
				  for (row = 0; row < width; row++) 
				  {

					cy = yMin + row * dy;
					
					for (col = 0; col < width; col++) 
					{
							  cx = xMin + col * dx;
							  x = -cx;
							  y = -cy;
							  depth = maxdepth;
							  do 
							  {
								x2 = x * x;
								y2 = y * y;
								y = 2 * x * y - cy;
								x = x2 - y2 - cx;
								depth--;
							  } while ((depth > 0) && ((x2 + y2) <= 5.0));
							  
							  cnt[row * width + col] = depth & 255;
					}

				  }
					*/





					if(my_rank==0)
					{

						// end time
						//gettimeofday(&end, NULL);
						//double runtime = end.tv_sec + end.tv_usec / 1000000.0 - start.tv_sec - start.tv_usec / 1000000.0;
						//printf("compute time: %.4f s\n", runtime);

						// verify result by writing it to a file
						if (width <= 1024) 
						{
						WriteBMP(width, width, cnt, "fractal.bmp");
						}

					  printf("The elapsed time = %e secondsnn\n", finish-start);
                                  //        free(segArray);
                                          free(cnt);
				

					}//end of Rank 0 duty
					

				//MPI_Barrier(MPI_COMM_WORLD);
				//free(segArray);
					MPI_Finalize();

                                  //        free(segArray);
                                  //        free(cnt);

				  return 0;
}

