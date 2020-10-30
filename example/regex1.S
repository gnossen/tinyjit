.text
.globl regex
.type regex, %function

// ba*b*a
regex:
    push    %rbp
    mov     %rsp, %rbp  //  manage the stack

    // State 1
    movb    $0x62, %al  // put 'a' into %al
    scasb               // compare %edi with %al
    jnz     .failure    // fail if not equal

    // State 2
    movb    $0x61, %al  // put 'b' into %al
    repe scasb          // match as many 'b's as possible
    dec     %rdi        // rewind %edi to the cause
    movb    $0x62, %al  // compare with 'b'
    scasb
    jnz .failure        // fail if not equal

    // State 3
    movb    $0x62, %al  // technically redundant, could remove
    repe scasb
    dec     %rdi        // rewind %edi to the cause
    movb    $0x61, %al  // compare with 'a'
    scasb
    jnz     .failure    // fail if not equal

    // State 4
    movb    (%rdi), %dil
    test    %dil, %dil
    jnz     .failure

    // Stage 5
    mov     $0x1, %rax
    pop     %rbp
    retq

.failure:
    // State 6
    mov     $0x0, %rax
    pop     %rbp
    retq
