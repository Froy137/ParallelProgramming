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

						printf("Prefix Sum v1.0 [Parallel]\n");

						printf("Total number of processes: %d\n",comm_sz);


						// check command line
						if (argc != 2) {fprintf(stderr, "usage: %s size\n", argv[0]); exit(-1);}
						if (size < 1) {fprintf(stderr, "size is too small: %d\n", size); exit(-1);}
						printf("configuration: %d elements\n", size);


						//checking to see that work is evenly distributed
						if(size%comm_sz!=0)
						{
							printf("The number of processes is not equally divisable by width :(\n" );
							exit(-1);
						}



						// allocate arrays
						arrayA = (int*)malloc(sizeof(int) * size);  if (arrayA == NULL) {fprintf(stderr, "cannot allocate arrayA\n");  exit(-1);}
						arrayB = (int*)malloc(sizeof(int) * size);  if (arrayB == NULL) {fprintf(stderr, "cannot allocate arrayB\n");  exit(-1);}

						// initialize
						for (int i = 0; i < size; i++) {
							arrayA[i] = arrayB[i] = f(i);
						}




				  }//end of process 0 duty
    
    

	double start, finish;

	MPI_Barrier(MPI_COMM_WORLD);
	start = MPI_Wtime();
    
    int local_array[size/comm_sz];//local prefix sum
    int local_array2[size/comm_sz];//local prefix sum

    
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
    
	//creating the array that holds the last elements of chunks
    int arr_last_elements[comm_sz];
	
	//creating the array that holds the sum scan elements
    int scaned_last_elements[comm_sz];
    
    int local_lastValue = local_array[(size/comm_sz)-1];
    int local_proc_presum;
    
	//prefix sum on the last elements of chunks
    MPI_Scan(&local_lastValue,&local_proc_presum,1,MPI_INT,MPI_SUM,MPI_COMM_WORLD);
    
    //collecting all prefixSum-last_element  , in array
    scaned_last_elements[my_rank]=local_proc_presum-local_lastValue;
    
        for(int x=0;x<(size/comm_sz);x++){
			//subtracting the result from the chunks
            local_array[x]=local_array[x]+scaned_last_elements[my_rank];
        
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
     MPI_Finalize();

 
  return 0;
}
