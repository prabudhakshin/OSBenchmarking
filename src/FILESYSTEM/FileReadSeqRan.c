#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

//for file operations
#include <sys/stat.h>
#include <fcntl.h>

// include the cycle counter header file
#include "counter.h"
#include "cycletimer.h"
#include "utilFunc.h"


#define BLOCKSIZE 4096 
#define MAXFILELEN 200
#define SAMPLECOUNT 100

void error(const char* msg) {
	perror(msg);
	exit(2);
}

void printUsage(char* argv[]) {
	printf("%s <jumpsize> <numsamples> <indexfile>\n", argv[0]);
}

double calcLoopOverhead(double counteroverhead) {
	double mean_loopoverhead = 0, stddev_loopoverhead = 0;
	double loopOverheadArray[SAMPLECOUNT];
	unsigned long long int start;
	unsigned long long int end;
	int index=0, i=0;

	for (i=0; i<SAMPLECOUNT; i++) {
		index = 0;
		start = count();
		while(index < SAMPLECOUNT) {
			index++;
		}
		end = count();
		loopOverheadArray[i] = (end-start-counteroverhead)/SAMPLECOUNT;
	}

	mean_loopoverhead = getMeanStddev(loopOverheadArray, SAMPLECOUNT, &stddev_loopoverhead); 
	printResult(SAMPLECOUNT, mean_loopoverhead, stddev_loopoverhead, "LOOP OVERHEAD");
	return mean_loopoverhead;
}

double calcSeekOverhead(double counteroverhead) {
	double mean_seekoverhead = 0, stddev_seekoverhead = 0;
	double seekOverheadArray[SAMPLECOUNT];
	unsigned long long int start;
	unsigned long long int end;
	int fd;

	if ((fd = open("tmp.txt", O_RDONLY|O_DIRECT)) == -1)
		error("error opening file tmp.txt");

	int index=0;

	for (index=0; index<SAMPLECOUNT; index++) {
		start = count();
		if (lseek(fd, 1024, SEEK_CUR) == -1)
			error("Seek error in calcSeekOverhead");
		end = count();
		seekOverheadArray[index] = end-start-counteroverhead;
	}
	close(fd);

	mean_seekoverhead = getMeanStddev(seekOverheadArray, SAMPLECOUNT, &stddev_seekoverhead); 
	printResult(SAMPLECOUNT, mean_seekoverhead, stddev_seekoverhead, "SEEK OVERHEAD");
	return mean_seekoverhead;
}

int main(int argc, char *argv[]) {

	if(argc < 4) {
		printUsage(argv);
		exit(1);
	}

	int jumpSize = atoi(argv[1]);
	int samplecount = atoi(argv[2]);

	//buffers
	//char* buffer = (char*)malloc(BLOCKSIZE);
	void* buffer;
	if (posix_memalign(&buffer, BLOCKSIZE, BLOCKSIZE) != 0 )
		error("Mem align failed");

	//file related
	char* indexFilename = argv[3];
	char* inputFileName;
	double inputFileSize;

	//file descriptors
	FILE* indexFiledesc;
	int inputFileDesc;

	//variable related to readings
	unsigned long long int start;
	unsigned long long int end;
	double mean_counteroverhead = 0, stddev_counteroverhead = 0,\
		   mean_loopoverhead = 0, mean_seekoverhead = 0,\
	       mean_seqAccessoverhead = 0, stddev_seqAccessoverhead = 0,\
		   mean_randAccessoverhead = 0, stddev_randAccessoverhead = 0;
	double seqOverheadArray[samplecount],\
   	       randOverheadArray[samplecount];

	//counter/temp variables
	int bytesRead, index;
	char* aLine = (char*)malloc(MAXFILELEN);
	unsigned long long int numBlocksReadSeq, numBlocksReadRand;
	char c;

	////////////////////////////////////////////////////////////////////
	mean_counteroverhead = getCounteroverhead(NUMSAMPLES, &stddev_counteroverhead);
	mean_loopoverhead = calcLoopOverhead(mean_counteroverhead);
	mean_seekoverhead = calcSeekOverhead(mean_counteroverhead);

	indexFiledesc = fopen(indexFilename, "r");
	printf("%10s %10s %10s %10s %10s\n", "FileSize", "SeqAccTime", "Deviation", "RandAccTime", "Deviation");
	printf("========================================================================\n");
	while (fgets(aLine, MAXFILELEN, indexFiledesc) != NULL) 
	{

		// parse the file name and the size
		aLine[strlen(aLine)-1] = '\0';
		inputFileName = strtok(aLine, " ");
		inputFileSize = atof(strtok(NULL, " "));

		//printf("File Name: \"%s\"\tsize: %d\n", inputFileName, inputFileSize);

		if ((inputFileDesc = open(inputFileName, O_RDONLY|O_DIRECT)) == -1)
			error("error opening file");


		//read randomly 
		for (index = 0; index < samplecount; index++) {

			if (lseek(inputFileDesc, 0, SEEK_SET) == -1)
				error("Error seeking");

			numBlocksReadRand = 0;

			start = count();
			while (1) {
				bytesRead = read(inputFileDesc, buffer, BLOCKSIZE);

				if (bytesRead <= 0)
					break;

				numBlocksReadRand++;

				if (lseek(inputFileDesc, jumpSize*BLOCKSIZE, SEEK_CUR) == -1)
					error("Error seeking");
			}	
			end = count();


			//printf("Blocks read rand: %llu\n", numBlocksReadRand);
			randOverheadArray[index] = end-start-mean_counteroverhead\
			                           -(mean_loopoverhead*numBlocksReadRand)\
									   -(mean_seekoverhead*numBlocksReadRand);
			if (bytesRead == -1)
				error("error");
		}

		//read sequentially 
		for (index = 0; index < samplecount; index++) {

			if (lseek(inputFileDesc, 0, SEEK_SET) == -1)
				error("Error seeking");

			numBlocksReadSeq = 0;
			start = count();
			while (1) {
				bytesRead = read(inputFileDesc, buffer, BLOCKSIZE);
				//if ((++numBlocksReadSeq == numBlocksReadRand) || bytesRead <= 0 )
				if (bytesRead <= 0 )
					break;
				++numBlocksReadSeq;

			}	
			end = count();
			//printf("Blocks read seq: %llu\n", numBlocksReadSeq);
			seqOverheadArray[index] = end-start-mean_counteroverhead\
			                         -(mean_loopoverhead*numBlocksReadSeq);
			if (bytesRead == -1)
				error("error");
		}

		mean_seqAccessoverhead = getMeanStddev(seqOverheadArray, samplecount, &stddev_seqAccessoverhead);
		mean_randAccessoverhead = getMeanStddev(randOverheadArray, samplecount, &stddev_randAccessoverhead);

		printf("%10f %10f %10f %10f %10f\n", inputFileSize,\
		                                getDurationinMilliSec(mean_seqAccessoverhead/numBlocksReadSeq),\
		                                getDurationinMilliSec(stddev_seqAccessoverhead/numBlocksReadSeq),\
										getDurationinMilliSec(mean_randAccessoverhead/numBlocksReadRand),\
										getDurationinMilliSec(stddev_randAccessoverhead/numBlocksReadRand));

		close(inputFileDesc);
	}

	return 0;
}
