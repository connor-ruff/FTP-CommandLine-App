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

#include <iostream>
#include <string>
#include <bits/stdc++.h>


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
/* Handle the DN command 
 * Things I could see failing: convert to string, reading into a file*/
	int valread;
	char buffer[BUFSIZ];
	std::string arg = get_arg(command);
	send(fd, (char *)"DN", 2, 0);
	// Send over the command
	send(fd, arg.c_str(), strlen(arg.c_str()), 0);

	
	
	// Recieve the size of the file
	// This might give us endian probs
	short int fileSize;
	valread = read(fd, (int *)&fileSize, sizeof(fileSize));
	if (fileSize == -1){
		std::cout << "No file found at " << arg << std::endl;
		return;
	}
	
	//Read in the md5hash
	valread = read(fd, buffer, BUFSIZ);
	buffer[valread] = '\0';
	std::string md5sum = buffer;

	// Read in the file
	std::ofstream myfile;
	myfile.open("newFile.txt"); 


	int totalSent = 0;
	bzero( &buffer, sizeof(buffer));
	while( (valread	= recv(fd, buffer, fileSize, 0))  > 0 ){
		totalSent += valread ;
		myfile << buffer; 
		bzero( &buffer, sizeof(buffer));
		if ( totalSent >= fileSize ) {
			break;
		}
	}
		
	myfile.close();

	//TODO: calculate the md5sum and print out match status
	char newmd5sum [40] = "md5sum ";
	strcat(newmd5sum, arg.c_str());
	FILE * dfile = popen(newmd5sum, "r");
	char md5sumOutput [50] ;
	fgets(md5sumOutput, 50, dfile);

	char * hash = strtok(md5sumOutput, " ");
	std::cout << "Client-Calculated Hash: " << hash << std::endl;
	if ( !strcmp(md5sum.c_str(), hash) ){
		std::cout << "Good\n" << std::endl;
	}

	return;	
	
	//TODO: print out the time
}

void handle_UP(int fd, std::string arg){
	return;
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
