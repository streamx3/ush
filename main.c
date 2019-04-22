#include <string.h>

#include "ush.h"

volatile int main_exit;

ush_cmd u_led_on, u_led_off, u_led_blink, u_blink, u_period,
        u_exit, u_help;
ush shell;

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

void help_single(ush* root, char* name){
    size_t i;

    i = 0;
    while(NULL != root->cmds[i] && NULL != root->cmds[i]->cmd){
        if(0 == strncmp(name, root->cmds[i]->cmd, root->cmds[i]->len)){
            printf("%s %s;\n",
                  name,
                  root->cmds[i]->help ? root->cmds[i]->help : "Usage unknown");
            return;
        }
        ++i;
    }
    printf("Error: command '%s' not found!\n", name);
}

void help_list(ush* root){
    size_t i;
    printf("Help:\n");
    i = 0;
    while (i < USH_MAX_CMDS){
        if(NULL != root->cmds[i] && NULL != root->cmds[i]->cmd){
            printf("%s %s;\n",
                  root->cmds[i]->cmd,
                  root->cmds[i]->help ? root->cmds[i]->help : "Usage unknown");
        }
        ++i;
    }
}

/// Internal function
void handler_help(int argc, char* argv[]){
    char *name;
    size_t i;
    name = NULL;
    if(argc == 2 && NULL != argv[1]){
        name = argv[1];
        help_single(&shell, name);
    }else{
        help_list(&shell);
    }
}

void setup(){
    period = 75;
    millis_last = 0;
    do_blink = 0;
    ush_reg_cmd(&shell, &u_exit, "exit", handler_exit, "Exits an application");
    ush_reg_cmd(&shell, &u_led_on, "on", led_on, "Turns off LED");
    ush_reg_cmd(&shell, &u_led_off, "off", led_off, "Turns on LED");
    ush_reg_cmd(&shell, &u_blink, "blink", led_blink, "dummy blinker");
    ush_reg_cmd(&shell, &u_period, "period", handler_period,
                "Toggles blink period");
    ush_reg_cmd(&shell, &u_help, "help", handler_help,
                "[name] Prints this help or shecific command if given");
}

int main(){
    int ch;
    main_exit = 0;

    setup();
    while ((ch=getchar()) != EOF){
        ush_cmd_process_byte(&shell, ch);
        if(main_exit){
            goto my_exit;
        }
    }

my_exit:;
    return 0;
}
