/**
 * Definition of the file_syscalls. 
 */

#include <types.h>
#include <file_syscall.h> 
#include <vfs.h>
#include <kern/fcntl.h>
#include <current.h>
#include <proc.h>
#include <kern/iovec.h>
#include <uio.h>
#include <kern/errno.h>
#include <copyinout.h>

/**
 * Init the file_table in the current thread.
 * This is only done once since this process will be forked when more threads are created. 
 * fd 0-2 is reserved for the console. 
 * [0] STDIN, read-only 
 * [1] STDOUT, write-only
 * [2] STDERR, write-only
 */
struct file_handle*
create_file_handle(off_t boffset, int flags); 

struct file_handle* 
create_file_handle(off_t boffset, int flags) {
	struct file_handle* fh = kmalloc(sizeof(struct file_handle));
	

	fh->file = (struct vnode*) kmalloc(sizeof(struct vnode)); 
	fh->offset = boffset; 
	fh->flag = flags;	

	fh->fh_lock = lock_create("fh_lock");
	if(fh->fh_lock == NULL) {
		kfree(fh->file); 
		return NULL;
	}
	return fh; 
}

int
filetable_init(void){
	int result; 
	struct vnode *stdin;
       	struct vnode *stdout;
	struct vnode *stderr;
	
	// file_table already initialized
	if(curproc->file_table[0] != NULL) {
		return -1;
	}
	char *stdinname;  
	char *stdoutname;
	char *stderrname;
	stdinname = kstrdup("con:");   
	stdoutname = kstrdup("con:");
       	stderrname = kstrdup("con:"); 	

	// init console
	result = vfs_open(stdinname, O_RDONLY, 0, &stdin); 
	if(result) { 
		return result;
	}

	result = vfs_open(stdoutname , O_WRONLY, 0, &stdout);	
	if(result) {
		return result;
	}

	result = vfs_open(stderrname, O_WRONLY, 0, &stderr);	
	if(result) {
		return result;
	}
	
	struct file_handle* stdinfh; 
	struct file_handle* stdoutfh;
	struct file_handle* stderrfh; 

	stdinfh = create_file_handle(0, O_RDONLY); 
	stdoutfh = create_file_handle(0, O_WRONLY);
	stderrfh = create_file_handle(0, O_WRONLY);   

	stdinfh->file = stdin; 
	stdoutfh->file = stdout; 
	stderrfh->file = stderr;
	
	for(int i=0; i<3; i++){
		curproc->file_table[i] = (struct file_handle*)kmalloc(sizeof(struct file_handle)); 
	}

	curproc->file_table[0] = stdinfh; 
	curproc->file_table[1] = stdoutfh;
	curproc->file_table[2] = stderrfh;
	return 0; 
	
}

int 
sys_open(const char *userpfilename, int flags, int* retval) { 	
	int res;
	int how; 

	// R, W, RW? 
	how = flags & O_ACCMODE; 
	
	*retval = -1; 


	char *filename=kmalloc(strlen(userpfilename));
	int result; 
	size_t len; 
	
	result = copyinstr((userptr_t) userpfilename, filename, PATH_MAX, &len);   
	// invalid userpointer
	if(result) {
		return EFAULT; 
	}
	
	int i;  
	int fd = -1;  
	for(i=3; i<=OPEN_MAX; i++){
		if(curproc->file_table[i]==NULL) {
			fd = i; 
			break;
		}
	}

	// File table is full
	if (fd == -1) {
		return EMFILE; 
	}
	
	// Open the file
	struct vnode* newvnode; 	
	res = vfs_open(filename, flags, 0, &newvnode); 	
	if(res){
		kfree(filename); 
		return res; 
	}

	// Create a new file_handle 
	struct file_handle* new_fh=create_file_handle(0, how); 
	new_fh->file = newvnode; 

	curproc->file_table[fd] = (struct file_handle*)kmalloc(sizeof(struct file_handle)); 
	curproc->file_table[fd] = new_fh;
	
	*retval = fd; 

	return 0; 

}

int
sys_write(int fd, void *buf, size_t buflen, int* retval){
	*retval = -1;

	if(fd>=OPEN_MAX||fd<0) {
		return EBADF;  
	}
	
	if(curproc->file_table[fd] == NULL) {
		return EBADF; 
	}

	if(curproc->file_table[fd]->flag == O_RDONLY){
		return EBADF; 
	}
	struct file_handle* current = curproc->file_table[fd];  
	
	
	struct iovec iov;
	struct uio ku; 
	

	iov.iov_ubase = (userptr_t)buf;
	iov.iov_len = buflen;
	ku.uio_iov = &iov;
	ku.uio_iovcnt = 1;
	ku.uio_offset = current->offset;
	ku.uio_resid = buflen;
	ku.uio_segflg = UIO_USERSPACE;
	ku.uio_rw = UIO_WRITE;
	ku.uio_space = curproc->p_addrspace;

	lock_acquire(current->fh_lock);	
	// case it was closed by another thread..
	if(current-> file == NULL){
		return EIO; 
	}

	int err; 	
	err = VOP_WRITE(current->file, &ku);
	if(err) {
		lock_release(current->fh_lock); 
		return err;
	}	

	current->offset += buflen;
	*retval = (int) buflen; 
	
	lock_release(current->fh_lock);
	

	return 0; 
}

int 
sys_read(int fd, void *buf, size_t buflen, int* retval){
	*retval = -1;
	
	if(fd>=OPEN_MAX||fd<0) {
		return EBADF;  
	}
	if(curproc->file_table[fd] == NULL) {
		return EBADF; 
	}
	
	struct file_handle* current = curproc->file_table[fd];  

	if (current->flag == O_WRONLY) return EBADF;
	
	struct iovec iov;
	struct uio ku; 

	lock_acquire(current->fh_lock);

	iov.iov_ubase = (userptr_t)buf;
	iov.iov_len = buflen; 
	ku.uio_iov = &iov;
	ku.uio_iovcnt = 1;
	ku.uio_resid = buflen; // amount to read from the file
	ku.uio_offset = current->offset;
	ku.uio_segflg = UIO_USERSPACE;
	ku.uio_rw = UIO_READ;
	ku.uio_space = curproc->p_addrspace;

	// case it was closed by another thread..
	if(current-> file == NULL){
		lock_release(current->fh_lock); 
		return EIO; 
	}

	int err; 	
	err = VOP_READ(current->file, &ku);
	if(err) {
		lock_release(current->fh_lock); 
		return err;
	}	

	current->offset += buflen;

	lock_release(current->fh_lock);

	*retval = (int) buflen; 
	return 0;
}

int
sys_close(int fd, int* retval) {
	*retval = -1; 

	if(fd > OPEN_MAX || fd < 0){
		return EBADF;
	}	

	struct file_handle* current = curproc->file_table[fd];
	if (current == NULL) return EBADF;
       	
	lock_acquire(current->fh_lock); 
	if(current->file!=NULL) {
		vfs_close(current->file); 	
	}

	lock_release(current->fh_lock); 

	lock_destroy(current->fh_lock); 

	kfree(current);
	curproc->file_table[fd] = NULL;

	*retval = 0;
	return 0;

}

/*
void fh_destroy(struct file_handle **fh){
	if(fh == NULL) {
		return;
	}

	lock_destroy(fh->fh_lock);  
}*/



