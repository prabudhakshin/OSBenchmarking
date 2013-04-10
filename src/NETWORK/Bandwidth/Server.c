#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define PACKETSIZE 1
#define STREAMSIZE (1024*1024*1024)
#define NUMPACKETS (STREAMSIZE/PACKETSIZE)

void error(const char *msg)
{
	perror(msg);
	//exit(1);
}

int main(int argc, char *argv[])
{
	// listening socket fd, new file descriptor for a connection
	int sockfd, newsockfd, portno;
	FILE* fd;
	socklen_t clilen;
	char* buffer = (char*) malloc(STREAMSIZE);
	char buffer_wr[] = "W";
	// sockaddr_in contains 3 fields: family, port and address
	struct sockaddr_in serv_addr, cli_addr;
	int i = 0, index = 0, sizeInGB = 0, samplecount = 0;
	//long long unsigned int n = 0;
	int n = 0;
	// argv[1] is the port on which the server wants to listen
	if (argc < 4) {
		fprintf(stderr,"Usage %s port sizeingb samplecount\n", argv[0]);
		exit(1);
	}
	// create socket of address domain AF_INET, use stream protocol
	// 0 specifies the OS to choose the right protocol
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		error("ERROR opening socket");
	
	// zero fill the sever address
	bzero((char *) &serv_addr, sizeof(serv_addr));
	// read the port number from the command line
	portno = atoi(argv[1]);
	sizeInGB = atoi(argv[2]);
	samplecount = atoi(argv[3]);

	// address domain: internet address; other possible domain is AF_UNIX
	serv_addr.sin_family = AF_INET;
	// INADDR_ANY gets the address of the machine which hosts the server program
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	// convert the port from host to network byte order
	serv_addr.sin_port = htons(portno);
	// bind the socket just created to the specific server address
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
				sizeof(serv_addr)) < 0) 
		error("ERROR on binding");
	// start listening in the socket; allow upto 5 client process in queue to establish a connection
	while(1) {
		listen(sockfd,5);
		clilen = sizeof(cli_addr);
		// accecpt new connection request and create a new socket for the connection that is just established
		newsockfd = accept(sockfd, 
				(struct sockaddr *) &cli_addr, 
				&clilen);

		printf("Accepted connection!\n");
		if (newsockfd < 0) 
			error("ERROR on accept");
		fd = fdopen(newsockfd, "w");

		// read and write to the socket
		for (i=0; i<samplecount; i++) {
			for(index = 0; index < sizeInGB; index++ ) {
				n = fwrite(buffer, PACKETSIZE, NUMPACKETS, fd);
				if(n != NUMPACKETS) {
					//printf("%d:ERROR; dint tranfer all bytes: %llu\n", index, n);
					printf("%d:ERROR; dint tranfer all bytes: %d\n", index, n);
				}
			}
			//sleep(2);
		}

		//printf("N value: %llu\n", n);
		if (n < 0) error("ERROR writing to socket");
		if (n == 0) error("ERROR - Remote side has closed the connection!");

		// close the socket for the particular client
		fclose(fd);
	}

	// close the listener socket 
	close(sockfd);

	return 0; 
}
