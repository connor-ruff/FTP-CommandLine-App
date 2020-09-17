

CC= g++
FLAGS= -std=c++11 -lcrypto -lz

myftpd:
	$(CC) server/serv.cpp $(FLAGS) -o server/server

myftp:
	$(CC) client/client.cpp $(FLAGS) -o client/client

clean:
	rm -f myftpd myftp
