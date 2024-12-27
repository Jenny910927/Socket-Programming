SRCS = Connection.cpp helper.cpp common.cpp ThreadPool.cpp UserInfo.cpp Chatroom.cpp
OBJS = $(patsubst %.cpp, %.o, $(SRCS))
CFLAGS = -lssl -lcrypto -lpthread

all: server client

server: server.o $(OBJS)
	g++ -g server.o $(OBJS) -o server $(CFLAGS)

server.o: server.cpp
	g++ -c server.cpp

client: client.o
	g++ -g client.o $(OBJS) -o client $(CFLAGS)

client.o: client.cpp
	g++ -c client.cpp

clean:
	rm -f server client *.o

test_client: test_client.o
	g++ -g test_client.o $(OBJS) -o test_client $(CFLAGS)

test_client.o: test_client.cpp
	g++ -c test_client.cpp