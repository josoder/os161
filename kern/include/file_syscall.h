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
       	volatile int nr_of_threads; 	
};

int 
sys_open(const char *filename, int flags);

int 
sys_write(int fd, void* buf, size_t buflen, int* retval); 

int
filetable_init(void); 

#endif /* _FILESYSCALL_H_ */
