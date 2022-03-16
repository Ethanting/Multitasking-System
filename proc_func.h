/////////////////////////////// proc function head file ///////////////////////////////
#ifndef PROC_FUNC_H
#define PROC_FUNC_H
int kexit(int value);
int kwakeup(int event);
int ksleep(int event);
int kfork();

//////////////////////////////////////////////////////////////////////////////////////////
//
int do_kfork();
int do_switch();
int do_exit();
int do_kill();
int do_ps();
int do_mount();
int proc_menu();

#endif //PROC_FUNC_H

