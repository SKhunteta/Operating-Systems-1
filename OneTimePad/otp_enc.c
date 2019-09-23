/*Shreyans Khunteta
March 15th 2017
Program 4
CS344*/

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
int i;
int socketfd;
int rec;
int confNum;

int main(int argc, char **argv) {

   int portNum = atoi(argv[3]); 
   //retrieve and convert the port
   int fdPlain = open(argv[1], O_RDONLY); 
   //read
   int fdKey = open(argv[2], O_RDONLY);
   if(fdPlain == -1 || fdKey == -1) { 
   //error when opening
  	fprintf(stderr, "Error: opening files\n");
  	exit(1);
   }
	//key text length and plain text length
   int plainLen = lseek(fdPlain, 0, SEEK_END); 
   int keyLen = lseek(fdKey, 0, SEEK_END); 

   if(keyLen < plainLen){ 
   //see if key is smaller
  	fprintf(stderr,
  	 "This is the error about " 
  	 "the too short key " 
  	 "you're looking for\n");
  	exit(1);
   }

   char *plainText = malloc(sizeof(char) * plainLen); 
   //allocate
   lseek(fdPlain, 0, SEEK_SET); 
   //set to the start of the file

   if(read(fdPlain, plainText, plainLen) == -1){   
   //text read
  fprintf(stderr, "Error: reading plain text enc\n");
  exit(1);
   }
   
   plainText[plainLen - 1] = '\0';

  //see if correct
   for(i = 0; i < plainLen - 1; i++){ 
   //check if space or letter.
  if(isalpha(plainText[i]) || 
  	 isspace(plainText[i])){ 
  	 //printf("all good in the hood");
    //huzzah
  }
  
  else{ 
  //error
     fprintf(stderr, "Error: A plaintext has an invalid char." 
     " Your plaintext is fake news. \n");
     exit(1);
  }
   }

   char *keyText = malloc(sizeof(char) * keyLen); 
   //allocate to hold the text
   lseek(fdKey, 0, SEEK_SET); 

   if(read(fdKey, keyText, keyLen) == -1){
  	fprintf(stderr, "Error:reading key text enc\n");
  	exit(1);
   }
   keyText[keyLen - 1] = '\0'; 
 
   for(i = 0; i < keyLen - 1; i++){
  	if(isalpha(keyText[i]) || isspace(keyText[i])) {
   
  }
  else { //error
     fprintf(stderr, "Error:Key text invalid char\n");
     exit(1);
  }
}

   if((socketfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
   //creation error
  	fprintf(stderr, 
  	"error: creating socket\n");
  	exit(1);
   }
	//requirement checked off
   struct hostent * server_ip_address;
   server_ip_address = gethostbyname("localhost");

	//say if we can't access a server or it doesn't exist
   if(server_ip_address == NULL){
  	fprintf(stderr, "error: with host name\n");
  	exit(1);
   }
   
   struct sockaddr_in server;
   memset((char *)&server, 0, sizeof(server));
   server.sin_family = AF_INET;
   server.sin_port = htons(portNum);
   memcpy(&server.sin_addr, server_ip_address->h_addr, server_ip_address->h_length);
  
   if(connect(socketfd, (struct sockaddr*) &server,sizeof(server)) == -1) { 
   //socket connection
  	fprintf(stderr, "Error: connecting \n");
  	exit(2);
   }
  
   if((rec = recv(socketfd, &confNum, sizeof(confNum), 0)) == -1){ 
   //error when recieving 
  	fprintf(stderr, "Error: receiving enc\n");
  	exit(1);
   } 
   
   else if(rec == 0){
  	fprintf(stderr, "Error: receiving enc is 0\n");
  	exit(1);
   }

   int confirm = ntohl(confNum);
   if(confirm != 1){ 
   //wrong confirmation
  	fprintf(stderr, 
  	"Error: could not contact otp_enc_d on port %d\n", 
  	portNum);
  	exit(2);
   }
   
   int plainLenSend = htonl(plainLen); //convert

   if(send(socketfd, &plainLenSend, sizeof(plainLenSend), 0) == -1){
  	fprintf(stderr, 
  	"Error: sending plain text file\n");
  	exit(1);
   }

   int keyLenSend = htonl(keyLen); //convert

   if(send(socketfd, &keyLenSend, sizeof(keyLenSend), 0) == -1){
  	fprintf(stderr, "Error: sending key text file\n");
  	exit(1);
   }

   int len = 0;
   while(len <= plainLen){ //file still hasn't been sent
   	
  	char plainSend[1024];
  	strncpy(plainSend, &plainText[len], 1023);//copy to text send
  	plainSend[1024] = '\0';//term
   
  	if(send(socketfd, &plainSend, 1024, 0) == -1){ //fail to send
    	 fprintf(stderr, "Error: sending plain text\n");
     	 exit(1);
  	}
  	len += 1023;//total length addition
   }

   //set len back to zero.
   len = 0;
   //while whole key not sent
   while (len <= keyLen) {
  	char sendKey[1024];
  	 //copy keyText
  	strncpy(sendKey, &keyText[len], 1023);
  	sendKey[1024] = '\0'; 

    if(send(socketfd, &sendKey, 1024, 0) == -1){
    fprintf(stderr, "Error: sending key text\n");
    exit(1);
    }
    
  	len += 1023; //total len
   }

   char *cipherText = malloc(sizeof(char) * plainLen); //memory allocated for cipher
   char buffer[1042]; 
   memset(cipherText, '\0', plainLen); //cipher text
   len = 0;
   rec = 0;
   
   while(len < plainLen) { //whole file isn't recieved 
    memset((char *)buffer, '\0', sizeof(buffer));
    rec = recv(socketfd, buffer, 1024, 0);
        if(rec == -1){
      fprintf(stderr, "Error: receiving cipher text file == -1 \n");
      exit(1);
    }      
     
    else if(rec == 0){
      if(len < plainLen){
        fprintf(stderr, "Error: receiving cipher text file <\n");
        exit(1);
      }
    }
    else {
      strncat(cipherText,buffer,(rec-1)); //string concat
    }    
    len += (rec-1); //Add total
   }

   cipherText[plainLen - 1] = '\0';

   printf("%s\n", cipherText); 
   //print

   //BE FREE MY LOVIES
   //BE FREE PLAINTEXT
   free(plainText);
   //BE FREE KEYTEXT
   free(keyText);
   //BE FREE CIPHERTEXT
   free(cipherText);

   return 0;
}