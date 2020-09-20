
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
#include<sys/stat.h>
#include<sstream>


char * parseArgs(int, char **);
int getSock(std::string);
void directUser(int);
void * getCliMsg(int);
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

void * getCliMsg(int cliFD){

    char buf[BUFSIZ];
    int received;
	
    received = recv(cliFD, buf, BUFSIZ, 0);

    if(received == -1) {
		std::cerr << "Server failed on recv(): " << std::endl;
		std::exit(-1);
    }

	
	void * ret = buf; 
	return ret;
}

void directUser(int cliFD) {

	
	char * msg = (char *)getCliMsg(cliFD) ;


    if(!strcmp(msg,"DN")) {

		std::cout << "Got DN" << std::endl;

		char * fileToDownload = (char * ) getCliMsg(cliFD);
		downloadFile(fileToDownload, cliFD);
 
    } else if(!strcmp(msg, "UP")) {
        std::cout << "upload" << std::endl;

    } else if(!strcmp(msg, "HEAD")) {

        std::cout << "head" << std::endl;
		char * fileToGet = (char *)getCliMsg(cliFD);
		std::cout << fileToGet << std::endl;

    } else if(!strcmp(msg, "RM")) {

        std::cout << "rm" << std::endl;
		char * fileToRem = (char *)getCliMsg(cliFD);
		std::cout << fileToRem << std::endl;

    } else if(!strcmp(msg, "LS")) {

        std::cout << "ls" << std::endl;
        listing(cliFD);

    } else if(!strcmp(msg, "MKDIR")) {

        std::cout << "mkdir" << std::endl;
		char * dirName = (char *)getCliMsg(cliFD);
		std::cout << dirName << std::endl;

    }else if(!strcmp(msg, "RMDIR")) {

        std::cout << "rmdir" << std::endl;
		char * dirName = (char *)getCliMsg(cliFD);
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
	if (  (send(cliFD, toSend, len, 0)) == -1 ) {
		std::cerr << "Error Sending Info To Client: " << strerror(errno) << std::endl;
		std::exit(-1);
	}	
}

void downloadFile(char * filey, int cliFD){
	std::string filename = filey; 
	std::ifstream ifs;
	ifs.open(filename);
	short int check ;
	if (!ifs){
		std::cout << "File Not Found" << std::endl;
		check = -1;
		sendToCli( (void *)&check, sizeof(short int), cliFD);
		return;
	}

    // Check if file exists and send size to user if it does
/**	if(stat(filey, &stat_file) == 0) {
            check = stat_file.st_size;
            sendToCli((void *)check, sizeof(check), cliFD);
        } else {
            check = -1;
            sendToCli((void *)check, sizeof(check), cliFD);
            std::exit(-1);
        }  **/          

	// Send file size
    struct stat stat_file;
	if (    (stat(filename.c_str(), &stat_file)) == -1 ){
		std::cerr << "Error on Stat: " << strerror(errno) << std::endl;
	}
	check = stat_file.st_size;
	std::cout << "File Inode: " << stat_file.st_ino << std::endl;
	std::cout << "Sending file size: " << stat_file.st_size << std::endl;
	sendToCli( (void *)&check, sizeof(short int), cliFD);

	// Get md5Sum and send to client
    char mdsum [40] = "md5sum ";
    strcat(mdsum, filename.c_str());
	FILE * fd = popen(mdsum, "r");
	char md5sumOutput [30] ;
	fgets(md5sumOutput, 30, fd);
	
	char * hash = strtok(md5sumOutput, " ");

	std::cout << "Sending hash: " << hash << std::endl;
	sendToCli( (void *)hash, strlen(hash)+1 , cliFD) ;


		
	char buf[BUFSIZ];
	while( ifs.peek() != EOF){
		ifs.read(buf, BUFSIZ);
		sendToCli((void *)buf, strlen(buf)+1, cliFD);
	}

	ifs.close();

}


    

	
	



