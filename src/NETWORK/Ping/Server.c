#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg)
{
	perror(msg);
	//exit(1);
}

int main(int argc, char *argv[])
{
	// listening socket fd, new file descriptor for a connection
	int sockfd, newsockfd, portno;
	socklen_t clilen;
	char buffer[] = "-";
	char buffer_wr[] = "W";
	// sockaddr_in contains 3 fields: family, port and address
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	// argv[1] is the port on which the server wants to listen
	if (argc < 2) {
		fprintf(stderr,"ERROR, no port provided\n");
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

		// read and write to the socket
		while((n = read(newsockfd,buffer,strlen(buffer))) > 0) {
			//printf("Read: %s\n", buffer);
			n = write(newsockfd,buffer_wr,strlen(buffer_wr));
		}

		printf("N value: %d\n", n);
		if (n < 0) error("ERROR writing to socket");
		if (n == 0) error("ERROR - Remote side has closed the connection!");

		// close the socket for the particular client
		close(newsockfd);
	}

	// close the listener socket 
	close(sockfd);

	return 0; 
}
