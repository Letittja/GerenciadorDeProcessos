#include <stdio.h>
#include <stdlib.h>
#include "interface.h"
#include "processos.h"

// Essa lógica garante que a pausa (animação) funcione tanto no Windows quanto no Linux/Mac
#ifdef _WIN32
    #include <windows.h>
    #define PAUSA(ms) Sleep(ms)
#else
    #include <unistd.h>
    #define PAUSA(ms) usleep((ms) * 1000)
#endif

// Códigos ANSI para colorir o texto no terminal
#define RESET "\x1b[0m"
#define VERDE "\x1b[32m"
#define AZUL "\x1b[34m"
#define AMARELO "\x1b[33m"
#define VERMELHO "\x1b[31m"
#define CIANO "\x1b[36m"
#define NEGRITO "\x1b[1m"

// Função multiplataforma para limpar o terminal
void limpar_tela(void) {
    system("cls || clear");
}

void mostrar_cabecalho_simulador(const char* nome_algoritmo, int quantum) {
    limpar_tela();
    printf(AZUL "=========================================================\n" RESET);
    printf(CIANO "        SIMULADOR DE SISTEMAS OPERACIONAIS 2026\n" RESET);
    printf(AZUL "=========================================================\n" RESET);
    printf(" Iniciando: " VERDE "%s" RESET " | Quantum: " AMARELO "%d\n" RESET, nome_algoritmo, quantum);
    printf(AZUL "=========================================================\n\n" RESET);
    PAUSA(2000); // Pausa de 2 segundos para ler o cabeçalho
}

// Esta é a função que vai "pular" na tela. Ela substitui a antiga "imprimir_execucao"
void animar_execucao(int tempo, int indice_cpu, const Processo processos[], int n, const char* nome_algoritmo) {
    limpar_tela();
    
    printf(AZUL "=========================================================\n" RESET);
    printf(CIANO " ALGORITMO EM AÇÃO: " VERDE "%s\n" RESET, nome_algoritmo);
    printf(CIANO " RELOGIO DO SISTEMA: " AMARELO "t = %d\n" RESET, tempo);
    printf(AZUL "=========================================================\n" RESET);

    // Nova função para imprimir status do processo quando estiver aguardando o I/O
    for (int i = 0; i < n; i++) {
        if (processos[i].tempo_conclusao != -1) {
            printf("  P%s: " VERDE "[CONCLUIDO]" RESET " \n", processos[i].pid);
        } else if (i == indice_cpu) {
            printf("  P%s: " VERMELHO "[NA CPU]" RESET " \n", processos[i].pid);
        } else if (processos[i].estado == ESTADO_BLOQUEADO) {
            printf("  P%s: " AMARELO "[BLOQUEADO]" RESET " (Aguardando I/O: restam %d ciclos)\n", processos[i].pid, processos[i].tempo_io_restante);
        } else if (processos[i].tempo_criacao <= tempo) {
            printf("  P%s: " AMARELO "[PRONTO]" RESET " (Aguardando na fila)\n", processos[i].pid);
        } else {
            printf("  P%s: " CIANO "[FUTURO]" RESET " (Chega no t=%d)\n", processos[i].pid, processos[i].tempo_criacao);
        }
    }

    // Mostra o status da CPU
    if (indice_cpu >= 0) {
        printf(VERMELHO " [ CPU ]" RESET " -> Executando: P%s (Restam %d unidades)\n",
               processos[indice_cpu].pid, processos[indice_cpu].tempo_restante);
    } else {
        printf(VERMELHO " [ CPU ]" RESET " -> " AMARELO "OCIOSA (Aguardando processos)\n" RESET);
    }

    printf(AZUL "---------------------------------------------------------\n" RESET);
    printf(CIANO " STATUS DOS PROCESSOS:\n" RESET);
    
    // Varre todos os processos para mostrar onde eles estão agora
    for (int i = 0; i < n; i++) {
        if (processos[i].tempo_conclusao != -1) {
            printf("  P%s: " VERDE "[CONCLUIDO]" RESET " \n", processos[i].pid);
        } else if (i == indice_cpu) {
            printf("  P%s: " VERMELHO "[NA CPU]" RESET " \n", processos[i].pid);
        } else if (processos[i].tempo_criacao <= tempo) {
            printf("  P%s: " AMARELO "[PRONTO]" RESET " (Aguardando na fila)\n", processos[i].pid);
        } else {
            printf("  P%s: " CIANO "[FUTURO]" RESET " (Chega no t=%d)\n", processos[i].pid, processos[i].tempo_criacao);
        }
    }
    printf(AZUL "=========================================================\n" RESET);
    
    // Pausa de 500 milissegundos para criar o efeito visual (animação)
    PAUSA(500); 
}

