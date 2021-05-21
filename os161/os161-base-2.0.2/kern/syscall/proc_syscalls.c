#include <types.h>
#include <kern/unistd.h>
#include <clock.h>
#include <copyinout.h>
#include <syscall.h>
#include <lib.h>
#include <thread.h>
#include <proc.h>
#include <addrspace.h>

#if OPT_EXIT
#include <synch.h>
#endif

void sys__exit(int status){
	/* in order to avoid a warning in campile fase */
	(void)status;
	
	#if OPT_EXIT
	struct proc *p = curproc;
	p->status = status;
	/* signal to waiting process */
	proc_remthread(curthread);
	V(p->proc_sem);
	#else
	/* get addrspace of the process */
	struct addrspace *as = proc_getas();

	/* destroy of addrspace */
	as_destroy(as);
	#endif

	thread_exit();
}

#if OPT_EXIT
int sys_waitpid(pid_t pid){
	(void) pid;
	return 0;
}
#endif
