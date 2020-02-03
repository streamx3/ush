#ifndef USH_H
#define USH_H

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

#define USH_MAX_COMMAND_LEN 32
#define USH_MAX_VARNAME_LEN 16
#define USH_MAX_CMDS 64
#define USH_MAX_VARS 64
#define USH_MAX_ARGS 16
#define USH_LINE_END "\r\n"

typedef void (*ush_cmd_handler)(int argc, char* argv[], void* ush_root);
typedef int (*ush_print_ptr)(const char* str, ...);
typedef enum {/*U32_T,*/ S32_T, FLOAT_T, STR_T }USH_VAR_T;

typedef struct{
    ush_cmd_handler handler;
    char            cmd[USH_MAX_COMMAND_LEN];
    char*           help;
    size_t          len;
}ush_cmd;

typedef union{
    uint32_t    d_u32;
    int32_t     d_s32;
    float       d_float;
    char*       d_str;
}ush_var_data;

typedef struct{
    ush_var_data    data;
    char            name[USH_MAX_VARNAME_LEN];
    size_t          name_len;
    USH_VAR_T       type;
}ush_var;

typedef struct{
    ush_print_ptr   print_handler;
    char            *argv[USH_MAX_ARGS];
    char            buf[USH_MAX_COMMAND_LEN];
    ush_cmd*        cmds[USH_MAX_CMDS];
    ush_var*        vars[USH_MAX_CMDS];
    char*           prefix;
    size_t          cur_cmds;
    size_t          cur_vars;
    size_t          cur_bufsz;
    int             argc;
    volatile int    exit;
}ush;

typedef int (*ush_loop_handler)(ush* root);

void ush_init(ush* root, ush_print_ptr print_func, const char* prefix);
void ush_reg_cmd(ush* root, ush_cmd* cmd, char* str,
                 ush_cmd_handler handler, char* help);
void ush_cmd_init(ush_cmd* unit, char* cmd, ush_cmd_handler handler,
                  char* help);
int  ush_cmd_prepare_args(ush *root, size_t cmd_id);
void ush_cmd_process_byte(ush* root, int data);
void ush_loop(ush* root, ush_loop_handler fn_get_char);

void ush_reg_var(ush* root, ush_var* var, char* str, USH_VAR_T type);

#endif // USH_H
