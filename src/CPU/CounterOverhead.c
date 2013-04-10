// Standard libraries
#include <stdlib.h>
#include <stdio.h>

// User defined headers
// The corresponding dir is added to the path
#include <counter.h>

void printUsage() {
	printf("Enter the number of samples that you want to run.\n");
	printf("Usage:\n\t./CounterOverhead <NUMSAMPLES>\n");
}

int main(int argc, char* argv[]) {

	if (argc < 2 ) {
		printUsage();
		exit(2);
	}

	int samplecount = atoi(argv[1]);
	double mean = 0, stddev = 0;
	mean = getCounteroverhead(samplecount, &stddev);
	printCounterOverheadResults(samplecount, mean, stddev);

	return 0;
}
