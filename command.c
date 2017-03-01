//
// Created by WangYe on 2/24/17.
//

#include "command.h"
#include <stdio.h>
#include <string.h>
#include <mm_malloc.h>

void initCommand(struct Command *command){
    command->typeOfRedirect = 0;
    command->num_sub_commands = 0;
    command->background = 0;
    command->stdin_redirect = NULL;
    command->stdout_redirect = NULL;
}

void ReadSubCommand(char *in, char **argv, int size){
    char *p;
    char *delim = " ";
    p = strtok(in, delim); // split the string with " "
    int i = 0;
    for(i = 0; i<size-1; i++){
        argv[i] = strdup(p);  // duplicate every parts of the string
        p = strtok(NULL, delim);
        if(p == NULL) break;
    }
    if(argv[i] != NULL){
        argv[i+1] = NULL;
    }
}

void ReadCommand(char *line, struct Command *command){
    char *p;
    char *delim = "|";
    p = strtok(line, delim);// split the string with "|"
    int i = 0;
    for(i=0; i<MAX_SUB_COMMANDS; i++){
        command->sub_commands[i].line = malloc(sizeof(p));
//        if(p[strlen(p)-1] == '\n'){
//            p[strlen(p)-1] = '\0';
//        }
        command->sub_commands[i].line = strdup(p); // duplicate every part of the string
        command->num_sub_commands = i+1; // record the numer of sub-command
        p = strtok(NULL, delim);
        if(p == NULL) break;
    }

    for(i=0; i< command->num_sub_commands; i++){
        char *in = strdup(command->sub_commands[i].line);
        ReadSubCommand(in, command->sub_commands[i].argv, MAX_ARGS); // split the sub-command
    }
}

void ReadRedirectsAndBackground(struct Command *command){
    int i = command -> num_sub_commands - 1;
    int numOfOut = 0;
    int numOfIn = 0;

    int lengthOfLastCommand = 0;
    while(command->sub_commands[i].argv[lengthOfLastCommand] != NULL){
        lengthOfLastCommand++;
    }

    int j = lengthOfLastCommand-1;
    while (j >= 0) {
        if (strncmp(command->sub_commands[i].argv[j], ">", 1) == 0 && command->sub_commands[i].argv[j+1] != NULL){
            command->stdout_redirect = malloc(sizeof(command->sub_commands[i].argv[j+1]));
            command->stdout_redirect = command->sub_commands[i].argv[j+1];
            command->typeOfRedirect = 0;
            numOfOut = j;
            break;
        }
        j--;
    }

    j = lengthOfLastCommand-1;
    while (j >= 0) {
        if (strncmp(command->sub_commands[i].argv[j], ">>", 2) == 0 && command->sub_commands[i].argv[j+1] != NULL){
            command->stdout_redirect = malloc(sizeof(command->sub_commands[i].argv[j+1]));
            command->stdout_redirect = command->sub_commands[i].argv[j+1];
            command->typeOfRedirect = 1;
            numOfOut = j;
            break;
        }
        j--;
    }

    j = lengthOfLastCommand-1;
    while (j >= 0) {
        if (strncmp(command->sub_commands[i].argv[j], "<", 1) == 0 && command->sub_commands[i].argv[j+1] != NULL){
            command->stdin_redirect = malloc(sizeof(command->sub_commands[i].argv[j+1]));
            command->stdin_redirect = command->sub_commands[i].argv[j+1];
            numOfIn = j;
            break;
        }
        j--;
    }

    if(strncmp(command->sub_commands[i].argv[lengthOfLastCommand-1], "&", 1) == 0){
        command->sub_commands[i].argv[lengthOfLastCommand-1] = NULL;
        command->background = 1;
    }

    if (numOfIn != 0){
        command->sub_commands[i].argv[numOfIn] = NULL;
    }
    if (numOfOut != 0){
        command->sub_commands[i].argv[numOfOut] = NULL;
    }
}

void PrintCommand(struct Command *command){
    int i;
    for(i = 0; i<MAX_SUB_COMMANDS; i++){
        if(command->sub_commands[i].line == NULL) break;
        printf("Commands %d:\n",i);
        print_args(command->sub_commands[i].argv); // print the command
    }
    printf("\nRedirect stdin: %s\n", command->stdin_redirect);
    printf("Redirect stdout: %s\n", command->stdout_redirect);
    printf("Background: %s\n", command->background == 0 ? "No" : "Yes");
    printf("type: %d\n", command->typeOfRedirect);
}

void print_args(char ** argv){
    int i = 0;
    int print = 0;
    while(argv[i] != NULL){
        if(argv[i][strlen(argv[i])-1] == '\n'){
            argv[i][strlen(argv[i])-1] = '\0';
        } // check the last char in the last string
        if (strncmp(argv[i], ">", 1) == 0 || strncmp(argv[i], "<", 1) == 0 || strncmp(argv[i], "&", 1) == 0){
            break;
        }
        printf("argv[%d]: '%s'\n",print++ ,argv[i]);
        i++;
    }
}