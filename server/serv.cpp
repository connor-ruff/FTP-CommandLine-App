// Connor Ruff - cruff
// Kelly Buchanan - kbuchana
// Ryan Wigglesworth - rwiggles
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
void * getCliMsg(int, int);
void listing(int);
size_t sendToCli(void *, int, int);
void downloadFile(char *, int);
void uploadFile(char *, int);
void getHead(char *, int);
void changeDir(int, char *);
void makeDir(int, char *);
void remDir(int, char *);
void remFile(int, char *);

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

	// Continuously Search For Connections
	while (true){
                std::cout << "Waiting for connection on port " << port << "...\n" ;
		if ( (cliFD = accept(sockfd, (struct sockaddr *)&cliAddr, (socklen_t *) &addr_len)) < 0){
			std::cerr << "Error on Accepting Client Connection" << std::endl;
			std::exit(-1);
		} else {

                    std::cout << "Connection Established" << std::endl;

                }

		// Send New Connection to User Directory
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

void * getCliMsg(int cliFD, int recSize){

    char buf[BUFSIZ];
    int received;

    bzero(&buf, sizeof(buf));
    received = recv(cliFD, buf, recSize, 0);

    if(received == -1) {
		std::cerr << "Server failed on recv(): " << std::endl;
		std::exit(-1);
    }

        buf[received] = '\0';


	
	void * ret = buf; 
	return ret;
}

void directUser(int cliFD) {

	int comm = 0;
	while (true) {	

		// Get Command From User (Encoded via int's)
		comm = *((int *)getCliMsg(cliFD, sizeof(int))) ;

		switch(comm) {
			case 1: { 
	
				short int filenameSize = *((short int *) getCliMsg(cliFD, sizeof(short int)));	
				char * fileToDownload = (char * ) getCliMsg(cliFD, (int) filenameSize);
				downloadFile(fileToDownload, cliFD);
			}
				break;
			
	 
			case 2: {

				short int filenameSize = *((short int *) getCliMsg(cliFD, sizeof(short int)));
				char *fileToUpload = (char * ) getCliMsg(cliFD, (int)filenameSize); // TODO
				uploadFile(fileToUpload, cliFD);
			}
				break;

			case 3: {
				short int filenameSize = *((short int *) getCliMsg(cliFD, sizeof(short int)));
				char * fileToGet = (char *)getCliMsg(cliFD, (int)filenameSize); // TODO
				getHead(fileToGet, cliFD);
			}
				break;

			case 4: {
				short int filenameSize = *((short int *) getCliMsg(cliFD, sizeof(short int)));
				char * fileToRem = (char *)getCliMsg(cliFD, (int)filenameSize); // TODO
				remFile(cliFD, fileToRem);
			}
				break;

			case 5: {
				listing(cliFD);
			}
				break;

			case 6: {
				short int filenameSize = *((short int *) getCliMsg(cliFD, sizeof(short int)));
				char * dirName = (char *)getCliMsg(cliFD, (int)filenameSize); // TODO
				makeDir(cliFD, dirName);
			}
				break;

			case 7: {
				short int filenameSize = *((short int *) getCliMsg(cliFD, sizeof(short int)));
				char * dirName = (char *)getCliMsg(cliFD, (int)filenameSize); // TODO
				remDir(cliFD, dirName);
			}
				break;

			case 8: {
				short int filenameSize = *((short int *) getCliMsg(cliFD, sizeof(short int)));
				char * dirName = (char *)getCliMsg(cliFD, (int)filenameSize); // TODO
				changeDir(cliFD, dirName);
			}
				break;


			case 9: {
				close(cliFD);
                                return;
			}
			case 0: 
				break;

			default: {
				std::cerr << "Command not recognized: " << std::endl;
			}
				break;
		}

		 

	}
}

void remFile(int cliFD, char * fileName){

	int ret;
        int success;

	struct stat dirList;
	if ( (stat(fileName, &dirList)) == -1 ){
		ret = -1;
		send(cliFD, (void *)&ret, sizeof(int), 0);
		return;
	}
	else {
		ret = 1;
		send(cliFD, (void *)&ret, sizeof(int), 0);
	}

	recv(cliFD, (void *)&ret, sizeof(int), 0);
	if ( ret == 1 ) {	
            success = remove(fileName);
        } 

	
	send(cliFD, (void *)&success, sizeof(int), 0);
}

 
void remDir(int cliFD, char * dirName){

	int ret;

	struct stat dirList;
	if ( (stat(dirName, &dirList)) == -1 ){
		ret = -1;
		send(cliFD, (void *)&ret, sizeof(int), 0);
		return;
	}
	
	else if (    (ret = rmdir(dirName)) == -1  ) {
		ret = -2;
		send(cliFD, (void *)&ret, sizeof(int), 0);
		return;
	}
	else {
		ret = 1;
		send(cliFD, (void *)&ret, sizeof(int), 0);
	}

	recv(cliFD, (void *)&ret, sizeof(int), 0);
	if ( ret == -1 ) {
		mkdir(dirName, 0777);
	}
	
	send(cliFD, (void *)&ret, sizeof(int), 0);
}

void makeDir(int cliFD, char * dirName){

	int ret; 

	struct stat dirList;
	if ( (stat(dirName, &dirList)) != -1 ){
		ret = -2;
		send(cliFD, (void *)&ret, sizeof(int), 0);
		return;
	}

	ret = mkdir(dirName, 0777);
	send(cliFD, (void *)&ret, sizeof(int), 0);

}

void changeDir(int cliFD, char * dirName){

	int ret; 

	struct stat dirList;
	if ( (stat(dirName, &dirList)) == -1 ){
		ret = -2;
		send(cliFD, (void *)&ret, sizeof(int), 0);
		return;
	}

	ret = chdir(dirName);
	send(cliFD, (void *)&ret, sizeof(int), 0);



}

void listing(int cliFD) {

	FILE * fList = popen("ls -l", "r");
	char buffer[BUFSIZ];
	if(!fList){
		std::cout << "Error on Popen" << std::endl;
		std::exit(1);
		return;
	}
	fread(buffer, 1, BUFSIZ, fList);
	int bufSiz = strlen(buffer)+1;
	sendToCli((void *)&bufSiz, sizeof(int), cliFD);
	sendToCli(buffer, bufSiz, cliFD);
	pclose(fList);


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
//	std::string filename = filey; 
    FILE *fd = fopen(filey, "rb");
	int check ;

	if (!fd){
		check = -1;
		sendToCli( (void *)&check, sizeof(int), cliFD);
		return;
	}
   

	// Send file size
    struct stat stat_file;
	if (    (stat(filey, &stat_file)) == -1 ){
		std::cerr << "Error on Stat: " << strerror(errno) << std::endl;
	}
	check = stat_file.st_size;
	sendToCli( (void *)&check, sizeof(int), cliFD);

	// Get md5Sum and send to client
    char mdsum[40] = "md5sum ";
    strcat(mdsum, filey);
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
	pclose(fsum);

	return;

}


void uploadFile(char * filey, int cliFD){
/* Server side implementation of UP */
	std::string filey_str = filey;
	int valread;
	char buffer[BUFSIZ];

	// Recieve the size of the file
	int fileSize = 0;
	fileSize = *( (int *)getCliMsg(cliFD, sizeof(int)));

	// Acknowledge that we are ready to recieve with a 1
	int readyCode = 1;
	sendToCli( (void *)&readyCode, sizeof(int), cliFD);

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
	FILE *wf = fopen(filey, "wb");
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
	sendToCli( (void *)&throughPut, sizeof(throughPut), cliFD) ;

	// Send the hash
	char md5sum[40] = "md5sum ";
	strcat(md5sum, filey_str.c_str());
	FILE *dfile = popen(md5sum, "r");
	char md5sumOutput[50];
	fgets(md5sumOutput, 50, dfile);
	pclose(dfile);

	char *hash = strtok(md5sumOutput, " ");
	sendToCli(hash, strlen(hash)+1, cliFD);
} 


void getHead(char * filey, int cliFD){
	/* server side implementation for HEAD.
	 * prints first 10 lines of a file */

	int valread;
	char buffer[BUFSIZ];
	
	FILE *fd = fopen(filey, "rb");
	int check;
	if(!fd){
		check = -1;
		send(cliFD, (void *)&check, sizeof(check), 0);
		return;
	}
	char command[200] =  "head -10 ";
	strcat(command, filey);
	FILE *fhead = popen(command, "r");
	char out[BUFSIZ];
	fread(out, 1, BUFSIZ, fhead);
	pclose(fhead);
	// send return size
	check = strlen(out) + 1;
	send(cliFD, (void *)&check, sizeof(int), 0);
	// send the head
	send(cliFD, out, check, 0);

}

	
	



