#include <ntddk.h>
#include "common.h"
#include "scan.h"
extern ULONG64 g_NT_BASE;

PVOID GetVirtualAddressMappedByPte(PMMPTE pte)
{
    return (PVOID)(((((ULONG64)pte - g_PTE_BASE) >> 3) << 12) | 0xffff000000000000);
}
PVOID GetVirtualAddressMappedByPde(PMMPTE pde)
{
    return (PVOID)(((((ULONG64)pde - g_PDE_BASE) >> 3) << 21) | 0xffff000000000000);
}
PVOID GetVirtualAddressMappedByPpe(PMMPTE ppe)
{
    return (PVOID)(((((ULONG64)ppe - g_PPE_BASE) >> 3) << 30) | 0xffff000000000000);
}
PVOID GetVirtualAddressMappedByPxe(PMMPTE pxe)
{
    return (PVOID)(((((ULONG64)pxe - g_PXE_BASE) >> 3) << 39) | 0xffff000000000000);
}
PMMPTE GetPxeAddress(PVOID addr)
{
    return (PMMPTE)(((((ULONG64)addr & 0xffffffffffff) >> 39) << 3) + g_PXE_BASE);
}
PMMPTE GetPpeAddress(PVOID addr)
{
    return (PMMPTE)(((((ULONG64)addr & 0xffffffffffff) >> 30) << 3) + g_PPE_BASE);
}
PMMPTE GetPdeAddress(PVOID addr)
{
    return (PMMPTE)(((((ULONG64)addr & 0xffffffffffff) >> 21) << 3) + g_PDE_BASE);
}
PMMPTE GetPteAddress(PVOID addr)
{
    return (PMMPTE)(((((ULONG64)addr & 0xffffffffffff) >> 12) << 3) + g_PTE_BASE);
}

NTSTATUS ScanBigPool()
{
    PSYSTEM_BIGPOOL_INFORMATION pBigPoolInfo;
    ULONG64 ReturnLength = 0;
    NTSTATUS status;
    ULONG i = 0;
    int num=0;


    pBigPoolInfo = (PSYSTEM_BIGPOOL_INFORMATION)ExAllocatePoolWithTag(NonPagedPool, sizeof(SYSTEM_BIGPOOL_INFORMATION), 'ttt');
    status = ZwQuerySystemInformation(0x42/*SystemBigPoolInformation*/, pBigPoolInfo, sizeof(SYSTEM_BIGPOOL_INFORMATION), &ReturnLength);
    //DbgPrint("pBigPoolInfo->Count - %d \n", pBigPoolInfo->Count);
    //DbgPrint("ReturnLength - %p \n", ReturnLength);
    ExFreePool(pBigPoolInfo);
    pBigPoolInfo = (PSYSTEM_BIGPOOL_INFORMATION)ExAllocatePoolWithTag(NonPagedPool, ReturnLength + 0x1000, 'ttt');
    if (!pBigPoolInfo)
        return STATUS_UNSUCCESSFUL;
    status = ZwQuerySystemInformation(0x42, pBigPoolInfo, ReturnLength + 0x1000, &ReturnLength);
    if (status != STATUS_SUCCESS)
    {
        DbgPrint("query BigPoolInfo failed: %p\n", status);
        return status;
    }
    DbgPrint("pBigPoolInfo: %p\n", pBigPoolInfo);
    for (i=0; i < pBigPoolInfo->Count; i++)
    {
        PVOID addr = pBigPoolInfo->AllocatedInfo[i].VirtualAddress;
        ULONG64 size = (ULONG64)pBigPoolInfo->AllocatedInfo[i].SizeInBytes;
        PULONG64 ppte = (PULONG64)GetPteAddress(addr);
        ULONG64 pte = *ppte;
        PULONG64 ppde = (PULONG64)GetPdeAddress(addr);
        ULONG64 pde = *ppde;


        if ( size >= 0x8000)
        {
            if (pde & 0x80){//big page

            } else {

                    if ((pte & 0x8000000000000000) == 0 && (pte & 1)) {
                        pte |= 0x8000000000000000;
                        *ppte = pte;
                        DbgPrint("addr: %p, size: %p, pte: %p, nom\n", addr, size, pte);
                        num += 1;
                }
            }
        }
    }
    DbgPrint("num: %d\n", num);
    ExFreePool(pBigPoolInfo);
    return status;
}


