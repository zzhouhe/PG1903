option casemap:none

MemCpyEbd Proto

EXTERN DebugData: ptr qword
EXTERN g_PTE_BASE: qword
EXTERN g_PDE_BASE: qword

NT_BASE equ 0FFFFF8014a819000h

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

END
