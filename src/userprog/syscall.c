#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) {
    printf ("system call!\n");
    void* esp = f->esp;
    int syscall_num = *(int *) esp;
    printf("System call num: %d\n", syscall_num);

    if(syscall_num == 9) {
        printf("arg1: %d\n", *((int *) esp + 1));
        printf("arg2: %d\n", *((int *) esp + 2));
        printf("arg3: %d\n", *((int *) esp + 3));
    }
    thread_exit ();
}
