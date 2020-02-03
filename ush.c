#include <string.h>

#include "ush.h"

void ush_handle_var(ush* root, ush_var* var);

static ush_cmd cmd_help;
static ush_cmd cmd_vars;

static void print_var(ush* root, ush_var* var){
    char print_spec[] = "%s = %u%s";
    switch(var->type) {
    // TODO implement me
//            case U32_T:
//                print_spec[6] = 'u';
//            break;
        case S32_T:
            print_spec[6] = 'd';
        break;
        case FLOAT_T:
            print_spec[6] = 'f';
        break;
        case STR_T:
            print_spec[6] = 's';
        break;
    }
    root->print_handler(print_spec, var->name, var->data, USH_LINE_END);
}

static void help_single(ush* root, char* name){
    size_t i;

    i = 0;
    while(NULL != root->cmds[i] && '\0' != root->cmds[i]->cmd[0]){
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
        root->print_handler("Error: command '%s' not found!"USH_LINE_END, name);
    }
}

static void help_list(ush* root){
    size_t i;
    printf("Help:"USH_LINE_END);
    i = 0;
    while (i < USH_MAX_CMDS){
        if(NULL != root->cmds[i] && '\0' != root->cmds[i]->cmd[0]){
            root->print_handler("%s %s;"USH_LINE_END,
                  root->cmds[i]->cmd,
                  root->cmds[i]->help ? root->cmds[i]->help : "Usage unknown");
        }
        ++i;
    }
}

static void handler_help(int argc, char* argv[], void* ush_root){
    if(NULL == ush_root){
        goto handler_help_exit;
    }
    if(argc == 2 && NULL != argv[1]){
        help_single(ush_root, argv[1]);
    }else{
        help_list(ush_root);
    }
handler_help_exit:;
}

static void handler_vars(int argc, char* argv[], void* ush_root){
    ush* root;
    size_t i;
    root = ush_root;
    i = 0;
    while (i < USH_MAX_VARS){
        if(NULL != root->vars[i]){
            print_var(root, root->vars[i]);
        }
        ++i;
    }
}

void ush_init(ush* root, ush_print_ptr print_func, const char* prefix){
    if(NULL == root){
        goto ush_init_exit;
    }
    memset(root->buf, 0, USH_MAX_COMMAND_LEN);
    memset(root->cmds, 0, USH_MAX_CMDS * sizeof (void*));
    memset(root->vars, 0, USH_MAX_CMDS * sizeof (void*));
    root->cur_cmds  = 0;
    root->cur_vars  = 0;
    root->cur_bufsz = 0;
    root->exit      = 0;
    root->prefix    = prefix;
    root->print_handler = print_func;
    ush_reg_cmd(root, &cmd_help, "help", handler_help,
                "[name] Prints this help or shecific command if given");
    ush_reg_cmd(root, &cmd_vars, "vars", handler_vars,
                "Prints existing variables");
    static ush_cmd cmd_env;
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
                                root->cmds[i]->len)){
                    found = 1;
                    break;
                }
            }
        }
        if(found){
            ush_cmd_prepare_args(root, i);
            root->cmds[i]->handler(root->argc, root->argv, root);
            goto ush_cmd_process_byte_cleanup;
        }
        for(i = 0; i < root->cur_vars; ++i){
            if(NULL != root->vars[i]){
                if(0 == strncmp(root->vars[i]->name,
                                root->buf,
                                root->vars[i]->name_len)){
                    found = 1;
                    ush_cmd_prepare_args(root, i);
                    ush_handle_var(root, root->vars[i]);
                    break;
                }
            }
        }
        if(!found){
            if(root->print_handler){
                root->print_handler("'%s' not found;"USH_LINE_END, root->buf);
            }
        }
ush_cmd_process_byte_cleanup:;
        memset(root->buf, 0, USH_MAX_COMMAND_LEN);
        root->cur_bufsz = 0;
        if(NULL != root->prefix && !root->exit){
            root->print_handler(root->prefix);
        }
    }
}

void ush_handle_var(ush* root, ush_var* var){

    if(1 == root->argc){
        print_var(root, var);
    }else if(2 == root->argc){
        if(S32_T == var->type){
             var->data.d_s32 = atoi(root->argv[1]);
        }else if(FLOAT_T == var->type){
            var->data.d_float = atof(root->argv[1]);
       }else if(STR_T == var->type){
            // TODO IMPLEMENT ME!
        }
    }else{
        root->print_handler("Invalid input!"USH_LINE_END);
    }
ush_handle_var_exit:;
}

void ush_reg_var(ush* root, ush_var* var, char* str, USH_VAR_T type){
    if(NULL == root || NULL == var || NULL == str){
        goto ush_reg_cmd_exit;
    }
    if(!(root->cur_cmds < USH_MAX_CMDS)){
        goto ush_reg_cmd_exit;
    }
    var->type = type;
    if(S32_T == type){
        var->data.d_s32 = 0;
    }else if(FLOAT_T == type){
        var->data.d_float = 0.0;
    }else if(STR_T == type){
        //TODO implement me
    }

    strncpy(var->name, str, USH_MAX_VARNAME_LEN);
    var->name_len = strnlen(var->name, USH_MAX_VARNAME_LEN);
    root->vars[root->cur_vars] = var;
    root->cur_vars++;
ush_reg_cmd_exit:;
}

void ush_loop(ush* root, ush_loop_handler fn_get_char){
    int ch;
    if(root->prefix){
        root->print_handler(root->prefix);
    }
    do{
        ch = fn_get_char(root);
        if(0 != ch){
            ush_cmd_process_byte(root, ch);
        }
    }while(0 == root->exit);
    root->exit = 0;
}
