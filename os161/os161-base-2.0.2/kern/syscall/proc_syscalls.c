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

static void
call_enter_forked_process(void *tfv, unsigned long dummy) {
  struct trapframe *tf = (struct trapframe *)tfv;
  (void)dummy;
  enter_forked_process(tf); 
 
  panic("enter_forked_process returned (should not happen)\n");
}

int sys_fork(struct trapframe *ctf, pid_t *retval) {
  struct trapframe *tf_child;
  struct proc *newp;
  int result;

  KASSERT(curproc != NULL);

  newp = proc_create_runprogram(curproc->p_name);
  if (newp == NULL) {
    return ENOMEM;
  }

  /* done here as we need to duplicate the address space 
     of thbe current process */
  as_copy(curproc->p_addrspace, &(newp->p_addrspace));
  if(newp->p_addrspace == NULL){
    proc_destroy(newp); 
    return ENOMEM; 
  }

  /* we need a copy of the parent's trapframe */
  tf_child = kmalloc(sizeof(struct trapframe));
  if(tf_child == NULL){
    proc_destroy(newp);
    return ENOMEM; 
  }
  memcpy(tf_child, ctf, sizeof(struct trapframe));

  /* TO BE DONE: linking parent/child, so that child terminated 
     on parent exit */

  result = thread_fork(
		 curthread->t_name, newp,
		 call_enter_forked_process, 
		 (void *)tf_child, (unsigned long)0/*unused*/);

  if (result){
    proc_destroy(newp);
    kfree(tf_child);
    return ENOMEM;
  }

  *retval = newp->pid;

  return 0;
}
#endif
