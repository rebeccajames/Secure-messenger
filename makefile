SHELL = /bin/sh
CCC = g++ -std=c++0x
CCFLAGS = -Wall  -g -pedantic
INCLUDES = -I. 

all: chatserver chatclient 

chatserver: server.cpp 
	$(CCC) -o chatserver $(CCFLAGS) $(INCLUDES) server.cpp

chatclient: client.cpp 
	$(CCC) -o  chatclient $(CCFLAGS) $(INCLUDES) client.cpp
	
.SUFFIXES:
clean :
	rm -f *.o *~ *.i chatclient chatserver
 
realclean :
	rm -f *.o *~ *.i chatclient chatserver