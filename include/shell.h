#ifndef SHELL_H
#define SHELL_H

void copia_string(const char* origem, uint8_t* destino);
char* acha_nome(const char* caminho);
void init();
void load();
void ls(const char *caminho);
void mkdir(const char *caminho, FILE *arquivo);
void create(const char *caminho);
void unlink (const char *caminho); // exclui arquivo ou diretorio
void write(const char *caminho, const char *conteudo, int append);
void read(const char *caminho);
void append(const char *caminho);







#endif