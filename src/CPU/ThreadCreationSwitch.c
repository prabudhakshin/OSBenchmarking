// Standard libraries
#include <stdlib.h>
#include <stdio.h>
// For threads
#include <pthread.h>

// User defined headers
// The corresponding dir is added to the path
#include "counter.h"
#include "cycletimer.h"

unsigned long long int start = 0, end = 0;

void printUsage() {
	printf("Enter the number of samples that you want to run.\n");
	printf("Usage:\n\t./ThreadOverhead <NUMSAMPLES>\n");
}

void printResult(int samples, double mean, double stddev, char* heading) {
	printf("\n");
	printf("---------------------------------------------\n");
	printf("         %s        \n", heading);
	printf("---------------------------------------------\n");
	printf("%14s %d\n", "NumSamples:", samples);
	printf("%14s %f\n", "Mean:", mean);
	printf("%14s %f\n", "StdDev:", stddev);
	printf("---------------------------------------------\n");
}

void *threadFunc( void *ptr ) {
	end = count();
	pthread_exit(NULL);
	//return;
}


int main(int argc, char* argv[]) {

	if (argc < 2 ) {
		printUsage();
		exit(2);
	}

	int samplecount = atoi(argv[1]);
	int threadId = 0, i = 0, index = 0;
	double mean_counteroverhead = 0, stddev_counteroverhead = 0,\
	       mean_threadoverhead  = 0, stddev_threadoverhead = 0,\
		   mean_threadswitchoverhead= 0, stddev_threadswitchoverhead = 0;

	double threadOverheadArray[samplecount];
	double threadswitchOverheadArray[samplecount];

	pthread_t aThread;

	mean_counteroverhead = getCounteroverhead(samplecount, &stddev_counteroverhead);

	// MEASURE THE THREAD CREATION TIME
	for ( index = 0; index < samplecount; index++) {
		start = count();
		threadId = pthread_create( &aThread, NULL, threadFunc, NULL);
		pthread_join(aThread, NULL);
		threadOverheadArray[index] = end-start-mean_counteroverhead;
	}

	mean_threadoverhead = getMeanStddev(threadOverheadArray, samplecount, &stddev_threadoverhead);
//	printResult(samplecount, mean_threadoverhead, stddev_threadoverhead, "THREAD CREATION TIME");
	printResult(samplecount, getDurationinMicroSec(mean_threadoverhead),\
	                         getDurationinMicroSec(stddev_threadoverhead),\
							 "THREAD CREATION TIME");

	// MEASURE THE THREAD CONTEXT SWITCH TIME
	for ( index = 0; index < samplecount; index++) {
		threadId = pthread_create( &aThread, NULL, threadFunc, NULL);
		start = count();
		pthread_join(aThread, NULL);
		threadswitchOverheadArray[index] = end-start-mean_counteroverhead;
	}

	mean_threadswitchoverhead = getMeanStddev(threadswitchOverheadArray, samplecount, &stddev_threadswitchoverhead);
	//printResult(samplecount, mean_threadswitchoverhead, stddev_threadswitchoverhead, "THREAD CONTEXT SWITCH TIME");
	printResult(samplecount, getDurationinMicroSec(mean_threadswitchoverhead),\
	                         getDurationinMicroSec(stddev_threadswitchoverhead),\
							 "THREAD CONTEXT SWITCH TIME");

	return 0;
}
