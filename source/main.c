#include <stdio.h>
#include "fat12.h"

#define VER_NUM 1

void printHelp();
void readImage(char** params, int parami);
void writeImage(char** params);
void createImage(char* img_name);
int isfat12(struct VOLUME_BOOT_RECORD* volume_id);
unsigned int fatsize(struct VOLUME_BOOT_RECORD* volume_id);
void setCluster(FILE* img, unsigned short cluster_num, unsigned short value, struct VOLUME_BOOT_RECORD* VolumeID);
unsigned short getCluster(FILE* img, unsigned short index, struct VOLUME_BOOT_RECORD* VolumeID);
unsigned int getFreeSize(FILE* img, struct VOLUME_BOOT_RECORD* VolumeID);
void printVBR(FILE* img, struct VOLUME_BOOT_RECORD* VolumeID);

void printHelp() {
    fprintf(stdout, "        FAT MANAGER (versiyon: %.1f)\n", VER_NUM);
    fprintf(stdout, "Kullanim: fat_man [-r -w] IMAJ_ISMI [-i -d -m -e -f] DOSYA_ISMI\n");
    fprintf(stdout, "veya:     fat_man -r IMAJ_ISMI [-l -x -b]\n");
    fprintf(stdout, "veya:     fat_man -c IMAJ_ISMI\n");
    fprintf(stdout, "veya:     fat_man [-v -h]\n");
    fprintf(stdout, "\nOperasyon Modlari:\n");
    fprintf(stdout, "   -c   Yeni bir disket imaji olusturmak icin secin. Imajin ozellikle-\n");
    fprintf(stdout, "        ri program icerisinde belirlenir. Ek komut parametresi kullan-\n");
    fprintf(stdout, "        maz.\n");
    fprintf(stdout, "   -w   Varolan bir disket imajinda veri duzenlemek icin secin. Bu pa-\n");
    fprintf(stdout, "        rametre \'-i\' ve \'-d\' komutlarinin kullanilmasini kapsar.\n");
    fprintf(stdout, "   -r   Varolan bir disket imajindan veri okumak icin secin. Bu param-\n");
    fprintf(stdout, "        etre \'-e\', \'-x\', \'-b\', \'-f\' ve \'-l\' komutlarinin kullanilmasi-\n");
    fprintf(stdout, "        ni kapsar.\n");
    fprintf(stdout, "   -h   Bu yardimi goruntuler.\n");
    fprintf(stdout, "   -v   Versiyon numarasini gosterir.\n");
    fprintf(stdout, "\nKomut Parametleri:\n");
    fprintf(stdout, "   -i   Disket imajina dosya eklemek icin kullanin. Bu parametreden s-\n");
    fprintf(stdout, "        onra eklenecek dosya adini girin.\n");
    fprintf(stdout, "   -I   Disket imajina dosya eklemek icin kullanin. Belirtilen dosya  \n");
    fprintf(stdout, "        adı imajda bulunuyorsa uzerine yazilir. Bu parametreden sonra \n");
    fprintf(stdout, "        eklenecek dosya adini girin.\n");
    fprintf(stdout, "   -d   Disket imajindan dosya silmek icin kullanin. Bu parametreden  \n");
    fprintf(stdout, "        sonra silinecek dosya adini girin.\n");
    fprintf(stdout, "   -m   Disket imajinda bulunan bir dosyanin ozniteliklerini ve ayarl-\n");
    fprintf(stdout, "        amanizi saglar. Bu parametreden sonra duzenlenecek dosya adini\n");
    fprintf(stdout, "        girin.\n");
    fprintf(stdout, "   -e   Disket imajindan dosya cikartmak icin kullanin. Bu parametred-\n");
    fprintf(stdout, "        en sonra cikartilacak dosya adini girin.\n");
    fprintf(stdout, "   -x   Disket imajinda bulunan tum dosyalari cikarir.\n");
    fprintf(stdout, "   -l   Disket imajinda bulunan dosyalari listeler.\n");
    fprintf(stdout, "   -b   Disket imajinin ozelliklerini yazdirir.\n");
    fprintf(stdout, "   -f   Disket imajinda yer alan bir dosyanin ozelliklerini yazdirir.\n");
    fprintf(stdout, "\nSurum Notlari:\n");
    fprintf(stdout, "   Bu program yalnizca FAT12 dosya sistemini destekler. Ayrica klasor \n");
    fprintf(stdout, "destegi ve LFN (Long File Name) destegi de bulunmamaktadir. Imajda bu \n");
    fprintf(stdout, "tur verilerin bulunmasi beklenmedik hatalara yol acabilir.\n");
    fprintf(stdout, "   Kullanilacak komut parametrelerinin uygun operasyon modlariyla kul-\n");
    fprintf(stdout, "lanilmasi sarttir. Ornegin disket imajina dosya eklenecekse operas-\n");
    fprintf(stdout, "yon modu -w (write) olmalidir.\n");
    fprintf(stdout, "   Duzenlenecek disket imajlarinin BPB verisi DOS 4.0 formatinda ola -\n");
    fprintf(stdout, "caktir.\n");
    fprintf(stdout, "   Veri istemlerinde varsayilan degerler koseli parantezler \'[\', \']\'\n");
    fprintf(stdout, "arasinda belirtilmistir.\n");
    fprintf(stdout, "\nHata Raporlari ve Iletisim:\n");
    fprintf(stdout, "GMAIL: b.kaan.ozkan@gmail.com\n\n");
    fprintf(stdout, "Bu program Berktug K. OZKAN tarafinca yazilmistir.\n");
}

int main(int argc, char** argv) {
    if(argc < 2) {
        fprintf(stderr, "[!] Parametre bildirilmedi! Kullanim bilgisi icin \'-h\' parametresini kullanin.\n");
    } else if(argv[1][1] == 'c') {
        createImage(argv[2]);
    } else if(argv[1][1] == 'w') {
        writeImage(argv);
    } else if(argv[1][1] == 'r') {
        readImage(argv, argc);
    } else if(argv[1][1] == 'h') {
        printHelp();
    } else if(argv[1][1] == 'v') {
        fprintf(stdout, "FAT MANAGER (versiyon: %.2f)\n", VER_NUM);
    } else {
        fprintf(stderr, "[!] Hatali operasyon modu! Kullanim bilgisi icin \'-h\' parametresini kullanin.\n");
    }

    return 0;
}
