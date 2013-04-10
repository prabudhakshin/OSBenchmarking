// Standard libraries
#include <stdlib.h>
#include <stdio.h>

// User defined headers
// The corresponding dir is added to the path
#include <counter.h>

#ifndef __MATHHEADER__
	#include <mathoperations.h>
#endif

void printUsage() {
	printf("Enter the number of samples that you want to run.\n");
	printf("Usage:\n\t./LoopOverhead <NUMITERS> <NUMSAMPLES>\n");
}

void printResult(int samples, int numiters, double mean, double stddev) {
	printf("\n");
	printf("---------------------------------------------\n");
	printf("           LOOP OVERHEAD                     \n");
	printf("---------------------------------------------\n");
	printf("%14s %d\n", "NumSamples:", samples);
	printf("%14s %d\n", "NumIterations:", numiters);
	printf("%14s %f\n", "Mean:", mean);
	printf("%14s %f\n", "StdDev:", stddev);
	printf("---------------------------------------------\n");
}

int main(int argc, char* argv[]) {

	if (argc < 3 ) {
		printUsage();
		exit(2);
	}

	int numiters = atoi(argv[1]);
	int samplecount = atoi(argv[2]);
	int index = 0, iter = 0;
	double mean_counteroverhead = 0, stddev_counteroverhead = 0, stddev_loopoverhead = 0, mean_loopoverhead= 0;
	double loopOverheadArray[samplecount];
	unsigned long long int start = 0, end = 0;
	mean_counteroverhead = getCounteroverhead(samplecount, &stddev_counteroverhead);
	printCounterOverheadResults(samplecount, mean_counteroverhead, stddev_counteroverhead);

	
	for (index = 0; index < samplecount; index++) {
		start = count();
		for (iter = 0; iter < numiters; iter++) {
		}
		end = count();
		loopOverheadArray[index] = end-start-mean_counteroverhead;
	}

	mean_loopoverhead = getMeanStddev(loopOverheadArray, samplecount, &stddev_loopoverhead);
	printResult(samplecount, numiters, mean_loopoverhead, stddev_loopoverhead);

	return 0;
}
