#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <iostream>
#include <string>
#include <bits/stdc++.h>
#include <sys/stat.h>
#include <unistd.h>


char* PROGRAM_NAME;

std::string get_user_input(){
	std::cout << "> ";
	std::string command;
	getline(std::cin, command);
	return command;
}


void usage(int arg){
	std::cout << PROGRAM_NAME << "[SERVER] [PORT]\n";
	exit(arg);
}

std::string get_arg(std::string command){
	/* seperate the arg from the command */
	return command.substr(command.find_last_of(' ', command.size())+1);
}

int get_socket(){
	int fd;
	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		std::cerr << "Could not create socket\n";
	   	return -1;	
	}	
	return fd;

}


void handle_DN(int fd, std::string command){
/* Client Side implementation of the DN request */
	int valread;
	char buffer[BUFSIZ];
	std::string arg = get_arg(command);
	int code = 1;
	send(fd, (void *)&code, sizeof(int), 0);  // 0 = code for "download" command

	// usleep(100); // TODO This might not be neccessary but it seemed like things where sending too fast and it was causing bugs for me
	
	// Send Size of Fucking FIl;ename
	short int nameSiz = arg.length();
	send(fd, (void *)&nameSiz, sizeof(short int), 0);

	// Send over the command
//	std::cout << "arg is " << arg << std::endl;
	send(fd, arg.c_str(), strlen(arg.c_str())+1, 0);
//	std::cout << "Sent over the command\n";
	
	
	// Recieve the size of the file
	// This might give us endian probs
	int fileSize;
	valread = read(fd, (int *)&fileSize, sizeof(int));
	if (fileSize == -1){
		std::cout << "No file found at " << arg << std::endl;
		return;
	}
//	std::cout << "Filesize: " << fileSize << std::endl; //TODO	
	//Read in the md5hash
	valread = read(fd, buffer, BUFSIZ);
	buffer[valread] = '\0';
	std::string md5sum = buffer;

//	std::cout << "Hash From Serv: " << md5sum << "  (size: " << md5sum.size() << ")." <<  std::endl; //TODO


	
	int totalSent = 0;
	bzero( &buffer, sizeof(buffer));
	// Start Calculating time
	struct timeval b4 ;
	gettimeofday(&b4, NULL);


	// Manipulate recieve maximum based on filesize
	size_t sizey;
	if (fileSize > BUFSIZ){
		sizey = BUFSIZ;
	}
	else 
		sizey = fileSize;


	FILE * wf = fopen(arg.c_str(), "wb"); // where to store file

//	std::cout << "Recieving File.... " << std::endl << std::endl;
	while( totalSent < fileSize ){
		valread = recv(fd, buffer, sizey, 0);
		totalSent += valread;
		fwrite(buffer, 1, valread, wf);                            
		bzero( &buffer, sizeof(buffer));
		 if ( totalSent >= fileSize ) {
			break;
		}
	}


	struct timeval after;
    gettimeofday(&after, NULL);
	fclose(wf);
		
	
	float elapsedTime = ( ((after.tv_sec - b4.tv_sec) * 1000000) + (after.tv_usec - b4.tv_usec) ) ;
	float throughPut = (  (fileSize * 8) ) / ( elapsedTime / 1000000) ;
	std::cout << fileSize << " bytes transferred in " << elapsedTime / 1000000 << " seconds: " << throughPut << " bits/sec" << std::endl;

	// calculate the md5sum and print out match status
	char newmd5sum [40] = "md5sum ";
	strcat(newmd5sum, arg.c_str());
	FILE * dfile = popen(newmd5sum, "r");
	char md5sumOutput [50] ;
	fgets(md5sumOutput, 50, dfile);

	char * hash = strtok(md5sumOutput, " ");
	std::cout << "MD5 Hash: " << md5sumOutput ;
	if ( !strcmp(md5sum.c_str(), hash) ){
		std::cout << " (matches)"  << std::endl;
	}
	else {
		std::cout << "\nERROR: Hash's do not match. Download CORRUPTED" << std::endl;
		return;
	}
}

