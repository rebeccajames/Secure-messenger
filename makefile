SHELL = /bin/sh
CCC = g++ -std=c++11
CCFLAGS = -Wall  -g -pedantic -fpermissive
INCLUDES = -I. 
LIBS = -lssl -lm -lcrypto

all: chatserver chatclient 

chatserver: aes_server.cpp 
	$(CCC) -o chatserver $(CCFLAGS) $(INCLUDES) aes_server.cpp $(LIBS)

chatclient: aes_client.cpp 
	$(CCC) -o  chatclient $(CCFLAGS) $(INCLUDES) aes_client.cpp $(LIBS)
	
.SUFFIXES:
clean :
	rm -f *.o *~ *.i chatclient chatserver
 
realclean :
	rm -f *.o *~ *.i chatclient chatserver
