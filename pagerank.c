% An implementation of google`s Pagerank  
% Algorithm with POSIX threads 
% It uses the pagerank implementation provided 
% by Cleve Moler and MathWorks
%
%   Iordanis P. Thoidis
%   Student @ AUTH
%   date: Mar 2014

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>


struct Graph
{
	int id;
	//char url[256];
	int* To;
	float E;
	int Sa;
	float P_t, P_t1;
};

// declaration of functions
void Pagerank();
void random_P_t_E();
void read_connections(char* filename);
void Pagerank_single();
void *Pagerank_multi(void *arg1);
void print_probabilities();


// Define
struct Graph *Node;
char* filename = "myFile.txt";
int N;
float d=0.85;
float threshold = 0.0003;
long iterations = 0;
float tot_max_error;
int NUM_THREADS, NUM_THREADS_R=-1;
int thread_sum, thread_extd, flag;

pthread_mutex_t pr_mutex, pr_mutex1, pr_mutex2, ext_mutex;
pthread_cond_t pr_threshold;


int main(int argc, char** argv) {
	
	int i;
	struct timeval start, end;
	
	if (argc != 2) {
    printf("Usage: %s N NUM_THREADS \n"
           " where\n"
           " NUM_THREADS    : number of threads to execute\n"
	   , argv[0]);
    	return (1);
  	}
  
  	NUM_THREADS = atoi(argv[1]);
   
    // Read the Node connections from file
	read_connections(filename);

  	printf("%d Threads & %d Nodes \n", NUM_THREADS, N);

	// initialize the P(0) and E with random numbers
	random_P_t_E();
   	
	printf("Pagerank Starts\n");
	
	gettimeofday(&start, NULL);

	// Run the Pagerank Algorithm
	Pagerank();

	//Pagerank_single();
	gettimeofday(&end, NULL);
	
	print_probabilities();

	double totaltime = (((end.tv_usec - start.tv_usec) / 1.0e6 + end.tv_sec - start.tv_sec) * 1000) / 1000;
	printf("\nTotaltime = %f seconds\n", totaltime);

	return 0;
}

// Allocate the To array, fill G array and read from file
void read_connections(char* filename){

	FILE *fid;
	int i,j, size, connections;
    int from_idx, to_idx, temp;
	char ch1[20], ch2[20];
	int flag1=1;

    fid = fopen(filename, "r");
   	if (fid == NULL){printf("Error opening data file\n");}

	fscanf(fid, "%d", &N);
	//printf("N = %d\n", N);

	// Memory Allocation for Node Structure
    Node = (struct Graph*) malloc(N * sizeof(struct Graph));

    for (i = 0; i < N; i++)
	{
		Node[i].id = i;
		Node[i].Sa = 0;
        Node[i].To = (int*) malloc(sizeof(int));
    }

    char str[20];
    int value, lp, inp;
    from_idx = -1;

	for (j = 0; j < N; j++)
	{

	if (!feof(fid))
	{
		fscanf(fid, "%d", &lp);
		from_idx++;
		for (i = 0; i < lp; i++)
		{		
			fscanf(fid, "%d", &to_idx);
			to_idx--;
			Node[from_idx].Sa++;
			Node[from_idx].To =(int*) realloc(Node[from_idx].To, Node[from_idx].Sa * sizeof(int));
			Node[from_idx].To[Node[from_idx].Sa - 1] = to_idx;
		}
	}
 }
	fclose(fid);	
}

// Generate random numbers for vectors P(0) and E
void random_P_t_E(){

	int i, j;
	float sumP_t = 0, sumE = 0;

    srand(time(NULL));

   	for (i = 0; i < N; ++i){
    	Node[i].P_t = (float) rand() / (float) RAND_MAX;
    	sumP_t += Node[i].P_t;
    	Node[i].E = (float) rand() / (float) RAND_MAX;
    	sumE += Node[i].E;
    } 
    for (i = 0; i < N; ++i){
		Node[i].P_t /= sumP_t; 
		Node[i].E /= sumE; 
    }
    sumP_t=0;
    sumE=0;
    for (i = 0; i < N; ++i){
    	sumP_t += Node[i].P_t;
    	sumE += Node[i].E;

    }

    //printf("Sum of P{0} = %f\n",sumP_t);
    assert(sumP_t = 1);
    //printf("Sum of E = %f\n",sumE);
    assert(sumE = 1);
}

void print_probabilities(){
	int i, j, iswap;
	float sumP_t = 0, swap;
	/*
	for (i = 0; i < N; i++)
	{
		sumP_t+=Node[i].P_t;
		printf("P(%d)= %f\n", i, Node[i].P_t);
	}
	*/
	for (i = 0 ; i < N-1; i++)
  	{
    	for (j = 0 ; j < N - i - 1; j++)
    	{
      		if (Node[j].P_t > Node[j+1].P_t)
       		{
        		swap       	  = Node[j].P_t;
        		Node[j].P_t   = Node[j+1].P_t;
        		Node[j+1].P_t = swap;
        		iswap         = Node[j].id;
        		Node[j].id    = Node[j+1].id;
        		Node[j+1].id  = iswap;
      		}
    	}
  	}
	for (i = 0; i < N; i++)
	{
		printf("%d: Node %d\n", i, Node[i].id );
	}
}

void Pagerank() {
	
	pthread_mutex_init(&pr_mutex, NULL);
	pthread_mutex_init(&pr_mutex1, NULL);
	pthread_mutex_init(&pr_mutex2, NULL);
	pthread_mutex_init(&ext_mutex, NULL);
  	pthread_cond_init (&pr_threshold, NULL);
  	thread_sum = 0;
  	thread_extd = 0;
  	tot_max_error = 1;
  	long i;


  	pthread_t threads[NUM_THREADS];
  	pthread_attr_t pthread_custom_attr;
  	pthread_attr_init(&pthread_custom_attr);
  	pthread_attr_setdetachstate(&pthread_custom_attr, PTHREAD_CREATE_JOINABLE);

  	//printf("Creating Threads\n");

  	if (NUM_THREADS >= N) NUM_THREADS = N;

  	for(i=0; i<NUM_THREADS; i++){
    		pthread_create(&threads[i],&pthread_custom_attr, Pagerank_multi, (void *) i );
  	}

  	for(i=0; i<NUM_THREADS; i++){
    	pthread_join(threads[i], NULL);
  	}
}

void Pagerank_single() {

	float temp, error = 1;
	long i, j;
	float contr_P;
	while (error > threshold){



		contr_P = 0;
		error = -1;
    	// find the probability
    	for (i = 0; i < N; i++) 
    	{
    		// For one Node
    		for (j = 0; j < Node[i].Sa; j++){
    				if (Node[Node[i].To[j]].Sa>0)
    				{
						Node[i].P_t1 += Node[Node[i].To[j]].P_t / Node[Node[i].To[j]].Sa;
    				} else { 
    					Node[i].P_t1 += Node[Node[i].To[j]].P_t / N;
    				}
			}
			Node[i].P_t1 = d * (Node[i].P_t1) + (1 - d)* Node[i].E;
			temp = fabs(Node[i].P_t1 - Node[i].P_t);
			if (temp>error) error = temp;
			
		}

		// update the old values with the new ones
		for (i = 0; i < N; i++)
        {
            // Update the "old" P table with the new one 
            Node[i].P_t = Node[i].P_t1;  
            Node[i].P_t1 = 0;
        }

        iterations++;
	}
    
    printf("Total iterations: %ld\n", iterations);
}

void *Pagerank_multi(void *arg1) {
  	
	long tid;
	float error;
	float loc_max_error = 0;
	long i, j, k;
	tid = (long) arg1;
	long Num_cells;
 	Num_cells = N / NUM_THREADS;
  	if (N%NUM_THREADS!=0)  Num_cells++;
  	long start = tid * Num_cells;
  	long end = (start+Num_cells-1 < N) ? start+Num_cells-1 : N-1;
  	if(start> N-1){
  		pthread_mutex_lock(&ext_mutex);
    	NUM_THREADS--;
    	pthread_mutex_unlock(&ext_mutex);
   		pthread_exit(NULL);
  	}

  	// add the threads flag to the kingdom
  	pthread_mutex_lock(&ext_mutex);
  	flag++;
    pthread_mutex_unlock(&ext_mutex);

  	printf("Thread %ld started. [%ld,%ld]\n", tid, start, end);
 
	do{           
			// sync to make sure all extra threads exited before locking the mutex 
			while(flag < NUM_THREADS) {}
  			loc_max_error = 0;
  			// Execute PageRank algorithm for a group of nodes
			for (i = start; i <= end; i++) 
			{
				// For one Node, compute the probability
    			for (j = 0; j < Node[i].Sa; j++){
    				if (Node[Node[i].To[j]].Sa>0)
    				{
						Node[i].P_t1 += Node[Node[i].To[j]].P_t / Node[Node[i].To[j]].Sa;
    				} else { 
    					Node[i].P_t1 += Node[Node[i].To[j]].P_t / N;
    				}
				}
			
				Node[i].P_t1 = d * Node[i].P_t1 + (1 - d) * Node[i].E;
				error = fabs(Node[i].P_t1 - Node[i].P_t);

				// find max error localy
				if (error > loc_max_error){
					loc_max_error = error;
				}
			}

			// Synchronise all threads to make sure
			// every thread finished computing the t1 probability
    		pthread_mutex_lock(&pr_mutex);

    			thread_sum++;

				if (thread_sum < NUM_THREADS){
					pthread_cond_wait(&pr_threshold, &pr_mutex);
				}
				if (tid == 0)  
        				tot_max_error=-1;
				if (thread_sum == NUM_THREADS){
					pthread_cond_broadcast(&pr_threshold);
				    thread_sum = 0;
				}
    		pthread_mutex_unlock(&pr_mutex);
     		

    		// Update the old values with the new ones
    		for (i = start; i <= end; i++){
				Node[i].P_t = Node[i].P_t1;  
            	Node[i].P_t1 = 0;
			}
            
            // thread 0 increases iterations
            if (tid == 0)  
        		iterations++;

        	// Sync again to make sure
        	// All threads updated the values
    		// find total max error accross threads
			pthread_mutex_lock(&pr_mutex);
			if (loc_max_error > tot_max_error)
					tot_max_error = loc_max_error;
			pthread_mutex_unlock(&pr_mutex);

			pthread_mutex_lock(&pr_mutex);
    			thread_sum++;

				if (thread_sum < NUM_THREADS)
					pthread_cond_wait(&pr_threshold, &pr_mutex);
			
				if (thread_sum == NUM_THREADS){
					pthread_cond_broadcast(&pr_threshold);
					thread_sum = 0;
			}
    		pthread_mutex_unlock(&pr_mutex);

    // Error must be greater than thresold, else finished
	}while ((tot_max_error > threshold));
	

	if (tid == 0)
    	printf("\nTotal iterations: %ld\n", iterations);

    return (void *)0;
}




