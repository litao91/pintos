#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "devices/shutdown.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/malloc.h"
#include "filesys/filesys.h"

static void syscall_handler (struct intr_frame *);
/*static int get_byte_user(const uint8_t *uaddr);*/
static int get_word_user(const uint8_t *uaddr);
/*static bool put_byte_user(uint8_t *udst, uint8_t byte);*/

static int* get_argv(struct intr_frame* f, int argc);

static void validate_user_addr(uint8_t* addr UNUSED);
typedef void (*sys_call_t) (struct intr_frame*);

static void sys_halt(struct intr_frame*);
static void sys_exit(struct intr_frame*);
static void sys_exec(struct intr_frame*);
static void sys_wait(struct intr_frame*);
static void sys_create(struct intr_frame*);
static void sys_remove(struct intr_frame*);
static void sys_open(struct intr_frame*);
static void sys_filesize(struct intr_frame*);
static void sys_read(struct intr_frame*);
static void sys_write(struct intr_frame*);
static void sys_seek(struct intr_frame*);
static void sys_tell(struct intr_frame*);
static void sys_close(struct intr_frame*);

static void sys_mmap(struct intr_frame*);
static void sys_munmap(struct intr_frame*);

static void sys_chdir(struct intr_frame*);
static void sys_mkdir(struct intr_frame*);
static void sys_readdir(struct intr_frame*);
static void sys_isdir(struct intr_frame*);
static void sys_inumber(struct intr_frame*);



sys_call_t syscall_funcs[SYSCALL_NUM];

void
syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");

  syscall_funcs[SYS_HALT] = sys_halt;
  syscall_funcs[SYS_EXIT] = sys_exit;
  syscall_funcs[SYS_EXEC] = sys_exec;
  syscall_funcs[SYS_WAIT] = sys_wait;
  syscall_funcs[SYS_CREATE] = sys_create;
  syscall_funcs[SYS_REMOVE] = sys_remove;
  syscall_funcs[SYS_OPEN] = sys_open;
  syscall_funcs[SYS_FILESIZE] = sys_filesize;
  syscall_funcs[SYS_READ] = sys_read;
  syscall_funcs[SYS_WRITE] = sys_write;
  syscall_funcs[SYS_SEEK] = sys_seek;
  syscall_funcs[SYS_TELL] = sys_tell;
  syscall_funcs[SYS_CLOSE] = sys_close;

  syscall_funcs[SYS_MMAP] = sys_mmap;

  syscall_funcs[SYS_MUNMAP] = sys_munmap;
  syscall_funcs[SYS_CHDIR] = sys_chdir;
  syscall_funcs[SYS_MKDIR] = sys_mkdir;
  syscall_funcs[SYS_READDIR] = sys_readdir;
  syscall_funcs[SYS_ISDIR] = sys_isdir;
  syscall_funcs[SYS_INUMBER] = sys_inumber;
}


/**
 * f -- the stack frame of the interruption
 */
static void
syscall_handler (struct intr_frame *f){
  printf ("system call!\n");
  int syscall_num = get_word_user(f->esp);
  if(syscall_num < 0 && syscall_num >= SYSCALL_NUM) {
      exit(-1);
  }
  syscall_funcs[syscall_num](f);
  //thread_exit ();
}


static void sys_halt(struct intr_frame* f UNUSED) {
    shutdown_power_off();
}

static void sys_exit(struct intr_frame* f) {
    // assign the first argument
    int *argv = get_argv(f, 1);
    int status = argv[1];
    free(argv);
    exit(status);
}

static void sys_exec(struct intr_frame* f UNUSED) {
}
static void sys_wait(struct intr_frame* f UNUSED) {
}
static void sys_create(struct intr_frame* f UNUSED) {
}
static void sys_remove(struct intr_frame* f UNUSED) {
}
static void sys_open(struct intr_frame* f UNUSED) {
}
static void sys_filesize(struct intr_frame* f UNUSED) {
}
static void sys_read(struct intr_frame* f UNUSED) {
}

static void sys_write(struct intr_frame* f UNUSED) {
    int argc = 3;
    int* argv = get_argv(f, argc);
    if(argv[1] == 0) {
    } else if(argv[1] == 1) { // write to console
        uint8_t* buffer = (uint8_t *) argv[2];
        validate_user_addr(buffer);
        size_t size = argv[3];

        int written = 0;

        // write to console 512 bytes at a time. (at most)
        if(size < 512)
            putbuf((char*) buffer, size);
        else{
            while(size > 512) {
                putbuf((char*)(buffer + written), 512);
                size-=512;
                written += 512;
            }
            putbuf((char*)(buffer+written), size);
            written += size;
        }
        f -> eax = written; //return value
    }
}

static void sys_seek(struct intr_frame* f UNUSED) {
}
static void sys_tell(struct intr_frame* f UNUSED) {
}
static void sys_close(struct intr_frame* f UNUSED) {
}

static void sys_mmap(struct intr_frame* f UNUSED) {
}
static void sys_munmap(struct intr_frame* f UNUSED) {
}

static void sys_chdir(struct intr_frame* f UNUSED) {
}
static void sys_mkdir(struct intr_frame* f UNUSED) {
}
static void sys_readdir(struct intr_frame* f UNUSED) {
}
static void sys_isdir(struct intr_frame* f UNUSED) {
}
static void sys_inumber(struct intr_frame* f UNUSED) {
}


/*******************helpers********************/


/**
 * Wrap every exit
 */
void exit(int status) {
    //TODO set the status
    struct thread* cur = thread_current();
    printf("%s: exit(%d)\n", cur->name, status);
    thread_exit();
}


/**
 * The arguments are pushed into the stack (pointed by esp)
 */
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


/***************Memory accessing **************************/
/**
 * Reads a byte at user virtual address uaddr.
 * uaddr must be below PHYS_BASE.
 * Return the byte value if successful
 */
/*static int get_byte_user(const uint8_t *uaddr) {*/
    /*int result;*/
    /*if((void*) uaddr >= PHYS_BASE) {*/
        /*exit(-1);*/
    /*}*/
    /*asm ("movl $1f, %0; movzbl %1, %0; 1:"*/
                       /*: "=&a" (result) : "m" (*uaddr));*/
    /*return result;*/
/*}*/

static int get_word_user(const uint8_t *uaddr) {
    int result;
    if((void*)uaddr >= PHYS_BASE) {
        exit(-1);
    }
    asm ("movl $1f, %0;  movl %1, %0; 1:"
            : "=&a" (result): "m" (*uaddr));
    return result;
}

static void validate_user_addr(uint8_t* addr UNUSED) {
    //TODO validate the address
}

/*static bool put_byte_user(uint8_t *udst, uint8_t byte) {*/
    /*int error_code;*/
    /*asm("movl $1f, %0; movb %b2, %1; 1:"*/
            /*: "=%a" (error_code), "=m" (*udst) : "q" (byte));*/
    /*return error_code != -1;*/
/*}*/

