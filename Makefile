cc=g++
server=ChatServer
client=ChatClient
INCLUDE=-I./lib/include
LIB_PATH=-L./lib/lib
LDFLAGS=-std=c++11 -lpthread  

.PHONY:all
all:$(server) $(client)

$(server):ChatServer.cc ./lib/lib/libjsoncpp.so
	$(cc) -o  $@ $^ $(INCLUDE) $(LIB_PATH) $(LDFLAGS) 
$(client):ChatClient.cc ./lib/lib/libjsoncpp.so
	$(cc) -o  $@ $^ $(INCLUDE) $(LIB_PATH)  $(LDFLAGS) -lncurses 
.PHONY:clean
clean:
	rm -f $(server) $(client)
