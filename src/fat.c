#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fat.h"


uint16_t fat[QUANTIDADE_CLUSTER];

void zera_fat(){
    FILE *arquivo = fopen("fat.part", "wb+");
    if (!arquivo) {
        perror("Erro ao criar fat.part");
        exit(1);
    }
    uint8_t *zera_fat = calloc(TAMANHO_CLUSTER,1); // FAT com zeros
    for (int i = 0; i < QUANTIDADE_CLUSTER; i++) {
        fwrite(zera_fat, 1, TAMANHO_CLUSTER, arquivo);
    }
    free(zera_fat);
    fclose(arquivo);
}

void init_fat() {
  
    zera_fat();

    FILE *arquivo = fopen("fat.part", "rb+");
    if (!arquivo) {
        perror("Erro ao criar fat.part");
        exit(1);
    }


    uint8_t boot_block = 0xBB; // escreve no cluster 0

    fseek(arquivo, 0, SEEK_SET);
    
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

    //inicializado com 0x00
    fseek(arquivo, ROOT * TAMANHO_CLUSTER, SEEK_SET);
    uint8_t vazio = 0x00;
    for (int i = 0; i < TAMANHO_CLUSTER; i++) {
        fwrite(&vazio, 1, 1, arquivo);
    }

    fclose(arquivo);
    printf("Sistema de arquivos inicializado com sucesso.\n");
}

void carrega_fat(FILE *arquivo){


    if (fseek(arquivo, TAMANHO_CLUSTER, SEEK_SET)){  // diretamento para a FAT
        fclose(arquivo);
        exit(1);
    } 

    size_t le_fat = fread(fat, sizeof(unsigned short), QUANTIDADE_CLUSTER,arquivo);
    if(le_fat!=QUANTIDADE_CLUSTER){  // se não ler todas as 4096 entradas
        fclose(arquivo);
        exit(1);
    }

    fclose(arquivo);
    printf("Finalizada leitura da FAT\n");
}

void salva_fat(FILE *arquivo){


    fseek(arquivo, TAMANHO_CLUSTER, SEEK_SET);
    fwrite(fat, sizeof(fat), 1, arquivo);
    fclose (arquivo);

}

int encontra_cluster_livre(FILE *arquivo){
    carrega_fat(arquivo);
    for(int i = 10;i<QUANTIDADE_CLUSTER;i++){ // procura um livre
        if(fat[i] == FAT_LIVRE){
            fat[i] = FAT_FIM; // marca ocupado
            return i;
        }
    }
    return -1;
}

void encadeia_clusters(uint16_t atual, uint16_t proximo) {
    fat[atual] = proximo;
}

void libera_clusters(uint16_t inicio) {
    uint16_t atual = inicio;
    while (atual != FAT_FIM && atual != FAT_LIVRE) {
        uint16_t proximo = fat[atual];
        fat[atual] = FAT_LIVRE;
        if (proximo == FAT_FIM) break;
        atual = proximo;
    }
}