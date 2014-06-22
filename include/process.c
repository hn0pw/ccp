
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <asm-generic/socket.h>
#include <sys/socket.h>
#include <unistd.h>
#include "process.h"
#include "helper.h"

volatile int file_loaded = 0; // Volatile tells the compiler not to optimize anything that has to do with the volatile variable.
pthread_mutex_t file_load_mutex = PTHREAD_MUTEX_INITIALIZER;
volatile int running_threads = 0; // Volatile tells the compiler not to optimize anything that has to do with the volatile variable.
pthread_mutex_t running_mutex = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t line_locking_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Start routine for each process
 * @param socket_pointer
 * @return
 */
void *start(void *socket_pointer) {
    socket2 = (int) socket_pointer;
    error=0;
    answerSent=0;
    
    pthread_mutex_lock(&running_mutex);
    running_threads++;
    pthread_mutex_unlock(&running_mutex);
    
    loadFile();
    wait_for_command_from_client(socket2);
    
    pthread_mutex_lock(&running_mutex);
    running_threads--;
    if(running_threads == 0) { // save the file if no more threads are running
        saveFile();
    }
    pthread_mutex_unlock(&running_mutex);
    
    pthread_exit(NULL);
}

/**
 * Opens file once for all threads
 * @return int
 */
int loadFile() {
    
    pthread_mutex_lock(&file_load_mutex);
    if(file_loaded == 1 || access( "textfile.txt", F_OK ) == -1) {
        file_loaded = 1;
        pthread_mutex_unlock(&file_load_mutex);
        return 1;
    }
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int lineNumber = 0;
    fileContents[255];

    fp = fopen("textfile.txt", "r");
    if (fp == NULL) {
        exit(EXIT_FAILURE);
    }
    
    while ((read = getline(&line, &len, fp)) != -1) {
        fileContents[lineNumber] = malloc(read + 2);
        strcpy(fileContents[lineNumber], line);
        strcat(fileContents[lineNumber], ""); /* add the extension */
        lineNumber++;
        if(lineNumber > 244) {
            break;
        }
    }

    if (line){
        free(line);
    }
    
    fclose(fp);
    file_loaded = 1;
    pthread_mutex_unlock(&file_load_mutex);
    return 1;
}

/**
 * Save the file
 * @return int
 */
int saveFile(){
    pthread_mutex_lock(&file_load_mutex);
    if(file_loaded == 0) {
        return 1;
    }

    FILE *fp = fopen("textfile.txt","w");
    if (fp != NULL) {
        int lineNumber = 0;
        
        while(lineNumber < 255) {
            if(fileContents[lineNumber] == 0) {
                break;
            }
            if (strcasecmp(fileContents[lineNumber], "\n") != 0) {
                fprintf (fp, "%s", fileContents[lineNumber]);
            }
            lineNumber++;
        }
        
        fclose(fp);
    }
    
    file_loaded = 0;
    int lineNumber = 0;
    
    while(lineNumber < 255) { // clear text content array
        fileContents[lineNumber] = 0;
        lineNumber++;
    }
    pthread_mutex_unlock(&file_load_mutex);
    return 1;
}

/**
 * Wait for a command from client
 * @param socket
 * @return
 */
