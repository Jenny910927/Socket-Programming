# SRCS = Connection.cpp UserInfo.cpp helper.cpp common.cpp

# OBJS = $(patsubst %.cpp, %.o, $(SRCS))
# all: $(OBJS)


# server: server.cpp $(OBJS)
# 	g++ -g server.cpp $(OBJS) -o server
# client: client.cpp
# 	g++ -g client.cpp -o client
# clean:
# 	rm -f server
# 	rm -f client



SRCS = Connection.cpp helper.cpp common.cpp ThreadPool.cpp UserInfo.cpp 
OBJS = $(patsubst %.cpp, %.o, $(SRCS))

all: server client

server: server.o $(OBJS)
	g++ -g server.o $(OBJS) -o server -lpthread

server.o: server.cpp
	g++ -c server.cpp

client: client.o
	g++ -g client.o -o client

client.o: client.cpp
	g++ -c client.cpp

clean:
	rm -f server client *.o