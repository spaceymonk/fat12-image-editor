#ifndef _FAT_12_SRC
#define _FAT_12_SRC

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "fat12.h"

#define ERROR 1
#define SUCCESS 0


int isfat12(struct VOLUME_BOOT_RECORD* volume_id) {
    float f_RootDirSectors = ((volume_id->MaxRootDirEntries * 32) + (volume_id->BytesPerSector-1)) / volume_id->BytesPerSector;
    int RootDirSectors = ceil(f_RootDirSectors);
    int DataSec;

    if(volume_id->TotalSectors16 != 0) DataSec= volume_id->TotalSectors16 - (volume_id->ReservedSectorCount + (volume_id->NumberOfFATs * volume_id->SectorsPerFAT) + RootDirSectors);
    else DataSec= volume_id->TotalSectors32 - (volume_id->ReservedSectorCount + (volume_id->NumberOfFATs * volume_id->SectorsPerFAT) + RootDirSectors);

    float f_CountOfClusters = DataSec / volume_id->SectorsPerCluster;
    int CountOfClusters = floor(f_CountOfClusters);

    if(CountOfClusters>0 && CountOfClusters<4085)
        return 1;
    else
        return 0;
}


uint32_t fatsize(struct VOLUME_BOOT_RECORD* volume_id) {
    float f_RootDirSectors = ((volume_id->MaxRootDirEntries * 32) + (volume_id->BytesPerSector-1)) / volume_id->BytesPerSector;
    int RootDirSectors = ceil(f_RootDirSectors);
    uint32_t DskSize;

    if(volume_id->TotalSectors16 != 0) DskSize = volume_id->TotalSectors16;
    else DskSize = volume_id->TotalSectors32;

    uint32_t TmpVal1 = DskSize - (volume_id->ReservedSectorCount + RootDirSectors);
    uint32_t TmpVal2 = (256 * volume_id->SectorsPerCluster) + volume_id->NumberOfFATs;
    uint32_t FATsz = (TmpVal1 + (TmpVal2 - 1)) / TmpVal2;
    return FATsz;
}


void setCluster(FILE* img, uint16_t cluster_num, uint16_t value, struct VOLUME_BOOT_RECORD* VolumeID) {
    uint16_t cluster_offset = (cluster_num >> 1) + cluster_num;
    uint16_t data;

    if(cluster_num & 1)
        value <<= 4;

    uint32_t i;

    for(i=0; i<VolumeID->NumberOfFATs; i++) {
        if(fseek(img, (((VolumeID->ReservedSectorCount + i * VolumeID->SectorsPerFAT) * VolumeID->BytesPerSector) + cluster_offset), SEEK_SET)) {
            fprintf(stderr, "[!] Konumlama hatasi!\n");
            exit(1);
        }

        if(fread(&data, sizeof(uint16_t), 1, img)!=1) {
            fprintf(stderr, "[!] Okuma Hatasi!\n");
            exit(1);
        }

        if(cluster_num & 1) data &= 0x000f;
        else data &= 0xf000;

        data |= value;
        fseek(img, (((VolumeID->ReservedSectorCount + i * VolumeID->SectorsPerFAT) * VolumeID->BytesPerSector) + cluster_offset), SEEK_SET);

        if(fwrite(&data, sizeof(uint16_t), 1, img)!=1) {
            fprintf(stderr, "[!] Yazma hatasi!\n");
            exit(1);
        }
    }
}

uint16_t getCluster(FILE* img, uint16_t index, struct VOLUME_BOOT_RECORD* VolumeID) {
    uint16_t cluster_offset= (index >> 1) + index;
    uint16_t data;

    if(fseek(img, (VolumeID->ReservedSectorCount * VolumeID->BytesPerSector)+cluster_offset, SEEK_SET)) {
        fprintf(stderr, "[!] Konumlama hatasi!\n");
        exit(1);
    }

    if(fread(&data, sizeof(uint16_t), 1, img)!=1) {
        fprintf(stderr, "[!] Okuma Hatasi!\n");
        exit(1);
    }

    if(index & 1) data >>= 4;
    else data &= 4095;

    return data;
}

uint16_t findFreeCluster(FILE* img, struct VOLUME_BOOT_RECORD* VolumeID){
    uint32_t i, data_cluster;

    if(VolumeID->TotalSectors16 != 0) {
        data_cluster=(VolumeID->TotalSectors16 / VolumeID->SectorsPerCluster) -
                     (VolumeID->MaxRootDirEntries * 32 / (VolumeID->SectorsPerCluster * VolumeID->BytesPerSector)) -
                     (VolumeID->NumberOfFATs * VolumeID->SectorsPerFAT) - VolumeID->ReservedSectorCount;
    } else {
        data_cluster=(VolumeID->TotalSectors32 / VolumeID->SectorsPerCluster) -
                     (VolumeID->MaxRootDirEntries * 32 / (VolumeID->SectorsPerCluster * VolumeID->BytesPerSector)) -
                     (VolumeID->NumberOfFATs * VolumeID->SectorsPerFAT) - VolumeID->ReservedSectorCount;
    }

    for(i=2; i< (2+data_cluster); ++i) {
        if(getCluster(img, i, VolumeID) == 0)
            return i;
    }

    return 0;
}

uint16_t findFreeEntry(FILE* img, struct VOLUME_BOOT_RECORD* VolumeID){
    uint16_t i=0;
    uint8_t buff;
    for(i=0; i<VolumeID->MaxRootDirEntries; i++){
        fseek(img, i*32+((VolumeID->BytesPerSector*VolumeID->ReservedSectorCount)+ (VolumeID->NumberOfFATs*VolumeID->SectorsPerFAT*VolumeID->BytesPerSector)), SEEK_SET);
        if(fread(&buff, sizeof(uint8_t), 1, img) != 1) {
            fprintf(stderr, "[!] Okuma hatasi!");
            exit(1);
        };
        if(buff==0xe5 || buff==0x00)
            return i+1;
    }
    return 0;
}

