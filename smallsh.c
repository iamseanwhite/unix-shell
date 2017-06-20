#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
//#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>

int isBackgroundProcessRunning = 0;
int isforGroundProcessRunning = 0;
int foregroundMode = 0;

void catchSIGINT(int signo) {
    
    if (signo == SIGINT && isforGroundProcessRunning == 1) {
        
        char* message = "terminated by signal 2\n";
        
        write(STDOUT_FILENO, message, 23);
        
        isforGroundProcessRunning = 0;
    }
    
    //if (signo == SIGINT && isBackgroundProcessRunning == 1) {
        
    //}
    
    if (signo == SIGTSTP && foregroundMode == 0) {
        
        char* message = "\nEntering foreground-only mode (& is now ignored)\n";
        
        write(STDOUT_FILENO, message, 50);
        
        foregroundMode = 1;
        
    }
        
    else if (signo == SIGTSTP && foregroundMode == 1) {
        
        char* message = "\nExiting foreground-only mode\n";
        
        write(STDOUT_FILENO, message, 30);
        
        foregroundMode = 0;
    }
    
    fflush(stdout);
    
}



//function that takes user input and builds up an array of that input using token

void getInfoFromUser(char* inputArray[], char* lineFromSource) {
    
    const char space[2] = " ";
    char* token;
    int counter = 0;
    
    token = strtok(lineFromSource, space);
    
    while (token != NULL){
        
        inputArray[counter] = token;
        
        //printf("%s\n", token);
        
        token = strtok(NULL, space);
        
        counter++;
    }
    
}


