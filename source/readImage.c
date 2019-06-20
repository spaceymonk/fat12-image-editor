#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "fat12.h"
extern int isfat12(struct VOLUME_BOOT_RECORD* volume_id);
extern uint16_t getCluster(FILE *img, uint16_t index, struct VOLUME_BOOT_RECORD* VolumeID);
extern uint32_t getFreeSize(FILE *img, struct VOLUME_BOOT_RECORD* VolumeID);
extern int LFN_to_SFN(char* dest, char* src);
extern int fileSearch(FILE* img, char* dos_file_name, struct VOLUME_BOOT_RECORD* VolumeID, struct ROOT_DIR_ENTRY* dest);
extern int SFN_to_LFN(char* dest, char* src);
extern void fat_date_conv(struct FAT_DATE* cal, uint16_t date);
extern void fat_time_conv(struct FAT_TIME* clo, uint16_t time);

struct VOLUME_BOOT_RECORD VolumeID;

void printVBR(FILE* img) {
    uint32_t i, cnt=0;
    uint8_t tmp;

    for(i=0; i<11; i++)
        putchar(VolumeID.VolumeLabel[i]);

    putchar('\n');

    for(i=0; i<VolumeID.MaxRootDirEntries; i++) {
        if(fseek(img, i*32+((VolumeID.ReservedSectorCount * VolumeID.BytesPerSector)+(VolumeID.NumberOfFATs * VolumeID.SectorsPerFAT * VolumeID.BytesPerSector)), SEEK_SET)) {
            fprintf(stderr, "[!] Konumlama hatasi!\n");
            exit(1);
        }

        if(fread(&tmp, sizeof(uint8_t), 1, img)!=1) {
            fprintf(stderr, "[!] Okuma Hatasi!\n");
            exit(1);
        };

        if(tmp != 0x00 && tmp != 0xe5) {
            if(fseek(img, 0x0a, SEEK_CUR)) {
                fprintf(stderr, "[!] Konumlama hatasi!\n");
                exit(1);
            }

            if(fread(&tmp, sizeof(uint8_t), 1, img)!=1) {
                fprintf(stderr, "[!] Okuma Hatasi!\n");
                exit(1);
            };

            if(tmp != 0x0f)
                cnt++;
        }
    }

    if(!isfat12(&VolumeID)) fprintf(stdout, "Dosya sistemi turu..........: BILINMIYOR\n");
    else fprintf(stdout, "Dosya sistemi turu..........: FAT12\n");

    if(VolumeID.TotalSectors16 != 0) fprintf(stdout, "Toplam boyut................: %d KB\n", VolumeID.TotalSectors16 * VolumeID.BytesPerSector / 1024);
    else fprintf(stdout, "Toplam boyut................: %d KB\n", VolumeID.TotalSectors32 * VolumeID.BytesPerSector / 1024);

    fprintf(stdout, "Bos alan....................: %d KB\n", getFreeSize(img, &VolumeID)*VolumeID.BytesPerSector/1024);
    fprintf(stdout, "Sektor boyutu...............: %d bayt\n", VolumeID.BytesPerSector);
    fprintf(stdout, "Cluster boyutu..............: %d sektor\n", VolumeID.SectorsPerCluster);
    fprintf(stdout, "Ayrilmis alan boyutu........: %d sektor\n", VolumeID.ReservedSectorCount);
    fprintf(stdout, "FAT sayisi..................: %d adet\n", VolumeID.NumberOfFATs);
    fprintf(stdout, "Bir FAT icin ayrilmis alan..: %d sektor\n", VolumeID.SectorsPerFAT);
    fprintf(stdout, "Dosya sayisi limiti.........: %d adet\n", VolumeID.MaxRootDirEntries);
    fprintf(stdout, "Icerdigi dosya sayisi.......: %d adet\n", cnt);
    fprintf(stdout, "Medya aciklamasi............: 0x%02x\n", VolumeID.MediaDesc);

    if(VolumeID.NumberOfHeads != 0 && VolumeID.SectorsPerTrack != 0) {
        (VolumeID.TotalSectors16 != 0)? (cnt=VolumeID.TotalSectors16-1): (cnt=VolumeID.TotalSectors32-1);
        fprintf(stdout, "Disk geometrisi (C/H/S).....: %d/%d/%d\n", (cnt/VolumeID.SectorsPerTrack)/VolumeID.NumberOfHeads+1, (cnt/VolumeID.SectorsPerTrack)%VolumeID.NumberOfHeads+1, (cnt%VolumeID.SectorsPerTrack)+1);
    } else {
        fprintf(stdout, "Disk geometrisi (C/H/S).....: BELIRTILMEMIS\n");
    }
}

