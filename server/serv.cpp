
#include<string>
#include<iostream>
#include<dirent.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/time.h>
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
			usleep(1000);

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
/* Server side implementation of UP */
	std::string filey_str = filey;
	int valread;
	char buffer[BUFSIZ];
	//usleep(100); //TODO check if matters
	// recieve the name of the file we are getting
	std::cout << "We are receiving the file: " << filey << std::endl;

	// Recieve the size of the file
	// possible endianness issues?
	int fileSize = 0;
	read(cliFD, (int *)&fileSize, sizeof(fileSize));
	std::cout << "Filesize: " << fileSize << std::endl;


	// Acknowledge that we are ready to recieve
	send(cliFD, "READY", strlen("READY")+1, 0);

	// Start calculating time
	struct timeval b4;
	gettimeofday(&b4, NULL);

	// Manipulate recive max based on filesize
	size_t sizey;
	if (fileSize > BUFSIZ)
		sizey = BUFSIZ;
	else 
		sizey = fileSize;
	int totalSent = 0;
	std::cout << "filey b4 fopen " << filey << std::endl;
	FILE *wf = fopen(filey, "wb");
	std::cout << "Recieving File... " << std::endl << std::endl;
	while (totalSent < fileSize) {
		valread = recv(cliFD, buffer, sizey, 0);
		totalSent += valread;
		fwrite(buffer, 1, valread, wf);
		bzero(&buffer, sizeof(buffer));
	}

	struct timeval after;
	gettimeofday(&after, NULL);
	fclose(wf);
	float elapsedTime = (((after.tv_sec - b4.tv_sec) * 1000000) + (after.tv_usec - b4.tv_usec));
	float throughPut = ((fileSize * 8)) / (elapsedTime / 1000000);
	// Send the throughput,
	send(cliFD, (void *)&throughPut, sizeof(throughPut), 0);

	// Send the hash
	std::cout << "Entering hash section" << std::endl;
	char md5sum[40] = "md5sum ";
	std::cout << "filey_str " << filey_str << std::endl;
	strcat(md5sum, filey_str.c_str());
	std::cout << md5sum << std::endl;
	FILE *dfile = popen(md5sum, "r");
	char md5sumOutput[50];
	fgets(md5sumOutput, 50, dfile);
	pclose(dfile);

	std::cout << "pre strtok " << md5sumOutput << std::endl;
	char *hash = strtok(md5sumOutput, " ");
	std::cout << "hash is: "<< hash << std::endl;
	send(cliFD, hash, strlen(hash) + 1, 0);
} 


	
	



