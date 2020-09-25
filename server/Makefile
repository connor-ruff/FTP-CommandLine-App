CC= g++
FLAGS= -std=c++11 -lcrypto -lz

TARGETS= myftpd

all: $(TARGETS)

myftpd:
	$(CC) serv.cpp $(FLAGS) -o myftpd

clean:
	rm -f myftpd
