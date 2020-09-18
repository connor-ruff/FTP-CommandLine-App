
#include<string>
#include<iostream>
#include<dirent.h>
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
#include<fstream>


char * parseArgs(int, char **);
int getSock(std::string);
void directUser(int);
char * getCliMsg(int);
void listing(int);
void sendToCli(void *, int, int);
void downloadFile(char *, int);

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
		if ( (cliFD = accept(sockfd, (struct sockaddr *)&cliAddr, (socklen_t *) &addr_len)) < 0){
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

char * getCliMsg(int cliFD){

    char buf[BUFSIZ];
    int received; 
	
    received = recv(cliFD, buf, BUFSIZ, 0);

    if(received == -1) {
		std::cerr << "Server failed on recv(): " << std::endl;
		std::exit(-1);
    }

	char * ret = buf;
	return ret;
}

void directUser(int cliFD) {

	
	char * msg = getCliMsg(cliFD) ;


    if(!strcmp(msg,"DN")) {

		char * fileToDownload = getCliMsg(cliFD);
		std::cout << fileToDownload << std::endl;
		downloadFile(fileToDownload, cliFD);
 
    } else if(!strcmp(msg, "UP")) {
        std::cout << "upload" << std::endl;

    } else if(!strcmp(msg, "HEAD")) {

        std::cout << "head" << std::endl;
		char * fileToGet = getCliMsg(cliFD);
		std::cout << fileToGet << std::endl;

    } else if(!strcmp(msg, "RM")) {

        std::cout << "rm" << std::endl;
		char * fileToRem = getCliMsg(cliFD);
		std::cout << fileToRem << std::endl;

    } else if(!strcmp(msg, "LS")) {

        std::cout << "ls" << std::endl;
        listing(cliFD);

    } else if(!strcmp(msg, "MKDIR")) {

        std::cout << "mkdir" << std::endl;
		char * dirName = getCliMsg(cliFD);
		std::cout << dirName << std::endl;

    }else if(!strcmp(msg, "RMDIR")) {

        std::cout << "rmdir" << std::endl;
		char * dirName = getCliMsg(cliFD);
		std::cout << dirName << std::endl;

    }else if(!strcmp(msg, "CD")) {

        std::cout << "cd" << std::endl;


    }else if(!strcmp(msg, "QUIT")) {

        std::cout << "quit" << std::endl;

    } else {

        std::cerr << "Command not recognized: " << std::endl;

    }
} 

void listing(int cliFD) {


    //BOOST FILESYSTEM?
    char list[BUFSIZ][BUFSIZ];
	FILE * out = popen("ls -l", "r");
	char buffer[BUFSIZ];

	int size = 0;
	while( !feof(out) ) {
		if ( fgets(buffer, sizeof(buffer), out) != NULL) {
			strcpy(list[size], buffer);
			size++;
		}
	}

	size--;  // Since first entry is not a file

	sendToCli( (void *)&size , sizeof(size), cliFD);
}

// TODO
void sendToCli(void * toSend, int len, int cliFD){
	send(cliFD, toSend, len, 0);	
}

void downloadFile(char * filey, int cliFD){

	// Check if file exists
	

	// Get md5Sum and send to client
	FILE * out = popen("md5sum " + filey, "r");
	
	// Get Size of File and send to client


	// Send file to client
	std::ifstream ifs;

	ifs.open(filey);
	if (!ifs){
		//TODO: return -1
		std::exit(-1);
	}
	
	char buf[BUFSIZ];
	while( ifs.peek() != EOF){
		ifs.read(buf, BUFSIZ);
		sendToCli((void *)buf, strlen(buf)+1, cliFD);
	}

}


    

	
	



