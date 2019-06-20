#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "fat12.h"
extern uint16_t getCluster(FILE *img, uint16_t index, struct VOLUME_BOOT_RECORD* VolumeID);
extern void setCluster(FILE* img, uint16_t cluster_num, uint16_t value, struct VOLUME_BOOT_RECORD* VolumeID);
extern int isfat12(struct VOLUME_BOOT_RECORD* volume_id);
extern int fileSearch(FILE* img, char* dos_file_name, struct VOLUME_BOOT_RECORD* VolumeID, struct ROOT_DIR_ENTRY* dest);
extern int LFN_to_SFN(char* dest, char* src);
extern int SFN_to_LFN(char* dest, char* src);
extern uint16_t fat_struct_date_conv(struct FAT_DATE* current_date);
extern uint16_t fat_struct_time_conv(struct FAT_TIME* current_time);
extern void fat_date_conv(struct FAT_DATE* cal, uint16_t date);
extern void fat_time_conv(struct FAT_TIME* clo, uint16_t time);
extern uint16_t getCluster(FILE* img, uint16_t index, struct VOLUME_BOOT_RECORD* VolumeID);
extern uint16_t real_date_conv(struct tm * current_date);
extern uint16_t real_time_conv(struct tm * current_time);
extern uint16_t findFreeEntry(FILE* img, struct VOLUME_BOOT_RECORD* VolumeID);
extern uint16_t findFreeCluster(FILE* img, struct VOLUME_BOOT_RECORD* VolumeID);
extern uint32_t getFreeSize(FILE* img, struct VOLUME_BOOT_RECORD* VolumeID);

struct VOLUME_BOOT_RECORD VolumeID;

void insertFile(FILE *img, FILE *fp, char* fname_dos, struct ROOT_DIR_ENTRY *entry, uint16_t index, uint32_t fsize){
    time_t t = time(NULL);
    struct tm * cur_time = localtime(&t);
    uint8_t buff;
    uint16_t cluster, cluster_bak;
    uint32_t i, j, addr_root_dir;

    time(&t);
    cur_time=localtime(&t);
    memcpy(entry->FileName, fname_dos, 11);
    entry->FileAttr=0x00;
    entry->FileSize=fsize;
    entry->EA_Index=0x0000;
    entry->ReservedNT=0x00;
    entry->CreateTimeRes=0x00;
    entry->CreateDate=real_date_conv(cur_time);
    entry->CreateTime=real_time_conv(cur_time);
    entry->LastAccessDate=real_date_conv(cur_time);
    entry->LastModifiedDate=real_date_conv(cur_time);
    entry->LastModifiedTime=real_time_conv(cur_time);
    cluster=entry->FirstCluster=findFreeCluster(img, &VolumeID);
    fseek(img, (index*32+((VolumeID.BytesPerSector*VolumeID.ReservedSectorCount)+ (VolumeID.NumberOfFATs*VolumeID.SectorsPerFAT*VolumeID.BytesPerSector))), SEEK_SET);
    fwrite(entry, sizeof(struct ROOT_DIR_ENTRY), 1, img);
    addr_root_dir=((VolumeID.BytesPerSector*VolumeID.ReservedSectorCount)+ (VolumeID.NumberOfFATs*VolumeID.SectorsPerFAT*VolumeID.BytesPerSector));
    rewind(fp);
    for(i=0; i<fsize;){
        for(j=0; j<VolumeID.BytesPerSector*VolumeID.SectorsPerCluster && j+i != fsize; j++){
            if(fread(&buff, sizeof(uint8_t), 1, fp) != 1) {
                fprintf(stderr, "[!] Okuma hatasi!");
                exit(1);
            };
            fseek(img, j+(VolumeID.SectorsPerCluster*VolumeID.BytesPerSector*(cluster-2)+(addr_root_dir+VolumeID.MaxRootDirEntries*32)), SEEK_SET);
            fwrite(&buff, sizeof(uint8_t), 1, img);
        }
        i += j;
        if(i==fsize)
            /*if(fsize > VolumeID.BytesPerSector*VolumeID.SectorsPerCluster)*/
                setCluster(img, cluster, 0xfff, &VolumeID);
        else{
            setCluster(img, cluster, cluster, &VolumeID);
            cluster_bak = findFreeCluster(img, &VolumeID);
            setCluster(img, cluster, cluster_bak, &VolumeID);
            cluster=cluster_bak;
        }
    }
}

