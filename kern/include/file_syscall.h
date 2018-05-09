/**
 * Defintion of file table and its handlers
 */

#ifndef _FILESYSCALL_H_
#define _FILESYSCALL_H_

#include <vnode.h>
#include <synch.h> 
#include <limits.h>
#include <current.h>
#include <types.h>
#include <proc.h>

/**
 * File handle. Handles open file. Shared between multiple threads.
 */
struct file_handle {
	off_t offset;  
	struct vnode* file; 
	int flag; 
	struct lock* fh_lock;
    volatile int nr_of_refs; 	
};

int 
sys_open(const char *filename, int flags, int* retval);

int 
sys_write(int fd, void* buf, size_t buflen, int* retval); 

int
filetable_init(void); 

int
sys_close(int fd, int* retval);

int
sys_read(int fd, void *buf, size_t buflen, int* retval);

int
sys_lseek(int fd, off_t pos, int whence, int* retval, int* retval2); 


#endif /* _FILESYSCALL_H_ */
