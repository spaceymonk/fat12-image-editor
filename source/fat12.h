#ifndef __FAT12_H
#define __FAT12_H
#include <stdint.h>

struct VOLUME_BOOT_RECORD{
    uint8_t OEM_LABEL[8];
    uint16_t BytesPerSector;
    uint8_t SectorsPerCluster;
    uint16_t ReservedSectorCount;
    uint8_t NumberOfFATs;
    uint16_t MaxRootDirEntries;
    uint16_t TotalSectors16;
    uint8_t MediaDesc;
    uint16_t SectorsPerFAT;
    uint16_t SectorsPerTrack;
    uint16_t NumberOfHeads;
    uint32_t HiddenSectors;
    uint32_t TotalSectors32;
    uint8_t DriveNumber;
    uint8_t UNUSED;
    uint8_t ExtBootSign;
    uint32_t SerialNumber;
    uint8_t VolumeLabel[11];
    uint8_t FileSystem[8];
};

struct ROOT_DIR_ENTRY{
    uint8_t FileName[11];
    uint8_t FileAttr;
    uint8_t ReservedNT;
    uint8_t CreateTimeRes;
    uint16_t CreateTime;
    uint16_t CreateDate;
    uint16_t LastAccessDate;
    uint16_t EA_Index;
    uint16_t LastModifiedTime;
    uint16_t LastModifiedDate;
    uint16_t FirstCluster;
    uint32_t FileSize;
};

struct FAT_DATE{
	uint16_t year;
	uint8_t month;
	uint8_t day;
};

struct FAT_TIME{
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
};

#endif // _STRUCT_VBR
