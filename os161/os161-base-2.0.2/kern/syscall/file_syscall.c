#include <types.h>
#include <kern/unistd.h>
#include <clock.h>
#include <copyinout.h>
#include <syscall.h>
#include <lib.h>
#if OPT_FILE
#include <limits.h>
#include <vfs.h>
#include <proc.h>
#include <uio.h>
#include <vnode.h>

#define SYSTEM_OPEN_MAX (10*OPEN_MAX)
#endif

#if OPT_FILE
struct openfile {
	struct vnode* vnode;
	unsigned int count_ref;
	off_t offset;
};

unsigned char table_initialization = 1;

struct openfile systemFileTable[SYSTEM_OPEN_MAX];
#endif

int sys_write(int fd, userptr_t buf_ptr, size_t size){
	int i;
	char *p = (char *)buf_ptr;

	if(fd < 0 || fd >=OPEN_MAX){
		return -1;
	}

	if(fd == STDOUT_FILENO || fd == STDERR_FILENO){
		if(curproc->proc_open_files[fd] == NULL){
			for(i=0; i<(int)size; i++){
				putch(p[i]);
			}
			return (int)size;
		}
		else{
			return file_write(fd, buf_ptr, size);
		}
	}
	else{
		return file_write(fd, buf_ptr, size);
	}
	return -1;
}

int sys_read(int fd, userptr_t buf_ptr, size_t size){
	int i;
	char *p = (char *)buf_ptr;

	if(fd < 0 || fd >=OPEN_MAX){
		return -1;
	}

	if(fd == STDIN_FILENO){
		if(curproc->proc_open_files[fd] == NULL){
			for(i=0; i<(int)size; i++){
				p[i] = getch();
				if(p[i] < 0) return i;
			}
			return size;
		}
		else{
			return file_read(fd, buf_ptr, size);
		}
	}
	else{
		return file_read(fd, buf_ptr, size);
	}
	return -1;
}

#if OPT_FILE
int sys_open(const char* file_name, int flags, mode_t mode, int *errp){
	int fd;
	int i;
	struct vnode* vnode;
	struct openfile *of;
	struct proc *p = curproc;

	if(table_initialization == 1){
		for(i=0; i<SYSTEM_OPEN_MAX; i++){
			table_initialization = 0;
			systemFileTable[i].vnode = NULL;
			systemFileTable[i].count_ref = 0;
		}
	}

	int err = vfs_open((char *)file_name, flags, mode, &vnode);
	if(err != 0){
		return -1;
	}

	for(i=0; i<SYSTEM_OPEN_MAX; i++){
		if(systemFileTable[i].vnode == NULL){
			systemFileTable[i].count_ref++;
			systemFileTable[i].vnode = vnode;
			systemFileTable[i].offset = 0;
			of = &systemFileTable[i];
			break;
		}
	}
	if(i == SYSTEM_OPEN_MAX){
		return -1;
	}

	if(of == NULL){
		return -1;
	}

	for(i=STDERR_FILENO+1; i<OPEN_MAX; i++){
		if(p->proc_open_files[i] == NULL){
			p->proc_open_files[i] = of;
			fd = i;
			break;
		}
	}

	if(i == OPEN_MAX){
		vfs_close(vnode);
		return -1;
	}

	(void) errp;

	return fd;
}

int sys_close(int fd){
	struct proc *p = curproc;
	struct openfile *of;

	if(fd < 0 || fd >= OPEN_MAX){
		return -1;
	}
	
	of = p->proc_open_files[fd];
	p->proc_open_files[fd] = NULL;
	if(of->count_ref == 1){
		vfs_close(of->vnode);
		of->vnode = NULL;
	}
	else{
		of->count_ref--;
	}

	return 0;
}

ssize_t file_read(int fd, userptr_t buf_ptr, size_t size){
	struct iovec iov;
	struct uio u;
	int result;
	ssize_t byte_read;

	struct vnode *v;
	struct openfile *of;

	if(fd < 0 || fd > OPEN_MAX){
		return -1;
	}

	of = curproc->proc_open_files[fd];
	if(of == NULL){
		return -1;
	}

	v = of->vnode;
	iov.iov_ubase = (void *)buf_ptr;
	iov.iov_len = size;

	u.uio_iov = &iov;
	u.uio_iovcnt = 1;
	u.uio_resid = size;
	u.uio_offset = of->offset;
	u.uio_segflg = UIO_USERISPACE;
	u.uio_rw = UIO_READ;
	u.uio_space = curproc->p_addrspace;

	result = VOP_READ(v, &u);
	if(result){
		return result;
	}

	byte_read = size - u.uio_resid;
	of->offset = of->offset + byte_read;

	return byte_read;
}

ssize_t file_write(int fd, userptr_t buf_ptr, size_t size){
	struct iovec iov;
	struct uio u;
	int result;
	ssize_t byte_write;

	struct vnode *v;
	struct openfile *of;

	if(fd < 0 || fd > OPEN_MAX){
		return -1;
	}

	of = curproc->proc_open_files[fd];
	if(of == NULL){
		return -1;
	}

	v = of->vnode;
	iov.iov_ubase = (void *)buf_ptr;
	iov.iov_len = size;

	u.uio_iov = &iov;
	u.uio_iovcnt = 1;
	u.uio_resid = size;
	u.uio_offset = of->offset;
	u.uio_segflg = UIO_USERISPACE;
	u.uio_rw = UIO_WRITE;
	u.uio_space = curproc->p_addrspace;

	result = VOP_WRITE(v, &u);
	if(result){
		return result;
	}

	byte_write = size - u.uio_resid;
	if(u.uio_resid > 0){
		kprintf("Short write\n");
		return -1;
	}
	of->offset = of->offset + byte_write;

	return byte_write;
}
#endif