uint32_t getFreeSize(FILE* img, struct VOLUME_BOOT_RECORD* VolumeID) {
    uint32_t i, c=0;
    uint32_t data_cluster;

    if(VolumeID->TotalSectors16 != 0) {
        data_cluster=(VolumeID->TotalSectors16 / VolumeID->SectorsPerCluster) -
                     (VolumeID->MaxRootDirEntries * 32 / (VolumeID->SectorsPerCluster * VolumeID->BytesPerSector)) -
                     (VolumeID->NumberOfFATs * VolumeID->SectorsPerFAT) - VolumeID->ReservedSectorCount;
    } else {
        data_cluster=(VolumeID->TotalSectors32 / VolumeID->SectorsPerCluster) -
                     (VolumeID->MaxRootDirEntries * 32 / (VolumeID->SectorsPerCluster * VolumeID->BytesPerSector)) -
                     (VolumeID->NumberOfFATs * VolumeID->SectorsPerFAT) - VolumeID->ReservedSectorCount;
    }

    for(i=2; i< (2+data_cluster); ++i) {
        if(getCluster(img, i, VolumeID) == 0)
            c += VolumeID->SectorsPerCluster;
    }

    return c;
}

int LFN_to_SFN(char* dest, char* src) {
    int i, j;
    char* ptr=strchr(src, '.');

    for(i=0; src[i]!=0; i++)
        if(isalnum(src[i])==0 && src[i]!='.')
            return ERROR;

    if(!ptr) {
        if(strlen(src)>8)
            return ERROR;

        for(i=0; i<strlen(src); i++)
            dest[i]=toupper(src[i]);

        for(; i<11; i++)
            dest[i]=' ';

        return SUCCESS;
    } else {
        if(strlen(src) > 12)
            return ERROR;

        if(ptr-src > 8)
            return ERROR;

        for(i=0; &src[i]!=ptr; i++)
            dest[i]=toupper(src[i]);

        j=i+1;

        for(; i<8; i++)
            dest[i]=' ';

        for(; src[j]!=0; i++) {
            dest[i]=toupper(src[j]);
            j++;
        }

        for(; i<11; i++)
            dest[i]=' ';

        return SUCCESS;
    }
}

int SFN_to_LFN(char* dest, char* src) {
    int i, j;

    for(i=0; i<8; i++)
        if(src[i]==' ')
            break;
        else
            dest[i]=src[i];

    j=i;

    if(src[8]!=' ')
        dest[j]='.';
    else {
        dest[j]=0;
        return SUCCESS;
    }

    j++;

    for(; src[i]==' '; i++) {}

    for(; i<11; i++) {
        if(src[i]==' ') {
            dest[j]=0;
            return SUCCESS;
        }

        dest[j]=src[i];
        j++;
    }

    dest[j]=0;
    return SUCCESS;
}

int fileSearch(FILE* img, char* dos_file_name, struct VOLUME_BOOT_RECORD* VolumeID, struct ROOT_DIR_ENTRY* dest) {
    uint32_t i, addr_root_dir=((VolumeID->BytesPerSector*VolumeID->ReservedSectorCount)+ (VolumeID->NumberOfFATs*VolumeID->SectorsPerFAT*VolumeID->BytesPerSector));

    for(i=0; i<VolumeID->MaxRootDirEntries; i++) {
        if(fseek(img, i*32+addr_root_dir, SEEK_SET)) {
            fprintf(stderr, "[!] Konumlama Hatasi!\n");
            exit(1);
        }

        if(fread(dest, sizeof(struct ROOT_DIR_ENTRY), 1, img)!=1) {
            fprintf(stderr, "[!] Okuma Hatasi!\n");
            exit(1);
        };

        if(memcmp(dos_file_name, dest->FileName, 11)==0)
            return i+1;
    }

    return 0;
}

void fat_date_conv(struct FAT_DATE* cal, uint16_t date) {
    cal->day   = date & 0x1f;
    cal->month = (date & 0x1e0) >> 5;
    cal->year  = (date >> 9) + 1980;
}

void fat_time_conv(struct FAT_TIME* clo, uint16_t time) {
    clo->second = (time & 0x1f)*2;
    clo->minute = (time & 0x7e0) >> 5;
    clo->hour   = time >> 11;
}

uint16_t fat_struct_time_conv(struct FAT_TIME* current_time) {
    uint16_t val=0;
    val = val | (current_time->second >> 1);
    val = val | (current_time->minute << 5);
    val = val | (current_time->hour << 11);
    return val;
}

uint16_t fat_struct_date_conv(struct FAT_DATE* current_date) {
    uint16_t val=0;
    val = val | (current_date->day);
    val = val | ((current_date->month)<<5);
    val = val | (((current_date->year)-1980)<<9);
    return val;
}

uint16_t real_time_conv(struct tm * current_time){
	uint16_t val=0;
	val = val | (current_time->tm_sec >> 1);
	val = val | (current_time->tm_min << 5);
	val = val | (current_time->tm_hour << 11);
	return val;
}

uint16_t real_date_conv(struct tm * current_date){
	uint16_t val=0;
	val = val | (current_date->tm_mday);
	val = val | ((current_date->tm_mon+1)<<5);
	val = val | ((current_date->tm_year-80)<<9);
	return val;
}

#endif // _FAT_12_SRC
