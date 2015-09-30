#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <mpi.h>

static int f(int val)
{
  return (val - 17) * 19;
}

static void prefixSumA(int data[], const int size)
{
  for (int i = 1; i < size; i++) {
    data[i] += data[i - 1];
  }
}

static void prefixSumB(int data[], const int size)
{
  for (int dist = 1; dist < size; dist *= 2) {
    for (int i = size - 1; i >= dist; i--) {
      data[i] += data[i - dist];
    }
  }
}

int main(int argc, char* argv[])
{

    
    
				int comm_sz,my_rank;

				MPI_Init(NULL, NULL);
				MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

				MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  int size = atoi(argv[1]);
 int* arrayA ;
int* arrayB ;    
    
if(my_rank==0){    
    
            printf("Prefix Sum v1.0 [serial]\n");

          // check command line
          if (argc != 2) {fprintf(stderr, "usage: %s size\n", argv[0]); exit(-1);}
          if (size < 1) {fprintf(stderr, "size is too small: %d\n", size); exit(-1);}
          printf("configuration: %d elements\n", size);

          // allocate arrays
           arrayA = (int*)malloc(sizeof(int) * size);  if (arrayA == NULL) {fprintf(stderr, "cannot allocate arrayA\n");  exit(-1);}
           arrayB = (int*)malloc(sizeof(int) * size);  if (arrayB == NULL) {fprintf(stderr, "cannot allocate arrayB\n");  exit(-1);}

          // initialize
          for (int i = 0; i < size; i++) {
            arrayA[i] = arrayB[i] = f(i);
          }


        
    
}//end of process 0 duty
    
    
  // time the prefix sum computation
 // struct timeval start, end;
 // gettimeofday(&start, NULL);
 
    			double start, finish;

				MPI_Barrier(MPI_COMM_WORLD);
				 start = MPI_Wtime();
    
    int local_array[size/comm_sz];//local prefix sum
    //int local_conCat[size/comm_sz];//will hold finalized partition
	
    MPI_Scatter(
    arrayB,
    size/comm_sz,
    MPI_INT,
    local_array,
    size/comm_sz,
    MPI_INT,
    0,
    MPI_COMM_WORLD);
    
    prefixSumB(local_array, size/comm_sz);
	
	int local_lastValue = local_array[size/comm_sz];
	int future_num;
	
	if(my_rank!=comm_sz-1){
		//everybody sends last value to the rank above them.
		MPI_Send(&local_lastValue,1,MPI_INT,my_rank + 1,0,MPI_COMM_WORLD);
	}
	
	if(my_rank!=0){
		//everybody receive the last num of the rank below them.
		MPI_Recv(&future_num,1,MPI_INT,my_rank-1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
		//and Reduce(add) the value you just received 
		//MPI_Reduce(&local_array,&local_conCat,size/comm_sz,MPI_INT,MPI_SUM,my_rank,MPI_COMM_WORLD);

		for(int x=0;x<size/comm_sz;x++){
		
			local_array[x]=local_array[x]+future_num;
		
		}
	}

		MPI_Gather(
		&local_array,
		size/comm_sz,
		MPI_INT,
		arrayB,
		size/comm_sz,
		MPI_INT,
		0,
		MPI_COMM_WORLD);
	
     finish = MPI_Wtime();
	 if(my_rank==0){
		 
		 printf("The elapsed time = %e seconds\n", finish-start);
		 
		  // compare results
		  prefixSumA(arrayA, size);
		  for (int i = 0; i < size; i++) {
			if (arrayA[i] != arrayB[i]) {fprintf(stderr, "result mismatch at position %d\n", i);  exit(-1);}
		  }

		  free(arrayA);  free(arrayB);
	 }

   // gettimeofday(&end, NULL);

  // print performance info
  //double runtime = end.tv_sec + end.tv_usec / 1000000.0 - start.tv_sec - start.tv_usec / 1000000.0;
  //printf("compute time: %.4f s\n", runtime);
  //printf("mega_elements/sec: %.3f\n", size * 0.000001 / runtime);
				
				MPI_Finalize();

 
  return 0;
}

