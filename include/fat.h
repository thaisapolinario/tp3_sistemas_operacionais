#ifndef FAT_H
#define FAT_H
#include <stdint.h>

#define TAMANHO_CLUSTER 1024  // 2 setores por cluster, cada 1 com 512 bytes
#define QUANTIDADE_CLUSTER 4096  // 4096 entradas para blocos
#define TAMANHO_FAT 8192 // número de clusters * 2 bytes
#define CLUSTER_FAT 8  
#define ROOT 9 // cluster 9
#define NOME_ARQUIVO "fat.part"  // 18 bytes para nome do arquivo
#define ENTRADA_DIRETORIO 32 // tamanho do cluster / 32, cada diretorio tem 32 bytes

#define ATRIBUTO_ARQUIVO 0
#define ATRIBUTO_DIRETORIO 1

#define FAT_LIVRE  0x0000
#define FAT_FIM   0xffff
#define FAT_BOOT  0xfffd
#define FAT_FAT   0xfffe

typedef struct Entrada_diretorio{
uint8_t arquivo[18]; // nome do arquivo
uint8_t atributos; // 0 -> arquivo e 1 -> diretorio
uint8_t reservado[7]; // reservado
uint16_t primeiro_bloco; 
uint32_t tamanho; // tamanho em bytes
} Entrada_diretorio;

typedef union dados_cluster {
    Entrada_diretorio dir[ENTRADA_DIRETORIO];
    uint8_t raw[TAMANHO_CLUSTER];
}dados_cluster;

extern unsigned short fat[QUANTIDADE_CLUSTER];
extern dados_cluster bloco_de_boot;
extern dados_cluster diretorio_raiz;
extern dados_cluster clusters[QUANTIDADE_CLUSTER];


void zera_fat();
void incia_fat();
void carrega_fat(FILE *arquivo);
int encontra_cluster_livre(FILE *arquivo);
void salva_fat(FILE *arquivo);
void encadeia_cluster(uint16_t cluster_atual, uint16_t cluster_proximo);
void libera(uint16_t inicio);
dados_cluster* carrega_cluster(int bloco);
void escreve_cluster(FILE* arquivo,int bloco, dados_cluster* cluster);
dados_cluster* encontra_diretorio_pai(dados_cluster* cluster_atual, const char* caminho, int* endereco);
int encontra_espaco_livre(Entrada_diretorio* dir);
dados_cluster* cluster_caminho(FILE *arquivo, dados_cluster *cluster_atual, const char *caminho, int *endereco);
int entrada_diretorio(FILE *arquivo, const char *caminho, Entrada_diretorio *resultado, uint16_t *cluster_pai, int *indice);
int le_cluster(FILE *arquivo, int bloco, dados_cluster *destino);
#endif