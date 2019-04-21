#ifndef USH_H
#define USH_H

#include <stdio.h>
#include <stddef.h>

#define USH_MAX_COMMAND_LEN 32
#define USH_MAX_CMDS 64
#define USH_MAX_ARGS 16

typedef void (*ush_cmd_handler)(int argc, char* argv[]);

typedef struct{
    ush_cmd_handler  handler;
    char            cmd[USH_MAX_COMMAND_LEN];
    char*           help;
    size_t          len;
}ush_cmd;

typedef struct{
    char            *argv[USH_MAX_ARGS];
    char            buf[USH_MAX_COMMAND_LEN];
    ush_cmd*        cmds[USH_MAX_CMDS];
    size_t          cur_cmds;
    size_t          cur_bufsz;
    int             argc;
}ush;

void ush_init(ush* root);
void ush_reg_cmd(ush* root, ush_cmd* unit);
void ush_cmd_init(ush_cmd* unit, char* cmd, ush_cmd_handler handler, char* help);
int  ush_cmd_prepare_args(ush *root, size_t cmd_id);
void ush_cmd_process_byte(ush* root, int data);

#endif // USH_H
