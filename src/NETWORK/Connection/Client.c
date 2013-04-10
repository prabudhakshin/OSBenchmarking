#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

// include the cycle counter header file
#include "counter.h"
#include "cycletimer.h"
#include "utilFunc.h"


void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno;

	int i, index = 0;
	int samplecount = 0;
    struct sockaddr_in serv_addr;
    struct hostent *server;

	// variables for tracking the time
	unsigned long long int start;
	unsigned long long int end;

	double mean_counteroverhead = 0, stddev_counteroverhead = 0,\
	       mean_setupoverhead = 0,   stddev_setupoverhead = 0,\
	       mean_teardownoverhead = 0,stddev_teardownoverhead = 0;
	
	mean_counteroverhead = getCounteroverhead(100, &stddev_counteroverhead);

    if (argc < 4) {
       fprintf(stderr,"usage %s hostname port samplecount \n", argv[0]);
       exit(0);
    }

	samplecount = atoi(argv[3]);

	double setupOverheadArray[samplecount];
	double teardownOverheadArray[samplecount];

    portno = atoi(argv[2]);
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);

	for (i=0; i<samplecount; i++) {

		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0) 
			error("ERROR opening socket");

		printf("waiting for server....\n");
		start = count();
		if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
			error("ERROR connecting");
		end = count();
		printf("Connected to server: %d\n", i+1);

		setupOverheadArray[i] = end-start-mean_counteroverhead;

		start = count();
		close(sockfd);
		end = count();

		teardownOverheadArray[i] = end-start-mean_counteroverhead;
	}

	mean_setupoverhead = getMeanStddev(setupOverheadArray, samplecount, &stddev_setupoverhead);
	printResult(samplecount, mean_setupoverhead, stddev_setupoverhead, "CONN SETUP - clockcycles");
	printResult(samplecount, getDurationinMilliSec(mean_setupoverhead),\
	                         getDurationinMilliSec(stddev_setupoverhead), "CONN SETUP - msecs");

	mean_teardownoverhead = getMeanStddev(teardownOverheadArray, samplecount, &stddev_teardownoverhead);
	printResult(samplecount, mean_teardownoverhead, stddev_teardownoverhead, "CONN teardown - clockcycles");
	printResult(samplecount, getDurationinMilliSec(mean_teardownoverhead),\
	                         getDurationinMilliSec(stddev_teardownoverhead), "CONN teardown - msecs");

    return 0;
}
