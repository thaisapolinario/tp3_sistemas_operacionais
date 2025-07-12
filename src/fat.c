#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fat.h"



dados_cluster bloco_de_boot;  
dados_cluster diretorio_raiz; 
dados_cluster clusters[QUANTIDADE_CLUSTER]; 


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


    if (fseek(arquivo, TAMANHO_CLUSTER, SEEK_SET)){  // diretamente para a FAT
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
            salva_fat(arquivo);
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


dados_cluster* carrega_cluster(int bloco) {

    dados_cluster* cluster = calloc(1, sizeof(dados_cluster));

    FILE* arquivo = fopen(NOME_ARQUIVO, "rb");
    if (!arquivo) {
        perror("Erro ao abrir arquivo fat.part");
        free(cluster);
        return NULL;
    }

    if (fseek(arquivo, bloco * TAMANHO_CLUSTER, SEEK_SET) != 0) {
        perror("Erro no fseek");
        fclose(arquivo);
        free(cluster);
        return NULL;
    }

    if (fread(cluster, sizeof(dados_cluster), 1, arquivo) != 1) {
        perror("Erro na leitura do cluster");
        fclose(arquivo);
        free(cluster);
        return NULL;
    }

    fclose(arquivo);
    return cluster;
}

//Grava um cluster no arquivo fat.part
void escreve_cluster(FILE* arquivo, int bloco, dados_cluster* cluster){
    fseek(arquivo, bloco * sizeof(dados_cluster), SEEK_SET);
    fwrite(cluster, sizeof(dados_cluster), 1, arquivo);
    fflush(arquivo);  
}

dados_cluster* encontra_diretorio_pai(dados_cluster* cluster_atual, const char* caminho, int* endereco){
	char caminho_aux[strlen(caminho)];
	strcpy(caminho_aux, caminho);
	char* nome_diretorio = strtok(caminho_aux, "/");
	char* caminho_restante = strtok(NULL, "\0");

	Entrada_diretorio* diretorio_atual = cluster_atual->dir;

	int i=0;
	while (i < 32) {
		Entrada_diretorio filho = diretorio_atual[i];
		if (strcmp((char*)filho.arquivo, nome_diretorio) == 0 && caminho_restante){
			dados_cluster* cluster = carrega_cluster(filho.primeiro_bloco);
			*endereco = filho.primeiro_bloco;
			return encontra_diretorio_pai(cluster, caminho_restante, endereco);
		}
		else if (strcmp((char*)filho.arquivo, nome_diretorio) == 0 && caminho_restante){
			return NULL;
		}
		i++;
	}

	if (!caminho_restante)
		return cluster_atual;

	return NULL;

}

int encontra_espaco_livre(Entrada_diretorio* dir){
	for (int i = 0; i < ENTRADA_DIRETORIO; i++){
		if (dir[i].atributos == 0)
			return i;
	}
	return -1;
}


dados_cluster* cluster_caminho(FILE *arquivo, dados_cluster *cluster_atual, const char *caminho, int *endereco) {
    static dados_cluster buffer; // buffer estático para evitar malloc/free

    if (!caminho || strcmp(caminho, "/") == 0) return cluster_atual;


    if (strcmp(caminho, "/") == 0) {
    if (endereco) {
        *endereco = ROOT;  
    }
    return cluster_atual;
}

    char copia_caminho[strlen(caminho)+1];
    strcpy(copia_caminho, caminho);

    char *nome_atual = strtok(copia_caminho, "/");
    char *final_caminho = strtok(NULL, "");

    Entrada_diretorio *dir = cluster_atual->dir;

    for (int i = 0; i < ENTRADA_DIRETORIO; i++) {
        if (dir[i].arquivo[0] != 0 && strncmp((char*)dir[i].arquivo, nome_atual, sizeof(dir[i].arquivo)) == 0) {
            if (final_caminho == NULL) {
                if (le_cluster(arquivo, dir[i].primeiro_bloco, &buffer) != 0) {
                    return NULL;
                }
                if (endereco) {
                    *endereco = dir[i].primeiro_bloco;
                }
                return &buffer;
            } else {
                if (le_cluster(arquivo, dir[i].primeiro_bloco, &buffer) != 0){
                    return NULL;
                }
                if (endereco){
                    *endereco = dir[i].primeiro_bloco;
                }
                return cluster_caminho(arquivo, &buffer, final_caminho, endereco);
            }
        }
    }
    return NULL;
}

dados_cluster* cluster_pai(FILE *arquivo, dados_cluster *cluster_atual, const char *caminho, int *endereco) {
    if (!caminho || strlen(caminho) == 0){
        return NULL;
    }

    char copia_caminho[strlen(caminho)+1];
    strcpy(copia_caminho, caminho);

    char *nome_caminho = strtok(copia_caminho, "/");
    char *final_caminho = strtok(NULL, "\0");

    Entrada_diretorio *dir = cluster_atual->dir;

    for (int i = 0; i < ENTRADA_DIRETORIO; i++) {
        if (dir[i].arquivo[0] != 0 && strncmp((char*)dir[i].arquivo, nome_caminho, sizeof(dir[i].arquivo)) == 0) {
            if (final_caminho == NULL) {
                return cluster_atual; // pai
            } else {
                static dados_cluster buffer;
                if (le_cluster(arquivo, dir[i].primeiro_bloco, &buffer) != 0){
                    return NULL;
                }
                if (endereco){
                    *endereco = dir[i].primeiro_bloco;
                } 
                return cluster_pai(arquivo, &buffer, final_caminho, endereco);
            }
        }
    }
    return NULL;
}


int entrada_diretorio(FILE *arquivo, const char *caminho, Entrada_diretorio *resultado, uint16_t *cluster_pai, int *indice) {
    if (!caminho || caminho[0] != '/') {
        return -1;
    }

    char copia_caminho[256];
    strncpy(copia_caminho, caminho, sizeof(copia_caminho) - 1);
    copia_caminho[sizeof(copia_caminho) - 1] = '\0';

    uint16_t atual = ROOT;
    if (cluster_pai) {
        *cluster_pai = ROOT;
    }

    char *token = strtok(copia_caminho, "/");
    if (token == NULL && strcmp(caminho, "/") == 0) {
        if (resultado) {
            memset(resultado, 0, sizeof(Entrada_diretorio));
            strcpy((char *)resultado->arquivo, "/");
            resultado->atributos = ATRIBUTO_DIRETORIO;
            resultado->primeiro_bloco = ROOT;
            resultado->tamanho = 0;
        }
        return 0;
    }

    while (token != NULL) {
        dados_cluster cluster_data;
        if (le_cluster(arquivo, atual, &cluster_data) != 0) return -1;

        char *prox = strtok(NULL, "/");
        int encontrou = 0;

        for (int i = 0; i < ENTRADA_DIRETORIO; i++) {
            Entrada_diretorio *entry = &cluster_data.dir[i];
            if (entry->arquivo[0] != '\0' && strncmp((char *)entry->arquivo, token, sizeof(entry->arquivo)) == 0) {
                if (prox == NULL) {
                    if (resultado) *resultado = *entry;
                    if (cluster_pai) *cluster_pai = atual;
                    if (indice) *indice = i;
                    return 0;
                }

                if (entry->atributos == ATRIBUTO_DIRETORIO) {
                    if (cluster_pai) *cluster_pai = atual;
                    atual = entry->primeiro_bloco;
                    encontrou = 1;
                    break;
                } else {
                    return -1; 
                }
            }
        }

        if (!encontrou) return -1;
        token = prox;
    }

    return -1;
}

int le_cluster(FILE *arquivo, int bloco, dados_cluster *destino) {
    if (!arquivo || !destino) return -1;

    fseek(arquivo, bloco * TAMANHO_CLUSTER, SEEK_SET);


    if (fread(destino, sizeof(dados_cluster), 1, arquivo) != 1) {
        perror("Erro na leitura do cluster");
        return -1;
    }

    return 0;
}