// Função para imprimir o relatório da memória
void imprimir_relatorio_memoria(int fifo, int lru, int nfu, int otimo) {
    printf("\n");
    printf(CIANO NEGRITO "==============================================================\n" RESET);
    printf(CIANO NEGRITO "||" RESET AMARELO NEGRITO "        RELATÓRIO DE FALHAS DE PÁGINA (PAGE FAULTS)         " CIANO NEGRITO "||\n" RESET);
    printf(CIANO NEGRITO "==============================================================\n" RESET);

    // Resultados formatados para alinhar os números
    printf(CIANO "|| " RESET VERDE NEGRITO "-> FIFO: " RESET "%-4d trocas                                     " CIANO "||\n" RESET, fifo);
    printf(CIANO "|| " RESET VERDE NEGRITO "-> LRU:  " RESET "%-4d trocas                                     " CIANO "||\n" RESET, lru);
    printf(CIANO "|| " RESET VERDE NEGRITO "-> NFU:  " RESET "%-4d trocas                                     " CIANO "||\n" RESET, nfu);
    printf(CIANO "|| " RESET VERDE NEGRITO  "-> ÓTIMO:" RESET "%-4d trocas                                    " CIANO "||\n" RESET, otimo);
    printf(CIANO NEGRITO "==============================================================\n" RESET);
    printf("\n");

    //Lógica para determinar o vencedor (quem chegou mais perto do ótimo) e imprimir a linha final do relatório, seguindo exatamente o formato exigido pelo gabarito
    int dif_fifo = fifo - otimo;
    int dif_lru = lru - otimo;
    int dif_nfu = nfu - otimo;
    
    // Encontra qual é a menor diferença (quem chegou mais perto do ótimo)
    int menor_dif = dif_fifo;
    if (dif_lru < menor_dif) menor_dif = dif_lru;
    if (dif_nfu < menor_dif) menor_dif = dif_nfu;
    
    // Conta quantos algoritmos empataram na primeira posição
    int empates = 0;
    if (dif_fifo == menor_dif) empates++;
    if (dif_lru == menor_dif) empates++;
    if (dif_nfu == menor_dif) empates++;
    
    // Define a string do vencedor exatamente como o professor pediu
    const char* vencedor = "";
    if (empates > 1) {
        vencedor = "empate"; // Se 2 ou mais empataram no melhor resultado
    } else if (dif_fifo == menor_dif) {
        vencedor = "FIFO";
    } else if (dif_lru == menor_dif) {
        vencedor = "menos recentemente usada";
    } else if (dif_nfu == menor_dif) {
        vencedor = "não usada frequentemente";
    }
    // ATENÇÃO: Esta linha imprime o formato cru exigido pelo gabarito
    printf("%d|%d|%d|%d|%s\n", fifo, lru, nfu, otimo, vencedor);
}

// Substitui a antiga função que imprimia o relatório final
void imprimir_relatorio_colorido(const Processo processos[], int n) {
    limpar_tela();
    printf(VERDE "\n=== SIMULACAO CONCLUIDA COM SUCESSO ===\n\n" RESET);
    printf(AZUL "PID\tCriacao\tConclusao\tTempo de Permanência\tTempoPronto\n" RESET);
    printf("---------------------------------------------------------\n");

    for (int i = 0; i < n; i++) {
        int turnaround = processos[i].tempo_conclusao - processos[i].tempo_criacao;
        printf("P%s\t%d\t%d\t\t%d\t\t%d\n",
               processos[i].pid,
               processos[i].tempo_criacao,
               processos[i].tempo_conclusao,
               turnaround,
               processos[i].tempo_espera);
    }
    printf("---------------------------------------------------------\n\n");
}
