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
void mkdir(const char  *caminho, FILE *arquivo)
{
	if(caminho == "/")
		return;

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
			escreve_cluster(endereco_raiz, cluster_pai);
		}
	}
	else{
	    printf("Caminho não encontrado\n");
    }
}
