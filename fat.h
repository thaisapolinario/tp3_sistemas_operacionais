#ifndef FAT_H
#define FAT_H
#include <stdint.h>

#define TAMANHO_CLUSTER 1024  // 2 setores por cluster, cada 1 com 512 bytes
#define QUANTIDADE_CLUSTER 4096  // 4096 entradas para blocos
#define TAMANHO_FAT 8192 // número de clusters * 2 bytes
#define CLUSTER_FAT 8  
#define ROOT 9 // cluster 9
#define NOME_ARQUIVO 18  // 18 bytes para nome do arquivo
#define ENTRADA_DIRETORIO 32 // tamanho do cluster / 32, cada diretorio tem 32 bytes

#define ATRIBUTO_ARQUVIVO 0
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

void zera_fat();
void incia_fat();
void carrega_fat();





#endif