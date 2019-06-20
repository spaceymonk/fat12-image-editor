#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include "fat12.h"
#include "misc.h"

extern void setCluster(FILE* img, uint16_t cluster_num, uint16_t value, struct VOLUME_BOOT_RECORD* VolumeID);
extern int isfat12(struct VOLUME_BOOT_RECORD* volume_id);
extern uint32_t fatsize(struct VOLUME_BOOT_RECORD* volume_id);
struct VOLUME_BOOT_RECORD VolumeID;

void saveImage(FILE* img) {
    uint8_t buff[3]= {0xeb, 0xfe, 0x90};
    uint8_t tmp=0;
    uint16_t buff2=0xffff;
    uint32_t i, j;

    if(VolumeID.TotalSectors16 != 0)
        for(i=0; i<VolumeID.TotalSectors16; i++)
            for(j=0; j<VolumeID.BytesPerSector; j++)
                fwrite(&tmp, 1, 1, img);
    else
        for(i=0; i<VolumeID.TotalSectors32; i++)
            for(j=0; j<VolumeID.BytesPerSector; j++)
                fwrite(&tmp, 1, 1, img);

    if(fseek(img, 0, SEEK_SET)) {
        fprintf(stderr, "[!] Konumlama Hatasi\n");
        exit(1);
    }
    if(fwrite(buff, 1, 3, img)!=3) {
        fprintf(stderr, "[!] Yazma Hatasi\n");
        exit(1);
    }
    if(fwrite(&VolumeID.OEM_LABEL, sizeof(uint8_t), 8, img)!=8) {
        fprintf(stderr, "[!] Yazma Hatasi\n");
        exit(1);
    }
    if(fwrite(&VolumeID.BytesPerSector, sizeof(uint16_t), 1, img)!=1) {
        fprintf(stderr, "[!] Yazma Hatasi\n");
        exit(1);
    }
    if(fwrite(&VolumeID.SectorsPerCluster, sizeof(uint8_t), 1, img)!=1) {
        fprintf(stderr, "[!] Yazma Hatasi\n");
        exit(1);
    }
    if(fwrite(&VolumeID.ReservedSectorCount, sizeof(uint16_t), 1, img)!=1) {
        fprintf(stderr, "[!] Yazma Hatasi\n");
        exit(1);
    }
    if(fwrite(&VolumeID.NumberOfFATs, sizeof(uint8_t), 1, img)!=1) {
        fprintf(stderr, "[!] Yazma Hatasi\n");
        exit(1);
    }
    if(fwrite(&VolumeID.MaxRootDirEntries, sizeof(uint16_t), 1, img)!=1) {
        fprintf(stderr, "[!] Yazma Hatasi\n");
        exit(1);
    }
    if(fwrite(&VolumeID.TotalSectors16, sizeof(uint16_t), 1, img)!=1) {
        fprintf(stderr, "[!] Yazma Hatasi\n");
        exit(1);
    }
    if(fwrite(&VolumeID.MediaDesc, sizeof(uint8_t), 1, img)!=1) {
        fprintf(stderr, "[!] Yazma Hatasi\n");
        exit(1);
    }
    if(fwrite(&VolumeID.SectorsPerFAT, sizeof(uint16_t), 1, img)!=1) {
        fprintf(stderr, "[!] Yazma Hatasi\n");
        exit(1);
    }
    if(fwrite(&VolumeID.SectorsPerTrack, sizeof(uint16_t), 1, img)!=1) {
        fprintf(stderr, "[!] Yazma Hatasi\n");
        exit(1);
    }
    if(fwrite(&VolumeID.NumberOfHeads, sizeof(uint16_t), 1, img)!=1) {
        fprintf(stderr, "[!] Yazma Hatasi\n");
        exit(1);
    }
    if(fwrite(&VolumeID.HiddenSectors, sizeof(uint32_t), 1, img)!=1) {
        fprintf(stderr, "[!] Yazma Hatasi\n");
        exit(1);
    }
    if(fwrite(&VolumeID.TotalSectors32, sizeof(uint32_t), 1, img)!=1) {
        fprintf(stderr, "[!] Yazma Hatasi\n");
        exit(1);
    }
    if(fwrite(&VolumeID.DriveNumber, sizeof(uint8_t), 1, img)!=1) {
        fprintf(stderr, "[!] Yazma Hatasi\n");
        exit(1);
    }
    if(fwrite(&VolumeID.UNUSED, sizeof(uint8_t), 1, img)!=1) {
        fprintf(stderr, "[!] Yazma Hatasi\n");
        exit(1);
    }
    if(fwrite(&VolumeID.ExtBootSign, sizeof(uint8_t), 1, img)!=1) {
        fprintf(stderr, "[!] Yazma Hatasi\n");
        exit(1);
    }
    if(fwrite(&VolumeID.SerialNumber, sizeof(uint32_t), 1, img)!=1) {
        fprintf(stderr, "[!] Yazma Hatasi\n");
        exit(1);
    }
    if(fwrite(&VolumeID.VolumeLabel, sizeof(uint8_t), 11, img)!=11) {
        fprintf(stderr, "[!] Yazma Hatasi\n");
        exit(1);
    }
    if(fwrite(&VolumeID.FileSystem, sizeof(uint8_t), 8, img)!=8) {
        fprintf(stderr, "[!] Yazma Hatasi\n");
        exit(1);
    }
    if(fseek(img, 510, SEEK_SET)) {
        fprintf(stderr, "[!] Konumlama Hatasi!\n");
        exit(1);
    }
    if(fwrite(&buff2, sizeof(uint16_t), 1, img)!=1) {
        fprintf(stderr, "[!] Yazma Hatasi\n");
        exit(1);
    }
    setCluster(img, 0, 0xf00 | VolumeID.MediaDesc, &VolumeID);
    setCluster(img, 1, 0xfff, &VolumeID);
    fclose(img);
    fprintf(stdout, "Imaj olusturuldu!\n");
}

