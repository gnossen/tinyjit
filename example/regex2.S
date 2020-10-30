.text
.globl regex
.type regex, %function


// c(a|b)c.
regex:
    push    %rbp
    mov     %rsp, %rbp

    // State 1
    mov     $0x63, %al  // 'c'
    scasb
    jnz     .failure

    // State 2
.state_2:
    mov     $0x61, %al // 'a'
    repe scasb
    dec     %rdi

    // State 3
    mov     $0x62, %al // 'b'
    scasb
    jz      .state_2
    dec     %rdi

    // State 4
    mov     $0x63, %al // 'c'
    scasb
    jnz    .failure

    // State 5
    mov     (%rdi), %al
    cmp     $0x61, %al
    jz      .state_8

    // State 6
    cmp     $0x62, %al // depending on the mov in state 5
    jz      .state_8

    // State 7
    cmp     $0x63, %al
    jnz     .failure

    // State 8
.state_8:
    inc     %rdi
    mov     (%rdi), %al
    test     %al, %al
    jnz     .failure

    // State 9
    mov     $0x1, %rax
    pop     %rbp
    retq

.failure:
    // State 10
    xor     %eax, %eax
    pop     %rbp
    ret
