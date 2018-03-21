
/*-----------------------------------------------------------------------------
 *  Note: If you compile this file with gcc, Please use the option -std=gnu99
 *  to compile the code.
 *
 *  $ gcc -std=gnu99 -o shell shell.c
 *  $ ./shell
 *-----------------------------------------------------------------------------*/

/*
 * =======================================================================================
 *
 *       Filename:  shell.c
 *
 *    Description:  The main parts of this program are parse(), spawnProcess(), execute().
 *                          parse()       : 324 - 361
 *                          spawnProcess(): 195 - 216
 *                          execute()     : 149 - 195
 *        Created:  03/27/16 17:04:18
 *       Compiler:  clang(no need of extra options), gcc(need extra option)
 *         Author:  ZHANG ZHENGWEN , lochuan@mail.com
 *      Student #:  2014920049
 *
 * =======================================================================================
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>

#define MAXLINELEN 100      //max chars in an input line
#define NWORDS 16           //max words on command line
#define MAXWORDLEN 64       //maximum word length
#define MAXPIPE 5
#define REOUT 5             // >
#define REIN 4              // <
#define PIPE_READ 0         //fd[0]
#define PIPE_WRITE 1        //fd[1]

extern char **environ;      //enviroment variables

char line[MAXLINELEN+1];    //input line
char *words[NWORDS];        //ptrs to words from the command line
int nwds;                   //number of words in the command line
int np;                     //number of pipeline symbols
int reidcr;                 //redirection indicator
char path[MAXWORDLEN];      //path to the command
char *argv[NWORDS+1];       //argv structure for execve

typedef struct command{
    char *words[NWORDS];
    int num;
}command;                   //the struct command is for storing commands which splited by '|'

typedef struct command* cp;
cp commandList[MAXPIPE];    //pointer type of struct command

int getLine(void);
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  getLine
 *  Description:  As same as the original source code literally, just put two functions
 *                inside.
 * =====================================================================================
 */
int spawnProcess(int in, int out, cp command);
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  spawnProcess
 *  Description:  Using fork system call to generate child processes one by one in a 
 *                proper order.
 * =====================================================================================
 */
int findPipe(char *line);
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  findPipe
 *  Description:  Retriving how many pipeline symbols within a line
 * =====================================================================================
 */
int parse(void);
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  parse
 *  Description:  Derived from the origin source code, taking '|', '<', '>l' as tokens,
 *                split a command line into separated words[MAXWORDLEN].
 * =====================================================================================
 */
int execok(cp command);
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  execok
 *  Description:  Little modified.
 * =====================================================================================
 */
void execute(void);
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  execute
 *  Description:  It is the core funtion over the whole program. All of forks, 
 *                redirections, and the other core features was supported by this guy.
 * =====================================================================================
 */
int makeFile(cp command);
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  makeFile
 *  Description:  Nothing but making a file.
 * =====================================================================================
 */
int openFile(cp command);
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  openFile
 *  Description:  Nothing but opening a file.
 * =====================================================================================
 */
int findRedirection(char *line);
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  findRedirection
 *  Description:  This one and findPipe are very similar. For checking which kind of 
 *                redirection that the users want to. Either input or output.                 
 * =====================================================================================
 */
void welcome(void);
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  welcome
 *  Description:  One no need to describe.
 * =====================================================================================
 */
void storeWords(char **words, int num, int comds);
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  storeWords
 *  Description:  Storing the words to CommandList.
 * =====================================================================================
 */

