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
										
#include <stdlib.h>
#include <memory.h>
#include <errno.h>
#include <fstream>

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


#define BUFFER_SZ 128					//I/O Buffer size max text length
//#define CIPHERTXT_SZ 512
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
	int decryptedtext_len, ciphertext_len;	//lengths for decrypted plaintext and ciphertext
	int buf_len;							//length for buffer that is read
	//FILE* fp = fopen("dh256.pem", "r");
	DH* dh = DH_new();
      
	//256 bit key :  THIS WILL HAVE TO BE RANDOMLY GENERATED
	unsigned char *key;// = (unsigned char*) malloc(DH_size(dh));
	//memset(key, 0, DH_size(dh));
	  // (unsigned char *)"01234567890123456789012345678901";

	//128 bit KEY :  THIS WILL HAVE TO BE RANDOMLY GENERATED
	unsigned char *iv = (unsigned char *)"0123456789012345";

	//Message to be encrypted
	unsigned char *plaintext;
// = (unsigned char *)"The quick brown fox jumps over the lazy dog";

	/* Buffer for ciphertext. Ensure the buffer is long enough for the
	* ciphertext which may be longer than the plaintext, dependant on the
	* algorithm and mode
	*/
	unsigned char ciphertext[BUFFER_SZ];

	//Buffer for the decrypted text 
	unsigned char decryptedtext[BUFFER_SZ];

	//Initialise the library
	ERR_load_crypto_strings();
	OpenSSL_add_all_algorithms();
	OPENSSL_config(NULL);
	
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
	
	/*if ((send(sd, &len, sizeof(len), 0)) < 0)
	  {

	    cerr<<"failed to send length of pubkey"<<endl;
	    exit(1);
	  }
	
	if ((send(sd, convertBn, len, 0)) < 0) 
	  {
	    cerr<<"failed to send pubkey"<<endl;
	    exit(1);

	  }
	
	valuerd = read(sd, &len, sizeof(len));	//try to read what is in buffer	
	if ((valuerd = read(sd, convertBn, len)) <= 0)
	  {
	    	       cerr << "ERROR: Failed to read g from server socket\n";

	  }
	
	key =  (unsigned char*) malloc(DH_size(dh));
	memset(key, 0, DH_size(dh));
	BIGNUM * pub_key2 = BN_new();
	BN_bin2bn(convertBn, len, pub_key2);
	check = BN_bn2dec(pub_key2);
	cout<<"pubkey2 = "<<check<<endl;
	DH_compute_key(key, pub_key2, dh); //computes shared key  
	
	cout<<"priv shared key = "<<key<<endl;
	
	*/
	bool SharedKeyISCalculated = false;

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
		    
		    key =  (unsigned char*) malloc(DH_size(dh));
		    memset(key, 0, DH_size(dh));
		    BIGNUM * pub_key2 = BN_new();
		    BN_bin2bn(convertBn, len, pub_key2);
		    check = BN_bn2dec(pub_key2);
		    cout<<"pubkey2 = "<<check<<endl;
		    DH_compute_key(key, pub_key2, dh); //computes shared key  
	
		    cout<<"priv shared key = "<<key<<endl;
	

		    SharedKeyISCalculated = true;

			  }



		    
		    	bzero(buf, 256);	//clear the buffer
			fgets(buf, 255, stdin);		//get the input and write it
			
			printf("inside FD_ISSET ready to write\n");
			//Message to be encrypted
			plaintext = (unsigned char *)buf; //(unsigned char *)"The quick brown fox jumps over the lazy dog";
               
			/* Encrypt the plaintext */
			ciphertext_len = encrypt (plaintext, strlen ((char *)plaintext), key, iv, ciphertext);
							
			printf("Ciphertext is:\n");
			BIO_dump_fp (stdout, (const char *)ciphertext, ciphertext_len);
	
			unsignedCharToChar(ciphertext, buf, ciphertext_len);
			if ((valuew = write(sd, buf, strlen(buf))) < 0)
			//if ((valuew = write(sd, ciphertext, strlen(ciphertext))) < 0)
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
	

		    key =  (unsigned char*) malloc(DH_size(dh));
		    memset(key, 0, DH_size(dh));
		    BIGNUM * pub_key2 = BN_new();
		    BN_bin2bn(convertBn, len, pub_key2);
		    check = BN_bn2dec(pub_key2);
		    cout<<"pubkey2 = "<<check<<endl;
		    DH_compute_key(key, pub_key2, dh); //computes shared key  
	
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
				int buf_len = size(&buf[0]);
				CharToUnsignedChar(buf, ciphertext, buf_len); 
				printf("inside FD_ISSET ready to read BEFORE make call to decrypt\n");
				decryptedtext_len = decrypt (ciphertext, /*ciphertext_len*/buf_len, key, iv, decryptedtext);
				printf("inside FD_ISSET ready to read AFTER make call to decrypt\n");
				/* Add a NULL terminator. We are expecting printable text */
				decryptedtext[decryptedtext_len] = '\0';

				/* Show the decrypted text */
				printf("Decrypted text is:\n");
				printf("%s\n", decryptedtext);

				//cout << "\n" << buf << endl;
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

int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
  unsigned char *iv, unsigned char *ciphertext)
{
  EVP_CIPHER_CTX *ctx;

  int len;

  int ciphertext_len;

  /* Create and initialise the context */
  if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();

  /* Initialise the encryption operation. IMPORTANT - ensure you use a key
   * and IV size appropriate for your cipher
   * In this example we are using 256 bit AES (i.e. a 256 bit key). The
   * IV size for *most* modes is the same as the block size. For AES this
   * is 128 bits */
  if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
    handleErrors();

  /* Provide the message to be encrypted, and obtain the encrypted output.
   * EVP_EncryptUpdate can be called multiple times if necessary
   */
  if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
    handleErrors();
  ciphertext_len = len;

  /* Finalise the encryption. Further ciphertext bytes may be written at
   * this stage.
   */
  if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) handleErrors();
  ciphertext_len += len;

  /* Clean up */
  EVP_CIPHER_CTX_free(ctx);

  return ciphertext_len;
}

int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
  unsigned char *iv, unsigned char *plaintext)
{
  EVP_CIPHER_CTX *ctx;

  int len;

  int plaintext_len;

  /* Create and initialise the context */
  if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();

  /* Initialise the decryption operation. IMPORTANT - ensure you use a key
   * and IV size appropriate for your cipher
   * In this example we are using 256 bit AES (i.e. a 256 bit key). The
   * IV size for *most* modes is the same as the block size. For AES this
   * is 128 bits */
  if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
    handleErrors();

  /* Provide the message to be decrypted, and obtain the plaintext output.
   * EVP_DecryptUpdate can be called multiple times if necessary
   */
  if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
    handleErrors();
  plaintext_len = len;

  /* Finalise the decryption. Further plaintext bytes may be written at
   * this stage.
   */
  if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) handleErrors();
  plaintext_len += len;

  /* Clean up */
  EVP_CIPHER_CTX_free(ctx);

  return plaintext_len;
}

void handleErrors(void)
{
  ERR_print_errors_fp(stderr);
  abort();
}

