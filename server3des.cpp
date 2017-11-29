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
#include <fstream>						//file I/O
#include <chrono>						//timer for algo
#include <sys/types.h> 					//for porting socket programs
#include <sys/socket.h>					//socket programming functions
#include <arpa/inet.h>					//inet_ntoa()
#include <netinet/in.h>					//byte order conversion functions
										//data structures defined by internet
										
#include <openssl/des.h>				//openssl library for DES enc/dec
#include <openssl/rand.h>				//openssl library generate random #
#include <openssl/bio.h>				//openssl library file I/O
										
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
	
	bzero((char*)&serv_addr, sizeof(serv_addr));		//zero the machine IP address
	serv_addr.sin_family = AF_INET;						//set to internet family
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
	
//https://www.openssl.org/docs/man1.0.2/crypto/DES_string_to_key.html
	//Here we generate random keys 1,2,3 to pass to clients for communication
    DES_key_schedule ks1, ks2, ks3;
	DES_cblock key1, key2, key3;
    DES_cblock seed = {0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};
    DES_cblock ivsetup = {0xE1, 0xE2, 0xE3, 0xD4, 0xD5, 0xC6, 0xC7, 0xA8};
    DES_cblock ivec;
	
	memcpy(ivec, ivsetup, sizeof(ivsetup));

	
	auto t1 = chrono::high_resolution_clock::now();
    RAND_seed(seed, sizeof(DES_cblock));  	//seed random generator
    DES_random_key(&key1);					//generate key1
    DES_random_key(&key2);					//generate key2
    DES_random_key(&key3);					//generate key3
	auto t2 = chrono::high_resolution_clock::now();
    chrono::duration<int64_t,nano> elapsed = t2 - t1; 
	cout << "3DES Key Generation Runtime is: " << elapsed.count() << " nanoseconds.";
 
	//converted keys into architecture dependent DES_key_schedule via the DES_set_key() fxn
    DES_set_key((C_Block *)key1, &ks1);		
    DES_set_key((C_Block *)key2, &ks2);
    DES_set_key((C_Block *)key3, &ks3);
	
	//printf("size of key1 = %lu\n", sizeof(key1));
	//printf("key1 before bn2dec = %s\n", key1);
	//char * check = BN_bn2dec(key1);  can't use this function
	printf("\nkey1 = %s\n", key1);
	//printf("key1 hex = %lu\n", key1);
	//printf("size of key2 = %lu\n", sizeof(key2));
	printf("\nkey2 = %s\n", key2);
	//printf("size of key3 = %lu\n", sizeof(key3));
	printf("\nkey3 = %s\n", key3);
	
	//printf("size of ivec = %lu\n", sizeof(ivec));
	printf("\nivec = %s\n", ivec);
	
	ofstream myfile;
	myfile.open("output.txt"); //, ios::out | ios::app); // | ios::binary);
	 if (myfile.is_open())
	  {
		myfile << "key1:" <<key1 << "\n";
		myfile << "key2:" <<key2 << "\n";
		myfile << "key3:" <<key3 << "\n";
		myfile << "iv:" << ivec << "\n";
		myfile.close();
	  }
	  else cout << "Unable to open file";
	
	
//#Read more: https://blog.fpmurphy.com/2010/04/openssl-des-api.html#ixzz4zdzRlZ3d
		
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
						cout << "\nSuccessfully added new socket to list\n";
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
							cout << "\nServer buffer to client " << clsock[c]
								<< " " << buf << endl;
								
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
	return 0; 	
}//End Main

/*****************************************************************************/
/*                              FUNCTION DEFINITIONS                         */
/*****************************************************************************/
void unsignedCharToChar(unsigned char ar1[], char ar2[], int hm)
{
	for (int i = 0; i <hm; ++i)
	{
			ar2[i]=static_cast<char>(ar1[i]);
	}
}

void CharToUnsignedChar(char ar1[], unsigned char ar2[], int hm)
{
	for (int i = 0; i <hm; ++i)
	{
		ar2[i]=static_cast<unsigned char>(ar1[i]);
	}
}

/////////////////////////////REMOVE THIS FUNCTION BEFORE TURN IN FOR GRADE/////////////////////////////////////
//returns the size of a character array using a pointer to the first element of the character array
int size(char *ptr)
{
    //variable used to access the subsequent array elements.
    int offset = 0;
    //variable that counts the number of elements in your array
    int count = 0;

    //While loop that tests whether the end of the array has been reached
    while (*(ptr + offset) != '\0')
    {
        //increment the count variable
        ++count;
        //advance to the next element of the array
        ++offset;
    }
    //return the size of the array
    return count;
}
