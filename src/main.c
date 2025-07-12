#include <stdio.h>
#include <stdlib.h>
#include "fat.h"
#include "shell.h"


int main (){

    char comando[256];

    while (1) {
        printf("FAT16$ ");
        fgets(comando, sizeof(comando), stdin);

        if (strncmp(comando, "init", 4) == 0) init();
        else if (strncmp(comando, "load", 4) == 0) load();
        else if (strncmp(comando, "ls", 2) == 0) ls("/");
        else if (strncmp(comando, "exit", 4) == 0) break;
        else printf("Comando não reconhecido.\n");
    }
    init_fat(); 
    return 0;
}