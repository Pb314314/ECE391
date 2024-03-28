#define ASM 1


.align 4
syscall_table:
    .long 0x0           # There is no syscall zero
    .long halt
    .long execute
    .long read
    .long write
    .long open
    .long close
    .long getargs
    .long vidmap
    .long set_handler
    .long sigreturn

.global syscall_handler
.align 4
syscall_handler:
    pushl %ebx                  
    pushl %ecx
    pushl %edx
    pushl %esi
    pushl %edi
    pushl %ebp
    pushl %esp
    pushfl                                 
    pushl %edx                  
    pushl %ecx                  
    pushl %ebx   
    
    cmpl    $0, %eax                  
    jle     Input_errer
    cmpl    $10, %eax                  
    jnle    Input_errer
    
    call    *syscall_table(, %eax, 4)

    addl    $12,%esp

    popfl
    popl %esp
    popl %ebp
    popl %edi
    popl %esi
    popl %edx
    popl %ecx
    popl %ebx 
    iret

Input_errer:
    addl    $12,%esp
    popfl       
    popl %esp
    popl %ebp
    popl %edi
    popl %esi
    popl %edx
    popl %ecx
    popl %ebx  
    movl $-1, %eax           
    iret
    
    
    
    
    
