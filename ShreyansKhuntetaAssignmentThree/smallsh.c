#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>

int background = 0;
int statusCode;
int nullValue = 0;
int backgroundProcessArray[100];
int numBackgroundProcesses = 0;
int status;
int quit = 0;
int i;
char line[512];
int numberArgs;
int redirection = 0;
char *command;
char *arguments[512];
char chosenDirectory[1024];


void printingDot(void) {
	fflush(stdout);
	fflush(stdin);
	printf(": ");
	fflush(stdin);
	//This whole thing above is how
	//we're going to create the whole
	// ":" that appears before
	//you give a command's name in the shell.
}

void grabInput(void) {
	printingDot();
	if (fgets(line, sizeof(line), stdin) != NULL) {
		char *position;
		position = strchr(line, '\n'); 
		//find the new line value
	*position = '\0' ; 
	//change it to end string value
	if ((position = strchr(line, '&')) != NULL) {
		//if there is an &
		*position = '\0'; 
		//remove it
		background = 1;
		//keeping track of background process
	}	else {
		background = 0; 
		//no background process.
	}
	} else {
		nullValue = 1;
		return;
	}
}

void grabDirectory(void) {

getcwd(chosenDirectory, sizeof(chosenDirectory));
//we just grabbed the current working directory!
char *path = strstr(line, " ");
	if (path) {
		path += 1; //move over one spot
		char *value;
		value = malloc(strlen(path));
		memcpy(value, path, strlen(path));
		*(value +strlen(path)) = 0;
		sprintf(chosenDirectory, "%s/%s", chosenDirectory,
		value); //create a full current working directory.
		free(value); //clear up memory.
	}
}

void checkProcessPrint(void) {
printf("Child %d exit value = %d \n",
			backgroundProcessArray[i],
			WEXITSTATUS(status));
}


void checkProcesses(void) {
//int meaninglessness;
//We run this
//before each Input and check
//if any processes have ended
	for (i = 0; i < numBackgroundProcesses; i++) {
		//we'll run through array of process ids
		if (waitpid(backgroundProcessArray[i], &status, 
		WNOHANG) >0 ) {
		
		if (WIFEXITED(status)) {
			checkProcessPrint();
		}
		if (WIFSIGNALED(status)) {
		//if signalled
			checkProcessPrint();
		}
	}
}
}


int main(int argc, char *argv[], char *envp[] ) {
while (quit == 0) {
	//check process before getting the input
	checkProcesses();
	grabInput();
	
	if (nullValue == 1) {
		//if the input was null
		nullValue = 0; //change value to 0.
		return 0; //end.
	}
	 else if (strcmp(line, "exit") == 0) {
	 	quit = 1;
	 	return 0;
	 }
	  else if (strstr(line, "#")) {
	  	continue;
	  }
	  
	  else if (strcmp(line, "status") == 0) {
	  		printf("exit value: %d \n", statusCode);
	  		continue;
	  }
	  else if(strncmp("cd", line, strlen("cd")) == 0) {
	  if(line[2] == ' ') { 
	  	//If you enter cd (to go into directory)
	  		grabDirectory();
	  		//check the function we have made above.
	  	chdir(chosenDirectory); //change it to that directory.
	  	
	  }	
	  else {
	  	char *home = getenv("HOME"); //grab home location
	  	chdir(home); //change current working directory to home.
	}
		
	} 

	else {
		pid_t pid, ppid;
		int status;
		
		
		command = strtok(line, " ");
		if (command == NULL) //look if this is the case
		//then nothing is gonna happen
		//cause it's null dummy.
		{
			continue;
			//a whole load of nothing.
			//thanks for wasting everyone's time.
		}
		//set up array for executing file.
			arguments[0] = command;
			numberArgs = 1;
			arguments[numberArgs] = strtok(NULL, " "); //place next item
		//into the next spot.
			while(arguments[numberArgs] != NULL) {
				numberArgs++;
				arguments[numberArgs] = strtok(NULL, " ");
		}
		//check if background process is true.
		if (background == 1) {
			//hell yeah bruh it be lit up in here.
			if ((pid = fork()) < 0) {
				perror("Error whilst forking");
				exit(1);
			}
			if(pid == 0) { //Child process
				for(i = 0; i < numberArgs; i++) {
					if(strcmp(arguments[i], "<") == 0) { //Input redirection
						if(access(arguments[i+1], R_OK) == -1) { 
						//If we cannot read the file
						//Specifically if we cannot access it.
							printf("cannot open %s for input\n", arguments[i+1]);
							redirection = 1;
						}
						else { 
						//If the file can be read
						//or accessed.
							int fd = open(arguments[i+1], O_RDONLY, 0);
							dup2(fd, STDIN_FILENO);
							close(fd);
							redirection = 1;
							execvp(command, &command); 
							//Execute the command
							//release the kracken.
						}
					}
					if(strcmp(arguments[i], ">") == 0) { 
					//Output redirection
						int fd = creat(arguments[i+1], 0644); 
						//Create the file
						dup2(fd, STDOUT_FILENO);
						close(fd);
						redirection = 1;
						execvp(command, &command); 
						//Execute the command
					}
				}
				if(redirection == 0) { //If no redirection
						execvp(command, arguments); //Execute the command
				}
					printf("%s no such file or directory\n", command); 
					//Only gets here if there is an error
					exit(1);
			}
			else { //Parent process
					int status;
					int process;
					printf("background pid is %d\n", pid);
					backgroundProcessArray[numBackgroundProcesses] = pid; 
					//Keep track of background processes
					numBackgroundProcesses++;
					process = waitpid(pid, &status, WNOHANG);
					continue;
				}
			}
			else { //If it's a foreground process
				if((pid = fork()) < 0) {
					perror("Error while forking");
					exit(1);
				}
				if(pid == 0) { //Child process
					for(i = 0; i < numberArgs; i++) {
						if(strcmp(arguments[i], "<") == 0) { 
						//Input redirection
							if(access(arguments[i+1], R_OK) == -1) {
							 //If we cannot read the file for w/e reason
								printf("cannot open %s for input\n", arguments[i+1]);
								redirection = 1;
							}
							else { //If the file can be read
								int fd = open(arguments[i+1], O_RDONLY, 0);
								dup2(fd, STDIN_FILENO);
								close(fd);
								redirection = 1;
								execvp(command, &command); 
								//Execute the command
							}
						}
						if(strcmp(arguments[i], ">") == 0) { 
						//Output redirection
							int fd = creat(arguments[i+1], 0644); 
							//Create the file
							dup2(fd, STDOUT_FILENO);
							close(fd);
							redirection = 1;
							execvp(command, &command); 
							//Execute the command
						}
					}
					if(redirection == 0) { 
					//If no redirection
						execvp(command, arguments);
						 //Execute the command
					}
					printf("%s no such file or directory\n", command); 
					//Only gets here if there is an error
					exit(0);
				}

			else { 
			//Parent process
					int status;
					waitpid(pid, &status, 0); 
					//Wait for the process
					if(WIFEXITED(status)) {
						statusCode = WEXITSTATUS(status);
					}
				}
			}
			
		}
		signal(SIGINT, SIG_IGN); 
		//Capture the sigint and ignore it
	}
	return 0;
	}
