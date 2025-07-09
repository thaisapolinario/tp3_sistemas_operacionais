#ifndef SHELL_H
#define SHELL_H

void init_shell();
void load_arquivo();
void cria_diretorio(const char *caminho);
void cria_arquivo (const char *caminho);
void remove (const char *caminho); // exclui arquivo ou diretorio
void escreve_arquivo (const char *caminho, const char *conteudo, int append);
void le_arquivo(const char *caminho);







#endif