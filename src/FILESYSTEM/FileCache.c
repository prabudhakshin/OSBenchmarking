// Standard libraries
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <strings.h>
#include <string.h>

// User defined headers
// The corresponding dir is added to the path
#include "counter.h"
#include "cycletimer.h"
#include "utilFunc.h"

#ifndef __MATHHEADER__
	#include <mathoperations.h>
#endif

#define BUFFSIZE 4096 

void printUsage(char** argv) {
	printf("%s <indexfile> <samplecount>\n", argv[0]);
}

int main(int argc, char* argv[]) {

	if (argc < 3 ) {
		printUsage(argv);
		exit(2);
	}

	char* indexFileName = argv[1];
	int samplecount = atoi(argv[2]);
	int index = 0, bytesRead = 0;
	FILE *indexFileDesc;
	int fd;
	int f;
	char* targetArray = (char*)malloc(BUFFSIZE);
	char* fileName = (char*)malloc(30);

	double mean_counteroverhead = 0, stddev_counteroverhead = 0,\
	       stddev_fileReadoverhead = 0, mean_fileReadoverhead= 0;
	double fileReadOverheadArray[samplecount];

	unsigned long long int start = 0, end = 0;

	mean_counteroverhead = getCounteroverhead(samplecount, &stddev_counteroverhead);
	printCounterOverheadResults(samplecount, mean_counteroverhead, stddev_counteroverhead);

	bzero(fileName, sizeof(fileName));
	indexFileDesc = fopen(indexFileName, "r");

	int fileSize = 400;
	char c;
	while (fgets(fileName, 30, indexFileDesc)) {
		fileName[strlen(fileName)-1] = '\0';
		printf("Gonna read file: \"%s\"; Clean the cache and press enter to continue...\n", fileName);
		c = getchar();
		for(index = -1; index < samplecount; index++) {

			fd = open(fileName, O_RDONLY);

			start = count();
			while ((bytesRead = read(fd, targetArray, BUFFSIZE)) > 0) {
			}
			end = count();
			if (bytesRead == -1) {
				perror("Error while reading");
				exit(2);
			}

			if (bytesRead == 0) {
				//perror("Read zero bytes");
			}

			// omit the first sample as it will come from the disk; 
			// For files which exceed the cache size, even further reads will come from disk
			if (index != -1)
				fileReadOverheadArray[index] = end-start-mean_counteroverhead;

			close(fd);
		}
		mean_fileReadoverhead = getMeanStddev(fileReadOverheadArray, samplecount, &stddev_fileReadoverhead);
		//printResult(samplecount, mean_fileReadoverhead, stddev_fileReadoverhead, "FILE READ - cycles");
		//printResult(samplecount, mean_fileReadoverhead, stddev_fileReadoverhead, "FILE READ - cycles");
		printf("%25s\t:%20f\t%20f\t%20f\n", fileName, getDurationinMilliSec(mean_fileReadoverhead),\
		                                            getDurationinMilliSec(stddev_fileReadoverhead),\
													fileSize/getDurationinSec(mean_fileReadoverhead));
//		printResult(samplecount, getDurationinMilliSec(mean_fileReadoverhead),\
//				getDurationinMilliSec(stddev_fileReadoverhead), fileName);
				//getDurationinMilliSec(stddev_fileReadoverhead), "FILE READ - msecs");
		bzero(fileName, sizeof(fileName));
		fileSize += 400;
	}

	return 0;
}
