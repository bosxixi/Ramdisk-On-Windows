#include "ramdisk.h"
#include "ntintsafe.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, RamDiskEvtDeviceAdd)
#pragma alloc_text(PAGE, RamDiskEvtDeviceContextCleanup)
#pragma alloc_text(PAGE, RamDiskQueryDiskRegParameters)
#pragma alloc_text(PAGE, RamDiskFormatDisk)
#endif
/*�������*/
NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
    WDF_DRIVER_CONFIG config;
    /*��������豸�ĺ�����ַ*/
    WDF_DRIVER_CONFIG_INIT(&config,RamDiskEvtDeviceAdd);

    return WdfDriverCreate(DriverObject, RegistryPath, WDF_NO_OBJECT_ATTRIBUTES,&config,WDF_NO_HANDLE);
}
//������
VOID RamDiskEvtIoRead(IN WDFQUEUE Queue,IN WDFREQUEST Request,IN size_t Length)
{
    PDEVICE_EXTENSION      devExt = QueueGetExtension(Queue)->DeviceExtension;
    //�Ӷ��ж������豸��չ
    NTSTATUS               Status = STATUS_INVALID_PARAMETER;
    WDF_REQUEST_PARAMETERS Parameters;
    LARGE_INTEGER          ByteOffset;
    WDFMEMORY              hMemory;

    //__analysis_assume(Length > 0);
    //������0
    WDF_REQUEST_PARAMETERS_INIT(&Parameters);
    //�������л�ȡ��Ϣ
    WdfRequestGetParameters(Request, &Parameters);
    //��ȡƫ��
    ByteOffset.QuadPart = Parameters.Parameters.Read.DeviceOffset;
    //����ȡ�Ƿ�Ϸ�
    if (RamDiskCheckParameters(devExt, ByteOffset, Length))
    {
        //����ڴ�״̬
        Status = WdfRequestRetrieveOutputMemory(Request, &hMemory);
        if(NT_SUCCESS(Status))
        {
            // Destination #��Offset into the destination ����source
            Status = WdfMemoryCopyFromBuffer(hMemory,0, devExt->DiskImage + ByteOffset.LowPart, Length);
        }
    }

    WdfRequestCompleteWithInformation(Request, Status, (ULONG_PTR)Length);
}
// д����
VOID RamDiskEvtIoWrite(IN WDFQUEUE Queue,IN WDFREQUEST Request,IN size_t Length)
{
    PDEVICE_EXTENSION      devExt = QueueGetExtension(Queue)->DeviceExtension;
    //�Ӷ��ж������豸��չ
    NTSTATUS               Status = STATUS_INVALID_PARAMETER;
    WDF_REQUEST_PARAMETERS Parameters;
    LARGE_INTEGER          ByteOffset;
    WDFMEMORY              hMemory;

    //__analysis_assume(Length > 0);
    //������0
    WDF_REQUEST_PARAMETERS_INIT(&Parameters);
    //�������л�ȡ��Ϣ
    WdfRequestGetParameters(Request, &Parameters);
    //д��ƫ��
    ByteOffset.QuadPart = Parameters.Parameters.Write.DeviceOffset;
    //���д���Ƿ�Ϸ�
    if (RamDiskCheckParameters(devExt, ByteOffset, Length))
    {
        // from output to input
        Status = WdfRequestRetrieveInputMemory(Request, &hMemory);
        //����ڴ�״̬
        if(NT_SUCCESS(Status))
        {
            // Source # offset in Source memory where the copy has to start # destination
            Status = WdfMemoryCopyToBuffer(hMemory, 0,devExt->DiskImage + ByteOffset.LowPart, Length);
            // from "from" to "to"
        }

    }

    WdfRequestCompleteWithInformation(Request, Status, (ULONG_PTR)Length);
}
VOID RamDiskEvtIoDeviceControl(IN WDFQUEUE Queue,IN WDFREQUEST Request,IN size_t OutputBufferLength,IN size_t InputBufferLength,IN ULONG IoControlCode)
{
    NTSTATUS          Status = STATUS_INVALID_DEVICE_REQUEST;
    ULONG_PTR         information = 0;
    size_t            bufSize;
    PDEVICE_EXTENSION devExt = QueueGetExtension(Queue)->DeviceExtension;
    //�Ӷ��ж������豸��չ
    UNREFERENCED_PARAMETER(OutputBufferLength);
    UNREFERENCED_PARAMETER(InputBufferLength);

    switch (IoControlCode)
    {
        case IOCTL_DISK_GET_PARTITION_INFO:
        {

            PPARTITION_INFORMATION outputBuffer;
            PBOOT_SECTOR bootSector = (PBOOT_SECTOR) devExt->DiskImage;

            information = sizeof(PARTITION_INFORMATION);
            //��Ϣ�ĳ���

            Status = WdfRequestRetrieveOutputBuffer(Request, sizeof(PARTITION_INFORMATION), &outputBuffer, &bufSize);
            if(NT_SUCCESS(Status) )
            {
                //�������� FAT16 FAT12
                outputBuffer->PartitionType = (bootSector->bsFileSystemType[4] == '6') ? PARTITION_FAT_16 : PARTITION_FAT_12;
                //��������
                outputBuffer->BootIndicator       = FALSE;
                //��ʶ�����
                outputBuffer->RecognizedPartition = TRUE;
                //��Ϣδ��
                outputBuffer->RewritePartition    = FALSE;
                //��ʼƫ����
                outputBuffer->StartingOffset.QuadPart = 0;
                //���̴�С
                outputBuffer->PartitionLength.QuadPart = devExt->DiskRegInfo.DiskSize;
                //���ط���
                outputBuffer->HiddenSectors       = (ULONG) (1L);
                //������
                outputBuffer->PartitionNumber     = (ULONG) (-1L);

                Status = STATUS_SUCCESS;
            }
        }
        break;

        case IOCTL_DISK_GET_DRIVE_GEOMETRY:
        {
            //������Ϣ
            PDISK_GEOMETRY outputBuffer;
            //��Ϣ�ĳ���
            information = sizeof(DISK_GEOMETRY);

            Status = WdfRequestRetrieveOutputBuffer(Request, sizeof(DISK_GEOMETRY), &outputBuffer, &bufSize);
            if(NT_SUCCESS(Status) )
            {
                //�ڴ�copy
                RtlCopyMemory(outputBuffer, &(devExt->DiskGeometry), sizeof(DISK_GEOMETRY));
                Status = STATUS_SUCCESS;
            }
        }
        break;
        //����У�� �Ƿ��д
        case IOCTL_DISK_CHECK_VERIFY:
        case IOCTL_DISK_IS_WRITABLE:

        Status = STATUS_SUCCESS;
        break;
    }

    WdfRequestCompleteWithInformation(Request, Status, information);
}
VOID RamDiskEvtDeviceContextCleanup(IN WDFOBJECT Device)
{
    PDEVICE_EXTENSION pDeviceExtension = DeviceGetExtension(Device);
    //����豸��չ
    PAGED_CODE();
    //��ղ���
    if(pDeviceExtension->DiskImage)
    {
        ExFreePool(pDeviceExtension->DiskImage);
    }
}
// ����豸
NTSTATUS RamDiskEvtDeviceAdd(IN WDFDRIVER Driver,IN PWDFDEVICE_INIT DeviceInit)
{
    WDF_OBJECT_ATTRIBUTES   deviceAttributes;
    NTSTATUS                status;
    WDFDEVICE               device;
    WDF_OBJECT_ATTRIBUTES   queueAttributes;
    WDF_IO_QUEUE_CONFIG     ioQueueConfig;
    PDEVICE_EXTENSION       pDeviceExtension;
    PQUEUE_EXTENSION        pQueueContext = NULL;
    WDFQUEUE                queue;
    //�豸��
    DECLARE_CONST_UNICODE_STRING(ntDeviceName, NT_DEVICE_NAME);

    PAGED_CODE();

    UNREFERENCED_PARAMETER(Driver);

    //���ָ������״̬
    status = WdfDeviceInitAssignName(DeviceInit, &ntDeviceName);
    if (!NT_SUCCESS(status))
    {
        return status;
    }
    //ָ���豸����
    WdfDeviceInitSetDeviceType(DeviceInit, FILE_DEVICE_DISK);
    //IO����
    WdfDeviceInitSetIoType(DeviceInit, WdfDeviceIoDirect);
    //�Ƿ��ռ
    WdfDeviceInitSetExclusive(DeviceInit, FALSE);
    //��ʼ��
    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&deviceAttributes, DEVICE_EXTENSION);
    //�趨cleanup ����ָ��
    deviceAttributes.EvtCleanupCallback = RamDiskEvtDeviceContextCleanup;
    //��ȡ�����豸״̬
    status = WdfDeviceCreate(&DeviceInit, &deviceAttributes, &device);
    if (!NT_SUCCESS(status))
    {
        return status;
    }

    pDeviceExtension = DeviceGetExtension(device);

    //
    // Configure a default queue so that requests that are not
    // configure-fowarded using WdfDeviceConfigureRequestDispatching to goto
    // other queues get dispatched here.
    //
    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE (&ioQueueConfig,WdfIoQueueDispatchSequential);

    ioQueueConfig.EvtIoDeviceControl = RamDiskEvtIoDeviceControl;
    ioQueueConfig.EvtIoRead          = RamDiskEvtIoRead;
    ioQueueConfig.EvtIoWrite         = RamDiskEvtIoWrite;

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&queueAttributes, QUEUE_EXTENSION);

    status = WdfIoQueueCreate( device,&ioQueueConfig,&queueAttributes,&queue );

    if (!NT_SUCCESS(status))
    {
        return status;
    }

    //����豸��չ
    pQueueContext = QueueGetExtension(queue);

    //���ö��е��豸��չΪ���豸
    pQueueContext->DeviceExtension = pDeviceExtension;

    status = SetForwardProgressOnQueue(queue);
    if (!NT_SUCCESS(status))
    {
        return status;
    }

    //�豸��ʼ��
    pDeviceExtension->DiskRegInfo.DriveLetter.Buffer = (PWSTR) &pDeviceExtension->DriveLetterBuffer;
    pDeviceExtension->DiskRegInfo.DriveLetter.MaximumLength = sizeof(pDeviceExtension->DriveLetterBuffer);

    //��ע����ȡ��Ϣ
    RamDiskQueryDiskRegParameters(WdfDriverGetRegistryPath(WdfDeviceGetDriver(device)),&pDeviceExtension->DiskRegInfo);

    //�ڴ���� Ҫʹ�÷�ҳ�ڴ�
    pDeviceExtension->DiskImage = ExAllocatePoolWithTag(PagedPool,pDeviceExtension->DiskRegInfo.DiskSize,RAMDISK_TAG);

    if (pDeviceExtension->DiskImage)
    {
        //�ɹ�����
        UNICODE_STRING deviceName;
        UNICODE_STRING win32Name;

        RamDiskFormatDisk(pDeviceExtension);
        //��ʽ��
        status = STATUS_SUCCESS;

        //������������
        RtlInitUnicodeString(&win32Name, DOS_DEVICE_NAME);
        RtlInitUnicodeString(&deviceName, NT_DEVICE_NAME);

        //�������ӳ�ʼ��
        pDeviceExtension->SymbolicLink.Buffer = (PWSTR)&pDeviceExtension->DosDeviceNameBuffer;
        pDeviceExtension->SymbolicLink.MaximumLength = sizeof(pDeviceExtension->DosDeviceNameBuffer);
        pDeviceExtension->SymbolicLink.Length = win32Name.Length;

        RtlCopyUnicodeString(&pDeviceExtension->SymbolicLink, &win32Name);
        RtlAppendUnicodeStringToString(&pDeviceExtension->SymbolicLink,&pDeviceExtension->DiskRegInfo.DriveLetter);
        //������������
        status = WdfDeviceCreateSymbolicLink(device,&pDeviceExtension->SymbolicLink);
    }

    return status;
}
VOID RamDiskQueryDiskRegParameters(__in PWSTR RegistryPath,__in PDISK_INFO DiskRegInfo)
{

    RTL_QUERY_REGISTRY_TABLE rtlQueryRegTbl[5 + 1];   //��Ҫһ������Ϊ'\0'
    NTSTATUS                 Status;
    DISK_INFO                defDiskRegInfo;

    PAGED_CODE();

    //ASSERT(RegistryPath != NULL);

    // Ĭ��ֵ

    defDiskRegInfo.DiskSize          = DEFAULT_DISK_SIZE;
    defDiskRegInfo.RootDirEntries    = DEFAULT_ROOT_DIR_ENTRIES;
    defDiskRegInfo.SectorsPerCluster = DEFAULT_SECTORS_PER_CLUSTER;

    //Ĭ���̷�
    RtlInitUnicodeString(&defDiskRegInfo.DriveLetter, DEFAULT_DRIVE_LETTER);

    RtlZeroMemory(rtlQueryRegTbl, sizeof(rtlQueryRegTbl));

    //���ò�ѯ��

    rtlQueryRegTbl[0].Flags         = RTL_QUERY_REGISTRY_SUBKEY;
    rtlQueryRegTbl[0].Name          = L"Parameters";
    rtlQueryRegTbl[0].EntryContext  = NULL;
    rtlQueryRegTbl[0].DefaultType   = (ULONG_PTR)NULL;
    rtlQueryRegTbl[0].DefaultData   = NULL;
    rtlQueryRegTbl[0].DefaultLength = (ULONG_PTR)NULL;

    //���̲���

    rtlQueryRegTbl[1].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    rtlQueryRegTbl[1].Name          = L"DiskSize";
    rtlQueryRegTbl[1].EntryContext  = &DiskRegInfo->DiskSize;
    rtlQueryRegTbl[1].DefaultType   = REG_DWORD;
    rtlQueryRegTbl[1].DefaultData   = &defDiskRegInfo.DiskSize;
    rtlQueryRegTbl[1].DefaultLength = sizeof(ULONG);

    //�ļ�ϵͳ�����
    rtlQueryRegTbl[2].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    rtlQueryRegTbl[2].Name          = L"RootDirEntries";
    rtlQueryRegTbl[2].EntryContext  = &DiskRegInfo->RootDirEntries;
    rtlQueryRegTbl[2].DefaultType   = REG_DWORD;
    rtlQueryRegTbl[2].DefaultData   = &defDiskRegInfo.RootDirEntries;
    rtlQueryRegTbl[2].DefaultLength = sizeof(ULONG);

    //����������
    rtlQueryRegTbl[3].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    rtlQueryRegTbl[3].Name          = L"SectorsPerCluster";
    rtlQueryRegTbl[3].EntryContext  = &DiskRegInfo->SectorsPerCluster;
    rtlQueryRegTbl[3].DefaultType   = REG_DWORD;
    rtlQueryRegTbl[3].DefaultData   = &defDiskRegInfo.SectorsPerCluster;
    rtlQueryRegTbl[3].DefaultLength = sizeof(ULONG);
    //�̷�
    rtlQueryRegTbl[4].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    rtlQueryRegTbl[4].Name          = L"DriveLetter";
    rtlQueryRegTbl[4].EntryContext  = &DiskRegInfo->DriveLetter;
    rtlQueryRegTbl[4].DefaultType   = REG_SZ;
    rtlQueryRegTbl[4].DefaultData   = defDiskRegInfo.DriveLetter.Buffer;
    rtlQueryRegTbl[4].DefaultLength = 0;


    Status = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,RegistryPath,rtlQueryRegTbl,NULL,NULL);

    if (NT_SUCCESS(Status) == FALSE)
    {

        DiskRegInfo->DiskSize          = defDiskRegInfo.DiskSize;
        DiskRegInfo->RootDirEntries    = defDiskRegInfo.RootDirEntries;
        DiskRegInfo->SectorsPerCluster = defDiskRegInfo.SectorsPerCluster;
        RtlCopyUnicodeString(&DiskRegInfo->DriveLetter, &defDiskRegInfo.DriveLetter);
    }

    return;
}
NTSTATUS RamDiskFormatDisk(IN PDEVICE_EXTENSION devExt)
{

    PBOOT_SECTOR bootSector = (PBOOT_SECTOR) devExt->DiskImage;
    PUCHAR       firstFatSector; // ָ���һ��FAT���ָ��
    ULONG        rootDirEntries; // ��¼�ж��ٸ�Ŀ¼��ڵ�
    ULONG        sectorsPerCluster; // ÿ�����ж��ٸ���������
    USHORT       fatType;        // ��¼FAT�ļ�ϵͳ����, FAT12/16
    USHORT       fatEntries;     // ��¼FAT�������ж��ٸ�����
    USHORT       fatSectorCnt;   // ���ڼ�¼һ��FAT������Ҫռ�ö��ٸ�����
    PDIR_ENTRY   rootDir;        // ��Ŀ¼��ڵ�

    PAGED_CODE();
    ASSERT(sizeof(BOOT_SECTOR) == 512);
    ASSERT(devExt->DiskImage != NULL);

    RtlZeroMemory(devExt->DiskImage, devExt->DiskRegInfo.DiskSize);

    devExt->DiskGeometry.BytesPerSector = 4096;//512;  //ÿ��������512���ֽ�
    devExt->DiskGeometry.SectorsPerTrack = 32;  // ÿ���ŵ���32������
    devExt->DiskGeometry.TracksPerCylinder = 2; // ÿ�������������ŵ�

    //������
    //devExt->DiskGeometry.Cylinders.QuadPart = devExt->DiskRegInfo.DiskSize / 512 / 32 / 2;
    devExt->DiskGeometry.Cylinders.QuadPart = devExt->DiskRegInfo.DiskSize / 4096 / 32 / 2;
    //���̵�����
    devExt->DiskGeometry.MediaType = RAMDISK_MEDIA_TYPE;

    rootDirEntries = devExt->DiskRegInfo.RootDirEntries;
    sectorsPerCluster = devExt->DiskRegInfo.SectorsPerCluster;

    //�ռ����

    if (rootDirEntries & (DIR_ENTRIES_PER_SECTOR - 1))
    {
        rootDirEntries = (rootDirEntries + (DIR_ENTRIES_PER_SECTOR - 1)) &~ (DIR_ENTRIES_PER_SECTOR - 1);
    }

    //�ļ�ϵͳ�ļ����

    bootSector->bsJump[0] = 0xeb;
    bootSector->bsJump[1] = 0x3c;
    bootSector->bsJump[2] = 0x90;

    //
    // Set OemName to "RajuRam "
    // NOTE: Fill all 8 characters, eg. sizeof(bootSector->bsOemName);
    //
    bootSector->bsOemName[0] = 'R';
    bootSector->bsOemName[1] = 'a';
    bootSector->bsOemName[2] = 'j';
    bootSector->bsOemName[3] = 'u';
    bootSector->bsOemName[4] = 'R';
    bootSector->bsOemName[5] = 'a';
    bootSector->bsOemName[6] = 'm';
    bootSector->bsOemName[7] = ' ';

    bootSector->bsBytesPerSec = (SHORT)devExt->DiskGeometry.BytesPerSector;
    bootSector->bsResSectors  = 1;
    bootSector->bsFATs        = 1;
    bootSector->bsRootDirEnts = (USHORT)rootDirEntries;

//    bootSector->bsSectors     = (USHORT)(devExt->DiskRegInfo.DiskSize /
//                                         devExt->DiskGeometry.BytesPerSector);
//    bootSector->bsMedia       = (UCHAR)devExt->DiskGeometry.MediaType;
//    bootSector->bsSecPerClus  = (UCHAR)sectorsPerCluster;


    if( devExt->DiskRegInfo.DiskSize/devExt->DiskGeometry.BytesPerSector < 65536 )
    {
        bootSector->bsSectors = (USHORT)(devExt->DiskRegInfo.DiskSize / devExt->DiskGeometry.BytesPerSector);
    }
    else
    {
        bootSector->bsSectors = 0;
        bootSector->bsHugeSectors = devExt->DiskRegInfo.DiskSize / devExt->DiskGeometry.BytesPerSector;
    }
    bootSector->bsMedia       = (UCHAR)devExt->DiskGeometry.MediaType;
    bootSector->bsSecPerClus  = (UCHAR)sectorsPerCluster;
    //
    // Calculate number of sectors required for FAT
    //

//    fatEntries =
//        (bootSector->bsSectors - bootSector->bsResSectors -
//         bootSector->bsRootDirEnts / DIR_ENTRIES_PER_SECTOR) /
//        bootSector->bsSecPerClus + 2;
    if(bootSector->bsSectors)
    {
        fatEntries = (bootSector->bsSectors - bootSector->bsResSectors - bootSector->bsRootDirEnts / DIR_ENTRIES_PER_SECTOR) /bootSector->bsSecPerClus + 2;
    }
    else
    {
        fatEntries =(USHORT)((bootSector->bsHugeSectors - bootSector->bsResSectors -bootSector->bsRootDirEnts / DIR_ENTRIES_PER_SECTOR) /bootSector->bsSecPerClus + 2);
    }
    //
    // Choose between 12 and 16 bit FAT based on number of clusters we
    // need to map
    //

    if (fatEntries > 4087)
    {
        fatType =  16;
        //fatSectorCnt = (fatEntries * 2 + 511) / 512;
        fatSectorCnt = (fatEntries * 2 + 4096-1) / 4096;
        fatEntries   = fatEntries + fatSectorCnt;
        //fatSectorCnt = (fatEntries * 2 + 511) / 512;
        fatSectorCnt = (fatEntries * 2 + 4096-1) / 4096;
    }
    else
    {
        fatType =  12;
        //fatSectorCnt = (((fatEntries * 3 + 1) / 2) + 511) / 512;
        fatSectorCnt = (((fatEntries * 3 + 1) / 2) + 4096-1) / 4096;
        fatEntries   = fatEntries + fatSectorCnt;
        //fatSectorCnt = (((fatEntries * 3 + 1) / 2) + 511) / 512;
        fatSectorCnt = (((fatEntries * 3 + 1) / 2) + 4096-1) / 4096;
    }

    bootSector->bsFATsecs       = fatSectorCnt;
    bootSector->bsSecPerTrack   = (USHORT)devExt->DiskGeometry.SectorsPerTrack;
    bootSector->bsHeads         = (USHORT)devExt->DiskGeometry.TracksPerCylinder;
    bootSector->bsBootSignature = 0x29;
    bootSector->bsVolumeID      = 0x12345678;

    //
    // Set Label to "RamDisk    "
    // NOTE: Fill all 11 characters, eg. sizeof(bootSector->bsLabel);
    //
    bootSector->bsLabel[0]  = 'R';
    bootSector->bsLabel[1]  = 'a';
    bootSector->bsLabel[2]  = 'm';
    bootSector->bsLabel[3]  = 'D';
    bootSector->bsLabel[4]  = 'i';
    bootSector->bsLabel[5]  = 's';
    bootSector->bsLabel[6]  = 'k';
    bootSector->bsLabel[7]  = ' ';
    bootSector->bsLabel[8]  = ' ';
    bootSector->bsLabel[9]  = ' ';
    bootSector->bsLabel[10] = ' ';

    //�ļ�ϵͳ��
    bootSector->bsFileSystemType[0] = 'F';
    bootSector->bsFileSystemType[1] = 'A';
    bootSector->bsFileSystemType[2] = 'T';
    bootSector->bsFileSystemType[3] = '1';
    bootSector->bsFileSystemType[4] = '?';
    bootSector->bsFileSystemType[5] = ' ';
    bootSector->bsFileSystemType[6] = ' ';
    bootSector->bsFileSystemType[7] = ' ';

    bootSector->bsFileSystemType[4] = ( fatType == 16 ) ? '6' : '2';

    bootSector->bsSig2[0] = 0x55;
    bootSector->bsSig2[1] = 0xAA;

    //
    // The FAT is located immediately following the boot sector.
    //

    firstFatSector    = (PUCHAR)(bootSector + 1);
    firstFatSector[0] = (UCHAR)devExt->DiskGeometry.MediaType;
    firstFatSector[1] = 0xFF;
    firstFatSector[2] = 0xFF;

    if (fatType == 16)
    {
        firstFatSector[3] = 0xFF;
    }

    //��ڵ�
    rootDir = (PDIR_ENTRY)(bootSector + 1 + fatSectorCnt);

    //���
    rootDir->deName[0] = 'M';
    rootDir->deName[1] = 'S';
    rootDir->deName[2] = '-';
    rootDir->deName[3] = 'R';
    rootDir->deName[4] = 'A';
    rootDir->deName[5] = 'M';
    rootDir->deName[6] = 'D';
    rootDir->deName[7] = 'R';

    //�豸��չ��
    rootDir->deExtension[0] = 'I';
    rootDir->deExtension[1] = 'V';
    rootDir->deExtension[2] = 'E';

    rootDir->deAttributes = DIR_ATTR_VOLUME;

    return STATUS_SUCCESS;
}
BOOLEAN RamDiskCheckParameters(IN PDEVICE_EXTENSION devExt,IN LARGE_INTEGER ByteOffset,IN size_t Length)
{
    //������̵Ĵ�СС�ڶ�ȡ�ĳ���
    if( devExt->DiskRegInfo.DiskSize < Length)
        return FALSE;
    // ƫ��С��0
    if(ByteOffset.QuadPart < 0)
        return FALSE;
    //ƫ�ƴ��ڴ��̴�С+��ȡ�ĳ���
    if ((ULONGLONG)ByteOffset.QuadPart > (devExt->DiskRegInfo.DiskSize - Length))
        return FALSE;
    //��ȡ������û�а�����������
    if( (Length & (devExt->DiskGeometry.BytesPerSector - 1)))
        return FALSE;
    return TRUE;
}
