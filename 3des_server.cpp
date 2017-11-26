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
/*   http://linux.die.net/man/2/select, Beej's Guide						 */
/*****************************************************************************/

#include <iostream>						//standard input/output stream objects
#include <cstdlib>						//C standard general utilities library
#include <cstring>						//C string class for string objects
#include <netdb.h>						//IP address and port # structures	
#include <unistd.h>						//Read and Close functions
#include <sys/types.h> 					//for porting socket programs
#include <sys/socket.h>					//socket programming functions
#include <arpa/inet.h>					//inet_ntoa()
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
	int sd;								//server socket descriptor
	int cd;					 			//client socket descriptor
	int nsd;							//new socket descriptor
	int portnum = 0;					//port number
	int fdmax;							//maximum file descriptor number
	int value;							//check # bytes returned by read 
	int yes = 1;						//for setsockopt() SO_REUSEADDR
	char buf[BUFFER_SZ];				//buffer for messaging (read/write)
	fd_set masterfds;					//master set of socket descriptors
	int clsock[MAX_CLIENTS];			//Array of client sockets
	char hostname[256];					//to store hostname
	struct sockaddr_in ma;  			//machine IP address
	socklen_t addlen;					//length of address
	socklen_t lengthvar;				//used for getsockname()
	struct hostent *server;				//ptr to structure that defines a host computer
	struct sockaddr_in serv_addr;  		//machine IP address
		
		
	FD_ZERO(&masterfds);				//zero/clear the master set
	//Socket is set up and bound and waiting for connections
	for (int i = 0; i < MAX_CLIENTS; ++i)
	{
		clsock[i] = 0;
	}
	
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)	//create socket for server 
	{
		cerr << "ERROR: Failed to create socket\n";
		exit(1);
	}
	
	//change socket settings - if problem with settings exit
	if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(yes)) < 0)
	{
		cerr << "ERROR: Server-setsockopt()\n";
		exit(1);
	}
	
	bzero((char*)&serv_addr, sizeof(serv_addr));				//zero the machine IP address
	serv_addr.sin_family = AF_INET;					//set to internet family
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);		//use any card on server/machine
	serv_addr.sin_port = htons(portnum);				//set port number

	//bind socket to local machine address
	if (bind(sd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		cerr << "ERROR: Failed to bind socket to local address\n";
		exit(1);
	}
	
	if (listen(sd, MAX_CLIENTS) < 0)		//socket listens for client request
	{
		cerr << "ERROR: Failed to listen\n";
		exit(1);
	}
	
	addlen = sizeof(serv_addr);						//get length of server address
	lengthvar = sizeof(struct sockaddr);  		//get length of address
	gethostname(hostname, sizeof(hostname));	//get server name
	server = gethostbyname(hostname);
	
	getsockname(sd, (struct sockaddr* )&serv_addr, &lengthvar);	//get socket name
	cout << "admin: started server on '" << server->h_name << "'" << " at port '" << ntohs(serv_addr.sin_port) << "'\n";
	cout << "admin: server IP address " << inet_ntoa(*(struct in_addr*)server->h_addr) 
	     << " at port " << ntohs(serv_addr.sin_port) << endl;
	//cout << "gethostaddr IP Address is: " << gethostaddr(hostname) << endl;
	
	
	while (1)						//while server is running
	{	
		FD_ZERO(&masterfds);		// clear the master socket set
		FD_SET(sd, &masterfds);		// add server/listener socket to master set

		fdmax = sd;					// keep track of the biggest file descriptor  
		
		for (int i = 0; i < MAX_CLIENTS; ++i)
		{
			cd = clsock[i];
			if  (cd > 0)		//if socket descriptor is valid add to list
			{
				FD_SET(cd, &masterfds);
			}
				
			if (cd > fdmax)	//if higher file descriptor # set fmax 
			{
				fdmax = cd;
			}
		}
		
		//wait for something to happen on one of the sockets
		if ((nsd = select(fdmax + 1, &masterfds, NULL, NULL, NULL)) < 0)
		{
			cerr << "ERROR: Select Failed\n";
		}

		if (FD_ISSET(sd, &masterfds))		//check if file descriptor is set
		{
			if ((nsd = accept(sd, (struct sockaddr *)&ma, (socklen_t*)&addlen)) < 0)
			{
				cerr << "ERROR: Failed to accept the incoming connection\n";
			}
			else	//if new connection was succesful add to list
			{
				/*cout << "new connection socket FD: is " << nsd << endl;
				cout << " IP address is: " << inet_ntoa(ma.sin_addr) << endl; 
				cout << " Port number is: " << ntohs(ma.sin_port) << endl;
				*/
				cout << "admin: connect from '" << server->h_name << "'" << " at '" << ntohs(ma.sin_port) << "'\n";
				
				for (int i = 0; i < MAX_CLIENTS; ++i)
				{
					if (clsock[i] == 0)	//available position in list 'not used'
					{
						clsock[i] = nsd;	//add new socket descriptor to list
						//cout << "\nSuccessfully added new socket to list\n";
						break;
					}
				}
			}
		}	
		//run through the connections in list looking for data to be read
		for (int j = 0; j < MAX_CLIENTS; ++j)
		{
			cd = clsock[j];   //check each "socket" in the list
			
			if (FD_ISSET(cd, &masterfds))	//if set file descriptor found
			{
				//cout << "We found a set file descriptor at client " << cd << endl;
				if ((value = read(cd, buf, BUFFER_SZ)) < 0)
				{
					close(cd);		//close socket if read fails
					clsock[j] = 0;	//reset that position to 0 for reuse
					FD_CLR(clsock[j], &masterfds); //clr closed socket from set
				}
				else if (value > 0) //otherwise there is text to be read
				{	
					buf[value] = '\0';
					for (int c = 0; c < MAX_CLIENTS; ++c)  //go through list
					{
						//if socket is in use and not the sending client
						if ((clsock[c] > 0)	 && (clsock[c] != cd))   
						{
							//CODE FOR TESTING PURPOSES
							//cout << "Your buffer to client " << clsock[c]
								//<< " " << buf << endl;
								
							//if send fails
							if (send(clsock[c], buf, strlen(buf), MSG_NOSIGNAL) < 0)
							{
								close(clsock[c]);  //close socket if send fails
								clsock[c] = 0;	   //reset position to 0 
								FD_CLR(clsock[c], &masterfds); //clr from set
							}
						}
					}
				}
			}
		}
	}	
	close(sd);
	return 0; 	//Exit Main
}	
