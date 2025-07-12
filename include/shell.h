#ifndef SHELL_H
#define SHELL_H

void copia_string(const char* origem, uint8_t* destino);
char* acha_nome(const char* caminho);
void init();
void ls(const char *caminho);
void mkdir(const char *caminho);
void append(char* caminho, char* conteudo);
void read(const char *caminho);


void load();
void create(const char *caminho);
void unlink (const char *caminho); // exclui arquivo ou diretorio
void write(const char *caminho, const char *conteudo, int append);







#endif