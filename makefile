SHELL = /bin/sh
CCC = g++ -std=c++11
CCFLAGS = -Wall  -g -pedantic -fpermissive
INCLUDES = -I. 
LIBS = -lssl -lm -lcrypto

all: chatserver chatclient 

chatserver: server.cpp 
	$(CCC) -o chatserver $(CCFLAGS) $(INCLUDES) server.cpp $(LIBS)

chatclient: client.cpp 
	$(CCC) -o  chatclient $(CCFLAGS) $(INCLUDES) client.cpp $(LIBS)
	
.SUFFIXES:
clean :
	rm -f *.o *~ *.i chatclient chatserver
 
realclean :
	rm -f *.o *~ *.i chatclient chatserver