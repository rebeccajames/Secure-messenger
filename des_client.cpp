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

/*****************************************************************************/
/*   This file does DES in ECB mode but only 8 bytes at a time               */
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
										
#include <stdlib.h>
#include <memory.h>
#include <errno.h>
#include <fstream>
#include <string.h>

#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>	
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/bn.h>
#include <openssl/dh.h>
#include <openssl/engine.h>
#include <openssl/buffer.h>
#include <openssl/pem.h>

#include <openssl/des.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>


#define BUFFER_SZ 128					//I/O Buffer size max text length
#define MAX_CLIENTS 100					//Maximum number of client connections

using namespace std;

/*****************************************************************************/
/*                             FUNCTION DECLARATONS                          */
/*****************************************************************************/
void unsignedCharToChar(unsigned char ar1[], char ar2[], int hm);
void CharToUnsignedChar(char ar1[], unsigned char ar2[], int hm);
int size(char *ptr);
int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
  unsigned char *iv, unsigned char *ciphertext);
int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
  unsigned char *iv, unsigned char *plaintext);
void handleErrors(void);
/*****************************************************************************/
/*******************************Beginning of Main*****************************/
/*****************************************************************************/

int main(int argc, char* argv[])			//main takes command line arguments
{ 
	int sd;									//socket descriptor
	int nsd;								//new socket descriptor
	int portnum;							//port number
	socklen_t sa_len;						//address length
	int valuew;								//check # bytes returned by write
	int valuerd;							//check # bytes returned by read
	struct sockaddr_in sa;					//server IP address to connect
	struct sockaddr_in addr;				//client IP address 
	char hostname[256];						//to store hostname
	char buf[BUFFER_SZ];					//buffer for messaging (read/write)
	fd_set masterfds;						//master set of socket descriptors
	struct hostent *server;					//ptr to structure that defines a host computer
	
	
	unsigned char in[BUFFER_SZ], out[BUFFER_SZ], back[BUFFER_SZ];
    unsigned char *e = out;
 
	//FILE* fp = fopen("dh256.pem", "r");
	DH* dh = DH_new();
      
	unsigned char *dhkey;	// = (unsigned char*) malloc(DH_size(dh));
	
	//Initialise the library
	ERR_load_crypto_strings();
	OpenSSL_add_all_algorithms();
	OPENSSL_config(NULL);
	
	//OTHER TUTORIAL  http://www.caole.net/diary/des.html
	DES_cblock key;
    /**//* DES_random_key(&key); */ /**//* generate a random key */
    DES_string_to_key("passwastooshortz", &key);

    DES_key_schedule schedule;
    DES_set_key_checked(&key, &schedule); 

    //const_DES_cblock input = "hehehe";
    DES_cblock output;
	
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
	
	/************************************************************/
	          //BEGIN IMPLEMENTATION OF DIFFIE HELLMAN:

	int len; 
	unsigned char* convertBn = new unsigned char;
	BIGNUM * num = BN_new();
	
	valuerd = read(sd, &len, sizeof(len));	//try to read what is in buffer	
	
	if ((valuerd = read(sd, convertBn, len)) <= 0)
	  {
	    cerr << "ERROR: Failed to read p from server socket\n";

	  }
	
	dh->p = BN_bin2bn(convertBn, len, NULL);
	char * check = BN_bn2dec(dh->p);
	cout<<"p = "<< check << endl;

	 convertBn = new unsigned char;

	valuerd = read(sd, &len, sizeof(len));	//try to read what is in buffer	
	if ((valuerd = read(sd, convertBn, len)) <= 0)
	  {
		cerr << "ERROR: Failed to read g from server socket\n";
	  }

	dh->g = BN_bin2bn(convertBn, len, NULL);
	check = BN_bn2dec(dh->g);
	cout<<"g = "<<check<<endl;
	int codes;
	if(-1 == DH_check(dh, &codes)) perror("something bad happened");
	if(codes != 0)
	  {
	    /* Problems have been found with the generated parameters */
	    printf("DH_check failed\n");
	  
	  }

	if (1 != DH_generate_key(dh))  //generates the client's public key g^a mod p, g^b mod p
	  {
	    cerr <<"error setting pub key"<<endl;
	    //exit(1);
	  }
	
	len = BN_num_bytes(dh->pub_key);
	convertBn = new unsigned char;
	len = BN_bn2bin(dh->pub_key, convertBn);
	cout << len<<endl;
	check = BN_bn2dec(dh->priv_key);
	cout<<"priv key "<<check<<endl;	

	check = BN_bn2dec(dh->pub_key);
	cout<<"pub key "<<check<<endl;
	
	bool SharedKeyISCalculated = false;

				//END IMPLEMENTATION OF DIFFIE HELLMAN:
	/************************************************************/

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
		  
		  if ( !SharedKeyISCalculated){
		
		    len = BN_num_bytes(dh->pub_key);
		    convertBn = new unsigned char;
		    len = BN_bn2bin(dh->pub_key, convertBn);
		    cout << len<<endl;
		    check = BN_bn2dec(dh->priv_key);
		    cout<<"priv key "<<check<<endl;	

		    check = BN_bn2dec(dh->pub_key);
		    cout<<"pub key "<<check<<endl;
		    
		    if ((send(sd, convertBn, len, 0)) < 0) 
		      {
				cerr<<"failed to send pubkey"<<endl;
				exit(1);
		      }
		    
		    if ((valuerd = read(sd, convertBn, len)) <= 0)
		      {
	    	     cerr << "ERROR: Failed to read g from server socket\n";
		      }
		    
		    dhkey =  (unsigned char*) malloc(DH_size(dh));
		    memset(dhkey, 0, DH_size(dh));
		    BIGNUM * pub_key2 = BN_new();
		    BN_bin2bn(convertBn, len, pub_key2);
		    check = BN_bn2dec(pub_key2);
		    cout<<"pubkey2 = "<<check<<endl;
		    DH_compute_key(dhkey, pub_key2, dh); //computes shared key  
	
		    cout<<"priv shared key = "<<dhkey<<endl;
	

		    SharedKeyISCalculated = true;
			
		  }

		    bzero(buf, 256);	//clear the buffer
			fgets(buf, 255, stdin);		//get the input and write it
			
			printf("inside FD_ISSET ready to write\n");

//Read more: https://blog.fpmurphy.com/2010/04/openssl-des-api.html#ixzz4zDAHrApK


			const_DES_cblock input = "\0";
			strcpy(input, buf);

/*   THIS BLOCK DEMONSTRATES HOW OT WRITE 64 BYTES = CURRENTLY WE CAN ONLY WRITE 8 BYTES 
			strcpy(in, "Now is the time for all men to stand up and be counted");
 
			printf("Plaintext: [%s]\n", in);
 
			for (i = 0; i < 63; i += 8) {
				DES_ecb3_encrypt((C_Block *)(in + i),(C_Block *)(out + i), &keysched, DES_ENCRYPT);
			}
*/			
 
	
					
			DES_ecb_encrypt(&input, &output, &schedule, DES_ENCRYPT);
			printf("\nEncrypted! ");

			unsignedCharToChar(output, buf, strlen(output));
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
		  if  (!SharedKeyISCalculated){
		    
			if ((valuerd = read(sd, convertBn, len)) <= 0)
		      {
	    	       cerr << "ERROR: Failed to read g from server socket\n";
		      }
	
		    dhkey =  (unsigned char*) malloc(DH_size(dh));
		    memset(dhkey, 0, DH_size(dh));
		    BIGNUM * pub_key2 = BN_new();
		    BN_bin2bn(convertBn, len, pub_key2);
		    check = BN_bn2dec(pub_key2);
		    cout<<"pubkey2 = "<<check<<endl;
		    DH_compute_key(dhkey, pub_key2, dh); //computes shared key  
	
		    cout<<"priv shared key = "<<key<<endl;
	
		    len = BN_num_bytes(dh->pub_key);
		    convertBn = new unsigned char;
		    len = BN_bn2bin(dh->pub_key, convertBn);
		    cout << len<<endl;
		    check = BN_bn2dec(dh->priv_key);
		    cout<<"priv key "<<check<<endl;	

		    check = BN_bn2dec(dh->pub_key);
		    cout<<"pub key "<<check<<endl;
	
		    if ((send(sd, convertBn, len, 0)) < 0) 
		      {
				cerr<<"failed to send pubkey"<<endl;
				exit(1);
		      }
		    SharedKeyISCalculated = true;

		  }

			printf("inside FD_ISSET ready to read\n");
			valuerd = read(sd, buf, 255);	//try to read what is in buffer	
			if ((valuerd < 0) || (valuerd == 0)) //if error or server is closed.
			{
				cerr << "ERROR: Failed to read from socket\n";
				exit(1);
			}
		
			if (valuerd > 0) //output the message that was read from buffer
			{
				const_DES_cblock input = "\0";
				printf("\nReceived Ciphertext:\n");
				printf("%s\n", buf);
				int buf_len = size(&buf[0]);
				printf("size of buf = %d\n", buf_len);
				CharToUnsignedChar(buf, output, buf_len); 
				printf("after chartounsignedchar before decryption\n");
				
				//for (int i = 0; i < 63; i += 8) {
				//DES_ecb_encrypt((C_Block *)(in + i),(C_Block *)(out + i), &keysched, DES_ENCRYPT);

				//}
				
				DES_ecb_encrypt(&output, &input, &schedule, DES_DECRYPT);
				int input_len = strlen(input);
				printf("Size of input = %d\n", input_len);
				
				input[input_len] = '\0';  //this is input_len not buf_len 
				
				printf("\nDecrypted!\n");
				printf("%s\n ", input);
				bzero(buf, BUFFER_SZ);
			}
		}
	}
	return 0;	//Exit Main
}


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

