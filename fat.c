#include <stdio.h>
#include <stdlib.h>
#include "fat.h"


uint16_t fat[QUANTIDADE_CLUSTER];

void zera_fat(){
    FILE *arquivo = fopen("fat.part", "wb+");
    if (!arquivo) {
        perror("Erro ao criar fat.part");
        exit(1);
    }
    uint8_t zera_fat = FAT_LIVRE; // preenche a FAT com zeros
    for (int i = 0; i < 4194304; i++) {
        fwrite(&zera_fat, 1, 1, arquivo);
    }
    fclose(arquivo);
}

void init_fat() {
  
    FILE *arquivo = fopen("fat.part", "wb+");
    if (!arquivo) {
        perror("Erro ao criar fat.part");
        exit(1);
    }
    zera_fat();
    fseek(arquivo, 0, SEEK_SET);
    uint8_t boot_block = 0xBB;
    for (int i = 0; i < TAMANHO_CLUSTER; i++) {
        fwrite(&boot_block, 1, 1, arquivo);
    }
    // Inicializa a FAT na RAM
    memset(fat, 0, sizeof(fat));   // tudo livre
    fat[0] = FAT_BOOT;             // cluster 0 = boot
    for (int i = 1; i <= CLUSTER_FAT; i++) {
        fat[i] = FAT_FAT;         
    }
    fat[ROOT] = FAT_FIM;   // cluster 9 
    // Escreve a FAT no disco (clusters 1 a 8)
    fseek(arquivo, TAMANHO_CLUSTER, SEEK_SET); // posição do cluster 1
    fwrite(fat, sizeof(uint16_t), QUANTIDADE_CLUSTER, arquivo);

    // Diretório root (cluster 9) inicializado com 0x00
    fseek(arquivo, ROOT * TAMANHO_CLUSTER, SEEK_SET);
    uint8_t vazio = 0x00;
    for (int i = 0; i < TAMANHO_CLUSTER; i++) {
        fwrite(&vazio, 1, 1, arquivo);
    }

    fclose(arquivo);
    printf("Sistema de arquivos inicializado com sucesso.\n");
}
