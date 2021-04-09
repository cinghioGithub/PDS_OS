#include <types.h>
#include <kern/unistd.h>
#include <clock.h>
#include <copyinout.h>
#include <syscall.h>
#include <lib.h>
#include <thread.h>
#include <proc.h>
#include <addrspace.h>

void sys__exit(int status){
	/* in order to avoid a warning in campile fase */
	(void)status;

	/* get addrspace of the process */
	struct addrspace *as = proc_getas();

	/* destroy of addrspace */
	as_destroy(as);

	thread_exit();
}