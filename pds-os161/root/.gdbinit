def dbos161
  dir ../../os161/os161-base-2.0.2/kern/compile/DUMBVM
  target remote unix:.sockets/gdb
end
def dbos161h
  dir ../../os161/os161-base-2.0.2/kern/compile/HELLO
  target remote unix:.sockets/gdb
end
def dbos161t
  dir ../../os161/os161-base-2.0.2/kern/compile/THREADS
  target remote unix:.sockets/gdb
end
def dbos161s
  dir ../../os161/os161-base-2.0.2/kern/compile/SYSCALLS
  target remote unix:.sockets/gdb
end
def dbos161l
  dir ../../os161/os161-base-2.0.2/kern/compile/LOCK
  target remote unix:.sockets/gdb
end
def dbos161f
  dir ../../os161/os161-base-2.0.2/kern/compile/FILE
  target remote unix:.sockets/gdb
end
dbos161f

