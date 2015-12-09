# Austin Schorsch
# Lab 1
# 9/7/15

#====== get_esp, get_ebp ======
# Note: provided by KC Wang ===
#==============================    
  .global get_esp, get_ebp
get_esp:
    movl %esp, %eax
    ret   # return esp in the eax register
get_ebp:
    movl %ebp, %eax
    ret   # return ebp in the eax register
    
#====== main, mymain, myprintf ======
# Note: Base provided by KC Wang ====
#====================================
  .global main, mymain, myprintf
main:
  pushl %ebp
  movl %esp, %ebp # Establish the stack frame

# (1). Write ASSEMBLY code to call myprintf(FMT)
#      HELP: How does mysum() call printf() in the class notes.
  pushl $FMT
  call myprintf
  addl $4, %esp   # restore the stack

# (2). Write ASSEMBLY code to call mymain(argc, argv, env)
#      HELP: When crt0.o calls main(int argc, char *argv[], char *env[]), 
#            it passes argc, argv, env to main(). 
#            Draw a diagram to see where are argc, argv, env?

  # Recall:
  # (1) When crt0.o calls main, it pushes the return address (current PC reg) onto stack 
  # (2) In the code provided by KC, ebp is already pushed on the stack, which explains why we start at 8 instead of 4
  # (3) Push parameters in reverse order, that is: env, argv, argc  
  pushl 16(%ebp)  # The location of env given main, argc, argv are already pushed on
  pushl 12(%ebp)
  pushl 8(%ebp)
  call mymain
  addl $12, %esp   # restore the stack

# (3). Write code to call myprintf(fmt,a,b) 
#      HELP: same as in (1) above
  # Simply push b, a, and fmt. Then call myprintf and restore (similar to first part)
  pushl b 
  pushl a
  pushl $fmt
  call myprintf
  addl $12, %esp  # restore the stack

# (4). Return to caller
  movl  %ebp, %esp
  popl  %ebp
  ret

#------------ DATA section of assembly code ---------------
  .data
FMT:  .asciz "main() in assembly call mymain() in C\n"
a:  .long 1234
b:  .long 5678
fmt:  .asciz "a=%d b=%d\n"
#------------  end of s.s file ----------------------------
