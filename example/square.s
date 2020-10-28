square:
        push    rbp
        mov     rbp, rsp
        mov     DWORD [rbp-4], edi
        mov     eax, DWORD [rbp-4]
        imul    eax, eax
        pop     rbp
        ret
