#ifndef SHELL_H
#define SHELL_H

void init();
void load();
void ls(const char *caminho);
void mkdir_arquivo(const char *caminho);
void mkdir_diretorio(const char *caminho);
void create(const char *caminho);
void unlink (const char *caminho); // exclui arquivo ou diretorio
void write(const char *caminho, const char *conteudo, int append);
void read(const char *caminho);
void append(const char *caminho);







#endif