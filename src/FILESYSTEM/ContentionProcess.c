#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

//for file operations
#include <sys/stat.h>
#include <fcntl.h>

#define BLOCKSIZE 4096 
#define MAXFILELEN 30

void error(const char* msg) {
	perror(msg);
	exit(2);
}

void printUsage(char* argv[]) {
	printf("%s <random> <jumpsize> <filetooperate>\n", argv[0]);
}

int main(int argc, char *argv[]) {

	if(argc < 3) {
		printUsage(argv);
		exit(1);
	}

	void* buffer;
	if (posix_memalign(&buffer, BLOCKSIZE, BLOCKSIZE) != 0 )
		error("Mem align failed");

	//file related
	int doRandom = atoi(argv[1]);
	int jumpSize = atoi(argv[2]);
	char* inputFileName = argv[3];

	//file descriptors
	int inputFileDesc;

	//counter/temp variables
	int bytesRead, index;

	////////////////////////////////////////////////////////////////////

	if ((inputFileDesc = open(inputFileName, O_RDONLY|O_DIRECT)) == -1)
		error("error opening file");


	//read randomly 
	while(1) {

		if (lseek(inputFileDesc, 0, SEEK_SET) == -1)
			error("Error seeking");

		if (doRandom) {
			while (1) {
				bytesRead = read(inputFileDesc, buffer, BLOCKSIZE);

				if (bytesRead <= 0)
					break;

				if (lseek(inputFileDesc, jumpSize*BLOCKSIZE, SEEK_CUR) == -1)
					error("Error seeking");
			}	
		}
		else {
			while (1) {
				bytesRead = read(inputFileDesc, buffer, BLOCKSIZE);
				if (bytesRead <= 0)
					break;
			}	
		}

		if (bytesRead == -1)
			error("error");
	}

	close(inputFileDesc);

	return 0;
}
