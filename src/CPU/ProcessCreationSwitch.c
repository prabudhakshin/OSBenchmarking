// Standard libraries
#include <stdlib.h>
#include <stdio.h>
// For pipe
#include <unistd.h>
#include <fcntl.h>

// User defined headers
// The corresponding dir is added to the path
#include "counter.h"
#include "cycletimer.h"

void printUsage() {
	printf("Enter the number of samples that you want to run.\n");
	printf("Usage:\n\t./ProcessOverhead <NUMSAMPLES>\n");
}

void printResult(int samples, double mean, double stddev, char* heading) {
	printf("\n");
	printf("---------------------------------------------\n");
	printf("       %s      \n", heading);
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
	int readwritedescp[2];
	int processId = 0, exitId = 0, index = 0;
	unsigned long long int start = 0, end = 0;
	double mean_counteroverhead = 0, stddev_counteroverhead = 0,\
	       mean_processoverhead = 0, stddev_processoverhead = 0,\
	       mean_processswitchoverhead = 0, stddev_processswitchoverhead = 0;
		   
	double processOverheadArray[samplecount];
	double processswitchOverheadArray[samplecount];

	mean_counteroverhead = getCounteroverhead(samplecount, &stddev_counteroverhead);

	if (!pipe2(readwritedescp, O_NONBLOCK))
		printf("Pipe created successfully!\n");
	else
		printf("Some problem with pipe creation\n");


	// MEASURE THE PROCESS CREATION TIME
	for ( index = 0; index < samplecount; index++) {
		// start the counter before forking - this will account the process creation time
		start = count();
		if ((processId = fork()) != 0) {
			exitId = wait();
			read(readwritedescp[0], (void*)&end, sizeof(end));
		}
		else {
			// as soon as new process starts executing, stop the timer
			end = count();
			write(readwritedescp[1], (void*)&end, sizeof(end));
			exit(1);
		}
		processOverheadArray[index] = end-start-mean_counteroverhead;
	}

	mean_processoverhead = getMeanStddev(processOverheadArray, samplecount, &stddev_processoverhead);
	printResult(samplecount, mean_processoverhead, stddev_processoverhead, "PROCESS CREATION TIME - cycles");
	printResult(samplecount, getDurationinMicroSec(mean_processoverhead),\
	                         getDurationinMicroSec(stddev_processoverhead),\
							 "PROCESS CREATION TIME - microsec");

	// MEASURE THE CONTEXT SWITCH TIME
	for ( index = 0; index < samplecount; index++) {
		if ((processId = fork()) != 0) {
			// as opposed to process creation time, start the timer just before doing a context switch
			start = count();
			exitId = wait();
			read(readwritedescp[0], (void*)&end, sizeof(end));
		}
		else {
			// stop the counter as soon as the child starts executing
			end = count();
			write(readwritedescp[1], (void*)&end, sizeof(end));
			exit(1);
		}

		processswitchOverheadArray[index] = end-start-mean_counteroverhead;
	}

	mean_processswitchoverhead= getMeanStddev(processswitchOverheadArray, samplecount, &stddev_processswitchoverhead) ;
	printResult(samplecount, mean_processswitchoverhead, stddev_processswitchoverhead, "CONTEXTSWITCH TIME - cycles");
	printResult(samplecount, getDurationinMicroSec(mean_processswitchoverhead),\
	                         getDurationinMicroSec(stddev_processswitchoverhead),\
							 "CONTEXTSWITCH TIME - microsec");

	return 0;
}
