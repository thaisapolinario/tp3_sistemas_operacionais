#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "fat.h"

void init_arquivo() {

    FILE *fp = fopen(NOME_ARQUIVO, "wb");
    if (!fp) {
        perror("Erro ao criar fat.part");
        exit(1);
    }

    uint8_t zero[TAMANHO_CLUSTER];
    memset(zero, 0x00, TAMANHO_CLUSTER);

    uint8_t boot[TAMANHO_CLUSTER];
    memset(boot, 0xBB, TAMANHO_CLUSTER);
    fwrite(boot, 1, TAMANHO_CLUSTER, fp);

    for (int i = 0; i < CLUSTER_FAT; i++) {
        fwrite(zero, 1, TAMANHO_CLUSTER, fp);
    }

    fwrite(zero, 1, TAMANHO_CLUSTER, fp); 

    for (int i = 10; i < QUANTIDADE_CLUSTER; i++) {
        fwrite(zero, 1, TAMANHO_CLUSTER, fp);
    }

    // inicializa e salva a FAT
    fclose(fp);

    fp = fopen(NOME_ARQUIVO, "r+b");
    if (!fp) {
        perror("Erro ao abrir fat.part para salvar FAT");
        exit(1);
    }

    inicia_fat();
    salva_fat(fp);
    fclose(fp);

    printf("Sistema de arquivos inicializado com sucesso.\n");
}


void carrega_arquivo() {
    FILE *fp = fopen(NOME_ARQUIVO, "r+b");
    if (!fp) {
        fprintf(stderr, "Arquivo %s não encontrado.\n", NOME_ARQUIVO);
        exit(1);
    }
    carrega_fat(fp);
    fclose(fp);
    printf("Sistema de arquivos carregado com sucesso.\n");
}