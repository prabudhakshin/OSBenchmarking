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

	printf("Tot: %llu\n\n", totalcount);
    return (double)totalcount/NUMSAMPLES;
}


int main() {

	unsigned long long int start = 0;
	unsigned long long int end = 0;
	int stride = 32;
	int sample = 0;
	int numSamples = 100;
	double cumulativeTotal = 0;
	double totalcount = 0;
    double counterOverHead = getCounteroverhead();
	printf("OH: %f\n", counterOverHead);
	int i;
	int lo;
	int **a = NULL, *p = NULL;
	int temp = 0;
	int numitems = 0;
	unsigned long numiters = 0; 
	int incsize = 2*1024;
	int startsize = 1*1024;
	int endsize = 12*1024*1024;

	while (startsize <= endsize) {

		cumulativeTotal = 0;
		for (sample = 1; sample <= numSamples; sample++) {
			numitems = startsize/sizeof(int*);
			// num of samples to run
			numiters = 1 * numitems;
			a = (int**)malloc(startsize);

			// calculate loop overhead
			start = count();
			for (i=1; i<=numiters; i++) {
			}
			end = count();
			lo = end - start - counterOverHead;

			for (i = 0; i<numitems; i++) {
				a[i] = (int*)&a[(i+stride)%numitems];
			}
			p = a[0];
			start = count();
			for (i=1; i<=numiters; i++) {
				p = (int*)*p;
			}
			end = count();

			totalcount = (end-start-counterOverHead-lo)/(numiters);
			cumulativeTotal = cumulativeTotal + totalcount;

			free(a);
			a = p = NULL;
		}

		printf("%d\t%0.2f\n", startsize, cumulativeTotal/numSamples);

		//startsize = startsize + incsize;
		startsize = startsize * 2;
	}

   //printf("Counter overhead = %f\n\n", counterOverHead);
   //printf("L1 access time for size = %d is %f\n\n", size/1024, totalcount/(numiters));

   return 0;
}
