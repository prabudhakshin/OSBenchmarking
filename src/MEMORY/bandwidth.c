#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>

#define NUMSAMPLES 100000


inline unsigned long long int count() __attribute__((always_inline));

inline unsigned long long int count() {
    unsigned int lo =0, hi=0;

  __asm__ __volatile__ (      // serialize
    "xorl %%eax,%%eax \n        "
    ::: "%rax", "%rbx", "%rcx", "%rdx");

    __asm__ __volatile__ ("rdtsc;"
         "mov %%eax, %0;"
         "mov %%edx, %1;"
             : "=r" ( lo ),
               "=r" ( hi )
         );
    return (unsigned long long int)hi<<32 | lo;
}


double getCounteroverhead() {
    int i = 0;
	unsigned long long int totalcount = 0;

    for (i=0;i<NUMSAMPLES; i++) {
        unsigned long long int start = count();
        unsigned long long int end = count();
        totalcount = totalcount + (end-start);
    }

	//printf("Tot: %llu\n\n", totalcount);
    return (double)totalcount/NUMSAMPLES;
}


int main() {

	unsigned long long int start = 0;
	unsigned long long int end = 0;
	int stride = 886432;
	int sample = 0;
	int numSamples = 1;
	double cumulativeReadTotal = 0;
	double cumulativeWriteTotal = 0;
	double totalReadcount = 0;
	double totalWritecount = 0;
    double counterOverHead = getCounteroverhead();
	int i;
	int lo;
	int *a = NULL, *p = NULL, *tempPtr = NULL;
	int temp = 0;
	unsigned long long int numitems = 0;
	unsigned long long int numiters = 0;
	double cpuFreq = 2.3*1024*1024*1024;
	int incsize = 2*1024;
	unsigned long long int startsize = 1024*1024*1024;


	cumulativeReadTotal = 0;
	cumulativeWriteTotal = 0;
	for (sample = 1; sample <= numSamples; sample++) {
		numitems = startsize/sizeof(int*);
		// num of samples to run
		numiters = 1 * numitems;
		a = (int*)malloc(startsize);

		// calculate loop overhead
		start = count();
		for (i=1; i<=numiters; i++) {
		}
		end = count();
		lo = end - start - counterOverHead;

		printf("num iters: %llu\n", numiters);
		start = count();
		for (i=0; i<numiters; i++) {
			//a[(i*stride)%numiters] = 540000000;
			//a[(i+stride)%numiters] = 540000000;
			a[i] = 540000000;
		}
		end = count();

		totalWritecount = (end-start-counterOverHead-lo);
		cumulativeWriteTotal = cumulativeWriteTotal + totalWritecount;

		int t;
		start = count();
		for (i=0; i<numiters; i++) {
			//t = a[(i*stride)%numiters];
			//t = a[(i+stride)%numiters];
			t = a[i];
		}
		end = count();

		totalReadcount = (end-start-counterOverHead-lo);
		cumulativeReadTotal = cumulativeReadTotal + totalReadcount;

		free(a);
		a = p = NULL;
	}

	double cyclesTakenWrite = cumulativeWriteTotal/numSamples;
	double timeTakeninSecWrite = cyclesTakenWrite/cpuFreq; 
	double sizeinMB = (double)startsize/(1024*1024);
	double bandwidthWrite = sizeinMB/timeTakeninSecWrite;

	double cyclesTakenRead = cumulativeReadTotal/numSamples;
	double timeTakeninSecRead = cyclesTakenRead/cpuFreq; 
	double bandwidthRead = sizeinMB/timeTakeninSecRead;

	printf("size: %f\tcyclesTaken: %f, %f\ttimeTaken: %f, %f\tbandwidth: %.20f\n", sizeinMB, cyclesTakenRead, cyclesTakenWrite, timeTakeninSecRead, timeTakeninSecWrite, bandwidthWrite);

	printf("Size=%llu\tRead BW=%.2f\tWrite BW=%0.2f\n", startsize, bandwidthRead, bandwidthWrite);

   return 0;
}
