#ifndef USH_H
#define USH_H

#include <stdio.h>
#include <stddef.h>

#define NZ_SHELL_MAX_COMMANDS_LEN 32
#define NZ_SHELL_MAX_CMDS 64
#define NZ_SHELL_MAX_ARGS 16

typedef void (*nz_cmd_handler)(int argc, char* argv[]);

typedef struct{
    nz_cmd_handler  handler;
    char            cmd[NZ_SHELL_MAX_COMMANDS_LEN];
    size_t          len;
}nz_sh_cmd;

typedef struct{
    char            *argv[NZ_SHELL_MAX_ARGS];
    char            buf[NZ_SHELL_MAX_COMMANDS_LEN];
    nz_sh_cmd*      cmds[NZ_SHELL_MAX_CMDS];
    size_t          cur_units;
    size_t          cur_bufsz;
    int             argc;
}nz_shell;

#endif // USH_H
