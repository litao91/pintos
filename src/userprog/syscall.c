#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/malloc.h"

static void syscall_handler (struct intr_frame *);
static int get_byte_user(const uint8_t *uaddr);
static int get_word_user(const uint8_t *uaddr);
static bool put_user(uint8_t *udst, uint8_t byte);

static void sys_exit(struct intr_frame*);
typedef void (*sys_call_t) (struct intr_frame*);

sys_call_t syscall_funcs[SYSCALL_NUM];

void
syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");

  syscall_funcs[SYS_EXIT] = sys_exit;

  syscall_argc[SYS_EXIT] = 1;
}


static void
syscall_handler (struct intr_frame *f UNUSED)
{
  printf ("system call!\n");
  thread_exit ();
}
static int* get_argv(struct intr_frame* f, int argc) {
    int* argv = (int *) malloc(argc);

    int i;
    for(i = 0; i <= argc; i++) {
        argv[i] = get_word_user(f->esp);
        if(argv[i] == -1) {
            exit(-1);
        }
    }
    return argv;
}

static void sys_exit(struct intr_frame* f) {
    // assign the first argument
    int *argv = get_argv(f, 1);
    int status = argv[1];
    free(argv);
    exit(status);
}


void exit(int status) {
    //TODO set the status
    struct thread* cur = thread_current();
    printf("%s: exit(%d)\n", cur->name, status);
    thread_exit();
}



/**
 * Reads a byte at user virtual address uaddr.
 * uaddr must be below PHYS_BASE.
 * Return the byte value if successful
 */
static int get_byte_user(const uint8_t *uaddr) {
    int result;
    if((void*) uaddr >= PHYS_BASE) {
        exit(-1);
    }
    asm ("movl $1f, %0; movzbl %1, %0; 1:"
                       : "=&a" (result) : "m" (*uaddr));
    return result;
}

static int get_word_user(const uint8_t *uaddr) {
    int result;
    if((void*)uaddr >= PHYS_BASE) {
        exit(-1);
    }
    asm ("movl $1f, %0;  movl %1, %0; 1:"
            : "=&a" (result): "m" (*uaddr));
    return result;
}

static bool put_user(uint8_t *udst, uint8_t byte) {
    int error_code;
    asm("movl $1f, %0; movb %b2, %1; 1:"
            : "=%a" (error_code), "=m" (*udst) : "q" (byte));
    return error_code != -1;
}


