//
// Created by WangYe on 2/24/17.
//

#ifndef SHELL_COMMAND_H
#define SHELL_COMMAND_H
#define MAX_SUB_COMMANDS 5
#define MAX_ARGS 10

struct SubCommand
{
    char *line;
    char *argv[MAX_ARGS];
};

struct Command
{
    int typeOfRedirect; // check the operator used is '>' or '>>'
    char *stdin_redirect;
    char *stdout_redirect;
    int background;
    struct SubCommand sub_commands[MAX_SUB_COMMANDS];
    int num_sub_commands;
};
void initCommand(struct Command *command);
void ReadSubCommand(char *in, char **argv, int size);
void ReadCommand(char *line, struct Command *command);
void ReadRedirectsAndBackground(struct Command *command);
void print_args(char ** argv);
void PrintCommand(struct Command *command);
#endif //SHELL_COMMAND_H
