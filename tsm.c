#include <stdio.h>
#include <stdlib.h>
#include <omp.h>


void one_by_one();

//GLOBAL VARIABLES
int numThreads; 
int numCities;
int currentPath;
int best_dist; //should get overwritten in first iteration
int best_thread; //id of the thread that has the best distance
int *best_path; //array that contains the best path 
int numPaths;
int pathcount_per_thread;
int *cities_to_check; //thread specific array
int thread_city;
int** distArray;

int main(int argc, char* argv[]) {

	if (argc != 4) {
		printf("You entered %d arguments, we require %d arguments", argc, 4);
		return;
	}

	numThreads = atoi (argv[1]);
	numCities = atoi (argv[2]);
	if (numThreads == numCities) {
		numThreads++;
	}
	currentPath = 0;
	best_dist = 1000000; //should get overwritten in first iteration
	best_path = (int *) malloc( numCities * sizeof(int)); //array that contains the best path 


	//READING MATRIX
	FILE *fptr;
	char *line = NULL;
	//char fileName[50];
	//sprintf(fileName, "cities%d.txt", numcities);
	size_t len = 0;
	int i, j;

	fptr = fopen(argv[3], "r");
	if (fptr == NULL) {
		printf("Sorry could not open the file %s\n", argv[3]);
		exit(1);
	}

	distArray = (int**) malloc(numCities * sizeof(int*)); //creating our array
	if (distArray == NULL) {
		printf("Sorry could not allocate memory for your matrix\n");
		exit(1);
	}
	for(i = 0; i < numCities; i++) {
		distArray[i] = (int*) malloc(numCities * sizeof(int)); //creating our array of arrays
	}
	//POPULATING OUR ARRAY
	for(i = 0; i < numCities; i++) {
		for (j = 0; j < numCities; j++) {
			fscanf(fptr, "%d", &distArray[i][j]);
		}
	}

	fclose(fptr);
	//FINISHED READING MATRIX


	//CALCULATING TOTAL NUMBER OF PERMUTATIONS/PATHS
	numPaths = 1;
	int z;
	for(z = numCities - 1; z > 0; z--) {
		numPaths = z * numPaths;
	}
	pathcount_per_thread = numPaths / numThreads; //num of paths that each thread is responsible for

	one_by_one();

} //end of main

void one_by_one() { 
	
  #pragma omp parallel num_threads(numThreads) private(cities_to_check,thread_city)
  {
    cities_to_check = (int *) malloc ((numThreads-2) * sizeof(int));
  
    //POPULTATING CITIES TO CHECK FOR EACH THREAD
    int idx = 0;
  	int i;
    thread_city = omp_get_thread_num()+1;
    for (i=1;i<numCities;i++) {
      if (i != thread_city) {
      	cities_to_check[idx++] = i;
      } 
    }

    int initial_dist = distArray[0][thread_city];
    int this_dist; //thread specific dist

    for (i=0;i<pathcount_per_thread;i++) {
    //while (1) {
      /* STEP 1: TEST THIS PERMUTATION
       *         Start with this_distance = base_distance (0->thread_id+1)
       */
      this_dist = initial_dist;
      this_dist += distArray[thread_city][cities_to_check[0]];

      for (i=1;i<numCities-2;i++) {  //adding distances
        this_dist += distArray[cities_to_check[i-1]][cities_to_check[i]];
        if (this_dist < best_dist) { //so far so good, keep iterating
        	continue;
        }
        else { //we only reach this if "this_dist" is worse than the global "best_dist"
        	break; 
        }
      }

      if (this_dist < best_dist) 
      {
        #pragma omp critical //make sure no more than one thread is accessing this bloc
        {
          if (this_dist < best_dist) //condition still holds?
          {
            best_dist = this_dist;
            best_path[0] = 0;
            best_path[1] = thread_city;
            for (i=0;i<numCities-2;i++) {
              best_path[i+2] = cities_to_check[i];
            }
            best_thread = omp_get_thread_num();
          }
        }//END CRITICAL SECTION
      }

    /*	The following snippet was partly inspired by a stackexchange post
    *		The main parts I needed help with here were regards to how to get the next permutation/path
    *		This part iterates through the thread specific number of cities to check and 
    *		makes sure that there is another permutation to check, if not then we're done
    *		if there are still more paths, next permutation located in cities_to_check 
    */
      int x,y,temp;
      x = numCities-3;
      while (x>0 && cities_to_check[x-1] >= cities_to_check[x])
        x--;

      if (x==0) //last permutation, so break out of computational loop
        break;

      y = numCities-3;
      while(cities_to_check[y] <= cities_to_check[x-1])
        y--;

      temp = cities_to_check[x-1];
      cities_to_check[x-1] = cities_to_check[y];
      cities_to_check[y] = temp;

      y = numCities-3;
      while (x<y) {
        temp = cities_to_check[x];
        cities_to_check[x] = cities_to_check[y];
        cities_to_check[y] = temp;
        x++;
        y--;
      }

    }//END MAIN LOOP
    #pragma omp barrier
  }//END PARALLELISM

  /* FINAL STEP: OUTPUT
   * Output final shortest path and distance.
   */
  printf("Best path: ");
  int j;
  for (j=0;j<numCities;j++)
    printf("%d ",best_path[j]);
  printf("\nDistance: %d\n",best_dist);
  //printf("Shortest path found by thread %d.\n",shortest_thread);

}
