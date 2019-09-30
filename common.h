#ifndef COMMON_H
#define COMMON_H

//http://www.geoffchappell.com/studies/windows/km/ntoskrnl/api/ex/sysinfo/bigpool_entry.htm
typedef struct _SYSTEM_BIGPOOL_ENTRY
{
    union {
        PVOID VirtualAddress;
        ULONG_PTR NonPaged : 1;
    };
    ULONG_PTR SizeInBytes;
    union {
        UCHAR Tag [4];
        ULONG TagUlong;
    };
} SYSTEM_BIGPOOL_ENTRY, *PSYSTEM_BIGPOOL_ENTRY;
typedef struct _SYSTEM_BIGPOOL_INFORMATION
{
    ULONG Count;
    SYSTEM_BIGPOOL_ENTRY AllocatedInfo[1];
} SYSTEM_BIGPOOL_INFORMATION, *PSYSTEM_BIGPOOL_INFORMATION;

NTSTATUS
ZwQuerySystemInformation (
    ULONG64 SystemInformationClass,
    PSYSTEM_BIGPOOL_INFORMATION SystemInformation,
    ULONG64 SystemInformationLength,
    ULONG64 *ReturnLength);

typedef struct _MMPTE_HARDWARE            // 18 elements, 0x8 bytes (sizeof)
          {
/*0x000*/     UINT64       Valid : 1;               // 0 BitPosition
/*0x000*/     UINT64       Dirty1 : 1;              // 1 BitPosition
/*0x000*/     UINT64       Owner : 1;               // 2 BitPosition
/*0x000*/     UINT64       WriteThrough : 1;        // 3 BitPosition
/*0x000*/     UINT64       CacheDisable : 1;        // 4 BitPosition
/*0x000*/     UINT64       Accessed : 1;            // 5 BitPosition
/*0x000*/     UINT64       Dirty : 1;               // 6 BitPosition
/*0x000*/     UINT64       LargePage : 1;           // 7 BitPosition
/*0x000*/     UINT64       Global : 1;              // 8 BitPosition
/*0x000*/     UINT64       CopyOnWrite : 1;         // 9 BitPosition
/*0x000*/     UINT64       Unused : 1;              // 10 BitPosition
/*0x000*/     UINT64       Write : 1;               // 11 BitPosition
/*0x000*/     UINT64       PageFrameNumber : 36;    // 12 BitPosition
/*0x000*/     UINT64       ReservedForHardware : 4; // 48 BitPosition
/*0x000*/     UINT64       ReservedForSoftware : 4; // 52 BitPosition
/*0x000*/     UINT64       WsleAge : 4;             // 56 BitPosition
/*0x000*/     UINT64       WsleProtection : 3;      // 60 BitPosition
/*0x000*/     UINT64       NoExecute : 1;           // 63 BitPosition
          }MMPTE_HARDWARE, *PMMPTE_HARDWARE;

#define MM_PTE_VALID_MASK         0x1
#if defined(NT_UP)
#define MM_PTE_WRITE_MASK         0x2
#else
#define MM_PTE_WRITE_MASK         0x800
#endif
#define MM_PTE_OWNER_MASK         0x4
#define MM_PTE_WRITE_THROUGH_MASK 0x8
#define MM_PTE_CACHE_DISABLE_MASK 0x10
#define MM_PTE_ACCESS_MASK        0x20
#if defined(NT_UP)
#define MM_PTE_DIRTY_MASK         0x40
#else
#define MM_PTE_DIRTY_MASK         0x42
#endif
#define MM_PTE_LARGE_PAGE_MASK    0x80
#define MM_PTE_GLOBAL_MASK        0x100
#define MM_PTE_COPY_ON_WRITE_MASK 0x200
#define MM_PTE_PROTOTYPE_MASK     0x400
#define MM_PTE_TRANSITION_MASK    0x800

typedef struct _MMPTE         // 1 elements, 0x8 bytes (sizeof)
          {
/*0x000*/     union {
                        MMPTE_HARDWARE  Hard;
                        ULONG64         Long;
                    } u; // 9 elements, 0x8 bytes (sizeof)
          }MMPTE, *PMMPTE;

extern ULONG64 g_NT_BASE;
extern ULONG64 g_PTE_BASE;
extern ULONG64 g_PDE_BASE;
extern ULONG64 g_PPE_BASE;
extern ULONG64 g_PXE_BASE;

void Asm_test();
void Asm_WriteProtectDisable();
void Asm_WriteProtectEnable();
void Asm_hook_PageFault();
void Asm_hook_exAllocate_entry();
void Asm_hook_memcpy_entry();
void Asm_hook_MmAllocateIndependentPages();
void Asm_hook_hMmSetPageProtection();
void Asm_hook_MiWriteValidPteNewProtectio();
void Asm_fault();
extern ULONG64 Except_addr;
#endif // COMMON_H

