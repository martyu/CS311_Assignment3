
#include <iostream>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sched.h>
#include <vector>
#include <math.h>
#include <fstream>

#define NUM_OF_THREADS 4

typedef struct {
	int thread_index;
	unsigned long long prime;
	unsigned long long marked_up_to;
	pthread_t thread;
	bool thread_is_done;
} thread_info_t;

typedef struct {
	unsigned long long x;
	unsigned long long y;
}indexes_t;

static const unsigned long bitmasks[] = {
	0x8000000000000000, 0x4000000000000000, 0x2000000000000000, 0x1000000000000000,
	0x800000000000000, 0x400000000000000, 0x200000000000000, 0x100000000000000,
	0x80000000000000, 0x40000000000000, 0x20000000000000, 0x10000000000000,
	0x8000000000000, 0x4000000000000, 0x2000000000000, 0x1000000000000,
	0x800000000000, 0x400000000000, 0x200000000000, 0x100000000000,
	0x80000000000, 0x40000000000, 0x20000000000, 0x10000000000,
	0x8000000000, 0x4000000000, 0x2000000000, 0x1000000000,
	0x800000000, 0x400000000, 0x200000000, 0x100000000,
	0x80000000, 0x40000000, 0x20000000, 0x10000000,
	0x8000000, 0x4000000, 0x2000000, 0x1000000,
	0x800000, 0x400000, 0x200000, 0x100000,
	0x80000, 0x40000, 0x20000, 0x10000,
	0x8000, 0x4000, 0x2000, 0x1000,
	0x800, 0x400, 0x200, 0x100,
	0x80, 0x40, 0x20, 0x10,
	0x8, 0x4, 0x2, 0x1
};
clock_t start;
clock_t stop;
static unsigned long *bitmap; //array of longs
static int bits_in_element = sizeof(unsigned long long)*8;
static thread_info_t info[NUM_OF_THREADS];
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
unsigned long long upper_limit;
unsigned long long sqrt_upper_limit;

indexes_t bit_indexes_from_number(unsigned long long number);
void print_bitmap();
bool check_if_bit_index_is_prime(unsigned long long i, unsigned long long j);


static void * threadFunc(void *arg){	
	thread_info_t *thread_info = (thread_info_t *)arg;
	
	unsigned long long prime = thread_info->prime;
	unsigned long long comp_number = prime*prime;
	int thread_index = thread_info->thread_index;
	indexes_t comp_index;
	
	pthread_mutex_lock(&mtx);
	// get rid of prime multiples
	for(; comp_number <= upper_limit; comp_number += 2*prime){
		comp_index = bit_indexes_from_number(comp_number);
		bitmap[comp_index.x] |= bitmasks[comp_index.y];
		// so main thread only checks for primes past what's been marked
		info[thread_index].marked_up_to = comp_number;
	}
	pthread_mutex_unlock(&mtx);
	
	info[thread_index].marked_up_to = upper_limit;
	thread_info->thread_is_done = true;
	return NULL;
}


int main (int argc, char * argv[]){
	long ncpus;
	double total_time;
	unsigned long long num_to_check;
	thread_info_t *thread_to_use;
	int thread_ret_val;
	
	if(argc != 2){
		std::cout << "Please only enter the upper end range value\n";
		return -1;
	}
	else{
		upper_limit = (unsigned long long)atoll(argv[1]);
		sqrt_upper_limit = sqrtl(upper_limit);
	}
	
	if(upper_limit > pow(2, 32)){
		std::cout << "Please keep the upper limit below 2^32\n";
	}
	
	start = clock();
	
	for(int i = 0; i < NUM_OF_THREADS; i++){
		info[i].thread_index = i;
		info[i].marked_up_to = 3;
		info[i].thread_is_done = true;
	}
	
	bitmap = (unsigned long *)malloc(sizeof(unsigned long *) * upper_limit/8);
	for(unsigned long long i = 0; i < upper_limit/8; i++){
		bitmap[i] = 0xAAAAAAAAAAAAAAAA; //wipe out evens
	}
	bitmap[0] = 0xCAAAAAAAAAAAAAAA;
	
	// go thru elements in array
	for(unsigned long long i = 0; i <= sqrt_upper_limit/bits_in_element; i++){
		//go thru bits in elements
		for(unsigned long long j = (i == 0 ? 3 : 0); 
			j < (bits_in_element > upper_limit ? upper_limit : bits_in_element) ; j++){
			num_to_check = (i * bits_in_element) + j;
			//make sure all threads are past num_to_check
			for(int k = 0; ; k++){
				if(k == NUM_OF_THREADS)
					k = 0;
				if(info[k].marked_up_to >= num_to_check)
					break;
			}
			
			 //check if bit index is prime
			if(check_if_bit_index_is_prime(i, j)){
				for(int k = 0; ; k++) {//wait for a finished thread to use
					if(k == NUM_OF_THREADS)
						k = 0;
					if(info[k].thread_is_done){
						thread_to_use = &info[k];
						info[k].thread_is_done = false;
						info[k].prime = (i * bits_in_element) + j;
						break;
					}
				}
				//thread gets rid of multiples
				thread_ret_val = pthread_create(&thread_to_use->thread,
												NULL,
												threadFunc,
												(void *)thread_to_use); 
				if(thread_ret_val != 0){
					std::cerr << "thread error: " << strerror(thread_ret_val) << "\n";
					return -1;
				}
			}
		}
	}
	
	for(int i = 0; i < NUM_OF_THREADS; i++){
		thread_ret_val = pthread_join(info[i].thread, NULL);
		if(thread_ret_val != 0){
			std::cerr << strerror(thread_ret_val);
		}
	}
	
	stop = clock();
	
	total_time = (double)(stop - start) / (double)CLOCKS_PER_SEC;
	
	ncpus = sysconf(_SC_NPROCESSORS_ONLN);
	
	/* Print performance results */
	printf ("Total time using %d threads : %.6f seconds\n",
	        NUM_OF_THREADS, total_time / 
			(NUM_OF_THREADS < ncpus ? NUM_OF_THREADS : ncpus));
	
	print_bitmap();
	
	free(bitmap);
	return 1;
}


indexes_t bit_indexes_from_number(unsigned long long number){
	indexes_t indexes;
	
	indexes.x = ceill(number / bits_in_element);
	
	if(indexes.x == 0)
		indexes.y = number;
	else
		indexes.y = number - indexes.x*bits_in_element;
	
	return indexes;
}


void print_bitmap(){
	unsigned long long count = 0;
	std::ofstream prime_numbers_output;
	
	prime_numbers_output.open("primes.txt");
	
	for(unsigned long long i = 0; i <= upper_limit; i++){
		if(check_if_bit_index_is_prime(bit_indexes_from_number(i).x,
									   bit_indexes_from_number(i).y)){
			count++;
			prime_numbers_output << i << "\n";
		}
	}
	prime_numbers_output.close();
	std::cout << "\nnumber of primes between 1 and " << upper_limit <<
												": " << count << "\n";
}


bool check_if_bit_index_is_prime(unsigned long long i, unsigned long long j){
	if(bitmap[i] & bitmasks[j])
		return false;
	else
		return true;
}


