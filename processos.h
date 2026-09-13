#ifndef PROCESSOS_H
#define PROCESSOS_H

#define MAX_PROCESSOS 100
//Tamanho seguro para arrays
#define MAX_MOLDURAS 50
#define MAX_ACESSOS 1000

typedef struct {
    char pid[10];
    int tempo_criacao;
    int tempo_execucao_total;
    int tempo_restante;
    int prioridade; // prioridade ou quantidade de bilhetes
    float vruntime; // usado no CFS
    int tempo_espera; // tempo em estado pronto
    int tempo_conclusao;
    int na_cpu;
    //Campos para os arquivos de entrada
    int qtde_memoria; // Quantidade de de memória exigida pelo processo
    int limite_molduras; // Quantidade de molduras que o processo pode usar calculado
    int sequencia_acessos[MAX_ACESSOS]; // Array que guarda: 1, 2, 3, 3, 4, 5...
    int total_acessos_sequencia;// Tamanho da sequência lida do arquivo
    int acesso_atual;// Índice de qual página o processo vai pedir agora
    //Campos para a memória física(Molduras)
    int paginas[MAX_MOLDURAS]; // páginas que o processo acessa
    int tempo_carregamento[MAX_MOLDURAS]; // Necessário para FIFO e LRU
    int contador_nfu[MAX_MOLDURAS];
    int ponteiro_fifo; // Índice circular para o FIFO
    int n_paginas_ocupadas; // Quantas molduras estão em uso
    //Campos novos para a animação
    int ultima_pagina_pedida;
    int sofreu_page_fault;
    //Campos para controle de I/O
    int estado; // 0 = Pronto, 1 = Executando, 2 = Bloqueado (I/O)
    int tempo_io_restante; // Quanto tempo falta para terminar a operação de I/O
    //Novos campos para dispositivos de saída
    int chance_requisitar_es; // Lido do final da linha do processo
    int dispositivo_alvo_es;  // ID do dispositivo que ele está usando/aguardando
} Processo;

//Nova struct para dispositivos de Saída
typedef struct {
    int id;
    int num_usos_simultaneos;
    int tempo_operacao;
    int em_uso_atual;
} DispositivoES;

void escalonar_alternancia(Processo processos[], int n, int quantum);
void escalonar_prioridade(Processo processos[], int n, int quantum);
void escalonar_loteria(Processo processos[], int n, int quantum);
void escalonar_cfs(Processo processos[], int n, int quantum);

#endif