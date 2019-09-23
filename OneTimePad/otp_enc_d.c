/*
Shreyans Khunteta
March 15th 2017
Program 4
CS344

This program will run in the background as a daemon. 
Upon execution, otp_enc_d must output an error 
if it cannot be run due to a network error, 
such as the ports being unavailable. 
Its function is to perform the actual encoding, 
as described above in the Wikipedia quote. 
This program will listen on a particular port/socket, 
assigned when it is first ran (see syntax below). 
When a connection is made, otp_enc_d must call accept() 
to generate the socket used for actual communication, 
and then use a separate process to handle the rest of the transaction, 
which will occur on the newly accepted socket.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <netinet/in.h>
int plainNum;
int keyNum;
int encNum;

int main(int argc, char ** argv){
  
int i, listeningPort, socketfd, clientSocket, status;

   if (argc < 2) {  //arguments
  	fprintf(stderr, "Error: You must include a port number\n");
  	exit(1);
   }
   
   else {
   //try to recieve listening port
    listeningPort = atoi(argv[1]); 
   }

   if((socketfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
  	fprintf(stderr, "Error: with socket creation\n");
  	exit(1);
   }

   struct sockaddr_in server;
   server.sin_family = AF_INET;
   server.sin_port = htons(listeningPort);
   server.sin_addr.s_addr = INADDR_ANY;

   if (bind(socketfd, (struct sockaddr *) &server, sizeof(server)) == -1){
 	 fprintf(stderr, "Error: bind call failed\n");
 	 exit(1);
   }
   
   //call listen error
   //we must be able to do five separate encryptions at a time.
   if (listen(socketfd, 5) == -1){  
  	 fprintf(stderr, "Error: listen call\n");
 	 exit(1);
   }
   
   while(1){ 
  clientSocket = accept(socketfd, NULL, NULL);
  if (clientSocket == -1){//if the accept fails
      fprintf(stderr, "Error: The accept call failed\n");
      exit(1);
  }
  
  int pid = fork(); //process gets forked
  if (pid == -1){//the fork process has an error
     fprintf(stderr, "fork error\n");
  }
  
  else if(pid == 0) {
     int toSend = htonl(1);
     if(send(clientSocket, &toSend, sizeof(toSend),0) == -1){ 
    fprintf(stderr, "Error: failed to send to client send\n"); 
    //cannot send to client
    exit(1);
     }

     int plainNum; //size of plain text
     if(recv(clientSocket, &plainNum, sizeof(plainNum), 0) == -1){
     //recieve error
    	fprintf(stderr, "Error: receiving plain text size end_d is -1\n");
     }
     
     else if(plainNum == 0){ //plain text is 0 
    	fprintf(stderr, "Error: plain text size of 0\n");
     }

     int pLen = ntohl(plainNum);
     int kNum;    //convert
     if(recv(clientSocket, &kNum, sizeof(kNum), 0) == -1){ //receive size has an error
   	 	fprintf(stderr, "Error: receiving key text size end_d == -1\n");
     }
     	
     else if(kNum == 0){ // = 0
    	fprintf(stderr, "Error: key text size of 0\n");
     }
     
     //The ntohl() function converts the unsigned integer netlong 
     //from network byte order to host byte order.
     int kLen = ntohl(kNum); //convert
       char *plainText = malloc(sizeof(char) * pLen); //allocation
       char buffer[1024];
       memset(plainText, '\0', pLen); //cleared 

     int len = 0;
     int rec;
     while(len <= pLen){//file not  sent
        memset((char *)buffer, '\0', sizeof(buffer));//clear buffer 
      rec = recv(clientSocket, &buffer, 1024, 0);//receive

    if(rec == -1){ //recieving data is causing an error
      fprintf(stderr, "Error:receiving plain text file == -1\n");
      break;
        }
        else if(rec == 0){
        
      if (len < pLen){
      break;
      }
        }
        else{
        strncat(plainText,buffer,(rec - 1)); 
        } 
        len += (rec-1);
        //total len is added
     }
     plainText[pLen - 1] = '\0'; 

    char *keyText = malloc(sizeof(char) * kLen); 
    //keytext allocation
      
    memset((char *)buffer, '\0', sizeof(buffer)); 
    //buffer
    memset(keyText, '\0', kLen);
    len = 0;

     while(len <= kLen){//not yet recieved
          memset((char *)buffer, '\0', sizeof(buffer)); 
      	  rec = recv(clientSocket, &buffer, 1024, 0); 

      if(rec == -1){ //data recieve is not working correctly
        fprintf(stderr, "Error: receiving key text file = -1\n");
        break;
      }
      
      else if(rec == 0){
           break; //end of data
      }
      
      else {
        strncat(keyText,buffer,(rec - 1)); 
    	}
         
        len += (rec - 1);
     }
     keyText[kLen - 1] = '\0'; //add total len

  //thus begins our seemingly fucking endless series of error checks
  //YOU READY FAM?
    for (i = 0; i < pLen - 1; i++){ //encrypt
    if(plainText[i] == ' ') {
      plainNum = 26; 
    }
    
    else{ 
       //letter
       plainNum = plainText[i] - 65; //spacing 
    }
    
    if(keyText[i] == ' ') {
       keyNum = 26;
    }
    
    else {
       keyNum = keyText[i] - 65;
    }
  
    encNum = plainNum + keyNum;
    if (encNum >= 27){
       encNum -= 27;
    }
    if(encNum == 26){ 
       plainText[i] = ' ';
    }
    else{
       plainText[i] = 'A' + (char)encNum;
    }
} //mercifully our checks end

    
    len = 0;
    while (len <= pLen) { 
    //not complete sent
    char cipherSend[1024];
    strncpy(cipherSend, &plainText[len], 1023); 
    cipherSend[1024] = '\0'; 

      if(send(clientSocket, &cipherSend, 1024, 0) == -1){ //sending an error
        fprintf(stderr, 
        "Error: sending encryption text\n");
    }
    len += 1023; //total len
    
    }                
     //we gotta free plainText and keyText
     free(plainText);
     free(keyText);
  }      
  else{
     close(clientSocket); 
     do {
     
    waitpid(pid, &status, 0);
    }
     while(!WIFEXITED(status) && !WIFSIGNALED(status));   //close all connections
     //man thank god for that shell script assignment or this would have messed me
     //right up.
 	 }
	}
   close(socketfd); 
   return 0;
}