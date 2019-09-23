/*
Shreyans Khunteta
March 15th 2017
Program 4
CS344

this program will connect to otp_dec_d 
and will ask it to decrypt ciphertext 
using a passed-in ciphertext and key, 
and otherwise performs exactly like otp_enc, 
and must be runnable in the same three ways. 
otp_dec should NOT be able to connect to otp_enc_d, 
even if it tries to connect on the correct port - 
you'll need to have the programs reject each other, 
as described in otp_enc.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>

int main(int argc, char **argv){
  
  if (argc < 4){ //if there are more than 4 arguments
    fprintf(stderr, "incorrect # of arguments,  specify");
    exit(1);
  }
  
   //portnum = conversation
  int portNum = atoi(argv[3]);
   //key open
   //O_RDONLY means open and read only
  int fdKey = open(argv[2], O_RDONLY);
  //ciphertext is open
  int CipherNoChiphing = open(argv[1], O_RDONLY); 
  
  
  int cryptLen = lseek(CipherNoChiphing, 0, SEEK_END); //length of cipher
  int keyLen = lseek(fdKey, 0, SEEK_END);//length of key
  
  
  if (CipherNoChiphing == -1 || fdKey == -1){ //double check if opened
  	fprintf(stderr, "couldn't open the files\n");
  	exit(1);
  }
  
  
  
  if (keyLen < cryptLen) { 
  //if key > cipher
  	fprintf(stderr, "Key is shorter than Cipher\n");
  	exit(1);
    }
  
  char *cText = malloc(sizeof(char)* cryptLen); 
  //cipher is held
  lseek(CipherNoChiphing, 0, SEEK_SET); 
  //filelength start
  
  if(read(CipherNoChiphing, cText, cryptLen) == -1){
    fprintf(stderr, "Reading cipher text dec\n");
    exit(1);
  }
  
  cText[cryptLen] = '\0';
  
  int i;
  for(i = 0; i < cryptLen; i++){
	//if letter and spacing does nothing
    if(isalpha(cText[i]) 
    || isspace(cText[i]) 
    || ispunct(cText[i])) { 
    
    }
    else{
      fprintf(stderr, "Cipher text has invalid characters\n");
      exit(1);
    }
  }
  
  char *kText = malloc(sizeof(char)* keyLen);
  lseek(fdKey, 0, SEEK_SET);
  
  if(read(fdKey, kText, keyLen) == -1){
    fprintf(stderr, "Reading key text\n");
    exit(1);
  }
  
  int socketfd;
  if((socketfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
    fprintf(stderr, "socket error\n");
    exit(1);
  }
  
  //creating struct for the address 
   struct hostent * server_ip_address;
   server_ip_address = gethostbyname("localhost"); 
   
   //empty address
   if(server_ip_address == NULL) { 
  	fprintf(stderr, "host name\n");
  	exit(1);
   }
   
   struct sockaddr_in server;
   memset((char *)&server, 0, sizeof(server)); 
   server.sin_family = AF_INET;
   server.sin_port = htons(portNum);
   memcpy(&server.sin_addr, server_ip_address->h_addr, server_ip_address->h_length);
   
   if(connect(socketfd, (struct sockaddr*) &server, sizeof(server)) == -1){
  	fprintf(stderr, "cant connecting\n");
  	exit(2);
   }
   
   int rec;
   int confirmationNum;
   if((rec = recv(socketfd, &confirmationNum, sizeof(confirmationNum), 0)) == -1){
  fprintf(stderr, "not able to receiving enc\n");
  exit(1);
   } 
   else if(rec == 0){
  	fprintf(stderr, "not able to receive enc is equal to 0\n");
  	exit(1);
   }
   
   int conf = ntohl(confirmationNum); // = confirmation number

   if (conf!= 0){ //if its wrong # 
   
  		fprintf(stderr, "It failed," 
  		"giving error that otp_dec cannot use otp_enc_d on port %d\n", portNum);
 		exit(2);
   }
   
    //converts to unsigned integer from host byte to network byte    
   int cryptLenSend = htonl(cryptLen); 

   if(send(socketfd, &cryptLenSend, sizeof(cryptLenSend), 0) == -1) { 
  	fprintf(stderr, "Error: sending cipher text file\n");
  	exit(1);
   }

	//converts to unsigned integer from host byte to network byte
   int keyLenSend = htonl(keyLen); 

   if(send(socketfd, &keyLenSend, sizeof(keyLenSend), 0) == -1) { //key not sent properly
  	fprintf(stderr, "Error: sending key text file\n");
  	exit(1);
   }
   
   
 	int lengt = 0;
 	//while length and cipher length match up.
	while (lengt <= cryptLen) {
  	char cipherSend[1024];
  	//copy of cipher to send
  	strncpy(cipherSend, &cText[lengt], 1023); 
  	//term
  	cipherSend[1024] = '\0'; 
  	
	//yo sometimes shit doesn't get sent right ya know
    if(send(socketfd, cipherSend, 1024, 0) == -1){
     printf("Error: sending cipher text!!\n");
     exit(1);
  }
  	lengt += 1023; //Add sent
}
   
   lengt = 0; //go back to 0
   
    
   while (lengt <= keyLen) { 
  char keySend[1024];
  strncpy(keySend, &kText[lengt], 1023);
  keySend[1024] = '\0';

    if(send(socketfd, &keySend, 1024, 0) == -1){
     fprintf(stderr, "Error: sent key text\n");
     exit(1);
  }
  lengt += 1023; //add len sent to len
   }
   
   char *plainText = malloc(sizeof(char) * cryptLen); //allocate memory 
   char buffer[1024]; //buffer created
   memset(plainText, '\0', cryptLen); 
   lengt = 0;
   rec = 0;
   while(lengt < cryptLen){ 
    memset((char *)buffer, '\0', sizeof(buffer));
    rec = recv(socketfd, &buffer, 1024, 0); 
    
      if( rec == -1){
        fprintf(stderr, "Error receiving plain text file data\n");
        exit(1);
      }
      else if(rec == 0){
        if( lengt < cryptLen){ 
          fprintf(stderr, "Error receiving plain text file \n");
          exit(1);
        }
      }
      else{
        strncat(plainText, buffer, (rec - 1));
      }
    lengt += (rec-1);
   }
   
   plainText[cryptLen - 1] = '\0';
   printf("%s\n", plainText);
   
   //free plain text 
   free(plainText); 
    //free Key text 
   free(kText); 
 	//free cipherText 
   free(cText); 
   return 0;
}