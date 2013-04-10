#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

//for file operations
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

// include the cycle counter header file
#include "counter.h"
#include "cycletimer.h"
#include "utilFunc.h"

#define PAGESIZE 4096 
#define FILESIZE 200
#define SAMPLECOUNT 100

void error(const char* msg) {
	perror(msg);
	exit(2);
}

void printUsage(char* argv[]) {
	printf("%s <jumpsize> <filetomap>\n", argv[0]);
}

int main(int argc, char *argv[]) {

	if(argc < 3) {
		printUsage(argv);
		exit(1);
	}

	int jumpSize = atoi(argv[1]);
	unsigned long long int skipSize = jumpSize*PAGESIZE;
	//int samplecount = atoi(argv[2]);
	int samplecount = 0;
	char* inputFileName = argv[2];
	int inputFileDesc;

	//mmap buffer - let the mmap assign the virtual address
	char* buffer = NULL;

	//variable related to readings
	unsigned long long int start;
	unsigned long long int end;
	double mean_counteroverhead = 0, stddev_counteroverhead = 0,\
	       mean_pagefaultoverhead = 0, stddev_pagefaultoverhead = 0;
	// file related
	struct stat fileStat;
	unsigned long long int fileSize;

	//temp variables
	int i;
	char c;

	/////////////////////////////////////////////////////////////////////
	mean_counteroverhead = getCounteroverhead(NUMSAMPLES, &stddev_counteroverhead);

	if ((inputFileDesc = open(inputFileName, O_RDONLY|O_DIRECT)) == -1)
		error("error opening file");

    if (fstat(inputFileDesc, &fileStat) == -1)
		error("error getting stats");

	fileSize = fileStat.st_size;

	printf("File size: %llu\n", fileSize);
	printf("Actual number of pages: %llu\n", fileSize/PAGESIZE);

	if ((buffer = mmap(NULL, fileSize, PROT_READ,\
		               MAP_PRIVATE, inputFileDesc, 0)) == MAP_FAILED)
		error("error mmapping");

	// calc the total pages possible with the given jump
	i = 0;
	while(1) {
		if ((i*skipSize) >= fileSize)
			break;
		samplecount++;
		i++;
	}

	printf("Total pages gonna sample: %d\n", samplecount);

    double pagefaultOverheadArray[samplecount];

    for (i=0; i<samplecount; i++) {
		start = count();
	    c = buffer[(i*skipSize)%(fileSize-1)];	
		end = count();
		pagefaultOverheadArray[i] = end-start-mean_counteroverhead;
	}

	mean_pagefaultoverhead = getMeanStddev(pagefaultOverheadArray, samplecount, &stddev_pagefaultoverhead);

	//printResult(samplecount, mean_pagefaultoverhead, stddev_pagefaultoverhead, "PAGEFAULT");
	printResult(samplecount, 
	            getDurationinMilliSec(mean_pagefaultoverhead),\
				getDurationinMilliSec(stddev_pagefaultoverhead),\
			    "PAGEFAULT");

	if (munmap(buffer, fileSize) == -1)
		error("error unmapping");

    close(inputFileDesc);

	return 0;
}
