CC= g++
FLAGS= -std=c++11 -lcrypto -lz

TARGETS= myftp

all: $(TARGETS)

myftp:
	$(CC) client.cpp $(FLAGS) -o myftp

clean:
	rm -f myftp