int main(int argc, char* argv[]) {
   
    
    struct sigaction SIGINT_action = {0};
    
    SIGINT_action.sa_handler = catchSIGINT;
    sigfillset(&SIGINT_action.sa_mask);
    SIGINT_action.sa_flags = SA_RESTART;
    
    sigaction(SIGINT, &SIGINT_action, NULL);
    sigaction(SIGTSTP, &SIGINT_action, NULL);
    
    pid_t shellPid = getpid();
    char shellPidString[8];
    sprintf(shellPidString, "%i", shellPid);
    
    pid_t backgroundJobs[100];
    pid_t lastForegroundJob;
    int numOfBGJobs = 0;
    
    pid_t spawnpid = -5;
    int childExitStatus = -5;
    //int foregroundExistStatus = -5;
    pid_t returnPid;
    pid_t foregroundReturnPid = -5;
    
    
    
    while(1){
       
        //checks array of bg processes to see if any have finished
        for(int i = 0; i < numOfBGJobs; i++) {
            returnPid = waitpid(backgroundJobs[i], &childExitStatus, WNOHANG);
            if (returnPid > 1) {
                printf("background pid %d is done: ", backgroundJobs[i]);
                //backgroundJobs[i] = NULL;
                //numOfBGJobs--;                        //how does this array work, exactly...
            
                //prints out exit status
                if (WIFEXITED(childExitStatus) != 0) {
                    //printf("Process exited normally\n");
                    int exitStatus = WEXITSTATUS(childExitStatus);
                    printf("exit value: %d\n", exitStatus);
                    fflush(stdout);                         //need to add the signal one too?
                }
                
                if (WIFSIGNALED(childExitStatus) != 0) {
                    int termSignal = WTERMSIG(childExitStatus);
                    printf("terminated by signal %d\n", termSignal);
                    fflush(stdout);  
                }
            }
        }
        
        isforGroundProcessRunning = 0;
        printf(":");
        fflush(stdout);
        
        //resets these values every time a new command is entered
        
        int numberOfCharsEntered = -5;
        size_t bufferSize = 0;
        char* lineEntered = NULL;
        char* tokenizedInputArray[513];
        memset(tokenizedInputArray, '\0', 513);
        int numberOfArgumentsEntered = 0;
        int backgroundFlag = 0;
    
        //get user input
        
        while(1){
            
            numberOfCharsEntered = getline(&lineEntered, &bufferSize, stdin);
            
            if (numberOfCharsEntered == -1) {
                
                clearerr(stdin);
            }
            
            if (numberOfCharsEntered == 1) {
                printf(":");
                fflush(stdout);
            }
            
            else {
                break;
            }
            
        }
        
       
        
        //get rid of new liine - replace with null terminator
        lineEntered[strcspn(lineEntered, "\n")] = '\0';
        
       
        
        
        //calculates how many arguments were entered based on how many spaces
        
        for (int i = 0; i < numberOfCharsEntered; i++) {
            
            if (lineEntered[i] == ' ') numberOfArgumentsEntered++; 
        }
        
        
        
        
        //function takes user input line and an empty array and uses strtok() to fill array with that input
        
        getInfoFromUser(tokenizedInputArray, lineEntered);
        
        
        
        //So the first letter of the first word can later be checked to see if its a '#'
        
        char* firstWord = tokenizedInputArray[0];
        
        
        
        //checks if '&' was entered as the last argument and if so removes it, decrements num of args and sets background flag
        
        if (strcmp(tokenizedInputArray[numberOfArgumentsEntered], "&") == 0) {
            tokenizedInputArray[numberOfArgumentsEntered] = NULL;
            numberOfArgumentsEntered--;
            backgroundFlag = 1;
        }
        
        
        
        //searches each token of user input for "$$" and replaces with shellPid
        
        for (int i = 0; i < numberOfArgumentsEntered + 1; i++) {
            
            char* wordThatMayHaveDollars = tokenizedInputArray[i];
            for (int j = 0; j < strlen(wordThatMayHaveDollars); j++) {
                if (wordThatMayHaveDollars[j] == '$' && wordThatMayHaveDollars[j+1] == '$'){
                    wordThatMayHaveDollars[j+1] = '\0';
                    wordThatMayHaveDollars[j+2] = '\0';
                    wordThatMayHaveDollars[j+3] = '\0';
                    wordThatMayHaveDollars[j+4] = '\0';
                    wordThatMayHaveDollars[j+5] = '\0';
                    wordThatMayHaveDollars[j+6] = '\0';
                    wordThatMayHaveDollars[j+7] = '\0';
                    for (int k = 0; k < strlen(shellPidString); k++) {
                        wordThatMayHaveDollars[j] = shellPidString[k];
                        j++;
                        
                    }
                    
                }
           }
       
        }
        
        
        
        //checks to see if command is one of the three built in functions
        
        if(strcmp(tokenizedInputArray[0], "exit") == 0) {
          
          //checks array of bg processes to see if any have finished 
           
            for(int i = 0; i < numOfBGJobs; i++) {
                kill(backgroundJobs[i], SIGKILL);
                waitpid(backgroundJobs[i], &childExitStatus, WNOHANG);
            
            }
           
           //need to kill all other proceses first..... kill -KILL 0 for fg?... atexit? _exit?
           
           
           exit(0);
        }
        
        
        else if(strcmp(tokenizedInputArray[0], "cd") == 0) {
        
            
            if (numberOfArgumentsEntered == 0) {
                
                //printf("HOME : %s\n", getenv("HOME"));
                int chdirReturnValue = chdir(getenv("HOME"));
                if (chdirReturnValue == 0) {
                  //printf("Sucessful dir change\n");
                }
            }
            
            else {
               
                int chdirReturnValue = chdir(tokenizedInputArray[1]);
                if (chdirReturnValue == 0) {
                  //printf("Sucessful dir change\n");
                }
                
            }
        }
        
        
        else if (strcmp(tokenizedInputArray[0], "status") == 0) {
            
            if (WIFEXITED(childExitStatus) != 0) {
                //printf("Process exited normally\n");
                int foregroundExitStatus = WEXITSTATUS(childExitStatus);
                printf("exit value %d\n", foregroundExitStatus);
                fflush(stdout);                         
            }
            
            if (WIFSIGNALED(childExitStatus) != 0) {
                int termSignal = WTERMSIG(childExitStatus);
                printf("terminated by signal %d\n", termSignal);
                fflush(stdout);  
                }
        }
        
        
        else if (firstWord[0] == '#') {
            continue;
        }
        
        
        else {
            
            spawnpid = fork();
            
            switch (spawnpid) {
                case -1: {
                    //done goofed
                    printf("Something seems to have gone wrong. -__-");
                    exit(1);
                    break;
                }
                
                case 0: {
                    //child
                    
                    int positionOfOut = -1;
                    int positionOfIn = -1;
                    
                    //checks to see if there is a > or a < in the user input and store its position if so
                    for (int i = 0; i < numberOfArgumentsEntered; i++) {
                        
                        if (strcmp(tokenizedInputArray[i], ">") == 0) {
                            
                            positionOfOut = i;
                        }
                        
                         if (strcmp(tokenizedInputArray[i], "<") == 0) {
                            
                            positionOfIn = i;
                        }
                    }
                    
                    
                        
                    //if there is a >
                    if (positionOfOut != -1) {
                            
                        int targetFD = open(tokenizedInputArray[positionOfOut+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    
                        if (targetFD == -1) { 
                            perror("open()"); 
                            exit(1); 
                        }
                        
                        //printf("targetFD = %i\n", targetFD);
                        
                        int result = dup2(targetFD, 1);
                        
                        if (result == -1) { 
                            perror("dup2"); 
                            exit(2); 
                        }
                        
                        //printf("targetFD = %i, result = %i\n", targetFD, result);
                        
                       // printf("%s and %s are being nulled \n", tokenizedInputArray[i], tokenizedInputArray[i+1]);
                        tokenizedInputArray[positionOfOut] = NULL;
                        tokenizedInputArray[positionOfOut+1] = NULL;
                       
                        fflush(stdout);
                    } 
                       
                        
                     
                    //if there is a <
                    if  (positionOfIn != -1) {
                        // printf("3\n");
                        
                        int sourceFD = open(tokenizedInputArray[positionOfIn+1], O_RDONLY);
                    
                        if (sourceFD == -1) { 
                            perror("open()"); 
                            exit(1); 
                        }
                        
                        //printf("sourceFD = %i\n", sourceFD);
                        
                        int result2 = dup2(sourceFD, 0);
                        
                            if (result2 == -1) { 
                            perror("dup2"); 
                            exit(2); 
                        }
                        
                        //printf("sourceFD = %i, result2 = %i\n", sourceFD, result2);
                        
                       // printf("%s and %s are being nulled \n", tokenizedInputArray[i], tokenizedInputArray[i+1cat]);
                        
                        fflush(stdout);
                        tokenizedInputArray[positionOfIn] = NULL;
                        tokenizedInputArray[positionOfIn+1] = NULL;    
                    }
                    //    printf("4\n");
                   
                    
                    //    printf("5\n");
                    
                    //tokenizedInputArray[1] = NULL;
                    //tokenizedInputArray[2] = NULL;
                    
                    //if running in bg
                    if (backgroundFlag == 1 && positionOfIn == -1 && foregroundMode == 0) {
                        
                        //redirect stdin
                        int sourceBackgroundFD = open("/dev/null", O_RDONLY);
                    
                        if (sourceBackgroundFD == -1) { 
                            perror("open()"); 
                            exit(1); 
                        }
                        
                        //printf("sourceBackgroundFD = %i\n", sourceBackgroundFD);
                        
                        int result3 = dup2(sourceBackgroundFD, 0);
                        
                            if (result3 == -1) { 
                            perror("dup2"); 
                            exit(2); 
                        }
                        
                        //redirect stdout
                        int targetBackgroundFD = open("/dev/null", O_WRONLY, 0644);     //DO I NEED DIFFERENT FLAGS FOR THIS?
                    
                        if (targetBackgroundFD == -1) { 
                            perror("open()"); 
                            exit(1); 
                        }
                        
                        //printf("targetBackgroundFD = %i\n", targetBackgroundFD);
                        
                        int result4 = dup2(targetBackgroundFD, 1);
                        
                            if (result4 == -1) { 
                            perror("dup2"); 
                            exit(2); 
                        }
                        
                    }
                        
                    
                    
                    //printf("6\n");
                    fflush(stdout);
                    
                    int execResult = execvp(tokenizedInputArray[0], tokenizedInputArray);
                    if (execResult == -1) { 
                            perror("exec()");
                            exit(1);
                    }
                   // printf("That command doesn't mean anything\n");
                   
                    break;
                }
                
                default: {
                    //parent
                    if (backgroundFlag == 0) {
                        isforGroundProcessRunning = 1;
                        pid_t actualPid = waitpid(spawnpid, &childExitStatus, 0);
                        lastForegroundJob = spawnpid;
                        break;
                    }
                    
                    else if (backgroundFlag == 1 && foregroundMode == 1){
                        isforGroundProcessRunning = 1;
                        pid_t actualPid = waitpid(spawnpid, &childExitStatus, 0);
                        lastForegroundJob = spawnpid;
                        break;
                    }
                    else
                        printf("background pid is %d\n", spawnpid);
                        fflush(stdout);
                        isBackgroundProcessRunning = 1;
                        backgroundJobs[numOfBGJobs] = spawnpid;
                        numOfBGJobs++;
                        break;
                    
                        
                    
                }
                
            }
            
        }
        
        
        free(lineEntered);
        
        lineEntered = NULL;
    
    }
    
    return 0;
}