void predefinedImage(FILE* img) {
    char buff[16];
    uint32_t i;
    fprintf(stdout, "\nMevcut disket kaliplari:\n[DG: Disk Geometrisi (C/H/S formatinda)]\n");
    fprintf(stdout, "   1- 160 KB (DG: 40/1/8)\n");
    fprintf(stdout, "   2- 180 KB (DG: 40/1/9)\n");
    fprintf(stdout, "   3- 320 KB (DG: 40/2/8)\n");
    fprintf(stdout, "   4- 320 KB (DG: 80/1/8)\n");
    fprintf(stdout, "   5- 360 KB (DG: 40/2/9)\n");
    fprintf(stdout, "   6- 640 KB (DG: 80/2/8)\n");
    fprintf(stdout, "   7- 1200 KB (DG: 80/2/15)\n");
    fprintf(stdout, "   8- 1440 KB (DG: 80/2/18)\n");
    fprintf(stdout, "   9- 2880 KB (DG: 80/2/36)\n");
    fprintf(stdout, "Seciminizi yapin [8]> ");

    if(fgets(buff, 16, stdin)==NULL) {
        perror("[!] HATA");
        exit(1);
    }

    switch(buff[0]) {
    case '\n':
    case '8':
        memcpy(VolumeID.OEM_LABEL, "MSWIN4.1", 8);
        VolumeID.BytesPerSector=512;
        VolumeID.SectorsPerCluster=1;
        VolumeID.ReservedSectorCount=1;
        VolumeID.NumberOfFATs=2;
        VolumeID.MaxRootDirEntries=224;
        VolumeID.TotalSectors16=2880;
        VolumeID.MediaDesc=0xf0;
        VolumeID.SectorsPerFAT=9;
        VolumeID.SectorsPerTrack=18;
        VolumeID.NumberOfHeads=2;
        VolumeID.HiddenSectors=0;
        VolumeID.TotalSectors32=0;
        VolumeID.DriveNumber=0;
        VolumeID.UNUSED=0;
        VolumeID.ExtBootSign=0x29;
        VolumeID.SerialNumber=0x00000000;
        memcpy(VolumeID.FileSystem, "FAT12   ", 8);
        fprintf(stdout, "Disk kalibinin etiketini girin [NO NAME]> ");

        if(fgets(buff, 16, stdin)==NULL) {
            perror("[!] HATA");
            exit(1);
        }

        if(isalpha(buff[0])) {
            buff[strlen(buff)-1]=0;

            if(strlen(buff) > 11) {
                fprintf(stderr, "[!] Etiket 11 karakteri asamaz!\n");
                exit(1);
            }

            for(i=strlen(buff); i<11; i++)
                buff[i]=' ';

            memcpy(VolumeID.VolumeLabel, buff, 11);
        } else
            memcpy(VolumeID.VolumeLabel, "NO NAME    ", 11);

        saveImage(img);
        break;

    case '1':
        memcpy(VolumeID.OEM_LABEL, "MSWIN4.1", 8);
        VolumeID.BytesPerSector=512;
        VolumeID.SectorsPerCluster=1;
        VolumeID.ReservedSectorCount=1;
        VolumeID.NumberOfFATs=2;
        VolumeID.MaxRootDirEntries=224;
        VolumeID.TotalSectors16=320;
        VolumeID.MediaDesc=0xfe;
        VolumeID.SectorsPerFAT=1;
        VolumeID.SectorsPerTrack=8;
        VolumeID.NumberOfHeads=1;
        VolumeID.HiddenSectors=0;
        VolumeID.TotalSectors32=0;
        VolumeID.DriveNumber=0;
        VolumeID.UNUSED=0;
        VolumeID.ExtBootSign=0x29;
        VolumeID.SerialNumber=0x00000000;
        memcpy(VolumeID.FileSystem, "FAT12   ", 8);
        fprintf(stdout, "Disk kalibinin etiketini girin [NO NAME]> ");

        if(fgets(buff, 16, stdin)==NULL) {
            perror("[!] HATA");
            exit(1);
        }

        if(isalpha(buff[0])) {
            buff[strlen(buff)-1]=0;

            if(strlen(buff) > 11) {
                fprintf(stderr, "[!] Etiket 11 karakteri asamaz!\n");
                exit(1);
            }

            for(i=strlen(buff); i<11; i++)
                buff[i]=' ';

            memcpy(VolumeID.VolumeLabel, buff, 11);
        } else
            memcpy(VolumeID.VolumeLabel, "NO NAME    ", 11);

        saveImage(img);
        break;

    case '2':
        memcpy(VolumeID.OEM_LABEL, "MSWIN4.1", 8);
        VolumeID.BytesPerSector=512;
        VolumeID.SectorsPerCluster=1;
        VolumeID.ReservedSectorCount=1;
        VolumeID.NumberOfFATs=2;
        VolumeID.MaxRootDirEntries=224;
        VolumeID.TotalSectors16=360;
        VolumeID.MediaDesc=0xfc;
        VolumeID.SectorsPerFAT=2;
        VolumeID.SectorsPerTrack=9;
        VolumeID.NumberOfHeads=1;
        VolumeID.HiddenSectors=0;
        VolumeID.TotalSectors32=0;
        VolumeID.DriveNumber=0;
        VolumeID.UNUSED=0;
        VolumeID.ExtBootSign=0x29;
        VolumeID.SerialNumber=0x00000000;
        memcpy(VolumeID.FileSystem, "FAT12   ", 8);
        fprintf(stdout, "Disk kalibinin etiketini girin [NO NAME]> ");

        if(fgets(buff, 16, stdin)==NULL) {
            perror("[!] HATA");
            exit(1);
        }

        if(isalpha(buff[0])) {
            buff[strlen(buff)-1]=0;

            if(strlen(buff) > 11) {
                fprintf(stderr, "[!] Etiket 11 karakteri asamaz!\n");
                exit(1);
            }

            for(i=strlen(buff); i<11; i++)
                buff[i]=' ';

            memcpy(VolumeID.VolumeLabel, buff, 11);
        } else
            memcpy(VolumeID.VolumeLabel, "NO NAME    ", 11);

        saveImage(img);
        break;

    case '3':
        memcpy(VolumeID.OEM_LABEL, "MSWIN4.1", 8);
        VolumeID.BytesPerSector=512;
        VolumeID.SectorsPerCluster=1;
        VolumeID.ReservedSectorCount=1;
        VolumeID.NumberOfFATs=2;
        VolumeID.MaxRootDirEntries=224;
        VolumeID.TotalSectors16=640;
        VolumeID.MediaDesc=0xff;
        VolumeID.SectorsPerFAT=2;
        VolumeID.SectorsPerTrack=8;
        VolumeID.NumberOfHeads=2;
        VolumeID.HiddenSectors=0;
        VolumeID.TotalSectors32=0;
        VolumeID.DriveNumber=0;
        VolumeID.UNUSED=0;
        VolumeID.ExtBootSign=0x29;
        VolumeID.SerialNumber=0x00000000;
        memcpy(VolumeID.FileSystem, "FAT12   ", 8);
        fprintf(stdout, "Disk kalibinin etiketini girin [NO NAME]> ");

        if(fgets(buff, 16, stdin)==NULL) {
            perror("[!] HATA");
            exit(1);
        }

        if(isalpha(buff[0])) {
            buff[strlen(buff)-1]=0;

            if(strlen(buff) > 11) {
                fprintf(stderr, "[!] Etiket 11 karakteri asamaz!\n");
                exit(1);
            }

            for(i=strlen(buff); i<11; i++)
                buff[i]=' ';

            memcpy(VolumeID.VolumeLabel, buff, 11);
        } else
            memcpy(VolumeID.VolumeLabel, "NO NAME    ", 11);

        saveImage(img);
        break;

    case '4':
        memcpy(VolumeID.OEM_LABEL, "MSWIN4.1", 8);
        VolumeID.BytesPerSector=512;
        VolumeID.SectorsPerCluster=1;
        VolumeID.ReservedSectorCount=1;
        VolumeID.NumberOfFATs=2;
        VolumeID.MaxRootDirEntries=224;
        VolumeID.TotalSectors16=640;
        VolumeID.MediaDesc=0xfa;
        VolumeID.SectorsPerFAT=2;
        VolumeID.SectorsPerTrack=8;
        VolumeID.NumberOfHeads=1;
        VolumeID.HiddenSectors=0;
        VolumeID.TotalSectors32=0;
        VolumeID.DriveNumber=0;
        VolumeID.UNUSED=0;
        VolumeID.ExtBootSign=0x29;
        VolumeID.SerialNumber=0x00000000;
        memcpy(VolumeID.FileSystem, "FAT12   ", 8);
        fprintf(stdout, "Disk kalibinin etiketini girin [NO NAME]> ");

        if(fgets(buff, 16, stdin)==NULL) {
            perror("[!] HATA");
            exit(1);
        }

        if(isalpha(buff[0])) {
            buff[strlen(buff)-1]=0;

            if(strlen(buff) > 11) {
                fprintf(stderr, "[!] Etiket 11 karakteri asamaz!\n");
                exit(1);
            }

            for(i=strlen(buff); i<11; i++)
                buff[i]=' ';

            memcpy(VolumeID.VolumeLabel, buff, 11);
        } else
            memcpy(VolumeID.VolumeLabel, "NO NAME    ", 11);

        saveImage(img);
        break;

    case '5':
        memcpy(VolumeID.OEM_LABEL, "MSWIN4.1", 8);
        VolumeID.BytesPerSector=512;
        VolumeID.SectorsPerCluster=1;
        VolumeID.ReservedSectorCount=1;
        VolumeID.NumberOfFATs=2;
        VolumeID.MaxRootDirEntries=224;
        VolumeID.TotalSectors16=720;
        VolumeID.MediaDesc=0xfd;
        VolumeID.SectorsPerFAT=3;
        VolumeID.SectorsPerTrack=9;
        VolumeID.NumberOfHeads=2;
        VolumeID.HiddenSectors=0;
        VolumeID.TotalSectors32=0;
        VolumeID.DriveNumber=0;
        VolumeID.UNUSED=0;
        VolumeID.ExtBootSign=0x29;
        VolumeID.SerialNumber=0x00000000;
        memcpy(VolumeID.FileSystem, "FAT12   ", 8);
        fprintf(stdout, "Disk kalibinin etiketini girin [NO NAME]> ");

        if(fgets(buff, 16, stdin)==NULL) {
            perror("[!] HATA");
            exit(1);
        }

        if(isalpha(buff[0])) {
            buff[strlen(buff)-1]=0;

            if(strlen(buff) > 11) {
                fprintf(stderr, "[!] Etiket 11 karakteri asamaz!\n");
                exit(1);
            }

            for(i=strlen(buff); i<11; i++)
                buff[i]=' ';

            memcpy(VolumeID.VolumeLabel, buff, 11);
        } else
            memcpy(VolumeID.VolumeLabel, "NO NAME    ", 11);

        saveImage(img);
        break;

    case '6':
        memcpy(VolumeID.OEM_LABEL, "MSWIN4.1", 8);
        VolumeID.BytesPerSector=512;
        VolumeID.SectorsPerCluster=1;
        VolumeID.ReservedSectorCount=1;
        VolumeID.NumberOfFATs=2;
        VolumeID.MaxRootDirEntries=224;
        VolumeID.TotalSectors16=1280;
        VolumeID.MediaDesc=0xfb;
        VolumeID.SectorsPerFAT=4;
        VolumeID.SectorsPerTrack=8;
        VolumeID.NumberOfHeads=2;
        VolumeID.HiddenSectors=0;
        VolumeID.TotalSectors32=0;
        VolumeID.DriveNumber=0;
        VolumeID.UNUSED=0;
        VolumeID.ExtBootSign=0x29;
        VolumeID.SerialNumber=0x00000000;
        memcpy(VolumeID.FileSystem, "FAT12   ", 8);
        fprintf(stdout, "Disk kalibinin etiketini girin [NO NAME]> ");

        if(fgets(buff, 16, stdin)==NULL) {
            perror("[!] HATA");
            exit(1);
        }

        if(isalpha(buff[0])) {
            buff[strlen(buff)-1]=0;

            if(strlen(buff) > 11) {
                fprintf(stderr, "[!] Etiket 11 karakteri asamaz!\n");
                exit(1);
            }

            for(i=strlen(buff); i<11; i++)
                buff[i]=' ';

            memcpy(VolumeID.VolumeLabel, buff, 11);
        } else
            memcpy(VolumeID.VolumeLabel, "NO NAME    ", 11);

        saveImage(img);
        break;

    case '7':
        memcpy(VolumeID.OEM_LABEL, "MSWIN4.1", 8);
        VolumeID.BytesPerSector=512;
        VolumeID.SectorsPerCluster=1;
        VolumeID.ReservedSectorCount=1;
        VolumeID.NumberOfFATs=2;
        VolumeID.MaxRootDirEntries=224;
        VolumeID.TotalSectors16=2400;
        VolumeID.MediaDesc=0xf9;
        VolumeID.SectorsPerFAT=8;
        VolumeID.SectorsPerTrack=15;
        VolumeID.NumberOfHeads=2;
        VolumeID.HiddenSectors=0;
        VolumeID.TotalSectors32=0;
        VolumeID.DriveNumber=0;
        VolumeID.UNUSED=0;
        VolumeID.ExtBootSign=0x29;
        VolumeID.SerialNumber=0x00000000;
        memcpy(VolumeID.FileSystem, "FAT12   ", 8);
        fprintf(stdout, "Disk kalibinin etiketini girin [NO NAME]> ");

        if(fgets(buff, 16, stdin)==NULL) {
            perror("[!] HATA");
            exit(1);
        }

        if(isalpha(buff[0])) {
            buff[strlen(buff)-1]=0;

            if(strlen(buff) > 11) {
                fprintf(stderr, "[!] Etiket 11 karakteri asamaz!\n");
                exit(1);
            }

            for(i=strlen(buff); i<11; i++)
                buff[i]=' ';

            memcpy(VolumeID.VolumeLabel, buff, 11);
        } else
            memcpy(VolumeID.VolumeLabel, "NO NAME    ", 11);

        saveImage(img);
        break;

    case '9':
        memcpy(VolumeID.OEM_LABEL, "MSWIN4.1", 8);
        VolumeID.BytesPerSector=512;
        VolumeID.SectorsPerCluster=2;
        VolumeID.ReservedSectorCount=1;
        VolumeID.NumberOfFATs=2;
        VolumeID.MaxRootDirEntries=224;
        VolumeID.TotalSectors16=5760;
        VolumeID.MediaDesc=0xf0;
        VolumeID.SectorsPerFAT=9;
        VolumeID.SectorsPerTrack=36;
        VolumeID.NumberOfHeads=2;
        VolumeID.HiddenSectors=0;
        VolumeID.TotalSectors32=0;
        VolumeID.DriveNumber=0;
        VolumeID.UNUSED=0;
        VolumeID.ExtBootSign=0x29;
        VolumeID.SerialNumber=0x00000000;
        memcpy(VolumeID.FileSystem, "FAT12   ", 8);
        fprintf(stdout, "Disk kalibinin etiketini girin [NO NAME]> ");

        if(fgets(buff, 16, stdin)==NULL) {
            perror("[!] HATA");
            exit(1);
        }

        if(isalpha(buff[0])) {
            buff[strlen(buff)-1]=0;

            if(strlen(buff) > 11) {
                fprintf(stderr, "[!] Etiket 11 karakteri asamaz!\n");
                exit(1);
            }

            for(i=strlen(buff); i<11; i++)
                buff[i]=' ';

            memcpy(VolumeID.VolumeLabel, buff, 11);
        } else
            memcpy(VolumeID.VolumeLabel, "NO NAME    ", 11);

        saveImage(img);
        break;

    default:
        fprintf(stderr, "[!] Lutfen verilen aralikta bir secim yapiniz!\n");
        exit(1);
    }
}