int deleteFile(FILE* img, char* dos_name) {
    struct ROOT_DIR_ENTRY entry;
    uint8_t sign=0xE5;
    uint16_t cluster, tmp;
    uint32_t i;
    if((i=fileSearch(img, dos_name, &VolumeID, &entry)) == 0) {
        fprintf(stderr, "[!] Dosya bulunamadi!\n");
        exit(1);
    }
    --i;
    fseek(img, (i*32+((VolumeID.BytesPerSector*VolumeID.ReservedSectorCount)+ (VolumeID.NumberOfFATs*VolumeID.SectorsPerFAT*VolumeID.BytesPerSector))), SEEK_SET);
    fwrite(&sign, sizeof(uint8_t), 1, img);
    cluster = entry.FirstCluster;
    while((cluster >= 0x002 && cluster <= 0xfef)) {
        tmp=getCluster(img, cluster, &VolumeID);
        setCluster(img, cluster, 0x000, &VolumeID);
        cluster=tmp;
    }
    return 0;
}

void writeImage(char** params) {
    FILE *fp, *img;
    struct ROOT_DIR_ENTRY entry;
    struct FAT_DATE fat_date;
    struct FAT_TIME fat_time;
    char fname_dos[11], fname_long[16];
    char tmp[32], *ptr;
    uint32_t index, fsize;

    if((img=fopen(params[2], "r+b"))==NULL) {
        perror("[!]");
        exit(1);
    }

    if(fseek(img, 3, SEEK_SET)) {
        fprintf(stderr, "[!] Konumlama hatasi!\n");
        exit(1);
    };

    if(fread(&VolumeID.OEM_LABEL, sizeof(uint8_t), 8, img) != 8) {
        fprintf(stderr, "[!] Okuma hatasi!\n");
        exit(1);
    };

    if(fread(&VolumeID.BytesPerSector, sizeof(uint16_t), 1, img) != 1) {
        fprintf(stderr, "[!] Okuma hatasi!");
        exit(1);
    };

    if(fread(&VolumeID.SectorsPerCluster, sizeof(uint8_t), 1, img) != 1) {
        fprintf(stderr, "[!] Okuma hatasi!");
        exit(1);
    };

    if(fread(&VolumeID.ReservedSectorCount, sizeof(uint16_t), 1, img) != 1) {
        fprintf(stderr, "[!] Okuma hatasi!");
        exit(1);
    };

    if(fread(&VolumeID.NumberOfFATs, sizeof(uint8_t), 1, img) != 1) {
        fprintf(stderr, "[!] Okuma hatasi!");
        exit(1);
    };

    if(fread(&VolumeID.MaxRootDirEntries, sizeof(uint16_t), 1, img) != 1) {
        fprintf(stderr, "[!] Okuma hatasi!");
        exit(1);
    };

    if(fread(&VolumeID.TotalSectors16, sizeof(uint16_t), 1, img) != 1) {
        fprintf(stderr, "[!] Okuma hatasi!");
        exit(1);
    };

    if(fread(&VolumeID.MediaDesc, sizeof(uint8_t), 1, img) != 1) {
        fprintf(stderr, "[!] Okuma hatasi!");
        exit(1);
    };

    if(fread(&VolumeID.SectorsPerFAT, sizeof(uint16_t), 1, img) != 1) {
        fprintf(stderr, "[!] Okuma hatasi!");
        exit(1);
    };

    if(fread(&VolumeID.SectorsPerTrack, sizeof(uint16_t), 1, img) != 1) {
        fprintf(stderr, "[!] Okuma hatasi!");
        exit(1);
    };

    if(fread(&VolumeID.NumberOfHeads, sizeof(uint16_t), 1, img) != 1) {
        fprintf(stderr, "[!] Okuma hatasi!");
        exit(1);
    };

    if(fread(&VolumeID.HiddenSectors, sizeof(uint32_t), 1, img) != 1) {
        fprintf(stderr, "[!] Okuma hatasi!");
        exit(1);
    };

    if(fread(&VolumeID.TotalSectors32, sizeof(uint32_t), 1, img) != 1) {
        fprintf(stderr, "[!] Okuma hatasi!");
        exit(1);
    };

    if(fread(&VolumeID.DriveNumber, sizeof(uint8_t), 1, img) != 1) {
        fprintf(stderr, "[!] Okuma hatasi!");
        exit(1);
    };

    if(fread(&VolumeID.UNUSED, sizeof(uint8_t), 1, img) != 1) {
        fprintf(stderr, "[!] Okuma hatasi!");
        exit(1);
    };

    if(fread(&VolumeID.ExtBootSign, sizeof(uint8_t), 1, img) != 1) {
        fprintf(stderr, "[!] Okuma hatasi!");
        exit(1);
    };

    if(fread(&VolumeID.SerialNumber, sizeof(uint32_t), 1, img) != 1) {
        fprintf(stderr, "[!] Okuma hatasi!");
        exit(1);
    };

    if(fread(&VolumeID.VolumeLabel, sizeof(uint8_t), 11, img) != 11) {
        fprintf(stderr, "[!] Okuma hatasi!");
        exit(1);
    };

    if(fread(&VolumeID.FileSystem, sizeof(uint8_t), 8, img) != 8) {
        fprintf(stderr, "[!] Okuma hatasi!");
        exit(1);
    };

    if(!isfat12(&VolumeID)) {
        fprintf(stderr, "[!] HATA: Bilinmeyen dosya sistemi! Program sonlandiriliyor...");
        exit(1);
    }

    switch(params[3][1]) {
    case 'i':
        if(LFN_to_SFN(fname_dos, params[4]) == 1) {
            fprintf(stderr, "[!] Gecersiz dosya adi!\n");
            exit(1);
        }
        if(fileSearch(img, fname_dos, &VolumeID, &entry)!=0){
            fprintf(stderr, "[!] Ayni isimde dosya bulunuyor. Uzerine yazmak icin \'-I\' parametresini kullanin.\n");
            exit(1);
        }
        if((index=findFreeEntry(img, &VolumeID))==0){
            fprintf(stderr, "[!] Bos girdi bulunamadi!\n");
            exit(1);
        }
        --index;
        if((fp=fopen(params[4], "rb"))==NULL){
            perror("[!]");
            exit(1);
        }
        fseek(fp, 0, SEEK_END);
        fsize=ftell(fp);
        if(fsize > getFreeSize(img, &VolumeID)*VolumeID.BytesPerSector){
            fprintf(stderr, "[!] Bos alan yok!\n");
            exit(1);
        }
        insertFile(img, fp, fname_dos, &entry, index, fsize);
        break;
    case 'm':
        if(LFN_to_SFN(fname_dos, params[4]) == 1) {
            fprintf(stderr, "[!] Gecersiz dosya adi!\n");
            exit(1);
        }
        if((index = fileSearch(img, fname_dos, &VolumeID, &entry)) == 0) {
            fprintf(stderr, "[!] Dosya bulunamadi!\n");
            exit(1);
        }
        SFN_to_LFN(fname_long, fname_dos);
        fprintf(stdout, "Yeni dosya adini girin [%s]: ", fname_long);
        if(fgets(fname_long, 16, stdin)==NULL){
            perror("[!]");
            exit(1);
        }
        if(fname_long[0]!='\n') {
            fname_long[strlen(fname_long)-1]=0;
            if(LFN_to_SFN((char*)entry.FileName, fname_long) == 1) {
                fprintf(stderr, "[!] Gecersiz dosya adi!\n");
                exit(1);
            }
        }
        fprintf(stdout, "Oznitelikleri belirtin [0x%02x]: ", entry.FileAttr);
        if(fgets(tmp, 32, stdin)==NULL) {
            perror("[!] HATA");
            exit(1);
        }
        if(tmp[0]!='\n')
            sscanf(tmp, "%x", (unsigned int*)&entry.FileAttr);

        fat_date_conv(&fat_date, entry.CreateDate);
        fprintf(stdout, "Olusturulma tarihini girin (GG/AA/YYYY formatinda) [%02d/%02d/%04d]: ", fat_date.day, fat_date.month, fat_date.year);
        if(fgets(tmp, 32, stdin)==NULL) {
            perror("[!] HATA");
            exit(1);
        }        if(tmp[0]!='\n') {
            ptr=tmp;
            fat_date.day = atoi(ptr);
            while(*(++ptr)!='/');
            ptr++;
            fat_date.month = atoi(ptr);
            while(*(++ptr)!='/');
            ptr++;
            fat_date.year = atoi(ptr);
            entry.CreateDate=fat_struct_date_conv(&fat_date);
        }
        fat_time_conv(&fat_time, entry.CreateTime);
        fprintf(stdout, "Olusturulma saatini girin (Saat:Dakika:Saniye formatinda) [%02d:%02d:%02d]: ", fat_time.hour, fat_time.minute, fat_time.second);
        if(fgets(tmp, 32, stdin)==NULL) {
            perror("[!] HATA");
            exit(1);
        }
        if(tmp[0]!='\n') {
            ptr=tmp;
            fat_time.hour = atoi(ptr);
            while(*(++ptr)!=':');
            ptr++;
            fat_time.minute = atoi(ptr);
            while(*(++ptr)!=':');
            ptr++;
            fat_time.second = atoi(ptr);
            entry.CreateTime=fat_struct_time_conv(&fat_time);
        }

        fat_date_conv(&fat_date, entry.LastModifiedDate);
        fprintf(stdout, "Degistirilme tarihini girin (GG/AA/YYYY formatinda) [%02d/%02d/%04d]: ", fat_date.day, fat_date.month, fat_date.year);
        if(fgets(tmp, 32, stdin)==NULL) {
            perror("[!] HATA");
            exit(1);
        }
        if(tmp[0]!='\n') {
            ptr=tmp;
            fat_date.day = atoi(ptr);
            while(*(++ptr)!='/');
            ptr++;
            fat_date.month = atoi(ptr);
            while(*(++ptr)!='/');
            ptr++;
            fat_date.year = atoi(ptr);
            entry.LastModifiedDate=fat_struct_date_conv(&fat_date);
        }
        fat_time_conv(&fat_time, entry.LastModifiedTime);
        fprintf(stdout, "Degistirilme saatini girin (Saat:Dakika:Saniye formatinda) [%02d:%02d:%02d]: ", fat_time.hour, fat_time.minute, fat_time.second);
        if(fgets(tmp, 32, stdin)==NULL) {
            perror("[!] HATA");
            exit(1);
        }
        if(tmp[0]!='\n') {
            ptr=tmp;
            fat_time.hour = atoi(ptr);
            while(*(++ptr)!=':');
            ptr++;
            fat_time.minute = atoi(ptr);
            while(*(++ptr)!=':');
            ptr++;
            fat_time.second = atoi(ptr);
            entry.LastModifiedTime=fat_struct_time_conv(&fat_time);
        }

        fat_date_conv(&fat_date, entry.LastAccessDate);
        fprintf(stdout, "Erisim tarihini girin (GG/AA/YYYY formatinda) [%02d/%02d/%04d]: ", fat_date.day, fat_date.month, fat_date.year);
        if(fgets(tmp, 32, stdin)==NULL) {
            perror("[!] HATA");
            exit(1);
        }
        if(tmp[0]!='\n') {
            ptr=tmp;
            fat_date.day = atoi(ptr);
            while(*(++ptr)!='/');
            ptr++;
            fat_date.month = atoi(ptr);
            while(*(++ptr)!='/');
            ptr++;
            fat_date.year = atoi(ptr);
            entry.LastAccessDate=fat_struct_date_conv(&fat_date);
        }

        fprintf(stdout, "Veriler imaja kaydediliyor..\n");
        --index;
        fseek(img, (index*32+((VolumeID.BytesPerSector*VolumeID.ReservedSectorCount)+ (VolumeID.NumberOfFATs*VolumeID.SectorsPerFAT*VolumeID.BytesPerSector))), SEEK_SET);
        fwrite(&entry, sizeof(struct ROOT_DIR_ENTRY), 1, img);
        fprintf(stdout, "Basarili!\n");
        break;
    case 'I':
        if(LFN_to_SFN(fname_dos, params[4]) == 1){
            fprintf(stderr, "[!] Gecersiz dosya adi!\n");
            exit(1);
        }
        if((index = fileSearch(img, fname_dos, &VolumeID, &entry)) != 0) {
            --index;
            if((fp=fopen(params[4], "rb"))==NULL){
                perror("[!]");
                exit(1);
            }
            if(deleteFile(img, fname_dos)== 1){
                fprintf(stderr, "HATA: Dosya silinemedi!\n");
                exit(1);
            }
            fseek(fp, 0, SEEK_END);
            fsize=ftell(fp);
            if(fsize > getFreeSize(img, &VolumeID)*VolumeID.BytesPerSector){
                fprintf(stderr, "[!] Bos alan yok!\n");
                exit(1);
            }
            insertFile(img, fp, fname_dos, &entry, index, fsize);
        }else{
            if((index=findFreeEntry(img, &VolumeID))==0){
                fprintf(stderr, "[!] Bos girdi bulunamadi!\n");
                exit(1);
            }
            --index;
            if((fp=fopen(params[4], "rb"))==NULL){
                perror("[!]");
                exit(1);
            }
            fseek(fp, 0, SEEK_END);
            fsize=ftell(fp);
            if(fsize > getFreeSize(img, &VolumeID)*VolumeID.BytesPerSector){
                fprintf(stderr, "[!] Bos alan yok!\n");
                exit(1);
            }
            insertFile(img, fp, fname_dos, &entry, index, fsize);
        }
        fclose(fp);
        fclose(img);
        break;
    case 'd':
        if(LFN_to_SFN(fname_dos, params[4]) == 1) {
            fprintf(stderr, "[!] Gecersiz dosya adi!\n");
            exit(1);
        }
        if(deleteFile(img, fname_dos) == 1) {
            fprintf(stderr, "[!] Dosya silinemedi!\n");
            exit(1);
        } else {
            fprintf(stdout, "Dosya basariyla silindi!\n");
        }
        fclose(img);
        break;
    default:
        fprintf(stderr, "[!]: Gecersiz parametre!\n");
        exit(1);
    }
}
