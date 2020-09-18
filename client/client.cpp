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
	return command.substr(command.find_last_of(' ', command.size()));
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
	send(fd, arg.c_str(), strlen(arg.c_str()), 0);
	
	//Read in the md5hash
	valread = read(fd, buffer, BUFSIZ);
	buffer[valread] = '\0';
	std::string md5sum = buffer;
	
	// Recieve the size of the file as a 32 bit int
	// This might give us endian probs
	short int fileSize;
	valread = read(fd, (short int *)&fileSize, sizeof(fileSize));
	if (fileSize == (short int)-1){
		std::cout << "No file found at " << arg << std::endl;
		return;
	}
	// Read in the file
	//std::ostream file(arg, std::ios::out|std::ios::binary);
	std::ofstream myfile;
	myfile.open(arg);

	do {
		valread = read(fd, buffer, BUFSIZ);
		buffer[valread] = '\0'; // this line might be redundant
		myfile << buffer;
	}
	while (valread > 0);
	myfile.close();
	

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
			break;
		} else if (command == "UP"){
			break;
		} else if (command == "HEAD"){
			break;
		} else if (command == "RM"){
			break;
		} else if (command == "LS"){
			break;
		} else if (command == "RMDIR"){
			break;
		} else if (command == "CD"){
			break;
		} else if (command == "QUIT"){
			break;
		} else {
			break;
		}



		// Recieve the response
	}
	close(fd);
	
}
