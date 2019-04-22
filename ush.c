#include <string.h>

#include "ush.h"

static ush_cmd cmd_help;

void ush_init(ush* root){
    if(NULL == root){
        goto ush_init_exit;
    }
    memset(root->buf, 0, USH_MAX_COMMAND_LEN);
    memset(root->cmds, 0, USH_MAX_CMDS * sizeof (void*));
    root->cur_cmds = 0;
    root->cur_bufsz = 0;
ush_init_exit:;
}

void ush_reg_cmd(ush* root, ush_cmd* cmd, char* str,
                 ush_cmd_handler handler, char* help){
    if(NULL == root || NULL == cmd || NULL == str || NULL == handler){
        goto ush_reg_cmd_exit;
    }
    if(!(root->cur_cmds < USH_MAX_CMDS)){
        goto ush_reg_cmd_exit;
    }
    strncpy(cmd->cmd, str, USH_MAX_COMMAND_LEN);
    cmd->handler = handler;
    cmd->len = strnlen(cmd->cmd, USH_MAX_COMMAND_LEN);
    cmd->help = help;

    root->cmds[root->cur_cmds] = cmd;
    root->cur_cmds++;

ush_reg_cmd_exit:;
}

int ush_cmd_prepare_args(ush *root, size_t cmd_id){
    int rv, was_backslash, was_null;
    size_t i;

    rv = 0;
    was_null = 0;
    was_backslash = 0;
    if(NULL == root || cmd_id >=  USH_MAX_CMDS){
        rv = -1;
        goto ush_cmd_prepare_args_exit;
    }
    root->argc = 1;
    memset(root->argv, 0, sizeof(void*) * USH_MAX_ARGS);

    root->argv[0] = root->buf;
    for (i = 0; i < root->cur_bufsz; ++i){
        if(' ' == root->buf[i] && 0 == was_backslash){
            root->buf[i] = '\0';
            was_null = 1;
        }else{
            if(was_null){
                root->argv[root->argc] = root->buf + i;
                root->argc++;
            }
            was_null = 0;
        }
        was_backslash = root->buf[i] == '\\' ? 1 : 0;
    }
ush_cmd_prepare_args_exit:;
    return rv;
}

void ush_cmd_process_byte(ush* root, int data){
    size_t i;
    if(USH_MAX_COMMAND_LEN == root->cur_bufsz){
        memcpy(root->buf, &(root->buf[1]), USH_MAX_COMMAND_LEN -1);
        root->cur_bufsz--;
    }
    if((int)'\n' != data){
        root->buf[root->cur_bufsz] = (char)(data);
        root->cur_bufsz++;
    }else{
        for(i = 0; i < root->cur_cmds; ++i){
            if(NULL != root->cmds[i]){
                if(0 == strncmp(root->cmds[i]->cmd,
                                root->buf,
                                //((root->buf) + (root->cur_bufsz - root->cmds[i]->len) -1),
                                root->cmds[i]->len)){
                    ush_cmd_prepare_args(root, i);
                    root->cmds[i]->handler(root->argc, root->argv);
                    memset(root->buf, 0, USH_MAX_COMMAND_LEN);
                    root->cur_bufsz = 0;
                    break;
                }
            }
        }
    }
}