void customImage(FILE* img) {
    char buff[16];
    uint32_t i;
    int32_t tmp;
    float MaxDataSz;
    fprintf(stdout, "\nUYARI: Program icerisinde gerceklestirilen hicbir islemin garantisi yoktur.\n");
    fprintf(stdout, "OEM etiketini girin [MSWIN4.1]: ");

    if(fgets(buff, 16, stdin)==NULL) {
        perror("[!] HATA");
        exit(1);
    }

    buff[strlen(buff)-1]=0;

    if(buff[0]==0) memcpy(VolumeID.OEM_LABEL, "MSWIN4.1", 8);
    else {
        if(strlen(buff) > 8) fprintf(stderr, "[!] OEM etiketi 8 karakteri asamaz!\n");
        else if(strlen(buff) < 8)
            for(i=strlen(buff); i<8; i++)
                buff[i]=' ';

        memcpy(VolumeID.OEM_LABEL, buff, 8);
    }

    fprintf(stdout, "Sektor boyutunu girin (bayt cinsinden) [512]: ");

    if(fgets(buff, 16, stdin)==NULL) {
        perror("[!] HATA");
        exit(1);
    }

    if(buff[0]=='\n') VolumeID.BytesPerSector=512;
    else VolumeID.BytesPerSector=atoi(buff);

    fprintf(stdout, "Cluster boyutunu girin (sektor cinsinden) [1]: ");

    if(fgets(buff, 16, stdin)==NULL) {
        perror("[!] HATA");
        exit(1);
    }

    if(buff[0]=='\n') VolumeID.SectorsPerCluster=1;
    else VolumeID.SectorsPerCluster=atoi(buff);

    VolumeID.ReservedSectorCount=1;
    fprintf(stdout, "FAT sayisini girin [2]: ");

    if(fgets(buff, 16, stdin)==NULL) {
        perror("[!] HATA");
        exit(1);
    }

    if(buff[0]=='\n') VolumeID.NumberOfFATs=2;
    else VolumeID.NumberOfFATs=atoi(buff);

    fprintf(stdout, "Kok dizini girdi sayisini girin [224]: ");

    if(fgets(buff, 16, stdin)==NULL) {
        perror("[!] HATA");
        exit(1);
    }

    if(buff[0]=='\n') VolumeID.MaxRootDirEntries=224;
    else VolumeID.MaxRootDirEntries=atoi(buff);

    fprintf(stdout, "Toplam sektor sayisini girin [max: %d sektor]: ", VolumeID.SectorsPerCluster * 4084);

    if(fgets(buff, 16, stdin)==NULL) {
        perror("[!] HATA");
        exit(1);
    }

    if(buff[0]=='\n') {
        if(VolumeID.SectorsPerCluster * 4084 <= 0xffff) {
            VolumeID.TotalSectors16=VolumeID.SectorsPerCluster * 4084;
            VolumeID.TotalSectors32=0;
        } else {
            VolumeID.TotalSectors16=0;
            VolumeID.TotalSectors32=VolumeID.SectorsPerCluster * 4084;
        }
    } else {
        if(atoi(buff) <= 0xffff) {
            VolumeID.TotalSectors16=atoi(buff);
            VolumeID.TotalSectors32=0;
        } else {
            VolumeID.TotalSectors32=atoi(buff);
            VolumeID.TotalSectors16=0;
        }
    }

    fprintf(stdout, "Media tipini girin [0xf8]: ");

    if(fgets(buff, 16, stdin)==NULL) {
        perror("[!] HATA");
        exit(1);
    }

    if(buff[0]=='\n') VolumeID.MediaDesc=0xf8;
    else sscanf(buff, "%x", (unsigned int*)&VolumeID.MediaDesc);

    fprintf(stdout, "FAT boyutunu girin (sektor cinsinden) [otomatik hesapla]: ");

    if(fgets(buff, 16, stdin)==NULL) {
        perror("[!] HATA");
        exit(1);
    }

    if(buff[0]=='\n') tmp=-1;
    else tmp=atoi(buff);

    fprintf(stdout, "Track boyutunu girin (sektor cinsinden) [0]: ");

    if(fgets(buff, 16, stdin)==NULL) {
        perror("[!] HATA");
        exit(1);
    }

    if(buff[0]=='\n') VolumeID.SectorsPerTrack=0;
    else VolumeID.SectorsPerTrack=atoi(buff);

    fprintf(stdout, "Head sayisini girin [0]: ");

    if(fgets(buff, 16, stdin)==NULL) {
        perror("[!] HATA");
        exit(1);
    }

    if(buff[0]=='\n') VolumeID.NumberOfHeads=0;
    else VolumeID.NumberOfHeads=atoi(buff);

    VolumeID.HiddenSectors=0;
    fprintf(stdout, "Disket adini girin [NO NAME]: ");

    if(fgets(buff, 16, stdin)==NULL) {
        perror("[!] HATA");
        exit(1);
    }

    if(buff[0]=='\n') memcpy(VolumeID.VolumeLabel, "NO NAME    ", 11);
    else {
        buff[strlen(buff)-1]=0;

        if(strlen(buff) > 11) {
            fprintf(stderr, "[!] Etiket 11 karakteri asamaz!\n");
            exit(1);
        }

        for(i=strlen(buff); i<11; i++)
            buff[i]=' ';

        memcpy(VolumeID.VolumeLabel, buff, 11);
    }

    memcpy(VolumeID.FileSystem, "FAT12   ", 8);
    fprintf(stdout, "\nHesaplamalar yapiliyor.\n");

    if(!is2pow(VolumeID.BytesPerSector)) {
        fprintf(stderr, "[!] HATA: Sektor boyutu 2 \'nin kuvveti olmak zorunda!\n");
        exit(1);
    }

    if(VolumeID.BytesPerSector != 512)
        fprintf(stdout, "[*] UYARI: Sektor boyutu 512 \'den farkli bir degerde. Bu hesaplamalarda bazi hatalara yol acabilir!\n");

    if(!is2pow(VolumeID.SectorsPerCluster)) {
        fprintf(stderr, "[!] HATA: Cluster boyutu 2 \'nin kuvveti olmak zorunda!\n");
        exit(1);
    }

    if(VolumeID.SectorsPerCluster > 64)
        fprintf(stdout, "[*] UYARI: Cluster boyutu 32 KB \'dan fazla. Bazi sistemlerde uyumluluk problemlerine yol acabilir!\n");

    if((VolumeID.MaxRootDirEntries*32)%VolumeID.BytesPerSector != 0)
        fprintf(stdout, "[*] UYARI: Dosya sayisi limiti sektor boyutunu tam doldurmuyor. Ileride bazi sorunlara yol acabilir!\n");

    if(VolumeID.MediaDesc == 0xf8 && (VolumeID.NumberOfHeads != 0 || VolumeID.SectorsPerTrack != 0))
        fprintf(stdout, "[*] BILGI: Medya aciklamasi sabit disk (0xf8) olarak tanimlandiysa head ve track sayilari 0 degerini alabilir.\n");

    if(tmp == -1) {
        if(VolumeID.BytesPerSector != 512) {
            fprintf(stderr, "[!] Sektor boyutu 512 olmayan sistemlerde FAT alani hesaplanamiyor.. Lutfen bir daha deneyin!\n");
            exit(1);
        }

        fprintf(stdout, "FAT alani hesaplaniyor...");
        VolumeID.SectorsPerFAT=fatsize(&VolumeID);
        fprintf(stdout, "FAT alani %d sektor bulundu!\n", VolumeID.SectorsPerFAT);
    } else {
        VolumeID.SectorsPerFAT=tmp;
        MaxDataSz = (VolumeID.BytesPerSector * VolumeID.SectorsPerFAT);
        MaxDataSz = MaxDataSz / 1.5;
        MaxDataSz *= VolumeID.SectorsPerCluster;

        if(VolumeID.TotalSectors16 != 0) {
            if(MaxDataSz < (float)VolumeID.TotalSectors16)
                fprintf(stdout, "[*] UYARI: FAT adreslemesi tum imaji kapsamiyor!\n");
        } else {
            if(MaxDataSz < (float)VolumeID.TotalSectors32)
                fprintf(stdout, "[*] UYARI: FAT adreslemesi tum imaji kapsamiyor!\n");
        }
    }

    fprintf(stdout, "TAMAMLANDI: Hesaplamalar tamamlandi! Son kontrol yapiliyor..\n");

    if(!isfat12(&VolumeID)) {
        fprintf(stderr, "[!] HATA: Bir sorunla karsilasildi. Program sonlandiriliyor..\n");
        exit(1);
    }

    fprintf(stdout, "\nDisket imaji olusturuluyor..\n");
    saveImage(img);
}

void createImage(char* img_name) {
    FILE* img= fopen(img_name, "w+b");
    char buff[4];

    if(!img) {
        perror("[!]");
        exit(1);
    }

    fprintf(stdout, "FAT MANAGER \'a hosgeldiniz. Imaj olusturmaya baslamadan once lutfen seciminizi yapiniz:\n");
    fprintf(stdout, "   1- Onceden tanimlanmis kaliplardan sec (onerilir)\n");
    fprintf(stdout, "   2- Disket ozelliklerini siz belirtin (tecrubeli kullanicilar)\n");
    fprintf(stdout, "Seciminiz (1/2) [1]> ");

    if(fgets(buff, 4, stdin)==NULL) {
        perror("[!] HATA");
        exit(1);
    };

    switch(buff[0]) {
    case '\n':
    case '1':
        predefinedImage(img);
        break;

    case '2':
        customImage(img);
        break;

    default:
        fprintf(stdout, "Lutfen verilen aralikta bir secim yapiniz!\n");
        exit(1);
    }
}
