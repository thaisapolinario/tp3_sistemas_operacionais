#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "fat.h"
#include "shell.h"

//Copia a string de origem para a string de destino
void copia_string(const char* origem, uint8_t* destino){
    int comprimento = strlen(origem);
    for (int i = 0; i < comprimento; i++) {
        destino[i] = origem[i];
    }
}

//Acha o último nome do arquivo ou diretório
char* acha_nome(const char* caminho){

	char copia_caminho[strlen(caminho)];
	strcpy(copia_caminho, caminho);

	char* nome = strtok(copia_caminho, "/");
	char* caminho_restante = strtok(NULL, "\0");
	if (caminho_restante != NULL)
		return acha_nome(caminho_restante);

	return (char*) nome;
}

//Inicia o shell
void init(){
	FILE* arquivo;
	arquivo = fopen(NOME_ARQUIVO,"wb");
	for (int i = 0; i < TAMANHO_CLUSTER; i++)
		bloco_de_boot.raw[i] = 0xbb;

	fwrite(&bloco_de_boot, sizeof(bloco_de_boot), 1,arquivo);

	fat[0] = 0xfffd;
	for (int i = 1; i < 9; i++)
		fat[i] = 0xfffe;

	fat[9] = 0xffff;
	for (int i = 10; i < QUANTIDADE_CLUSTER; i++)
		fat[i] = 0x0000;

	fwrite(&fat, sizeof(fat), 1, arquivo);
	fwrite(&diretorio_raiz, sizeof(diretorio_raiz), 1,arquivo);

	for (int i = 0; i < 4086; i++)
		fwrite(&clusters, sizeof(dados_cluster), 1, arquivo);

	fclose(arquivo);
}

//Cria um diretório
void mkdir(const char  *caminho)
{

    FILE* arquivo = fopen(NOME_ARQUIVO, "r+b");
    if (!arquivo) {
        perror("Erro ao abrir o arquivo");
        return;
    }

	if (strcmp(caminho, "/") == 0){
        return;
    }

	int endereco_raiz = 9;
	dados_cluster* cluster_raiz = carrega_cluster(endereco_raiz);
	dados_cluster* cluster_pai = encontra_diretorio_pai(cluster_raiz, caminho, &endereco_raiz);

	if (cluster_pai){
		int posicao_livre = encontra_espaco_livre(cluster_pai->dir);
		int bloco_fat = encontra_cluster_livre(arquivo);

		if (bloco_fat && posicao_livre != -1) {
			char* nome_diretorio = acha_nome(caminho);
			copia_string(nome_diretorio, cluster_pai->dir[posicao_livre].arquivo);
			cluster_pai->dir[posicao_livre].atributos = 1;
			cluster_pai->dir[posicao_livre].primeiro_bloco = bloco_fat;
			escreve_cluster(arquivo, endereco_raiz, cluster_pai);
		}
	}
	else{
	    printf("Caminho não encontrado\n");
    }
}

void ls(const char* caminho) {
    FILE* arquivo = fopen(NOME_ARQUIVO, "rb");
    if (!arquivo) {
        perror("Erro ao abrir arquivo");
        return;
    }

    int endereco_cluster = ROOT;
    dados_cluster* cluster_raiz = carrega_cluster(ROOT);

    dados_cluster* cluster = cluster_caminho(arquivo, cluster_raiz, caminho, &endereco_cluster);

    for (int i = 0; i < ENTRADA_DIRETORIO; ++i) {
        if (cluster->dir[i].atributos == ATRIBUTO_DIRETORIO ||
            cluster->dir[i].atributos == ATRIBUTO_ARQUIVO) {
            printf("%s\n", cluster->dir[i].arquivo);
        }
    }

    fclose(arquivo);
}

void append(char* caminho, char* conteudo) {

    FILE* arquivo = fopen(NOME_ARQUIVO, "r+b");
    if (!arquivo) {
        perror("Erro ao abrir o arquivo");
        return;
    }

    int endereco_inicial = ROOT;
    dados_cluster* cluster_raiz = carrega_cluster(ROOT);
    dados_cluster* cluster_atual = cluster_caminho(arquivo, cluster_raiz, caminho, &endereco_inicial);


    // Percorre os clusters até o último
    uint16_t bloco_atual = endereco_inicial;
    while (fat[bloco_atual] != FAT_FIM) {
        bloco_atual = fat[bloco_atual];
    }

    dados_cluster* ultimo_cluster = carrega_cluster(bloco_atual);

    int usado = strlen((char*)ultimo_cluster->raw);
    int disponivel = TAMANHO_CLUSTER - usado;

    const char* ptr = conteudo;
    int restante = strlen(conteudo);
    int escrito = 0;

    // Escreve no espaço restante do cluster atual
    if (disponivel > 0) {
        int n = (restante < disponivel) ? restante : disponivel;
        strncat((char*)ultimo_cluster->raw, ptr, n);
        escreve_cluster(arquivo, bloco_atual, ultimo_cluster);
        ptr += n;
        restante -= n;
        escrito += n;
    }

    // Aloca novos clusters se necessário
    while (restante > 0) {
        int novo = encontra_cluster_livre(arquivo);

        encadeia_cluster(bloco_atual, novo);
        fat[novo] = FAT_FIM;
        salva_fat(arquivo);

        dados_cluster novo_cluster;
        memset(&novo_cluster, 0, sizeof(dados_cluster));

        int n = (restante < TAMANHO_CLUSTER) ? restante : TAMANHO_CLUSTER;
        strncpy((char*)novo_cluster.raw, ptr, n);
        escreve_cluster(arquivo,novo, &novo_cluster);

        ptr += n;
        restante -= n;
        escrito += n;
        bloco_atual = novo;
    }

    fclose(arquivo);
}

void read(const char* caminho) {

    FILE* arquivo = fopen(NOME_ARQUIVO, "rb");
    if (!arquivo) {
        perror("Erro ao abrir o arquivo");
        return;
    }

    int endereco_inicial = ROOT;
    dados_cluster* cluster_raiz = carrega_cluster(ROOT);
    dados_cluster* cluster_atual = cluster_caminho(arquivo, cluster_raiz, caminho, &endereco_inicial);


    uint16_t cluster_id = endereco_inicial;

    while (cluster_id != FAT_FIM) {
        dados_cluster* cluster = carrega_cluster(cluster_id);
        if (!cluster) {
            printf("Erro ao carregar cluster %d\n", cluster_id);
            break;
        }

        cluster->raw[TAMANHO_CLUSTER - 1] = '\0';
        printf("%s", (char*)cluster->raw);

        cluster_id = fat[cluster_id];
    }

    printf("\n");
    fclose(arquivo);
}
