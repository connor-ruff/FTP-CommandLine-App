

CC= g++
FLAGS= -std=c++11 -lcrypto -lz

TARGETS= myftpd myftp

all: $(TARGETS)

myftpd:
	$(CC) server/serv.cpp $(FLAGS) -o server/myftpd

myftp:
	$(CC) client/client.cpp $(FLAGS) -o client/myftp

clean:
	rm -f client/myftpd client/myftp
