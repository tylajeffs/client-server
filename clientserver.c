#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
2


#define BUFFER_SIZE 3000
#define TOKENS_SIZE 50

//functions
void echoFunc();
int lsFunc(const char *dir,int op_a,int op_l);
void cdFunc();
void mkdirFunc();
void rmdirFunc();
void execute_normal();
void execute_redirect();

//globals
char buffer[BUFFER_SIZE];
char *tokens[TOKENS_SIZE];
int num_tokens = 0;
char *token_ptr;
char s[100];

//main function
int main(int argc, char **argv) {

    //if first token is ls, call ls function and then loop back through and ask for more input
    //if the command doesn't match, create a new process to run the program (g++ test.cpp) (./a.out hello class how are you)
    
    
    //infinite loop to ask user for input - only ends if they type in "exit"
    int ex = 0;
    
    while(ex == 0) {
        
        //reset stuff
        num_tokens = 0;
        memset(tokens, 0, TOKENS_SIZE);
        
        //print the current working directory
        printf("FLOWERCONSOLE:%s $ ", getcwd(s, 100));
        
        //Read in a line using fgets, don't forget to trim the new line at the end
        fgets(buffer, BUFFER_SIZE, stdin); //strip off newline
        
       
        buffer[strcspn(buffer, "\n")] = 0;
       
        //get the first token & add it to the array of tokens
        char* piece = strtok(buffer, " ");
        tokens[0] = piece;
        num_tokens++;
       
        //get the rest of the tokens
        while(piece != NULL) {
           
            //get the token, add it to the array
            piece = strtok(NULL," ");
           
            if(piece != NULL) {
                tokens[num_tokens] = piece;
                num_tokens++;
            }
           
        }
       
        //put a 0 (null) as the last token in the array
        tokens[num_tokens] = NULL;
            
            
        

        
        //check the user's inputs and do the correct process
        if((strcmp(tokens[0],"exit")) == 0) {
            
            //set ex to true so the infinite loop will break
            printf("...exiting while loop...\n");
            ex = 1;
            
        } else if(strcmp(tokens[0],"echo") == 0) {
            
            //call the echo function
            echoFunc();
            
        } else if(strcmp(tokens[0],"ls") == 0) {
            
            lsFunc(".",0,0);
            
            
        } else if(strcmp(tokens[0],"mkdir") == 0) {
            
            //call the mkdir function
            mkdirFunc();
            
        } else if(strcmp(tokens[0],"cd") == 0) {
            
            //call the cd function
            cdFunc();
            
        } else if(strcmp(tokens[0],"rmdir") == 0) {
            
            //call the cd function
            rmdirFunc();
            
        } else { //launch process
            
            execute_normal();
        }
    }
    
    return 0;
}


//method to run the echo function
//prints out the remaining arguments
void echoFunc() {
    
    //print out the remaining arguments
   for(int i = 0; i < num_tokens; i++) {
       
       printf("Token #%i: %s\n",i, tokens[i]);
   }

}





//method to list out all the files in the folder
int lsFunc(const char *dir,int op_a,int op_l)
{
	//Here we will list the directory
	struct dirent *d;
	DIR *dh = opendir(dir);
	if (!dh)
	{
		if (errno = ENOENT)
		{
			//If the directory is not found
			perror("Directory doesn't exist");
		}
		else
		{
			//If the directory is not readable then throw error and exit
			perror("Unable to read directory");
		}
		exit(EXIT_FAILURE);
	}
	//While the next entry is not readable we will print directory files
	while ((d = readdir(dh)) != NULL)
	{
		//If hidden files are found we continue
		if (!op_a && d->d_name[0] == '.')
			continue;
		printf("%s  ", d->d_name);
		if(op_l) printf("\n");
	}
	if(!op_l)
	printf("\n");
}


//method to change directory
void cdFunc() {

    // using the command
    //chdir(tokens[1]);
    
    
    int check;
    check = chdir(tokens[1]);
  
    // check if you can go into the directory
    if (!check)
        printf("entered directory\n");
    else {
        printf("invalid directory\n");
    }
    
}


//method to make a new folder
void mkdirFunc() {
    
    int c;
    c = mkdir(tokens[1],0777);
 
    // check if directory is created or not
    if (!c)
        printf("Directory created\n");
    else {
        printf("Unable to create directory\n");
    }
        
}


//method to remove a directory
void rmdirFunc() {
    
    int check;
    check = rmdir(tokens[1]);
  
    // check if directory is created or not
    if (!check)
        printf("Directory removed\n");
    else {
        printf("Unable to remove directory\n");
    }
      
    
}


//function to launch a process
void execute_normal() {
    
    pid_t pid;
    int status;
    pid = fork();
    
    if(pid < 0) {
        
        printf("ERROR fork failed\n");
        exit(1);
        
    } else if(pid == 0) {//child thread
    
        if (execvp(*tokens, tokens) < 0) {
            
            perror("execvp");
            exit(1);
        }
    } else {
        
    while(wait(&status) != pid) {} //parent, waits for completion
    
    }
}



//function to launch a process
void execute_redirect() {
    pid_t pid;
    int status;
    int defout;
    int fd;
    pid = fork();
    
    
    if (pid < 0)  {
        
    printf("ERROR fork failed\n");
    exit(1);
    
    } else if (pid == 0) { //child thread

    defout = dup(1);
    fd=open("/home/cslade/cs415/lsout.txt",O_RDWR|O_CREAT,0644);
    dup2(fd,1);
    
        if (execvp(*tokens, tokens) < 0)
        {
            perror("execvp");
            exit(1);
        }
        
    close(fd);
    
    } else {
        
        while (wait(&status) != pid) {} //parent, waits for completion
    }
}