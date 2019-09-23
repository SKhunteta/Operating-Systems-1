/*
Shreyans Khunteta
March 15th 2017
Program 4
CS344

This program performs exactly like otp_enc_d, in syntax and usage. 
In this case, however, otp_dec_d will decrypt ciphertext it is given, 
using the passed-in ciphertext and key. 
Thus, it returns plaintext again to otp_dec.
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
#include <sys/wait.h>
int cipherNum; 
int keyNum;
int decipherNum;

int main(int argc, char** argv){
  int i, listeningPort, socketfd, clientSocket,status;
  struct sockaddr_in server;
  
  if((socketfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){ 
    fprintf(stderr,"create the socket\n");   
    //socket can't be created
    exit(1);
    //exit(1) is getting a lot of use in this program huh.
    //so many ways to screw up
    //like I did writing this.
  }
  
   if(argc < 2) { 
    fprintf(stderr, "Please include port number\n"); 
    //if it isn't the right amount of arguments
    //as we can't have less than two
    exit(1);
  }
  
  else{
    listeningPort = atoi(argv[1]); 
  }    //listening port is converted

  
  //AF_INET is an address family 
  //that is used to designate the type 
  //of addresses that your socket can communicate with
  server.sin_family = AF_INET;
  server.sin_port = htons(listeningPort);
  //add any addresses trying to link
  server.sin_addr.s_addr = INADDR_ANY;
  
  if(bind(socketfd,(struct sockaddr *) &server, 
  sizeof(server)) == -1){ 
    fprintf(stderr,"failed to bind\n"); 
    //not able to bind together
    exit(1);
  }
  //gotta connect 
  if(listen(socketfd, 5) == -1) {
  fprintf(stderr, 
  "listen to call failed\n"); 
  //call failed
  exit(1); 
  } 
  
  while(1){
  	//what if we tried to accept the client
  	//but he gave us like no arguments
  	//what would we do?
    clientSocket = accept(socketfd, NULL, NULL);
    //this. We'd set it to -1
    if(clientSocket == -1){
      fprintf(stderr,"accept call failed." 
      "Your client is fake news \n"); 
      //and fail him.
      exit(1);
    }
    
    //you gotta fork it
    int pid = fork();
    if(pid == -1){ 
      fprintf(stderr,"fork\n"); 
      //process id has an error
    }
    else if(pid == 0){
    //WHAT IF IT WORKS THO
    //The htonl() function converts 
    //the unsigned integer hostlong 
    //from host byte order to network byte order.
      int toSend = htonl(0);
      
      //sending our stuff to client.
      if(send(clientSocket, &toSend, sizeof(toSend), 0) == -1){
        fprintf(stderr, 
        "client send failed\n"); 
        //send failed
      }
      int cipherNum;
      if(recv(clientSocket, &cipherNum, sizeof(cipherNum), 0) == -1){
        fprintf(stderr, 
        "receiving cipher text end_d -1\n"); 
        //recieve failed
      }
      else if(cipherNum == 0){
        fprintf(stderr, 
        "receiving cipher text size 0\n");
      }
      int cLen = ntohl(cipherNum); 
      int kNum; //converted
      
      if(recv(clientSocket, &kNum, sizeof(kNum), 0) == -1){ 
        fprintf(stderr, 
        "recieving key text size end_d -1\n"); 
        //fails to receive size
      } 
      
      else if(kNum == 0){
        fprintf(stderr, 
        "key text size of 0\n");
      }
      
      int keyLen = ntohl(kNum); //converts
      char *cipherText = malloc(sizeof(char)* cLen);  //allocated
      //now we can create cipherText files.
      char buffer[1024];
      memset(cipherText, '\0', cLen);
      int len = 0;
      int rec;
      while(len < cLen){ //while but not recieved
        memset((char *)buffer, '\0', sizeof(buffer));
        //The recv function receives data from a 
        //connected socket or a bound connectionless socket.
        //we're moving that data into rec.
        rec = recv(clientSocket, &buffer, 1024, 0); 
        
        //if rec doesn't have coherent data.
        if(rec == -1){
          fprintf(stderr, "recieving cipher text file -1\n");
          break;
        }
        //if it does have coherent data
        else if(rec == 0){
        //if the regular length is more than
        //encrypted len
        //break
          if(len < cLen){ 
            break; 
            //not enough
          }
        }
        else{
          strncat(cipherText, buffer, (rec-1));
        }
        len += (rec - 1); 
      }      
      //add to below len
      
      cipherText[cLen - 1] = '\0'; 
      
      char *keyText = malloc(sizeof(char) * keyLen); 
      memset((char *)&buffer, '\0', sizeof(buffer)); //allocate
      memset(keyText, '\0', keyLen);
      len = 0;
      
      while(len <= keyLen){
        memset((char *) buffer, '\0', sizeof(buffer));
        rec = recv(clientSocket, &buffer, 1024, 0);
          if(rec == -1){ 
          //failed
            fprintf(stderr, 
            "receiving key text file dec_d\n");
            break;
          }
          else if(rec == 0){ // end of data send
            break;
          }
          else{
            strncat(keyText,buffer, (rec-1)); 
            //concat
          }
          len += (rec - 1);
      }
      
      keyText[keyLen - 1] = '0';
      
      for(i = 0; i < cLen - 1; i++){
        if(cipherText[i] == ' '){ //space place
          cipherNum = 26;
        }
        else{
          cipherNum = cipherText[i] - 65; 
          //letter 
        }
        
        if(keyText[i] == ' '){//place space
          keyNum = 26l;
        }
        else{
          keyNum= keyText[i] - 65;
        }
        
        decipherNum = cipherNum - keyNum;
        if( decipherNum < 0){ //if < 0 then + 27
          decipherNum += 27;
        }
        if(decipherNum == 26){
          cipherText[i] = ' ';
        }
        else{
          cipherText[i] = 'A' + (char)decipherNum;
        }
      }
        
      len = 0;
      while(len <= cLen){
      //halleujah
        char plainSend[1024];
        strncpy(plainSend, &cipherText[len], 1023);
         //copy
        plainSend[1024] = '\0'; //term
          
        if(send(clientSocket, &plainSend, 1024, 0) == -1){
          fprintf(stderr, 
          "sending the decrypting text\n");
        }
        len += 1023;
      }
        
      free(cipherText);
      //and it is freed.
    }
    else{
      close(clientSocket);
        
      do{
        waitpid(pid, &status, 0);
      } while(!WIFEXITED(status) && !WIFSIGNALED(status));
      //thank you shell project for teaching me this
    }
  }
  close(socketfd);
  return 0;
}