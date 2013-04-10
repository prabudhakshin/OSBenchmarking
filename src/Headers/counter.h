#include<stdio.h>
#ifndef __MATHHEADER__
	#include "mathoperations.h"
#endif

#define NUMSAMPLES 100000

inline unsigned long long int count() __attribute__((always_inline));

inline unsigned long long int count() {
    unsigned int lo =0, hi=0;

//  __asm__ __volatile__ (      // serialize
//    "xorl %%eax,%%eax \n        "
//    ::: "%rax", "%rbx", "%rcx", "%rdx");

    __asm__ __volatile__ ("rdtsc;"
         "mov %%eax, %0;"
         "mov %%edx, %1;"
             : "=r" ( lo ),
               "=r" ( hi )
         );
    return ((unsigned long long int)hi<<32) | lo;
}

double getCounteroverhead(int samplecount, double* stddev) {
    int i = 0;
	unsigned long long int totalcount = 0;
	double cyclescounterArray[samplecount];
	double mean = 0;

    for (i=0;i<samplecount; i++) {
        unsigned long long int start = count();
        unsigned long long int end = count();
		cyclescounterArray[i] = end-start;
        //totalcount = totalcount + (end-start);
    }

	mean = getMeanStddev(cyclescounterArray, samplecount, stddev);
	return mean;
}

double getLoopoverhead(int samplecount, int numiters, double mean_counteroverhead) {
	int index = 0, iter = 0;
	unsigned long long int start = 0, end = 0;
	double loopOverheadArray[samplecount];
	double mean = 0, stddev = 0;
	for (index = 0; index < samplecount; index++) {
		start = count();
		for (iter = 0; iter < numiters; iter++) {
		}
		end = count();
		loopOverheadArray[index] = end-start-mean_counteroverhead;
	}
	mean = getMeanStddev(loopOverheadArray, samplecount, &stddev);
	return mean;
}

void printCounterOverheadResults(int samples, double mean, double stddev) {
	printf("\n");
	printf("---------------------------------------------\n");
	printf("            COUNTER RESULT                   \n");
	printf("---------------------------------------------\n");
	printf("%11s %d\n", "NumSamples:", samples);
	printf("%11s %f\n", "Mean:", mean);
	printf("%11s %f\n", "StdDev:", stddev);
	printf("---------------------------------------------\n");
}

