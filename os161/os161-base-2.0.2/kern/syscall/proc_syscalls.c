#include <types.h>
#include <kern/unistd.h>
#include <kern/errno.h>
#include <clock.h>
#include <copyinout.h>
#include <syscall.h>
#include <lib.h>
#include <thread.h>
#include <proc.h>
#include <addrspace.h>
#include <mips/trapframe.h>

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
pid_t sys_waitpid(pid_t pid, int *returncode, int flags){
	struct proc *proc;
	int status;
	pid_t return_pid;
	(void) flags;
	
	proc = proc_of_pid(pid);
	return_pid = proc->pid;
	(void)return_pid;
	status = proc_wait(proc);
	//proc = NULL;
	*returncode = status;
	return return_pid;
}

pid_t sys_getpid(void){
	struct proc *proc = curproc;
	pid_t pid = proc->pid;
	
	return pid;
}
#endif
