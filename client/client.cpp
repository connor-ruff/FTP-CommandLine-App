#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

char* PROGRAM_NAME;

void usage(int arg){
	std::cout << PROGRAM_NAME << "[PORT] [File/Text\n";
	exit(arg);
}

FILE* get_socket(){
	FILE * fd;
	if ((fd = socket(AF_INET, SOCK_STREAM, 0) < 0) {
		std::cerr << "Could not create socket\n";
	   	return NULL;	
	}	
	return fd;

}

int main(int argc, char* argv[]){
	struct hostent *hp; // host info
	struct sockaddr_in servaddr; // server address
	socklen_t addrlen = sizeof(servaddr);
	// Handle IO
	PROGRAM_NAME = argv[0];
	if (argc != 3)
		usage(EXIT_FAILURE);
	char* serverName = argv[1];
	int port = atoi(argv[2]);
	
	FILE *fd = get_socket();
	if (!fd){
		cout << "Creating file descriptor failed\n");
		exit(EXIT_FAILURE);
	}

	memset((char*)&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port htons(port);

	// Get host name
	hp = gethostbyname(serverName);
	if(!hp){
		fprintf(stderr, "could not obtain address of %s\n", serverName);
	}
	// put hosts address into server address structure
	memcpy((void *)&servaddr.sin_addr, hp->h_addr_list[0], hp->h_length);

	// send a test message
	char* testmsg = "THIS IS A TEST";
	if(sendto(fd, testmsg, strlen(msg), 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
		cout << " test failed\n";
	}
	
}
