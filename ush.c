#include <string.h>

#include "ush.h"

static ush_cmd cmd_help;

static void help_single(ush* root, char* name){
    size_t i;

    i = 0;
    while(NULL != root->cmds[i] && NULL != root->cmds[i]->cmd){
        if(0 == strncmp(name, root->cmds[i]->cmd, root->cmds[i]->len)){
            if(root->print_handler){
                root->print_handler("%s %s;\n",
                                    name,
                                    root->cmds[i]->help ?
                                        root->cmds[i]->help : "Usage unknown");
            }
            return;
        }
        ++i;
    }
    if(root->print_handler){
        root->print_handler("Error: command '%s' not found!\n", name);
    }
}

static void help_list(ush* root){
    size_t i;
    printf("Help:\n");
    i = 0;
    while (i < USH_MAX_CMDS){
        if(NULL != root->cmds[i] && NULL != root->cmds[i]->cmd){
            root->print_handler("%s %s;\n",
                  root->cmds[i]->cmd,
                  root->cmds[i]->help ? root->cmds[i]->help : "Usage unknown");
        }
        ++i;
    }
}

static void handler_help(int argc, char* argv[], void* ush_root){
    ush* root;
    char *name;
    size_t i;
    name = NULL;
    if(NULL != ush_root){
        root = ush_root;
    }
    if(argc == 2 && NULL != argv[1]){
        name = argv[1];
        help_single(root, name);
    }else{
        help_list(root);
    }
}

void ush_init(ush* root, ush_print_ptr* print_func){
    if(NULL == root){
        goto ush_init_exit;
    }
    memset(root->buf, 0, USH_MAX_COMMAND_LEN);
    memset(root->cmds, 0, USH_MAX_CMDS * sizeof (void*));
    root->cur_cmds  = 0;
    root->cur_bufsz = 0;
    root->exit      = 0;
    root->print_handler = print_func;
    ush_reg_cmd(root, &cmd_help, "help", handler_help,
                "[name] Prints this help or shecific command if given");
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
    int found;

    found = 0;
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
                    found = 1;
                    break;
                }
            }
        }
        if(found){
            ush_cmd_prepare_args(root, i);
            root->cmds[i]->handler(root->argc, root->argv, root);
        }else{
            if(root->print_handler){
                root->print_handler("'%s' not found;\n", root->buf);
            }
        }
        memset(root->buf, 0, USH_MAX_COMMAND_LEN);
        root->cur_bufsz = 0;
    }
}

void ush_loop(ush* root, ush_loop_handler fn_get_char){
    int ch, rv;
    rv = 0;
    do{
        ch = fn_get_char(root);
        if(0 != ch){
            ush_cmd_process_byte(root, ch);
        }
    }while(0 == root->exit);
}