int wait_for_command_from_client(int socket) {

    int maxLine = 255;
    char CommandBuffer[1024];
    ssize_t nBytes;
    
    nBytes = recv(socket, CommandBuffer, sizeof (CommandBuffer), 0);
    if (nBytes < 1) { // Socket in error state
        fprintf(stderr, "Socket %d closed or error", socket);
        perror("recv");
        return 0;
    }
    // Command read OK, let's process it!
    char commands_allowed2[] = "INSERTLINES, REPLACELINES, READLINES, DELETELINES, NUMLINES, Quit";
    char commands_allowed3[] = "READLINES, DELETELINES, NUMLINES, Quit";
    char *command;
    int command_start_num = 0;
    int command_num_lines = 0;
    int line_count = 0;
    char *saveptr1, *saveptr2;
    // Problem multiple usage: http://cboard.cprogramming.com/c-programming/132953-strtok-twice-means-problems.html
    // http://linux.die.net/man/3/strtok_r
    char *line = strtok_r(CommandBuffer, "\n", &saveptr1); // Parse each line of command buffer
    while (line != NULL) {
        line_count++;
        //printf("\nLine %i: \"%s\"\n", line_count, line);

        if (line_count == 1) { // command progress it
            char *command_line;
            for (command_line = line;; line = NULL) {
                int command_count = 0;
                char *command_split = strtok_r(command_line, " ", &saveptr2);
                while (command_split != NULL) {
                    command_count++;
                    if (command_count == 1) {
                        command = command_split;
                        command_split = strtok_r(NULL, " ", &saveptr2);
                        continue;
                    }
                    if (command_count == 2) {
                        //strcpy(command_start_num, command_split);
                        //command_start_num = (int) command_split - '0'; // fail my first attemp
                        //http://stackoverflow.com/questions/628761/character-to-integer-in-c
                        //int val2 = (int) command_split - '0'; // fail, 
                        command_start_num = atoi(command_split); // OK with 2 100 02 s->0
                        //int val3 = (int)strtol(command_split, (char **)NULL, 10); // OK with 2 100 02 s->0
                        //int numeral = (int) (command_split - '0'); //fail, 2.try found in the internet..
                        //int valu23e;
                        //int assigned = sscanf(command_split, "%d", &valu23e);  // OK with 2 100 02 s->0
                        command_split = strtok_r(NULL, " ", &saveptr2);
                        continue;
                    }
                    if (command_count == 3) {
                        //strcpy(command_num_lines, command_split);
                        command_num_lines = atoi(command_split);
                        command_split = strtok_r(NULL, " ", &saveptr2);
                        continue;
                    }
                    break;
                }
                break;
            }
            
            fprintf(stderr, "  Command is: %s Start Line: %i Num Lines: %i \n", command, command_start_num, command_num_lines);
            if (strcasestr(commands_allowed2, command) != NULL) { // search for command case insensitiv if its allowed
                //debug("  Command allowed\n");
            } else {
                //debug("  Command not allowed\n");
                break;
            }
            int totalLineReq = command_start_num + command_num_lines;
            if (command_start_num >= maxLine || totalLineReq > maxLine) {
                //debug("  Start / Num lines not allowed\n");
                break;
            }

            if (strcasestr(commands_allowed3, command) != NULL) { // single command
                //debug("  Single line command detected\n");
                if (strcasecmp(command, "NUMLINES") == 0) {
                    command_num_lines = 1;
                }
                while (command_num_lines > 0) {
                    while(pthread_mutex_trylock(&line_locking_mutex[command_start_num]) != 0) {
                        fprintf(stderr, "  Line %i locked, wait..\n", command_start_num);
                        sleep(1);
                    }
                    int response = commandSwitcher(command, command_start_num, command_num_lines, line);
                    pthread_mutex_unlock(&line_locking_mutex[command_start_num]); 
                    command_start_num++;
                    if (command_num_lines > 0) {
                        command_num_lines--;
                    }
                    if (response == 0) {
                        fprintf(stderr, "  Command processing problem\n");
                        sendAnswerToClient("NOK");
                        //break;
                    } else if (response == 1) {
                        printf("  Command OK\n");
                    } else if (response == 2) {
                        printf("  Command and exit\n");
                        sendAnswerToClient("OK");
                        break;
                    } else if (response == 3) {
                        printf("  Command Quit\n");
                        sendAnswerToClient("OK");
                        break;
                    }
                }
                break;
            }

        } else {

            int lineLen = sizeof (line) / sizeof (*line);
            if (lineLen > 253) { // \n need 2 too
                //debug("  Line should not be longer than 255 characters\n");
                sendAnswerToClient("Line should not be longer than 255 characters");
                break;
            }
            if (command_num_lines < 1) {
                //debug("  command_num_lines is now 0 exiting\n");
                sendAnswerToClient("OK");
                break;
            }
            
            while(pthread_mutex_trylock(&line_locking_mutex[command_start_num]) != 0) {
                fprintf(stderr, "  Line %i locked, wait..\n", command_start_num);
                sleep(1);
            }
            int response = commandSwitcher(command, command_start_num, command_num_lines, line);
            pthread_mutex_unlock(&line_locking_mutex[command_start_num]);
            
            command_start_num++;
            if (command_num_lines > 0) {
                command_num_lines--;
            }
            if (response == 0) {
                //debug("  Command processing problem\n");
                error = 1;
            } else if (response == 1) {
                //debug("  Command OK\n");
            } else if (response == 2) {
                //debug("  Command and exit\n");
                sendAnswerToClient("OK");
                break;
            } else if (response == 3) {
                //debug("  Command Quit\n");
                sendAnswerToClient("OK");
                break;
            }
        }
        line = strtok_r(NULL, "\n", &saveptr1);
    }

    if(answerSent == 0 && error == 0) {
        sendAnswerToClient("OK");
    }
    close(socket);
}

/**
 * Count lines in file
 * @return int
 */
int getTotalLinesInFile(){
    int linesWithContent = 0;
    while(fileContents[linesWithContent] != 0) {
        linesWithContent++;
    }
    return linesWithContent;
}

/**
 * Check if a line exist
 * @param line line number
 * @return 1 if exist else 0
 */
int lineExist(int line) {
    if (fileContents[line] == 0) {
        return 0;
    }
    if (strcasecmp(fileContents[line], "") == 0) {
        return 0;
    }
    return 1;
}

/**
 * Write a line
 * @param line line contents
 * @param lineNumber number of the line
 * @param addLineBreak 2: add 0: no
 * @return 
 */