void execute(void)
{
    int i = 0        ;
    int input, fd[2] ;
    if(reidcr == REIN){ // if there is a redirection input
        if(np == 0){    // no pipeline
            input = openFile(commandList[1]); // set the input with retured file descriptor. i.e.: $ sort < data.sample, here 
        }else{                                // data.sample as the input of sort.


            input = openFile(commandList[np+1]); // the difference with above is like -> Above: $ sort < data.sample
        }                                                                           //This one: $ wc | sort < data.sample
    }else{
        input = STDIN_FILENO; // if there is no redirection, set the input with stdin.  i.e.: $ ls -l
    }
    for(i = 0; i < np; i++){  // generate child process one by one, except the last command
        pipe(fd);
        spawnProcess(input, fd[PIPE_WRITE], commandList[i]); // we start to generate child process here, and set the first process's input
        close(fd[PIPE_WRITE]);                               // as stdin, and set its output as the **write-end** of the pipe
                                                             // so, the next child process can get its input from the current
                                                             // one's output.

        input = fd[PIPE_READ];  // Before we generate the next child process, we set the input as the
                                // **read-end** of the pipe. Again, the write-end of the pipe is the privious
                                // one's output, so, we can read from the privious one's output.
    }


    if(input != STDIN_FILENO){      
        dup2(input, STDIN_FILENO);  // Prepare for the last command running, read from the privious one's output and
    }                               // write to stdout or write to a file(redirection), here we set the input = fd[PIPE_READ],
                                    // therefore, we can read from the penultimate.

    if(reidcr == REOUT){  // if there is a redirection output
        if(np == 0){
            dup2(makeFile(commandList[1]), STDOUT_FILENO);    // we set the file's name which we want store the output into
        }else{                                                // as our stdout.

            dup2(makeFile(commandList[np+1]), STDOUT_FILENO); // same as above
        }
    }
    if(execok(commandList[i]) != 0){
            perror("PATH");
        }else{
            commandList[i]->words[commandList[i]->num] = NULL;
            execve(path, commandList[i]->words, environ);       // the last command in pipeline must run in original process!!
        }                                                       // if not, the main process of our shell would never get the status
}                                                               // whether all of the child processes terminated appropriately.
int spawnProcess(int input, int output, cp command)
{
    pid_t pid;

    if((pid = fork()) == 0){                  // spawn child process here
        if(input != STDIN_FILENO){
            dup2(input, STDIN_FILENO);        // input as stdin
            close(input);                     // closing original input fd, for avoiding unpredictable error
        }
        if(output != STDOUT_FILENO){
            dup2(output, STDOUT_FILENO);      // output as stdout
            close(output);                    // close original output fd, for avoiding unpredictable error
        }
    
        if(execok(command) != 0){
            perror("PATH");
        }else{
            command->words[command->num] = NULL;    // mark the last element of char* word[MAXWORDS] as NULL, it conveys OS that it has reach the EOF
            execve(path, command->words, environ);  // Here we go
        }
    }
    return pid;
}
int getLine(void)
{
    int n      ; //result of read system call
    int len    ; //length of input line
    int gotnb  ; //non-zero when non-whitespace was seen
    char c     ; //current input character
    char *msg  ; //error message
    int isterm ; //non-zero if input is from a terminal
    
    isterm = isatty(STDIN_FILENO);
    for(;;){
        if(isterm){
            write(STDOUT_FILENO, "# ", 2);
        }
        gotnb = len = 0;
        for(;;){
            
            n = read(STDIN_FILENO, &c, 1);
            if(n == 0){
                return 0;                     //test wether read arrive at EOF
            }
            if(n == -1){
                perror("Error reading command line");
                exit(EXIT_FAILURE);
            }
            
            if(!isterm){
                write(STDOUT_FILENO, &c, 1); //if input not from a terminal
            }
            if(c == '\n'){                   //end of line?
                break;
            }
            if(len >= MAXLINELEN){
                len++;
                continue;
            }
            if(c != ' ' && c != 't'){
                gotnb = 1;
            }
            line[len++] = c;                 //save the input not whitespace
        }
        if(len >= MAXLINELEN){
            msg = "Input line is too long.n";
            write(STDERR_FILENO, msg, strlen(msg));
            continue;
        }
        if(gotnb == 0){
            continue;
        }
        line[len] = '\0';
        np = findPipe(line);
        reidcr = findRedirection(line);
        return 1;
    }
}
int findPipe(char *line)
{
    int i = 0;
    int count = 0;
    while(*(line+i) != '\0'){
        if(*(line+i) == '|')
            count++;
        i++;
    }
    return count;
}
int findRedirection(char *line)
{
    int i = 0;
    while(*(line+i) != '\0'){
        if(*(line+i) == '<'){
            return 4;
        }
        if(*(line+i) == '>'){
            return 5;
        }

        i++;
    }

    return 0;
}
void welcome(void)
{
    puts("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
    puts("+   This is a homework shell, I haven't cover the whole  +");
    puts("+   function set that most common system shell does yet. +");
    puts("+                                                        +");
    puts("+   The pipeline and redirection works well, It supports +");
    puts("+   multi-pipeline, both input and output redirection.   +");
    puts("+                                                        +");
    puts("+   i.e.: ls | sort -r | head      //multi-pipeline      +");
    puts("+         ls -l > ls.out           //output redirection  +");
    puts("+         sort -r < ls.out         //input redirection   +");
    puts("+                                                        +");
    puts("+   You can also combine them together.                  +");
    puts("+                                                        +");
    puts("+   i.e.: ls | sort -r | head > example.out              +");
    puts("+         wc | sort -r < example.in                      +");
    puts("+                                                        +");
    puts("+   I missed out lots of details with error catches, if  +");
    puts("+   any bugs or crashes occur, please try re-opening it. +");
    puts("+                                                        +");
    puts("+   Prress CTRL+C to exit!!                              +");
    puts("+                                                        +");
    puts("+                     ZHANG ZHENGWEN                     +");
    puts("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
}
int parse(void)
{
    char *p     ; //pointer to current word
    char *msg   ; //error message
    int noc = 0 ; //number of commands
    nwds = 0    ;

    p = strtok(line, " \t");

    while(p != NULL){
        if(nwds == NWORDS){
            msg = "*** ERROR: Too many words.\n";
            write(STDERR_FILENO, msg, strlen(msg));
            return 0;
        }
        if(strlen(p) >= MAXWORDLEN){
            msg = "*** ERROR: Word too long.\n";
            write(STDERR_FILENO, msg, strlen(msg));
            return 0;
        }
            if(strcmp(p, "|") == 0 || strcmp(p, ">") == 0 || strcmp(p, "<") == 0){
            storeWords(words, nwds, noc);
            noc++;
            bzero(words, sizeof(words));
            nwds = 0;
            p = strtok(NULL, " \t");
            continue;
        }else{
            words[nwds++] = p; //save words
        }
            p = strtok(NULL, " \t");
    }
    if(noc > 0){
        storeWords(words, nwds, noc);
    }else{
        storeWords(words, nwds, 0);
    }

    return 1;
}
void storeWords(char **words, int num, int comds)
{
    cp commandPrt = (cp)malloc(sizeof(struct command)); // a pointer for one word
        for(int i = 0; i < num; i++){
                commandPrt->words[i] = words[i];        // copy the word to its relative struct
            }
            commandPrt->num = nwds;
            commandList[comds] = commandPrt;            // store the word to commandlist
}
int execok(cp command)
{
    char *p;
    char *pathenv;

    if(strchr(command->words[0], '/') != NULL){
        strcpy(path, command->words[0]);
        return access(path, X_OK);
    }

    pathenv = strdup(getenv("PATH"));                   //get copy of PATH value
    p = strtok(pathenv, ":");
    while(p != NULL){
        strcpy(path, p);
        strcat(path, "/");
        strcat(path, command->words[0]);
        if(access(path, X_OK) == 0){
            free(pathenv);
            return 0;
        }
        p = strtok(NULL, ":");
    }
    free(pathenv);
    return -1;
}
int makeFile(cp command)
{
    int fd;
    fd = open(command->words[0], O_RDWR|O_CREAT|O_TRUNC, 00777);
    return fd;
}
int openFile(cp command)
{
    int fd;
    fd = open(command->words[0], O_RDONLY, 00777);
    return fd;
}

int main(int argc, char *argv[])
{
    welcome();
    pid_t pid;
    while(getLine()){
        if(!parse()){
            continue;
        }
        if((pid = fork()) == 0){ 
           execute();               // If we don't fork one process out here, The main process of our shell 
        }                           // would be terminated whenever we input a command and execute it.
        wait(&pid);
        for(int i = 0; i < findPipe(line)+1; i++){
            free(commandList[i]);
        }
    
    }
    write(STDOUT_FILENO, "\n", 1);
    return 0;
}
