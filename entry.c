#include <ntddk.h>
#include "common.h"
#include "scan.h"

ULONG64 g_NT_BASE = 0xFFFFF8014a819000;
ULONG64 g_PTE_BASE;
ULONG64 g_PDE_BASE;
ULONG64 g_PPE_BASE;
ULONG64 g_PXE_BASE;


VOID DriverUnload(PDRIVER_OBJECT driver)
{
    DbgPrint("Driver is unloading...\r\n");
}

//ed nt!Kd_SXS_Mask 0
//ed nt!Kd_FUSION_Mask 0


// 50                              | push rax                              |
// 48:B8 8967452301000000          | mov rax,123456789                     |
// 50                              | push rax                              |
// C3                              | ret                                   |

//fffff807`14084440 55              push    rbp
//fffff807`14084441 4881ec58010000  sub     rsp,158h
//fffff807`14084448 488dac2480000000 lea     rbp,[rsp+80h]
//fffff807`14084450 c645ab01        mov     byte ptr [rbp-55h],1
//fffff807`14084454 488945b0        mov     qword ptr [rbp-50h],rax
//fffff807`14084458 48894db8        mov     qword ptr [rbp-48h],rcx
void hookPageFault()
{
    *(PUCHAR)(g_NT_BASE + 0x1CA440) = 0x50;
    *(PUSHORT)(g_NT_BASE + 0x1CA441) = 0xb848;
    *(PULONG64)(g_NT_BASE + 0x1CA443) = (ULONG64)Asm_hook_PageFault;
    *(PUSHORT)(g_NT_BASE + 0x1CA44b) = 0xc350;
}


/*
 | 49:BB 8967452301000000          | mov r11,123456789                     |
 | 41:FFE3                         | jmp r11                               |
 90 90 90
 */
void hookMemCpy()
{
    *(PUSHORT)(g_NT_BASE + 0xFFFFF807140894C0 - 0xFFFFF80713EBA000) = 0xbb49;
    *(PULONG64)(g_NT_BASE + 0xFFFFF807140894C0 - 0xFFFFF80713EBA000 + 2) = (ULONG64)Asm_hook_memcpy_entry;
    *(PULONG)(g_NT_BASE + 0xFFFFF807140894C0 - 0xFFFFF80713EBA000 + 10) = 0x90e3ff41;
    *(PUSHORT)(g_NT_BASE + 0xFFFFF807140894C0 - 0xFFFFF80713EBA000 + 14) = 0x9090;
}


void MemCpyEbd(ULONG64 des, ULONG64 src, ULONG64 size)
{

}



ULONG64 DebugData[0x10] = {
    0x2C0C6AE7D1F94A9F,
    0xB83491EF07C6E12F,
};


//C:\Users\zhouhe\Desktop\PG1903.sys
NTSTATUS 
  DriverEntry( 
    PDRIVER_OBJECT  driver,
    PUNICODE_STRING RegistryPath
    )
{
    DbgPrint("Driver Entered!\n");
    //Asm_test();
    DbgPrint("DebugData: %p\n", DebugData);
    driver->DriverUnload = DriverUnload;

    g_PTE_BASE = *(PULONG64)(g_NT_BASE + 0xCB3C);
    g_PDE_BASE = (g_PTE_BASE + ((g_PTE_BASE & 0xffffffffffff) >> 9));
    g_PPE_BASE = (g_PTE_BASE + ((g_PDE_BASE & 0xffffffffffff) >> 9));
    g_PXE_BASE = (g_PTE_BASE + ((g_PPE_BASE & 0xffffffffffff) >> 9));
    DbgPrint("pte at: %p\n", g_PTE_BASE);
    DbgPrint("pde at: %p\n", g_PDE_BASE);
    DbgPrint("ppe at: %p\n", g_PPE_BASE);
    DbgPrint("pxe at: %p\n", g_PXE_BASE);
    EnumSysRegions();
    ScanBigPool();
    Asm_WriteProtectDisable();
    hookPageFault();
    hookMemCpy();   //seems this can touch off the PG quickly
    Asm_WriteProtectEnable();
	return STATUS_SUCCESS;
}

/* magic nums from https://github.com/tandasat/PgResarch
 * ULONG64 pgContextAddr = bugCheckParameter[0] - 0xA3A03F5891C8B4E8;
reasonInfoAddr = bugCheckParameter[1] - 0xB3B74BDEE4453415;
*/
