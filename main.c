//
//  main.c
//  shell
//
//  Created by WangYe on 2/18/17.
//  Copyright © 2017 Ye WANG. All rights reserved.
//


#include <stdio.h>
#include <string.h>
#include <mm_malloc.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include "readline/readline.h"
#include "readline/history.h"
#include "command.h"
#include <sys/wait.h>
#include <pwd.h>


#define MAX_SUB_COMMANDS 5
#define MAX_ARGS 10

int fds[MAX_SUB_COMMANDS-1][2];
int orphan = -1;

void changeDirc(char *argv[]){
    if (argv[1] != NULL){
        if (chdir(argv[1]) < 0){
            switch (errno){
                case ENOENT:
                    fprintf(stderr, "%s: no such a file or directory!\n", argv[1]);
                break;
            case ENOTDIR:
                fprintf(stderr, "%s: It is not a directory!\n", argv[1]);
                break;
            case EACCES:
                fprintf(stderr, "%s: Permission Denied!\n", argv[1]);
                break;
            default:
                fprintf(stderr, "%s: Failed to change directory!\n", argv[1]);
        }
        }
    }
    return;
}

void closePipe (int order, int numOfPipe){
    if (order == 0 && numOfPipe == 0){
        return;
    }
    if (order == 0 && numOfPipe > 0){
        for (int i = 0; i<numOfPipe; i++){
            for (int j = 0; j < 2; j++){
                if (i != order || j != 1){
                    close(fds[i][j]);
                }
            }
        }
        close(1);
        dup(fds[order][1]);
    }
    if (order != 0 && order == numOfPipe){
        for (int i = 0; i<numOfPipe; i++){
            for (int j = 0; j < 2; j++){
                if (i != order-1 || j != 0){
                    close(fds[i][j]);
                }
            }
        }
        close(0);
        dup(fds[order-1][0]);
    }
    if (order < numOfPipe && order > 0){
        int i = 0;
        for (; i<numOfPipe; i++){
            int j = 0;
            for (; j < 2; j++){
                if ((i == order && j == 1) || (i == order - 1 && j == 0)){
                    continue;
                }
                close(fds[i][j]);
            }
        }
        close(0);
        dup(fds[order-1][0]);
        close(1);
        dup(fds[order][1]);
    }
}

void ProcessCommand(struct Command *command) {
    int ret[MAX_SUB_COMMANDS] = {-1};
    int fdIn;
    int fdOut;

    if (command->stdin_redirect != NULL){
        fdIn = open(command->stdin_redirect, O_RDONLY);
        if (fdIn < 0){
            fprintf(stderr, "%s File not found\n", command->stdin_redirect);
            return;
        }
    }
    
    if (command->stdout_redirect != NULL){
        if (command->typeOfRedirect == 0){
            fdOut = open(command->stdout_redirect, O_WRONLY | O_CREAT, 0666);
        }else {
            fdOut = open(command->stdout_redirect, O_WRONLY | O_CREAT | O_APPEND, 0666);
        }
        if (fdOut < 0){
            fprintf(stderr, "%s Cannot create file\n", command->stdin_redirect);
            return;
        }
    }
    
    for (int j = 0; j < command->num_sub_commands - 1; j++) {
        int err = pipe(fds[j]);
        if (err == -1) {
            perror("pipe error\n");
        }
    }
    
    for (int i = 0; i < command->num_sub_commands; i++) {
        ret[i] = fork();
        if (ret[i] < 0) {
            perror("child error");
            return;
        }
        else if (ret[i] == 0) {
            closePipe(i, command->num_sub_commands - 1);
            if (i == command->num_sub_commands - 1 && command->stdout_redirect != NULL) {
                close(1);
                dup2(fdOut, 1);
            }
            if (i == 0 && command->stdin_redirect != NULL) {
                close(0);
                dup2(fdIn, 0);
            }
            execvp(command->sub_commands[i].argv[0], command->sub_commands[i].argv);
            fprintf(stderr, "%s: Command not found\n", command->sub_commands[i].line);
            exit(-1);
        }
        else if (ret[command->num_sub_commands-1] > 0) {
            for (int i = 0; i < command->num_sub_commands-1; i++){
                close(fds[i][0]);
                close(fds[i][1]);
            }
            int status;
            int w;
            if (command->background == 1) {
                orphan = ret[command->num_sub_commands-1];
                printf("[%d]\n", ret[command->num_sub_commands-1]);
                return;
            } else {
                w = waitpid(ret[command->num_sub_commands-1], &status, 0);
            }
            if (w == -1) {
                fprintf(stderr, "error\n");
            }
        }
    }
    return;
}

int main(int argc, const char * argv[]) {
    printf("Welcome to WangYe's shell\n");
    while(1){
        char buffer[200];
        getcwd(buffer, sizeof(buffer));
        strcat(buffer, "$ ");
        char *line = readline(buffer);
        add_history(line);
        if (orphan > 0){ // check if there are process in background
            int status;
            int w = waitpid(orphan, &status, WNOHANG);
            if (w != 0){
                printf("[%d] finished\n", orphan);
                orphan = -1;
            }
        }
        if (strlen(line) == 0){
            continue;
        }
        struct Command *command = malloc(sizeof(struct Command));
        initCommand(command);
        command->background = 0;
        ReadCommand(line, command);
        ReadRedirectsAndBackground(command);
        if (strcasecmp(command->sub_commands[0].argv[0], "cd") == 0){
            if (command->sub_commands[0].argv[1] == NULL){
                char *homeDir;
                if ((homeDir = getenv("HOME")) == NULL) {
                    homeDir = getpwuid(getuid())->pw_dir;
                }
                command->sub_commands[0].argv[1] = homeDir;
            }
            changeDirc(command->sub_commands[0].argv);
            continue;
        }
        if (strcasecmp(command->sub_commands[0].argv[0], "exit") == 0){
            printf("Bye!\n");
            exit(0);
        }
        ProcessCommand(command);
        free(command);
        free(line);
    }
}