int writeLine(char *line, int lineNumber, int addLineBreak) {
    // http://www.thegeekstuff.com/2011/12/c-arrays/
    //char* saveLine;
    //saveLine = malloc(strlen(line)+2); /* make space for the new string (should check the return value ...) */
    //strcpy(saveLine, line); /* copy name into the new var */
    //strcat(saveLine, "\n"); /* add the extension */

    fileContents[lineNumber] = malloc(strlen(line) + addLineBreak);
    strcpy(fileContents[lineNumber], line); /* copy name into the new var */
    if(addLineBreak == 2) {
        strcat(fileContents[lineNumber], "\n"); /* add the extension */
    }
    //fileContents[lineNumber] = saveLine;
    return 1;
}

/**
 * Replace a line if line exist
 * @param line line content
 * @param replaceLine line number
 * @return 1 for ok, 0 for nok
 */
int replaceLine(char *line, int replaceLine) {
    if (lineExist(replaceLine) == 1) {
        return writeLine(line, replaceLine, 2);
    }
    //debug("   Line not exist, can't replace");
    return 0;
}

int deleteLineAndMoveOthers(int lineNumber) {
    int nextLine = lineNumber + 1;
    neext:
    if(getTotalLinesInFile() == nextLine || fileContents[lineNumber] == 0) { // this is last line
        //char *nix;
        fileContents[lineNumber] = 0;
        return 1;
    }
    
    while(pthread_mutex_trylock(&line_locking_mutex[nextLine]) != 0) {
        fprintf(stderr, "  Line %i locked, wait..\n", nextLine);
        sleep(1);
    }
    
    char *nextLineContent = fileContents[nextLine];
    writeLine(nextLineContent, lineNumber, 0);
    pthread_mutex_unlock(&line_locking_mutex[nextLine]);
    lineNumber++;
    nextLine++;
    if(nextLine > 244) {
        return 1;
    }
    goto neext;
    return 1;
}

/**
 * Insert a line if line not already exist
 * @param line line content
 * @param insertOnLine line number
 * @return 1 for ok, 0 for nok
 */
int insertLine(char *line, int insertOnLine) {
    int nextLine = insertOnLine + 1;
    int addLineBreak = 2;
    nextInsert:
    if (lineExist(insertOnLine) == 0) {
        return writeLine(line, insertOnLine, 2); // if line empty if can be inserted
    }
    if(nextLine > 244) {
        return 0;
    }
    char *existingLine = fileContents[insertOnLine];
    writeLine(line, insertOnLine, addLineBreak);
    line = existingLine;
    insertOnLine++;
    nextLine++;
    addLineBreak = 0;
    goto nextInsert;
    return 0;
}

/**
 * Switch command received an run it
 * @param command command name (INSERTLINES, REPLACELINES, READLINES, DELETELINES, NUMLINES, Quit)
 * @param start_line the line which will it start
 * @param num_lines the count of lines
 * @param line the line content
 * @return 0:nok 1:ok 2:ok&exit 3:quit command
 */
int commandSwitcher(char *command, int start_line, int num_lines, char *line) {
    // "INSERTLINES, REPLACELINES, READLINES, DELETELINES, NUMLINES, Quit";
    if (strcasecmp(command, "Quit") == 0) {
        return 3;
    }
    if (strcasecmp(command, "INSERTLINES") == 0) {
        if(start_line > getTotalLinesInFile()) {
            sendAnswerToClient("Line number bigger than existing lines, can't insert\n");
            return 0;
        }
        int s = insertLine(line, start_line);
        if(s == 0) {
            sendAnswerToClient("Line exist, can't insert\n");
            error = 1;
        }
        return s;
    }
    if (strcasecmp(command, "REPLACELINES") == 0) {
        int s = replaceLine(line, start_line);
        if(s == 0) {
            sendAnswerToClient("Line not exist, can't replace\n");
            error = 1;
        }
        return s;
    }
    if (strcasecmp(command, "READLINES") == 0) {
        if (lineExist(start_line) == 0) {
            //debug("  Line didn't exist!\n");
            sendAnswerToClient("Line not exist, can't read\n");
            error = 1;
            return 0;
        }
        return sendAnswerToClient(fileContents[start_line]);
    }
    if (strcasecmp(command, "DELETELINES") == 0) {    
        if(start_line >= getTotalLinesInFile()) {
            sendAnswerToClient("Line not exist, can't delete\n");
            return 0;
        }
        return deleteLineAndMoveOthers(start_line);
    }
    if (strcasecmp(command, "NUMLINES") == 0) {
        char rsp[10];
        sprintf(rsp, "%d\n", getTotalLinesInFile());
        sendAnswerToClient(rsp);
        return 1;
    }
    return 0;
}

/**
 * Sends answer back to client
 * @param answer answer text
 * @return 1:ok 0:nok
 */
int sendAnswerToClient(char *answer) {
    answerSent = 1;
    //printf("Send to Client: %s\n", answer);
    int error = 0;
    socklen_t len = sizeof (error);
    int retval = getsockopt (socket2, SOL_SOCKET, SO_ERROR, &error, &len );
    if(retval == 0){
        send(socket2, answer, strlen(answer), 0);
        return 1;
    }
    return 0;
}
