#include <string.h>

#include "ush.h"

volatile int main_exit;

void nz_shell_init(nz_shell* root){
    if(NULL == root){
        goto nz_shell_init_exit;
    }
    memset(root->buf, 0, NZ_SHELL_MAX_COMMANDS_LEN);
    memset(root->cmds, 0, NZ_SHELL_MAX_CMDS * sizeof (void*));
    root->cur_units = 0;
    root->cur_bufsz = 0;
nz_shell_init_exit:;
}

void nz_shell_reg_cmd(nz_shell* root, nz_sh_cmd* unit){
    if(NULL == root || NULL == unit){
        goto nz_shell_reg_cmd_exit;
    }
    if(!(root->cur_units < NZ_SHELL_MAX_CMDS)){
        goto nz_shell_reg_cmd_exit;
    }
    root->cmds[root->cur_units] = unit;
    root->cur_units++;

nz_shell_reg_cmd_exit:;
}


void nz_cmd_init(nz_sh_cmd* unit, char* cmd, nz_cmd_handler handler){
    if(NULL == unit || NULL == cmd || NULL == handler){
        goto nz_cmd_init_unit_exit;
    }
    strncpy(unit->cmd, cmd, NZ_SHELL_MAX_COMMANDS_LEN);
    unit->handler = handler;
    unit->len = strnlen(unit->cmd, NZ_SHELL_MAX_COMMANDS_LEN);
nz_cmd_init_unit_exit:;
}

int nz_cmd_prepare_args(nz_shell *root, size_t cmd_id){
    int rv;
    char *start;

    rv = 0;
    if(NULL == root || cmd_id >=  NZ_SHELL_MAX_CMDS){
        rv = -1;
        goto nz_cmd_prepare_args_exit;
    }
    root->argc = 0;
    memset(root->argv, 0, sizeof(void*) * NZ_SHELL_MAX_ARGS);

    start = (char*) (root->buf + root->cmds[cmd_id]->len);
nz_cmd_prepare_args_exit:;
    return rv;
}

void nz_cmd_process_byte(nz_shell* root, int data){
    size_t i;
    if(NZ_SHELL_MAX_COMMANDS_LEN == root->cur_bufsz){
        memcpy(root->buf, &(root->buf[1]), NZ_SHELL_MAX_COMMANDS_LEN -1);
        root->cur_bufsz--;
    }
    root->buf[root->cur_bufsz] = (char)(data);
    root->cur_bufsz++;
    for(i = 0; i < NZ_SHELL_MAX_CMDS; ++i){
        if(NULL != root->cmds[i]){
            if(0 == strncmp(root->cmds[i]->cmd,
//                            root->buf,
                           (char*)((void*)(root->buf) + (
                                       root->cur_bufsz - root->cmds[i]->len)
                                   ),
                            root->cmds[i]->len)){
                nz_cmd_prepare_args(root, i);
                root->cmds[i]->handler(root->argc, root->argv);
                memset(root->buf, 0, NZ_SHELL_MAX_COMMANDS_LEN);
                root->cur_bufsz = 0;
            }
        }
    }
}

nz_sh_cmd u_led_on, u_led_off, u_led_blink, u_blink, u_period, u_exit;
nz_shell cmd1;

unsigned long period, millis_last, millis_cur;
int do_blink, state;

void led_on(int argc, char* argv[]){
  do_blink = 1;
  printf("led_on\n");
  fflush(stdout);
}

void led_off(int argc, char* argv[]){
  do_blink = 0;
  printf("led_off\n");
  fflush(stdout);
}

void led_blink(int argc, char* argv[]){
    do_blink = 1;
    printf("do_blink\n");
    fflush(stdout);
}

void handler_period(int argc, char* argv[]){
    period = period == 75 ? 1500 : 75;
    printf("handle_period\n");
    fflush(stdout);
}

void handler_exit(int argc, char* argv[]){
    printf("Leaving...\n");
    fflush(stdout);
    main_exit = 1;
}

void setup() {
  period = 75;
  millis_last = 0;
  do_blink = 0;
  nz_cmd_init(&u_exit, "exit", handler_exit);
  nz_cmd_init(&u_led_on, "on", led_on);
  nz_cmd_init(&u_led_off, "off", led_off);
  nz_cmd_init(&u_blink, "blink", led_blink);
  nz_cmd_init(&u_period, "period", handler_period);
  nz_shell_reg_cmd(&cmd1, &u_led_on);
  nz_shell_reg_cmd(&cmd1, &u_led_off);
  nz_shell_reg_cmd(&cmd1, &u_blink);
  nz_shell_reg_cmd(&cmd1, &u_period);
}

int main() {
    int ch;
    main_exit = 0;

    setup();
    while ((ch=getchar()) != EOF) {
        nz_cmd_process_byte(&cmd1, ch);
//        printf("%c", ch);
        if(main_exit){
            goto my_exit;
        }
    }

my_exit:;

    return 0;
}