/* Following are from https://github.com/9176324/Shark
*
* Copyright (c) 2015 - 2019 by blindtiger. All rights reserved.
*
* The contents of this file are subject to the Mozilla Public License Version
* 2.0 (the "License")); you may not use this file except in compliance with
* the License. You may obtain a copy of the License at
* http://www.mozilla.org/MPL/
*
* Software distributed under the License is distributed on an "AS IS" basis,
* WITHOUT WARRANTY OF ANY KIND, either express or implied. SEe the License
* for the specific language governing rights and limitations under the
* License.
*
* The Initial Developer of the Original e is blindtiger.
*
*/
VOID
InitializeSystemPtesBitMap(
    __inout PMMPTE BasePte,
    __in PFN_NUMBER NumberOfPtes,
    __out PRTL_BITMAP BitMap
)
{
    PMMPTE PointerPxe = NULL;
    PMMPTE PointerPpe = NULL;
    PMMPTE PointerPde = NULL;
    PMMPTE PointerPte = NULL;
    PVOID PointerAddress = NULL;
    ULONG BitNumber = 0;
    PVOID BeginAddress = NULL;
    PVOID EndAddress = NULL;

    /*
    PatchGuard Context pages allocate by MmAllocateIndependentPages
    */

#define VALID_PTE_SET_BITS \
            ( MM_PTE_VALID_MASK | MM_PTE_DIRTY_MASK | MM_PTE_WRITE_MASK | MM_PTE_ACCESS_MASK)

#define VALID_PTE_UNSET_BITS \
            ( MM_PTE_WRITE_THROUGH_MASK | MM_PTE_CACHE_DISABLE_MASK | MM_PTE_COPY_ON_WRITE_MASK )

    BeginAddress = GetVirtualAddressMappedByPte(BasePte);
//    __debugbreak();
    EndAddress = GetVirtualAddressMappedByPte(BasePte + NumberOfPtes);

    PointerAddress = BeginAddress;

    do {
        PointerPxe = GetPxeAddress(PointerAddress);

        if (0 != PointerPxe->u.Hard.Valid) {
            PointerPpe = GetPpeAddress(PointerAddress);

            if (0 != PointerPpe->u.Hard.Valid) {
                PointerPde = GetPdeAddress(PointerAddress);

                if (0 != PointerPde->u.Hard.Valid) {
                    if (0 == PointerPde->u.Hard.LargePage) {
                        PointerPte = GetPteAddress(PointerAddress);

                        if (0 != PointerPte->u.Hard.Valid) {
                            if (0 == PointerPte->u.Hard.NoExecute) {
                                if (VALID_PTE_SET_BITS == (PointerPte->u.Long & VALID_PTE_SET_BITS)) {
                                    if (0 == (PointerPte->u.Long & VALID_PTE_UNSET_BITS)) {
                                        BitNumber = (ULONG)(PointerPte - BasePte);
                                        RtlSetBit(BitMap, BitNumber);
                                    }
                                }
                            }
                        }

                        PointerAddress = GetVirtualAddressMappedByPte(PointerPte + 1);
                    }
                    else {
                        PointerAddress = GetVirtualAddressMappedByPde(PointerPde + 1);
                    }
                }
                else {
                    PointerAddress = GetVirtualAddressMappedByPde(PointerPde + 1);
                }
            }
            else {
                PointerAddress = GetVirtualAddressMappedByPpe(PointerPpe + 1);
            }
        }
        else {
            PointerAddress = GetVirtualAddressMappedByPxe(PointerPxe + 1);
        }
    } while ((ULONG_PTR)PointerAddress < (ULONG_PTR)EndAddress);
}

VOID EnumSysRegions()
{
    PMMPTE BasePte;
    PFN_NUMBER NumberOfPtes;
    ULONG BitMapSize;
    ULONG64 system_pte_strc;
    PRTL_BITMAP BitMap;
    ULONG HintIndex = 0;
    ULONG StartingRunIndex = 0;
    PVOID BaseAddress;
    ULONG64 RegionSize;
    system_pte_strc = 0xFFFFF8071431D780 - 0xFFFFF80713EBA000 + g_NT_BASE;
    DbgPrint("system_pte_strc: %p", system_pte_strc);
    BasePte = (PMMPTE)*(PULONG64)(system_pte_strc + 0x10);
    NumberOfPtes = *(PULONG64)(system_pte_strc) * 8;
    BitMapSize =
            sizeof(RTL_BITMAP) + (ULONG)((((NumberOfPtes + 1) + 31) / 32) * 4);

    BitMap = ExAllocatePoolWithTag(NonPagedPool, BitMapSize, 'BMP');
    RtlInitializeBitMap(
        BitMap,
        (PULONG)(BitMap + 1),
        (ULONG)(NumberOfPtes + 1));
    RtlClearAllBits(BitMap);
    InitializeSystemPtesBitMap(
                BasePte,
                NumberOfPtes,
                BitMap);
//__debugbreak();
    do {
        HintIndex = RtlFindSetBits(
                    BitMap,
                    1,
                    HintIndex);

        if (MAXULONG != HintIndex) {
            RtlFindNextForwardRunClear(
                        BitMap,
                        HintIndex,
                        &StartingRunIndex);

            RtlClearBits(BitMap, HintIndex, StartingRunIndex - HintIndex);


            BaseAddress =
                    GetVirtualAddressMappedByPte(BasePte + HintIndex);
            RegionSize =
                    (SIZE_T)(StartingRunIndex - HintIndex) * 0x1000;

            if (RegionSize > 0x8000) {
                DbgPrint(
                            "found region in system ptes < %p - %08x >\n",
                            BaseAddress,
                            RegionSize);
                /*****************handle it*****************/
                GetPpeAddress(BaseAddress)->u.Long |= 0x8000000000000000;
            }

            HintIndex = StartingRunIndex;
        }
    } while (HintIndex < NumberOfPtes);

    ExFreePool(BitMap);
}