void handle_UP(int servFD, std::string arg){
	/* This is the client side implementation of the UP command.
	 * basically, should allow the server to get uploaded file */
	std::string filename = get_arg(arg);

	FILE *fd = fopen(filename.c_str(), "rb");
	int check;
	if(!fd){
		std::cout << "File Not Found" << std::endl;
		//check = -1;
		//send(servFD, (void *)&check, sizeof(check), 0);
		return;
	}
	// send notice we intend to upload
	int comCode = 2;
	send(servFD, (void *)&comCode, sizeof(int), 0);
	// Send length of filename
	//send(servFD, filename.c_str()
	

	// Send size of filename
	short int fileLen = (short int) filename.length();
	send(servFD, (void *)&fileLen, sizeof(short int), 0);
	// Send filename
	send(servFD, filename.c_str(), strlen(filename.c_str())+1, 0);

	// Send file size
	struct stat stat_file;
	if ((stat(filename.c_str(), &stat_file)) == -1){
		std::cerr << "Error on Stat: " << strerror(errno) << std::endl;
	}
	check = stat_file.st_size;
	size_t sent_check = send(servFD, (void *)&check, sizeof(int), 0);
	if (sent_check == -1)
		std::cerr <<"Ending Info To Client: " << strerror(errno) << std::endl;
	std::cout << "File size sent with value " << check << std::endl;
	std::cout << "Sent check: " << sent_check << std::endl;
	// Get md5Sum for later
	char mdsum[40] = "md5sum ";
	strcat(mdsum, filename.c_str());
	FILE* fsum= popen(mdsum, "r");
	char md5sumOutput [50];
	fgets(md5sumOutput, 50, fsum);
	pclose(fsum);
	char * hash = strtok(md5sumOutput, " ");

	// recieve acknowlegement
	int reccode = 0;
	while (reccode != 1)
		read(servFD, (void *)&reccode, sizeof(0));
	

	char buf[BUFSIZ];
	size_t num;
	do {
		bzero(&buf, BUFSIZ);
		if (check < BUFSIZ) {
			num = check;
		} else {
			num = BUFSIZ;
		}

		num = fread(buf, 1, num, fd);
		if (num < 1)
			break;
		if(!send(servFD, (void *)buf, num, 0))
			break;
		check -= num;
	}
	while (check > 0);
	
	fclose(fd);

	// Recieve the throughput
	float throughPut;
	read(servFD, (float *)&throughPut, sizeof(throughPut));
	std::cout << throughPut << " bits/sec" << std::endl;
	int valread;
	// recieve the hash
	valread = read(servFD, buf, BUFSIZ);
	buf[valread] = '\0';
	std::cout << "server hash is " << buf << std::endl;
	std::cout << "client hash is " << hash << std::endl;
	if(!strcmp(buf, hash))
		std::cout << "Hashes match!" << std::endl;
	else
		std::cout << "Hashes do not match, CORRUPTED\n";
}


void handle_HEAD(int fd, std::string command){
	int valread;
	char buffer[BUFSIZ];
	std::string arg = get_arg(command);
	send(fd, (char *)"HEAD", 4, 0);
	// Send over the command
	std::cout << arg << std::endl;
	send(fd, arg.c_str(), strlen(arg.c_str()), 0);
	// Recieve the size of the file as a 32 bit int
	// possible endian probs
	int fileSize;
	valread = read(fd, (int *)&fileSize, sizeof(fileSize));
	if (fileSize == (int)-1){
		std::cout << "No file found at " << arg << std::endl;
		return;
	}
	do {
		valread = read(fd, buffer, BUFSIZ);
		buffer[valread] = '\0'; // redundant?
		std::cout << buffer << std::endl;
	}
	while (valread > 0);

	return;
}


void handle_RM(int fd, std::string command) {
	int valread;
	char buffer[BUFSIZ];
	std::string arg = get_arg(command);
	send(fd, (char *)"RM", 2, 0);

	// send over the command
	send(fd, arg.c_str(), strlen(arg.c_str()), 0);
	
	// recieve a response if file exists
	int exists;
	valread = read(fd, (int *)&exists, sizeof(exists));
	if (valread != 1){
		std::cout << "File not available to be deleted\n";
		return;
	}
	// confirm user choice and send to server
	std::cout << "Are you sure? ";
	std::string response;
	getline(std::cin, response);
	send(fd, response.c_str(), strlen(response.c_str()), 0);

	// handle response
	int deleted;
	if (response == "Yes"){
		valread = read(fd, (int *)&deleted, sizeof(deleted));
		if (deleted == 1)
			std::cout << "File deleted." << std::endl;
		else
			std::cout << "ERROR: File not deleted." << std::endl;
	}
	

	return;
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
	
	int fd = get_socket();
	if (fd < 0){
		std::cout << "Creating file descriptor failed\n";
		exit(EXIT_FAILURE);
	}

	memset((char*)&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);

	// Get host name
	hp = gethostbyname(serverName);
	if(!hp){
		fprintf(stderr, "could not obtain address of %s\n", serverName);
	}
	// put hosts address into server address structure
	memcpy((void *)&servaddr.sin_addr, hp->h_addr_list[0], hp->h_length);


	// connect
	if (connect(fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
		printf("\nConnection Failed \n");
		return -1; //TODO add back as soon as io works
	}
	

	// Prompt the user for input and handle it
	while(true){

		// Send out message
		std::string user_input = get_user_input();
		std::string command = user_input.substr(0, user_input.find(" "));
		//send(fd, user_input.c_str(), strlen(user_input.c_str()), 0);
		if (command == "DN"){
			handle_DN(fd, user_input);
		} else if (command == "UP"){
			handle_UP(fd, user_input);
		} else if (command == "HEAD"){
			handle_HEAD(fd, user_input);
		} else if (command == "RM"){
			handle_RM(fd, user_input);
		} else if (command == "LS"){
		} else if (command == "RMDIR"){
		} else if (command == "CD"){
		} else if (command == "QUIT"){
			break;
		} else {
		}



		// Recieve the response
	}
	close(fd);
	
}