void extractFile(FILE* img, char* fileName, struct ROOT_DIR_ENTRY* entry) {
    FILE *f1;

    if(entry->FileAttr==0x0f)
        return;

    fprintf(stdout, "Dosya adi.....: %s\n", fileName);
    fprintf(stdout, "Dosya boyutu..: %d bayt\n", entry->FileSize);
    fprintf(stdout, "Dosya cikartilmaya hazirlaniyor...");

    if((f1=fopen(fileName, "wb")) == NULL) {
        fprintf(stderr, "BASARISIZ\n");
        perror("[!]");
        exit(1);
    }

    fprintf(stdout, "BASARILI\n");
    uint32_t i=0;
    uint64_t tmp=0;
    uint32_t addr_fat= ((VolumeID.BytesPerSector*VolumeID.ReservedSectorCount));
    uint32_t addr_root_dir= addr_fat+(VolumeID.NumberOfFATs*VolumeID.SectorsPerFAT*VolumeID.BytesPerSector);
    uint32_t addr_data = addr_root_dir+VolumeID.MaxRootDirEntries*32;
    uint16_t cluster=entry->FirstCluster;
    uint8_t buff;
    fprintf(stdout, "Dosya imajdan cikariliyor.........");

    for(;;) {
        fseek(img, (addr_data+(cluster-2)*VolumeID.SectorsPerCluster*VolumeID.BytesPerSector), SEEK_SET);

        for(i=0; i<VolumeID.BytesPerSector*VolumeID.SectorsPerCluster; i++) {
            if(tmp > entry->FileSize) {
                fprintf(stdout, "BASARISIZ!\n");
                fprintf(stderr, "Ciddi bir sorunla karsilasildi! Hata, FAT yapisindaki bozulmadan kaynaklanabilir.\n");
                exit(1);
            } else if(tmp+i == entry->FileSize) {
                fprintf(stdout, "BASARILI!\n");
                return;
            }

            if(fread(&buff, sizeof(uint8_t), 1, img)!=1) {
                fprintf(stderr, "[!] Okuma Hatasi!\n");
                exit(1);
            };

            if(fwrite(&buff, sizeof(uint8_t), 1, f1)!=1) {
                fprintf(stderr, "[!] Okuma Hatasi!\n");
                exit(1);
            };
        }

        tmp += i;
        cluster=getCluster(img, cluster, &VolumeID);
    }
}

void file_attr_to_string(uint8_t attr, char* dest) {
    char* ptr=dest;
    (attr & 0x10)? (*ptr='D'):(*ptr='-');
    ptr++;
    (attr & 0x01)? (*ptr='R'):(*ptr='-');
    ptr++;
    (attr & 0x02)? (*ptr='H'):(*ptr='-');
    ptr++;
    (attr & 0x04)? (*ptr='S'):(*ptr='-');
    ptr++;
    (attr & 0x20)? (*ptr='A'):(*ptr='-');
    ptr++;
    (attr & 0x08)? (*ptr='V'):(*ptr='-');
    ptr++;
    (attr & 0x40)? (*ptr='E'):(*ptr='-');
    ptr++;
    (attr & 0x80)? (*ptr='U'):(*ptr='-');
    ptr++;
    *ptr=0;
}

