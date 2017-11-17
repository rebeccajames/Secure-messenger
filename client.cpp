/*****************************************************************************/
/*   Course: CNT5505 Data and Computer Communications                        */
/*   Semester: Spring 2016                                                   */
/*   Name: Rebecca James                                                     */
/*   Project 1 Chat Server and Chat Client                                   */
/*   REFERENCES:  Dr. Duan Class Lecture Notes Ch0 Socket Programming, CS 50 */
/*   Software Design and Implementation Lecture 19 Dartmouth College,        */
/*   COSC 4377 Programming Help Session Slides University of Houston,        */
/*   binarytides.com Handle multiple socket connections with fd_set and      */
/*   select on Linux, Computer Networks 5th Ed. Tannenbaum and Wetherall,    */
/*   http://linux.die.net/man/2/select, Beej's Guide     					 */
/*****************************************************************************/


#include <iostream>						//standard input/output stream objects
#include <cstdlib>						//C standard general utilities library
#include <cstring>						//C string class for string objects
#include <stdio.h>						//fgets 
#include <unistd.h>						//Read, Write, Close functions	
#include <netdb.h>						//IP address, hostent structures			
#include <sys/types.h> 					//for porting socket programs
#include <sys/socket.h>					//socket programming functions
#include <netinet/in.h>					//byte order conversion functions
										//data structures defined by internet

#define BUFFER_SZ 256					//I/O Buffer size max text length
#define MAX_CLIENTS 100					//Maximum number of client connections

using namespace std;

/*****************************************************************************/
/*******************************Beginning of Main*****************************/
/*****************************************************************************/

int main(int argc, char* argv[])		//main takes command line arguments
{ 
	int sd;								//socket descriptor
	int nsd;							//new socket descriptor
	int portnum;						//port number
	socklen_t sa_len;					//address length
	int valuew;							//check # bytes returned by write
	int valuerd;						//check # bytes returned by read
	struct sockaddr_in sa;				//server IP address to connect
	struct sockaddr_in addr;			//client IP address 
	char hostname[256];					//to store hostname
	char buf[BUFFER_SZ];				//buffer for messaging (read/write)
	fd_set masterfds;					//master set of socket descriptors
	struct hostent *server;				//ptr to structure that defines a host computer
	
    if (argc < 3) 
	{
       cerr << "USAGE: chatclient <servhost> <servport>\n";
       exit(1);
    }
	
	//store portnumber and server name
    portnum = atoi(argv[2]);
	server = gethostbyname(argv[1]);
		
    if (server == NULL) 	//check for valid server name 
	{
        cerr << "ERROR: no such host\n";
        exit(1);
    }
	
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)	//create socket for client 
	{
		cerr << "ERROR: Failed to create socket\n";
		exit(1);
	}		
	
	bzero((char*)&sa, sizeof(sa));				//zero the machine IP address
	sa.sin_family = AF_INET;					//set to internet family
	//set server address
	bcopy((char *)server->h_addr,(char *)&sa.sin_addr.s_addr, server->h_length);
	sa.sin_port = htons(portnum);				//set port number
	
	//connect client to server if connect fails exit
	if (connect(sd, (struct sockaddr *)&sa, sizeof(sa)) < 0)
	{
		cerr << "ERROR: Failed to connect client to server\n";
		exit(1);
	}
	
	sa_len = sizeof(struct sockaddr);	//get length of address
	gethostname(hostname, sizeof(hostname));	//get name of server
	server = gethostbyname(hostname);			//change format of server name
	getsockname(sd, (struct sockaddr*)&addr, &sa_len);
	cout << "admin: connected to server on '" << server->h_name << "' at '" 
		<< portnum << "'" << " thru " << ntohs(addr.sin_port)  << endl;
	
	while (1)	//while client is running
	{
		FD_ZERO(&masterfds);		// clear the master socket set
		FD_SET(sd, &masterfds);		// add socket descriptor to set 
		FD_SET(STDIN_FILENO, &masterfds);		// add stdin to set
		
		if ((nsd = select(sd + 1, &masterfds, NULL, NULL, NULL)) < 0)
		{
			cerr << "ERROR: Select Failed\n";
			//exit(1);
		}
		
		//check if there is anything to write 
		if (FD_ISSET(STDIN_FILENO, &masterfds))
		{
			bzero(buf, 256);	//clear the buffer
			fgets(buf, 255, stdin);		//get the input and write it
			if ((valuew = write(sd, buf, strlen(buf))) < 0)
			{
				close(STDIN_FILENO);
				FD_CLR(STDIN_FILENO, &masterfds);
			}	
		}
		bzero(buf, BUFFER_SZ);	//clear the buffer before read
		
		//check if there is anything to read
		if (FD_ISSET(sd, &masterfds)) 
		{
			valuerd = read(sd, buf, 255);	//try to read what is in buffer
			if ((valuerd < 0) || (valuerd == 0)) //if error or server is closed
			{
				cerr << "ERROR: Failed to read from socket\n";
				exit(1);
			}
		
			if (valuerd > 0) //output the message that was read from buffer
			{
				cout << "\n" << buf << endl;
				bzero(buf, BUFFER_SZ);
			}
		}
	}
	return 0;	//Exit Main
}
