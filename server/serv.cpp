
#include<string>
#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<cstdlib>
#include<cerrno>
#include<unistd.h>


char * parseArgs(int, char **);
int getSock(std::string);
void directUser(int);
void parseCommands(void *);
int main(int argc, char ** argv){

	// Get CL Arg
	std::string port;
	port = parseArgs(argc, argv);

	// Estbalish sock
	int sockfd = getSock(port);
	std::cout << "Waiting for connection on port " << port << "...\n" ;

	// Accept a Connection
	int cliFD;
	struct sockaddr_in cliAddr;
	int addr_len = sizeof(cliAddr);
	while (true){
		if ( cliFD = accept(sockfd, (struct sockaddr *)&cliAddr, (socklen_t *) &addr_len) < 0){
			std::cerr << "Error on Accepting Client Connection" << std::endl;
			std::exit(-1);
		} else {

                    std::cout << "Connection Established" << std::endl;

                }


		// Recieve information
		directUser(cliFD); 
	}


	return 0;
}

char * parseArgs(int argc, char ** argv){

	if (argc != 2){
		std::cerr << "Wrong Number of Arguments\n" ;
		std::exit(-1);
	}

	return argv[1];


}

int getSock(std::string port){

	int sockfd;
	struct sockaddr_in sin;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	struct addrinfo * results;

	// Load Host address info
	int status;
	if( ( status = getaddrinfo(NULL, port.c_str(), &hints, &results)) != 0 ){
		std::cerr << "Server Failure on getaddrinfo(): " << gai_strerror(status) << std::endl;
		std::exit(-1);
	}

	// socket
	if ( (sockfd = socket(results->ai_family, results->ai_socktype, 0)) < 0 ){
		std::cerr << "Server Error on Socket(): " << strerror(errno) << std::endl;
		std::exit(-1);
	}


	// Bind
	int bindRes;
	if( bindRes = bind(sockfd, results->ai_addr, results->ai_addrlen) == -1) {
		std::cerr << "Server Error on Bind(): " << strerror(errno) << std::endl;
		close(sockfd);
		std::exit(-1);
	}

	freeaddrinfo(results);

	// listen
	

	if ( listen(sockfd, 5) < 0) {
		std::cerr << "Server Failure on listen(): " << std::endl;
		close(sockfd);
		std::exit(-1);
	}

	return sockfd;
}

void directUser(int cliFD){

	char buf[BUFSIZ];
        int received; 

        std::cout << cliFD << std::endl;

	while(true) {
		
		received = recv(cliFD, buf, BUFSIZ, 0);
                if(received != -1) {
                    std::cout << "IN" << std::endl;
                    parseCommands((void *)buf);
                } else {
                    //std::cerr << "Server failed on recv(): " << std::endl;

                }

	}

}

void parseCommands(void *com) {
    char *command = (char *)com;

    if(strcmp(command,"DN")) {

        std::cout << "download" << std::endl;
        
    } else if(strcmp(command, "UP")) {

        std::cout << "upload" << std::endl;

    } else if(strcmp(command, "HEAD")) {

        std::cout << "head" << std::endl;

    } else if(strcmp(command, "RM")) {

        std::cout << "rm" << std::endl;

    } else if(strcmp(command, "LS")) {

        std::cout << "ls" << std::endl;

    } else if(strcmp(command, "MKDIR")) {

        std::cout << "mkdir" << std::endl;

    }else if(strcmp(command, "RMDIR")) {

        std::cout << "rmdir" << std::endl;

    }else if(strcmp(command, "CD")) {

        std::cout << "cd" << std::endl;


    }else if(strcmp(command, "QUIT")) {

        std::cout << "quit" << std::endl;

    } else {

        std::cerr << "Command not recognized: " << std::endl;

    }
} 
    

	
	