void readImage(char** params, int parami) {
    FILE *img;
    char fname_dos[11], tmp, fname_long[16], attr_str[9];
    char list[12][16]= {"Ocak", "Subat", "Mart", "Nisan", "Mayis", "Haziran", "Temmuz", "Agustos", "Eylul", "Ekim", "Kasim", "Aralik"};
    struct ROOT_DIR_ENTRY entry;
    struct FAT_DATE fdate;
    struct FAT_TIME ftime;
    uint32_t cnt=0, i=0, j=0;

    if((img=fopen(params[2], "rb+")) == NULL) {
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

    switch(parami) {
    case 4:
        switch(params[3][1]) {
        case 'l':
            fprintf(stdout, "Imajin icerisinde bulunan dosyalar listeleniyor..\n");
            fprintf(stdout, "[A: Arsiv,  R: Salt Okunur,  S: Sistem,  H: Gizli,  D: Klasor]\nOznitelik  Isim\t\tBoyut\n");

            for(i=0; i<VolumeID.MaxRootDirEntries; i++) {
                if(fseek(img, i*32+((VolumeID.ReservedSectorCount * VolumeID.BytesPerSector)+(VolumeID.NumberOfFATs * VolumeID.SectorsPerFAT * VolumeID.BytesPerSector)), SEEK_SET)) {
                    fprintf(stderr, "[!] Okuma Hatasi\n");
                    exit(1);
                }

                if(fread(&entry, sizeof(struct ROOT_DIR_ENTRY), 1, img)!=1) {
                    fprintf(stderr, "[!] Okuma Hatasi!\n");
                    exit(1);
                };

                if((entry.FileName[0]!= 0x00 && entry.FileName[0] != 0xe5) && entry.FileAttr!=0x0f) {
                    SFN_to_LFN(fname_long, (char*)entry.FileName);
                    file_attr_to_string(entry.FileAttr, attr_str);
                    fprintf(stdout, "%s   %s\t%.1f KB\n",attr_str, fname_long, (float)entry.FileSize/1024);
                }
            }

            break;

        case 'x':
            for(i=0; i<VolumeID.MaxRootDirEntries; i++) {
                if(fseek(img, i*32+((VolumeID.ReservedSectorCount * VolumeID.BytesPerSector)+(VolumeID.NumberOfFATs * VolumeID.SectorsPerFAT * VolumeID.BytesPerSector)), SEEK_SET)) {
                    fprintf(stderr, "[!] Konumlama hatasi!\n");
                    exit(1);
                }

                if(fread(&tmp, sizeof(uint8_t), 1, img)!=1) {
                    fprintf(stderr, "[!] Okuma Hatasi!\n");
                    exit(1);
                };

                if(tmp != 0x00 && tmp != 0xe5) {
                    if(fseek(img, 0x0a, SEEK_CUR)) {
                        fprintf(stderr, "[!] Konumlama hatasi!\n");
                        exit(1);
                    }

                    if(fread(&tmp, sizeof(uint8_t), 1, img)!=1) {
                        fprintf(stderr, "[!] Okuma Hatasi!\n");
                        exit(1);
                    };

                    if(tmp != 0x0f)
                        cnt++;
                }
            }

            fprintf(stdout, "%d dosya bulundu. Cikartma islemi baslatiliyor...\n", cnt);
            j=1;

            for(i=0; i<VolumeID.MaxRootDirEntries; i++) {
                if(fseek(img, i*32+((VolumeID.ReservedSectorCount * VolumeID.BytesPerSector)+(VolumeID.NumberOfFATs * VolumeID.SectorsPerFAT * VolumeID.BytesPerSector)), SEEK_SET)) {
                    fprintf(stderr, "[!] Okuma Hatasi\n");
                    exit(1);
                }

                if(fread(&entry, sizeof(struct ROOT_DIR_ENTRY), 1, img)!=1) {
                    fprintf(stderr, "[!] Okuma Hatasi!\n");
                    exit(1);
                };

                if((entry.FileName[0]!= 0x00 && entry.FileName[0] != 0xe5) && entry.FileAttr!=0x0f) {
                    SFN_to_LFN(fname_long, (char*)entry.FileName);
                    fprintf(stdout, "\n%d/%d Tamamlandi\n", j, cnt);
                    extractFile(img, fname_long, &entry);
                    j++;
                }
            }

            fprintf(stdout, "\nTum dosyalar basariyla cikartildi!\n");
            break;

        case 'b':
            printVBR(img);
            break;

        default:
            fprintf(stderr, "[!] Hatali komut!\n");
            exit(1);
        }

        break;

    case 5:
        switch(params[3][1]) {
        case 'e':
            if(LFN_to_SFN(fname_dos, params[4]) == 1) {
                fprintf(stderr, "[!] Gecersiz dosya adi!\n");
                exit(1);
            }

            if(fileSearch(img, fname_dos, &VolumeID, &entry) == 0) {
                fprintf(stderr, "[!] Dosya bulunamadi: %s", params[4]);
                exit(1);
            }

            extractFile(img, params[4], &entry);
            break;

        case 'f':
            if(LFN_to_SFN(fname_dos,params[4]) == 1) {
                fprintf(stderr, "[!] Gecersiz dosya adi!\n");
                exit(1);
            }

            if(fileSearch(img, fname_dos, &VolumeID, &entry) == 0) {
                fprintf(stderr, "[!] Dosya bulunamadi: %s", params[4]);
                exit(1);
            }

            SFN_to_LFN(fname_long, fname_dos);
            fprintf(stdout, "Dosya Adi....: %s\n", fname_long);
            fprintf(stdout, "Dosya Ozellikleri:\n");
            (entry.FileAttr & 0x01)? fprintf(stdout, "   [x] Salt Okunur"): fprintf(stdout, "   [ ] Salt Okunur");
            (entry.FileAttr & 0x02)? fprintf(stdout, "   [x] Gizli\n"): fprintf(stdout, "   [ ] Gizli\n");
            (entry.FileAttr & 0x04)? fprintf(stdout, "   [x] Sistem"): fprintf(stdout, "   [ ] Sistem");
            (entry.FileAttr & 0x20)? fprintf(stdout, "        [x] Arsiv\n"): fprintf(stdout, "        [ ] Arsiv\n");
            fat_date_conv(&fdate, entry.CreateDate);
            fat_time_conv(&ftime, entry.CreateTime);
            fprintf(stdout, "Olusturulma..: %d %s %d - %02d:%02d:%02d\n", fdate.day, list[fdate.month-1], fdate.year, ftime.hour, ftime.minute, ftime.second);
            fat_date_conv(&fdate, entry.LastModifiedDate);
            fat_time_conv(&ftime, entry.LastModifiedTime);
            fprintf(stdout, "Duzenleme....: %d %s %d - %02d:%02d:%02d\n", fdate.day, list[fdate.month-1], fdate.year, ftime.hour, ftime.minute, ftime.second);
            fat_date_conv(&fdate, entry.LastAccessDate);
            fprintf(stdout, "Erisim.......: %d %s %d\n", fdate.day, list[fdate.month-1], fdate.year);

            if(entry.FileSize < 1024) fprintf(stdout, "Dosya boyutu.: %d bayt\n", entry.FileSize);
            else if(entry.FileSize < 1024*1024) fprintf(stdout, "Dosya boyutu.: %.1f KB (%d bayt)\n", (float)entry.FileSize/1024, entry.FileSize);
            else fprintf(stdout, "Dosya boyutu.: %.1f MB (%d bayt)\n", (float)entry.FileSize/1024/1024, entry.FileSize);

            break;

        default:
            fprintf(stderr, "[!] Hatali komut\n");
            exit(1);
        }

        break;

    default:
        fprintf(stderr, "[!] Hatali parametre!\n");
        exit(1);
    }

    fclose(img);
}
