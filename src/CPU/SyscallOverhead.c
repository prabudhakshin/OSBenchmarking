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
	printf("Usage:\n\t./SyscallOverhead <NUMSAMPLES>\n");
}

void printResult(int samples, double mean, double stddev) {
	printf("---------------------------------------------\n");
	printf("%14s %d\n", "NumSamples:", samples);
	printf("%14s %f\n", "Mean:", mean);
	printf("%14s %f\n", "StdDev:", stddev);
	printf("---------------------------------------------\n");
}

int main(int argc, char* argv[]) {

	if (argc < 2 ) {
		printUsage();
		exit(2);
	}

	int samplecount = atoi(argv[1]);
	int index = 0;
	double mean_counteroverhead = 0, stddev_counteroverhead = 0, stddev_syscalloverhead = 0, mean_syscalloverhead = 0;
	double syscallOverheadArray[samplecount];
	unsigned long long int start = 0, end = 0;
	mean_counteroverhead = getCounteroverhead(samplecount, &stddev_counteroverhead);

	
	// First time, it takes so many clock cycles. Run it at once so that it does not 
	// affect the measurements
	dup(1);
	for (index = 0; index < samplecount; index++) {
		start = count();
		dup(1);
		end = count();
		syscallOverheadArray[index] = end-start-mean_counteroverhead;
	}

	mean_syscalloverhead = getMeanStddev(syscallOverheadArray, samplecount, &stddev_syscalloverhead);
	printResult(samplecount, mean_syscalloverhead, stddev_syscalloverhead);

	return 0;
}
