#include <stdio.h>
#include <string.h>
#include <windows.h>

// Função recursiva usando a API do Windows
void search_and_delete(const char *dir_path, const char *target_filename) {
    char search_path[MAX_PATH];
    WIN32_FIND_DATA find_file_data;
    HANDLE hFind;

    // Monta o caminho de busca para a API do Windows (ex: "C:\Caminho\*")
    snprintf(search_path, sizeof(search_path), "%s\\*", dir_path);

    // Inicia a busca no diretório atual
    hFind = FindFirstFile(search_path, &find_file_data);

    // Se der erro (ex: pasta bloqueada pelo sistema), simplesmente ignora e sai
    if (hFind == INVALID_HANDLE_VALUE) {
        return; 
    }

    do {
        // Ignora as referências de pasta atual "." e pasta pai ".."
        if (strcmp(find_file_data.cFileName, ".") == 0 || strcmp(find_file_data.cFileName, "..") == 0) {
            continue;
        }

        // Monta o caminho completo do item encontrado
        char path[MAX_PATH];
        snprintf(path, sizeof(path), "%s\\%s", dir_path, find_file_data.cFileName);

        // Verifica se o item encontrado é uma pasta
        if (find_file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // É pasta: chama a função de novo (recursividade) para entrar nela
            search_and_delete(path, target_filename);
        } else {
            // É arquivo: verifica se o nome bate com o nosso alvo
            if (strcmp(find_file_data.cFileName, target_filename) == 0) {
                printf("[!] Arquivo encontrado: %s\n", path);
                
                // Tenta deletar usando a função nativa do Windows
                if (DeleteFile(path)) {
                    printf("    -> Deletado com sucesso!\n");
                } else {
                    printf("    -> Erro ao deletar (Pode estar em uso ou sem permissao de Administrador)\n");
                }
            }
        }
    } while (FindNextFile(hFind, &find_file_data) != 0); // Continua até acabar os itens da pasta

    FindClose(hFind);
}

int main() {
    // Definimos o alvo e a raiz da busca direto no código
    const char *target = "servico_rede.exe";
    const char *root_dir = "C:"; // Começa no disco C inteiro

    printf("Iniciando varredura no disco %s procurando por '%s'...\n", root_dir, target);
    printf("Isso pode levar alguns minutos. Aguarde...\n\n");
    
    // Chama a função
    search_and_delete(root_dir, target);
    
    printf("\nVarredura concluida. Pode fechar o programa.\n");
    system("pause"); // Pausa a tela para você conseguir ler o resultado antes de fechar
    
    return 0;
}