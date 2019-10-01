option casemap:none

MemCpyEbd Proto

EXTERN DebugData: ptr qword
EXTERN g_PTE_BASE: qword
EXTERN g_PDE_BASE: qword

NT_BASE equ 0FFFFF8017a0b8000h

.data

.code


Asm_test	Proc
        ret
Asm_test 	Endp


Asm_WriteProtectDisable	Proc
        cli
        mov rax, cr0
        and rax, not 10000h
        mov cr0, rax
        ret
Asm_WriteProtectDisable 	Endp


Asm_WriteProtectEnable	Proc
        mov rax, cr0
        or rax, 10000h
        mov cr0, rax
        sti
        ret
Asm_WriteProtectEnable 	Endp


;fffff807`16a98c50  00000000`00000011	error code
;fffff807`16a98c58  ffff8986`e513000c	eip
;fffff807`16a98c60  00000000`00000010	cs
;fffff807`16a98c68  00000000`00010246	eflags 10000001001000110
;fffff807`16a98c70  fffff807`16a98c88	esp
;fffff807`16a98c78  00000000`00000018	ss
;fffff807`16a98c80  fffff807`16a9b000	align
Asm_hook_PageFault	Proc


        push rcx

        mov rax, [rsp + 10h]      ;error code
        cmp rax, 11h
        jnz @back


        mov rax, [rsp + 18h]    ;addr
        mov eax, dword ptr [rax]
        cmp eax, 1131482eh
        jnz @back

        mov [DebugData + 30h], rdx
;----------------------------------------
        mov rax, [DebugData+10h]
        inc rax
        mov [DebugData+10h], rax
        mov [DebugData+18h], rcx

        mov rax, [rsp + 18h]    ;addr
        mov [DebugData+20h], rax

        mov rax, [rsp+28h]  ;efl
        push rax
        popfq
        mov rax, [rsp+38h]  ;ss
        mov ss, ax
        mov rax, [rsp+30h]  ;rsp
        mov [DebugData+40h], rax
        mov rsp, rax

        ret
;-----------------------------------------

@back:
        pop rcx
        pop rax
        push rbp
        sub rsp, 158h
        lea rbp, [rsp+80h]
        sub rsp, 8
        mov dword ptr [rsp], (NT_BASE+ 1CA450h)and 0ffffffffh
        mov dword ptr [rsp+4], NT_BASE shr 32
        ret
Asm_hook_PageFault 	Endp


Asm_hook_memcpy_entry       Proc
        push rax
        push rcx
        push rdx
        push r8
        push r9
        push r10

        sub rsp, 20h
        call MemCpyEbd
        add rsp, 20h

        pop r10
        pop r9
        pop r8
        pop rdx
        pop rcx
        pop rax

        mov r11, rcx
        sub rdx, rcx
        jb @1

        sub rsp, 8
        mov dword ptr [rsp], (NT_BASE+ 1CF4D0h)and 0ffffffffh
        mov dword ptr [rsp+4], NT_BASE shr 32
        cmp r8, 4fh
        ret
@1:
        sub rsp, 8
        mov dword ptr [rsp], (NT_BASE+ 1CF66Eh)and 0ffffffffh
        mov dword ptr [rsp+4], NT_BASE shr 32
        ret

Asm_hook_memcpy_entry       Endp

Asm_hook_exAllocate_entry	Proc
        test edi, edi
        jnz @ret
        cmp rsi, 8000h
        jb @ret

        mov rcx, rbx
        mov rax, 0ffffffffffffh
        and rcx, rax
        shr rcx, 21
        shl rcx, 3
        mov rax, g_PDE_BASE
        add rax, rcx
        mov rcx, rax        ;ppde
        mov rax, [rcx]       ;pde
        test rax, 80h
        jnz @ret

            mov rcx, rbx
            mov rax, 0ffffffffffffh
            and rcx, rax
            shr rcx, 12
            shl rcx, 3
            mov rax, g_PTE_BASE
            add rax, rcx
            mov rcx, rax        ;ppte
                   push rcx
                   mov rax, [rcx]       ;pte
                   mov rcx, 08000000000000000h
                   or  rax, rcx
                   pop rcx
                   mov [rcx], rax

@ret:
        mov     rbp, [rsp+58h]
        mov     rax, rbx
        mov     rbx, [rsp+50h]
        mov     rsi, [rsp+60h]
        add     rsp, 30h
        pop     r15
        pop     r14
        pop     rdi
        ret
Asm_hook_exAllocate_entry 	Endp

Asm_hook_MmAllocateIndependentPages      Proc
        mov rax, [rsp+8]
        pop rax
        sub rsp, 28h

        cmp rcx, 8000h
        jb  @r
        xor r9d, r9d
        xor r8d, r8d
        call @to
;-------------------------------
        push rax
        push rcx

        mov rcx, rax
        mov rax, 0ffffffffffffh
        and rcx, rax
        shr rcx, 21
        shl rcx, 3
        mov rax, g_PDE_BASE
        add rax, rcx
        mov rcx, rax        ;ppde
        mov rax, [rcx]       ;pde
        test rax, 80h
        jnz @pop

            mov rcx, [rsp + 8]

            mov rax, 0ffffffffffffh
            and rcx, rax
            shr rcx, 12
            shl rcx, 3
            mov rax, g_PTE_BASE
            add rax, rcx
            mov rcx, rax        ;ppte
                   push rcx
                   mov rax, [rcx]       ;pte
                   mov rcx, 08000000000000000h
                   or  rax, rcx
                   pop rcx
           mov [rcx], rax
@pop:
        pop rcx
        pop rax
        add rsp, 28h
        ret

@r:
        xor r9d, r9d
        xor r8d, r8d
        call @to
        add rsp, 28h
        ret
@to:
        sub rsp, 8
        mov dword ptr [rsp], (NT_BASE+ 1044E8h)and 0ffffffffh
        mov dword ptr [rsp+4], NT_BASE shr 32
        ret
Asm_hook_MmAllocateIndependentPages       Endp

Asm_hook_hMmSetPageProtection       Proc
        cmp rdx, 8000h
        jb  @r
        cmp r8d, 40h
        jnz @r

        mov r8d, 4
@r:
        pop rax
        mov     [rsp+20h], rbx
        push    rbp
        push    rsi
        push    rdi
        push    r14
        push    r15
        sub     rsp, 100h
        mov     rax, [NT_BASE + 427F70h]
        sub rsp, 8
        mov dword ptr [rsp], (NT_BASE+ 12A75Ah)and 0ffffffffh
        mov dword ptr [rsp+4], NT_BASE shr 32
        ret
Asm_hook_hMmSetPageProtection       Endp

END
