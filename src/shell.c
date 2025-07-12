#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "fat.h"
#include "shell.h"

//Copia a string de origem para a string de destino
void copia_string(char* origem, char* destino){
    int comprimento = strlen(origem);
    for (int i = 0; i < comprimento; i++) {
        destino[i] = origem[i];
    }
}

//Acha o último nome do arquivo ou diretório
char* acha_nome(char* caminho){

	char copia_caminho[strlen(caminho)];
	strcpy(copia_caminho, caminho);

	char* nome = strtok(copia_caminho, "/");
	char* caminho_restante = strtok(NULL, "\0");
	if (caminho_restante != NULL)
		return acha_nome(caminho_restante);

	return (char*) nome;
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
