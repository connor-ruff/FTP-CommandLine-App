
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
#include<algorithm>


char * parseArgs(int, char **);
int getSock(std::string);
void directUser(int);
void * getCliMsg(int);
void listing(int);
size_t sendToCli(void *, int, int);
void downloadFile(char *, int);
void uploadFile(char *, int);

int main(int argc, char ** argv){

	// Get CL Arg
	std::string port;
	port = parseArgs(argc, argv);

	// Estbalish sock
	int sockfd = getSock(port);
	

	// Accept a Connection
	int cliFD;
	struct sockaddr_in cliAddr;
	int addr_len = sizeof(cliAddr);
	while (true){
                std::cout << "Waiting for connection on port " << port << "...\n" ;
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

    bzero(&buf, sizeof(buf));
    received = recv(cliFD, buf, BUFSIZ, 0);
	

    std::cout << "BUFFER: " << buf  << " length: " << received << std::endl;

    if(received == -1) {
		std::cerr << "Server failed on recv(): " << std::endl;
		std::exit(-1);
    }

        buf[received] = '\0';


	
	void * ret = buf; 
	return ret;
}

void directUser(int cliFD) {

	while (true) {	

		char * msg = (char *)getCliMsg(cliFD) ;
        std::cout << "MESSAGE: " << msg << std::endl;

		if(!strcmp(msg,"DN")) {

			std::cout << "Got DN" << std::endl;

			char * fileToDownload = (char * ) getCliMsg(cliFD);
			downloadFile(fileToDownload, cliFD);
			usleep(100); // USELESS?
 
		 } else if(!strcmp(msg, "UP")) {

			std::cout << "upload" << std::endl;
			char *fileToUpload = (char * ) getCliMsg(cliFD);
			uploadFile(fileToUpload, cliFD);

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

		} else if(!strcmp(msg, "RMDIR")) {

			std::cout << "rmdir" << std::endl;
			char * dirName = (char *)getCliMsg(cliFD);
			std::cout << dirName << std::endl;

		} else if(!strcmp(msg, "CD")) {

			std::cout << "cd" << std::endl;


		} else if(!strcmp(msg, "QUIT")) {

			std::cout << "quit" << std::endl;
                        break;

		 } else {

			std::cerr << "Command not recognized: " << std::endl;
                        break;

		 }

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
size_t sendToCli(void * toSend, int len, int cliFD){
    size_t sent = send(cliFD, toSend, len, 0);
	if ( sent == -1 ) {
		std::cerr << "Error Sending Info To Client: " << strerror(errno) << std::endl;
		std::exit(-1);
	} 

        return sent;
                
}

void downloadFile(char * filey, int cliFD){
	// THIS is the server side implementation of the DN
	std::string filename = filey; 
    FILE *fd = fopen(filey, "rb");
	int check ;
	if (!fd){
		std::cout << "File Not Found" << std::endl;
		check = -1;
		sendToCli( (void *)&check, sizeof(int), cliFD);
		return;
	}
   

	// Send file size
        struct stat stat_file;
	if (    (stat(filename.c_str(), &stat_file)) == -1 ){
		std::cerr << "Error on Stat: " << strerror(errno) << std::endl;
	}
	check = stat_file.st_size;
	sendToCli( (void *)&check, sizeof(int), cliFD);

	// Get md5Sum and send to client
    char mdsum[40] = "md5sum ";
    strcat(mdsum, filename.c_str());
	FILE * fsum = popen(mdsum, "r");
	char md5sumOutput[50];
	fgets(md5sumOutput, 50, fsum);
	
	char * hash = strtok(md5sumOutput, " ");

	sendToCli( (void *)hash, strlen(hash)+1 , cliFD) ;
	
	char buf[BUFSIZ];
    size_t siz;
    size_t ck;
    size_t num;

	if(check > 0){
            do {
				bzero(&buf, BUFSIZ);
                if(check < BUFSIZ) {
                    num = check;
                } else {
                    num = BUFSIZ;
                }

                num = fread(buf, 1, num, fd);


                if(num < 1)
                    break;
              
                if(!(siz = sendToCli((void *)buf, num, cliFD))) 
                    break;
                check -= num;
            }
            while (check > 0);
        }
	

	fclose(fd);

	return;

}


void uploadFile(char * filey, int cliFD){
	/*
        std::string filename = filey; 
	std::ofstream ofs (filename);
	ifs.open(filename);
        short int check;
	if (!ifs){
		std::cout << "File Not Made" << std::endl;
                check = -1;
		return;
	} 

        // acknowledge ready to recieve
        sendToCli((void *)check, sizeof(check), cliFD);

        // get size of file
        short int fileSize;
	valread = read(fd, (int *)&fileSize, sizeof(fileSize));
	if (fileSize == -1){
		return;
	}

        char buffer[BUFSIZ];
        int totalSent = 0;
	bzero( &buffer, sizeof(buffer));
	while( (valread	= recv(fd, buffer, fileSize, 0))  > 0 ){
		totalSent += valread ;
		ofs << buffer;
		bzero( &buffer, sizeof(buffer));
		if ( totalSent >= fileSize ) {
			break;
		}

        }







   

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
                printf("%s", buf);
		sendToCli((void *)buf, strlen(buf)+1, cliFD);
                bzero(&buf, sizeof(buf));
	}

	ifs.close(); */

} 


	
	



