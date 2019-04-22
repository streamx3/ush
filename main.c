#include <string.h>

#include "ush.h"

ush_cmd u_led_on, u_led_off, u_led_blink, u_blink, u_period,
        u_exit, u_help, u_set_opt,
        cmd_conf_shell, cmd_exit_conf;
ush shell_main, shell_conf;

unsigned long period, millis_last, millis_cur;
int do_blink, state;

void led_on(int argc, char* argv[], void* ush_root){
  do_blink = 1;
  printf("led_on\n");
  fflush(stdout);
}

void led_off(int argc, char* argv[], void* ush_root){
  do_blink = 0;
  printf("led_off\n");
  fflush(stdout);
}

void led_blink(int argc, char* argv[], void* ush_root){
    do_blink = 1;
    printf("do_blink\n");
    fflush(stdout);
}

void handler_period(int argc, char* argv[], void* ush_root){
    period = period == 75 ? 1500 : 75;
    printf("handle_period\n");
    fflush(stdout);
}

void handler_exit(int argc, char* argv[], void* ush_root){
    printf("Leaving main;\n");
    fflush(stdout);
    shell_main.exit = 1;
}

//void handler_operate_opt(int argc, char* argv[]){
//    int set;
//    set = strcmp("set", argv[0]) ? 0 : 1;
//    if(set && argc != 3 && ){
//
//    }
//}

int loop_handler(ush* root){
    int ch;
    if((ch = getchar()) == EOF){
        ch = 0;
        root->exit = 1;
    }
    return ch;
}

void handler_exit_conf(int argc, char* argv[], void* ush_root){
    printf("Leaving conf;\n");
    fflush(stdout);
    shell_conf.exit = 1;
}

void handler_conf_shell(int argc, char* argv[], void* ush_root){
    printf("Configuration shell:\n");
    ush_loop(&shell_conf, loop_handler);
}

void setup(){
    period = 75;
    millis_last = 0;
    do_blink = 0;
    ush_init(&shell_main, printf);
    ush_reg_cmd(&shell_main, &u_exit, "exit", handler_exit, "Exits an application");
    ush_reg_cmd(&shell_main, &u_led_on, "on", led_on, "Turns off LED");
    ush_reg_cmd(&shell_main, &u_led_off, "off", led_off, "Turns on LED");
    ush_reg_cmd(&shell_main, &u_blink, "blink", led_blink, "dummy blinker");
    ush_reg_cmd(&shell_main, &u_period, "period", handler_period,
                "Toggles blink period");

    ush_init(&shell_conf, printf);
    ush_reg_cmd(&shell_conf, &cmd_exit_conf, "exit", handler_exit_conf,
                "Exit the Configuration shell");

    ush_reg_cmd(&shell_main, &cmd_conf_shell, "conf", handler_conf_shell,
                "Configuration shell");
}

int main(){
    setup();
    ush_loop(&shell_main, loop_handler);

my_exit:;
    return 0;
}
