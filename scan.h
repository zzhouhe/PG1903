#ifndef SCAN_H
#define SCAN_H

VOID
InitializeSystemPtesBitMap(
    __inout PMMPTE BasePte,
    __in PFN_NUMBER NumberOfPtes,
    __out PRTL_BITMAP BitMap
);

VOID EnumSysRegions();
NTSTATUS ScanBigPool();
#endif // SCAN_H
