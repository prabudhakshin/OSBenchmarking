#define CPUSPEED (2.3*1024*1024*1024)
#define TIMEFORACYCLE (1/CPUSPEED)

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
    int sockfd, portno, n;
	int i;
	int samplecount = 0;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    //char buffer_rd[256];
    char buffer_rd[] = "Y";
    char buffer_wr[] = "X";

	// variables for tracking the time
	unsigned long long int start;
	unsigned long long int end;

	double mean_counteroverhead = 0, stddev_counteroverhead = 0,\
	       mean_pingoverhead = 0,    stddev_pingoverhead = 0;
	
	mean_counteroverhead = getCounteroverhead(100, &stddev_counteroverhead);


    if (argc < 4) {
       fprintf(stderr,"usage %s hostname port samplecount\n", argv[0]);
       exit(0);
    }

	samplecount = atoi(argv[3]);

	double pingOverheadArray[samplecount];

    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
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
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

	//printf("Strlen: %d\n", strlen(buffer_wr));
	//memset(buffer_wr, '\0', strlen(buffer_wr));
	for (i=0; i<strlen(buffer_wr); i++) {
		buffer_wr[i] = 'X';
	}

	printf("Buffer: %s\n", buffer_wr);

	for (i=0; i<samplecount; i++) {
		start = count();
		n = write(sockfd,buffer_wr,strlen(buffer_wr));
		n = read(sockfd,buffer_rd,strlen(buffer_rd));
		end = count();
		pingOverheadArray[i] = end-start-mean_counteroverhead;
	}

	printf("Read: %s\n", buffer_rd);

	mean_pingoverhead = getMeanStddev(pingOverheadArray, samplecount, &stddev_pingoverhead);
	printResult(samplecount, mean_pingoverhead, stddev_pingoverhead, "PING - clockcycles");
	printResult(samplecount, getDurationinMilliSec(mean_pingoverhead),\
	                         getDurationinMilliSec(stddev_pingoverhead), "PING - msecs");

//	printf("cycles: %f\n", end-start-counteroverhead);
//	printf("rtt for 256 bytes of data: %0.20f\n", getDurationinMicroSec(end-start-counteroverhead));
    close(sockfd);

    return 0;
}
