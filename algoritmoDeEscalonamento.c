#include "processos.h"
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include "interface.h"
#include "rbtree.h"
#include "memoria.h"

//Campo de definição de estado do processo
#define ESTADO_PRONTO 0
#define ESTADO_EXECUTANDO 1
#define ESTADO_BLOQUEADO 2
#define ESTADO_CONCLUIDO 3

// Lista de apelidos que representam números inteiros
// Ao invés de usar números soltos, usa-se nomes descritivos para cada política de escalonamento, facilitando a leitura e manutenção do código
typedef enum{
    // politica está sendo usado com a finalidade de representar os escalonamentos.
    POLITICA_ALTERNANCIA, // = 0
    POLITICA_PRIORIDADE, // = 1
    POLITICA_LOTERIA, // = 2
    POLITICA_CFS // = 3
} Politica;

// Garantindo que nenhum processo tenha prioridade zero ou inferior, para evitar divisão por zero no CFS
static int prioridade_efetiva(const Processo *p) {
    if (p->prioridade > 0) {
        return p->prioridade;
    } else {
        return 1;
    }
}

/////ALGORITMOS DE ESCALONAMENTO/////
//Funções auxiliar para organizar a fila por prioridade, deixando O(log N).
static void trocar_indices(int *a, int *b) {
    int temp = *a; //ponteiro para o valor de a, guardando ele temporariamente
    *a = *b; //ponteiro para o valor de a recebe o valor de b, ou seja, a e b trocam de lugar
    *b = temp; //ponteiro para o valor de b recebe o valor temporário, que é o valor original de a, completando a troca
}

//Função auxiliar para fazer o processo subir na árvore do array até a posição correta,
// garantindo que a propriedade de heap seja mantida, ou seja, 
//que o processo com maior prioridade esteja sempre no topo da árvore, 
//permitindo que a escolha do próximo processo a ser executado seja feita em O(1) e a inserção e remoção sejam feitas em O(log N)
static void subir_max_heap(int heap[], int i, const Processo processos[]) {
    while (i > 0) {
        int pai = (i - 1) / 2; // Índice do pai na árvore binária representada por array
        if (prioridade_efetiva(&processos[heap[i]]) > prioridade_efetiva(&processos[heap[pai]])) {
            trocar_indices(&heap[i], &heap[pai]); // Troca o processo atual com seu pai se tiver prioridade maior
            i = pai; // Move para o índice do pai para continuar subindo na árvore
        } else {
            break; // Se a prioridade do processo atual não for maior que a do pai, a propriedade de heap está satisfeita e o loop pode ser interrompido
        }
    }
}

//Função auxiliar para fazer o processo descer na árvore do array até a posição correta,
//garantindo que a propriedade de heap seja mantida, ou seja, que o processo com maior 
//prioridade esteja sempre no topo da árvore.
static void descer_max_heap(int heap[], int i, int tamanho, const Processo processos[]) {
    while (2 * i + 1 < tamanho) { // Enquanto houver pelo menos um filho
        int filho_esquerdo = 2 * i + 1; // Índice do filho esquerdo
        int filho_direito = 2 * i + 2; // Índice do filho direito
        int maior = i; // Assume que o processo atual é o maior

        if (filho_esquerdo < tamanho && prioridade_efetiva(&processos[heap[filho_esquerdo]]) > prioridade_efetiva(&processos[heap[maior]])) {
            maior = filho_esquerdo; // O filho esquerdo tem prioridade maior que o processo atual
        }
        if (filho_direito < tamanho && prioridade_efetiva(&processos[heap[filho_direito]]) > prioridade_efetiva(&processos[heap[maior]])) {
            maior = filho_direito; // O filho direito tem prioridade maior que o processo atual ou o filho esquerdo
        }
        if (maior != i) {
            trocar_indices(&heap[i], &heap[maior]); // Troca o processo atual com o maior dos filhos
            i = maior; // Move para o índice do maior para continuar descendo na árvore
        } else {
            break; // Se o processo atual for maior que ambos os filhos, a propriedade de heap está satisfeita e o loop pode ser interrompido
        }
    }
}

//Função auxiliar para adicionar um processo e ajustar a ordem
static void inserir_max_heap(int heap[], int *tamanho, int indice_processo, const Processo processos[]) {
    heap[*tamanho] = indice_processo; // Adiciona o índice do processo ao final do heap
    (*tamanho)++; // Incrementa o tamanho do heap
    subir_max_heap(heap, *tamanho - 1, processos); // Ajusta a posição do novo processo para manter a propriedade de heap
}

//Função para remover e retornar o processo com maior prioridade (o processo no topo do heap), e ajustar a ordem dos processos restantes para manter a propriedade de heap, garantindo que o próximo processo com maior prioridade esteja no topo para a próxima escolha
static int extrair_max_heap(int heap[], int *tamanho, const Processo processos[]) {
    if (*tamanho <= 0) {
        return -1; // Retorna -1 se o heap estiver vazio, indicando que não há processos prontos para execução
    }
    int indice_processo = heap[0]; // O processo com maior prioridade está no topo do heap (índice 0)
    heap[0] = heap[*tamanho - 1]; // Move o último processo do heap para o topo
    (*tamanho)--; // Decrementa o tamanho do heap
    descer_max_heap(heap, 0, *tamanho, processos); // Ajusta a posição do processo movido para manter a propriedade de heap
    return indice_processo; // Retorna o índice do processo com maior prioridade que foi extraído do heap
}

// Função para escolher um processo usando o algoritmo de loteria
// Sorteia um "bilhete" e percorre a lista até encontrar o processo dono daquele bilhete.
static int escolher_por_loteria(const Processo processos[], const int pronto[], int n) {
    int i;
    int total_bilhetes = 0;
    int acumulado = 0;
    int sorteio;

    // 1. Somar todos os bilhetes dos processos que estão prontos
    for (i = 0; i < n; i++) {
        if (pronto[i]) {
            total_bilhetes += prioridade_efetiva(&processos[i]);
        }
    }

    if (total_bilhetes <= 0) {
        return -1;
    }

    //2. Gerar um número aleatório entre 1 e total_bilhetes
    sorteio = (rand() % total_bilhetes) + 1;

    //3. Percorrer os processos prontos acumulando seus bilhetes até que o sorteio seja menor
    // ou igual ao acumulado, indicando que o processo atual é o escolhido
    for (i = 0; i < n; i++) {
        if (!pronto[i]) {
            continue;
        }
        acumulado += prioridade_efetiva(&processos[i]);
        if (sorteio <= acumulado) {
            return i;// Processo atual segura o bilhete sorteado, então ele é escolhido para execução
        }
    }

    return -1;
}

// Função que calcula o número de trocas reais criando um "clone" da memória 
// para não interferir na simulação da CPU
static int calcular_trocas_isoladas(const Processo processos[], int n, int politica) {
    int total_trocas = 0;
    
    for (int i = 0; i < n; i++) {
        // Copia todo o estado do processo (incluindo o limite_molduras calculado)
        Processo clone = processos[i]; 
        
        // Zera a "RAM" do clone para a simulação ser justa
        clone.n_paginas_ocupadas = 0;
        clone.ponteiro_fifo = 0;
        for(int j = 0; j < MAX_MOLDURAS; j++) {
            clone.paginas[j] = -1;
            clone.tempo_carregamento[j] = 0;
            clone.contador_nfu[j] = 0;
        }
        
        // Simula a sequência inteira do processo para este algoritmo específico
        for (int a = 0; a < clone.total_acessos_sequencia; a++) {
            int pagina = clone.sequencia_acessos[a];
            int *futuro = &clone.sequencia_acessos[a + 1];
            int tam_futuro = clone.total_acessos_sequencia - (a + 1);
            
            // Passa o endereço do clone. Se a política pedir troca, contabiliza!
            if (gerenciar_acesso(&clone, pagina, politica, futuro, tam_futuro) == 1) {
                total_trocas++;
            }
        }
    }
    return total_trocas;
}

//// INFRAESTRUTURA DE SIMULAÇÃO ////
// Funções para manipular a fila de processos no algoritmo de alternância (Round Robin)
// Remove o primeiro processo da fila para ele ir para a CPU
static int pop_fila_rr(int fila[], int *inicio, int *tamanho) {
    int indice = fila[*inicio];
    // O '%' garante que o índice volte a 0 ao chegar no final do array
    *inicio = (*inicio + 1) % MAX_PROCESSOS;
    (*tamanho)--;
    return indice;
}

// Função para adicionar um processo à fila de alternância, garantindo que ele não seja adicionado mais de uma vez
// Adiciona um processo ao final da fila
static void push_fila_rr(int fila[], int *fim, int *tamanho, int em_fila[], int indice) {
    if (indice < 0 || em_fila[indice] || *tamanho >= MAX_PROCESSOS) {
        return;
    }
    fila[*fim] = indice;
    *fim = (*fim + 1) % MAX_PROCESSOS;
    (*tamanho)++;
    // Marca que o processo já está na fila para evitar duplicatas
    em_fila[indice] = 1;
}

//// FUNÇÃO DE SIMULAÇÃO PRINCIPAL ////
// Função principal para simular o escalonamento de processos com base na política escolhida
// Esta função gerencia a passagem do tempo e chama o algoritmo correto para escolher o 
// próximo processo a ser executado, além de atualizar os tempos de espera e conclusão dos processos
static void simular(Processo processos[], int n, int quantum, Politica politica) {
    int i;
    int tempo = 0;
    int concluidos = 0;
    int em_execucao = -1; // -1 = CPU ociosa, ou seja, sem processos em execução
    int quantum_usado = 0;

    //VARIÁVEIS PARA A SIMULAÇÃO DE MEMÓRIA
    int total_trocas_fifo = 0;
    int total_trocas_lru = 0;
    int total_trocas_nfu = 0;
    int total_trocas_otimo = 0;

    //Novas variáveis para controle de I/O
    int tempo_io_restante[MAX_PROCESSOS] = {0};
    int estado_processo[MAX_PROCESSOS] = {0};

    // Vetor de booleanos para indicar quais 
    //processos estão prontos para execução para Loteria, Prioridade e CFS
    int pronto[MAX_PROCESSOS] = {0};

    // Variáveis de controle para a fila de processos no algoritmo de alternância (Round Robin)
    int fila_rr[MAX_PROCESSOS] = {0};
    int rr_inicio = 0;
    int rr_fim = 0;
    int rr_tamanho = 0;
    int em_fila_rr[MAX_PROCESSOS] = {0};

    // Variáveis para controle da interface gráfica
    const char* nome_algoritmo = "";
    if (politica == POLITICA_ALTERNANCIA) nome_algoritmo = "Round Robin";
    else if (politica == POLITICA_PRIORIDADE) nome_algoritmo = "Prioridade";
    else if (politica == POLITICA_LOTERIA) nome_algoritmo = "Loteria";
    else nome_algoritmo = "CFS (Rubro-Negra)";

    //Estrutuas para Prioridade e CFS
    int max_heap_prioridade[MAX_PROCESSOS] = {0};
    int heap_p_tamanho = 0;
    Node* rbtree_cfs = NULL;//Raiz da árvore rubro negra

    // Garantir que o quantum seja positivo para evitar loops infinitos
    if (quantum <= 0) {
        quantum = 1;
    }

    // Inicializar os processos com seus tempos restantes, tempos de espera e conclusão
    // Resetando o tempo restante para o tempo total de execução, o tempo de espera para 0,
    // o tempo de conclusão para -1 (indicando que ainda não foi concluído) e o vruntime para 0.0f (usado no CFS)
    for (i = 0; i < n; i++) {
        processos[i].tempo_restante = processos[i].tempo_execucao_total;
        processos[i].tempo_espera = 0;
        processos[i].tempo_conclusao = -1;
        processos[i].na_cpu = 0;
        processos[i].vruntime = 0.0f;
    }

    // Loop principal de simulação, que continua até que todos os processos sejam concluídos
    // Relógio do SO, cada loop é uma unidade de tempo, e a cada unidade de tempo o sistema verifica se há novos processos criados,
    // se o processo em execução terminou ou esgotou seu quantum, e atualiza os tempos de espera dos processos prontos,
    //além de imprimir o estado atual da CPU e, ao final, um relatório com os tempos de criação, conclusão,
    // turnaround e tempo em estado pronto de cada processo
    while (concluidos < n) {
        // 0. Atualizar processos bloqueados em I/O
        for (i = 0; i < n; i++) {
            if (processos[i].estado == ESTADO_BLOQUEADO) {
                processos[i].tempo_io_restante--;
                if (processos[i].tempo_io_restante <= 0) {
                    processos[i].estado = ESTADO_PRONTO;
                    // Retorna para a estrutura de prontos correspondente
                    if (politica == POLITICA_ALTERNANCIA) {
                        push_fila_rr(fila_rr, &rr_fim, &rr_tamanho, em_fila_rr, i);
                    } else if (politica == POLITICA_PRIORIDADE) {
                        inserir_max_heap(max_heap_prioridade, &heap_p_tamanho, i, processos);
                    } else if (politica == POLITICA_CFS) {
                        inserir_rbtree(&rbtree_cfs, i, processos[i].vruntime);
                    } else {
                        pronto[i] = 1;
                    }
                }
            }
        }
        // 1. Verificar chegadas. Alguém "nasceu" neste exato 'tempo'?
        for (i = 0; i < n; i++) {
            if (processos[i].tempo_criacao == tempo && processos[i].tempo_restante > 0) {
                if (politica == POLITICA_ALTERNANCIA) {
                    // Botando no final da fila de alternância, garantindo que ele seja considerado para execução na próxima escolha de processo
                    push_fila_rr(fila_rr, &rr_fim, &rr_tamanho, em_fila_rr, i);
                } 
                //1. Direcionando para as estruturas novas de cada algoritmo, para que eles possam ser escolhidos na próxima escolha de processo
                else if (politica == POLITICA_PRIORIDADE) {
                    //Insere o processo na árvore de prioridade
                    inserir_max_heap(max_heap_prioridade, &heap_p_tamanho, i, processos);
                } else if (politica == POLITICA_CFS) {
                    inserir_rbtree(&rbtree_cfs, i, processos[i].vruntime);
                // Código anterior era somente esse else
                } else {
                    // Fica disponível para Loteria, marcando como pronto para que eles possam ser escolhidos na próxima escolha de processo
                    pronto[i] = 1;
                }
            }
        }
        
        // 2. Se a CPU está livre, vamos escolher alguém.
        // Se não há processo em execução, escolher um novo processo com base na política de escalonamento
        if (em_execucao < 0) {
            if (politica == POLITICA_ALTERNANCIA) {
                if (rr_tamanho > 0) {
                    em_execucao = pop_fila_rr(fila_rr, &rr_inicio, &rr_tamanho);
                    // Saiu da fila, vai para a CPU
                    em_fila_rr[em_execucao] = 0;
                }
            // Atualização para puxar as novas estruturas de cada algoritmo
            } else if (politica == POLITICA_PRIORIDADE) {
                em_execucao = extrair_max_heap(max_heap_prioridade, &heap_p_tamanho, processos);
            } else if (politica == POLITICA_LOTERIA) {
                em_execucao = escolher_por_loteria(processos, pronto, n);
                if (em_execucao >= 0) pronto[em_execucao] = 0;
            } else {
                em_execucao = extrair_menor_rbtree(&rbtree_cfs);
            }
            quantum_usado = 0;
        }

        // 3. Se mesmo após o escalonamento ninguém foi escolhido (nenhum processo chegou ainda).
        // Se há um processo em execução, processá-lo por um ciclo de tempo
        if (em_execucao < 0) {
            //imprime IDLE, indicando que a CPU está ociosa, e avança o tempo para a próxima unidade de tempo,
            // onde pode haver novos processos chegando ou prontos para execução
            animar_execucao(tempo, -1, processos, n, nome_algoritmo);
            //imprimir_execucao(tempo, -1, processos);
            tempo++;
            continue;// Avança o relógio e pula para o próximo ciclo
        }

        // 4. Execução na CPU (Trabalho sendo feito)
        // Marca que o processo está na CPU para controle de estado, e imprime o estado atual da CPU, mostrando qual processo está rodando e quanto tempo restante ele tem
        processos[em_execucao].na_cpu = 1;
        //ADIÇÃO DE ANIMAÇÃO PARA A INTERFACE DA MEMÓRIA
        Processo *p = &processos[em_execucao];
        p->ultima_pagina_pedida = -1; 
        p->sofreu_page_fault = 0;

        // LÓGICA DE ACESSO À MEMÓRIA (Acessa primeiro)
        if (p->acesso_atual < p->total_acessos_sequencia) {
            int pagina_pedida = p->sequencia_acessos[p->acesso_atual];
            p->ultima_pagina_pedida = pagina_pedida; 

            // Preparando os dados para qualquer algoritmo de memória (FIFO, LRU, Ótimo)
            int *futuro = &p->sequencia_acessos[p->acesso_atual + 1];
            int tamanho_futuro = p->total_acessos_sequencia - (p->acesso_atual + 1);
            
            // Simulação para o FIFO
            int houve_troca = gerenciar_acesso(p, pagina_pedida, POLITICA_FIFO, NULL, 0);
            
            if (houve_troca == 1) {
                total_trocas_fifo++;
                p->sofreu_page_fault = 1; // Avisa a interface que ocorreu uma troca
            }
            p->acesso_atual++;
        }
        //Anima a execução com os dados do acesso recém feito
        animar_execucao(tempo, em_execucao, processos, n, nome_algoritmo);

        // CONTABILIZAÇÃO DO TEMPO DE CPU
        p->tempo_restante--; 
        quantum_usado++;

        // Matemática do CFS
        if (politica == POLITICA_CFS) {
            processos[em_execucao].vruntime += 1.0f / prioridade_efetiva(&processos[em_execucao]);
        }
        
        //5. Atualizar o tempo de espera dos processos prontos, garantindo que apenas os processos 
        //que estão prontos e não em execução tenham seu tempo de espera incrementado, para refletir o tempo que eles passaram esperando na fila para serem executados
        // Incrementar o tempo de espera dos processos que estão prontos, mas não em execução
        for (i = 0; i < n; i++) {
            // Se for o processo atual, se já terminou ou se ainda não nasceu, ignora.
            if (i == em_execucao || processos[i].tempo_restante <= 0 || processos[i].tempo_criacao > tempo) {
                continue;
            }

        //ATUALIZAÇÃO 3. Os processos agora podem estar no array pronto[], na fila_rr, no Max-Heapou na RB-Tree.
        // A forma mais genérica de contar tempo de espera é simplesmente aumentar se ele nasceu e não está na CPU.
        // O IF original abaixo funcionava apenas para as estruturas antigas. Simplificamos a contagem geral.
        processos[i].tempo_espera = processos[i].tempo_espera +  1; // Incrementa o tempo de espera para todos os processos que nasceram e não estão na CPU, independentemente da estrutura de dados usada para gerenciar os processos prontos, garantindo que o tempo de espera seja contabilizado corretamente para todos os processos, mesmo com as novas estruturas de dados implementadas para cada algoritmo de escalonamento
        }

        //6. Condições de saída da CPU: Verificar se o processo em execução foi concluído ou se o quantum foi esgotado, para decidir se ele deve ser removido da CPU e, no caso do Round Robin, colocado de volta na fila para esperar sua próxima vez de execução
        // Verificar se o processo em execução foi concluído ou se o quantum foi esgotado
        if (processos[em_execucao].tempo_restante == 0) {
            // O processo terminou sua execução, então registra o tempo de conclusão, marca que ele não está mais na CPU, e incrementa o contador de processos concluídos
            processos[em_execucao].tempo_conclusao = tempo + 1;
            processos[em_execucao].na_cpu = 0;
            // Libera a CPU, permitindo que um novo processo seja escolhido no próximo ciclo de simulação
            em_execucao = -1;
            quantum_usado = 0;
            concluidos++;
        } else if (quantum_usado >= quantum) {
            // O processo não terminou, mas atingiu a sua fatia de tempo máxima, então ele deve ser preemptado e colocado de volta na fila (no caso do Round Robin) ou marcado como pronto para os outros algoritmos, para que possa ser escolhido novamente no futuro
            processos[em_execucao].na_cpu = 0;

            //ATUALIZAÇÃO 4. Reinsere nas estruturas otimizadas
            if (politica == POLITICA_ALTERNANCIA) {
                push_fila_rr(fila_rr, &rr_fim, &rr_tamanho, em_fila_rr, em_execucao);
            } else if (politica == POLITICA_PRIORIDADE) {
                inserir_max_heap(max_heap_prioridade, &heap_p_tamanho, em_execucao, processos);
            } else if (politica == POLITICA_CFS) {
                inserir_rbtree(&rbtree_cfs, em_execucao, processos[em_execucao].vruntime);
            } else {
                // Volta para o estado pronto (Loteria)
                pronto[em_execucao] = 1;
            }

            // Libera a CPU
            em_execucao = -1;
            quantum_usado = 0;
        }
        // O ciclo termina, então avança o tempo para a próxima unidade de tempo, onde pode haver novos processos chegando ou prontos para execução, e o processo em execução pode continuar ou ser preemptado dependendo do algoritmo de escalonamento e do quantum
        tempo++;
    }

    //Libera a memória da Árvore Rubro-Negra para evitar Memory Leak (vazamento de memória)
    if (politica == POLITICA_CFS) {
        libertar_rbtree(rbtree_cfs);
    }

    // Agora que a CPU terminou, calculamos a performance real e independente
    // de cada política de memória.
    total_trocas_fifo = calcular_trocas_isoladas(processos, n, POLITICA_FIFO);
    total_trocas_lru = calcular_trocas_isoladas(processos, n, POLITICA_LRU);
    total_trocas_nfu = calcular_trocas_isoladas(processos, n, POLITICA_NFU);
    total_trocas_otimo = calcular_trocas_isoladas(processos, n, POLITICA_OTIMO);

    // Simulação terminou, então imprime o relatório final com os tempos de criação, conclusão, turnaround e tempo em estado pronto de cada processo, para que o usuário possa analisar o desempenho do algoritmo de escalonamento escolhido
    //imprimir_relatorio_final(processos, n);
    imprimir_relatorio_colorido(processos, n);
    //Imprimindo relatório da memória
    imprimir_relatorio_memoria(total_trocas_fifo, total_trocas_lru, total_trocas_nfu, total_trocas_otimo);
}

//// FUNÇÕES DE INTERFACE PARA CADA ALGORITMO DE ESCALONAMENTO ////
// Funções para iniciar a simulação de cada algoritmo de escalonamento, imprimindo o nome do algoritmo e chamando a função de simulação com a política correspondente
void escalonar_alternancia(Processo processos[], int n, int quantum) {
    printf("=== Escalonamento: Alternancia Circular (Round Robin) ===\n");
    simular(processos, n, quantum, POLITICA_ALTERNANCIA);
}

void escalonar_prioridade(Processo processos[], int n, int quantum) {
    printf("=== Escalonamento: Prioridade ===\n");
    simular(processos, n, quantum, POLITICA_PRIORIDADE);
}

void escalonar_loteria(Processo processos[], int n, int quantum) {
    printf("=== Escalonamento: Loteria ===\n");
    simular(processos, n, quantum, POLITICA_LOTERIA);
}

void escalonar_cfs(Processo processos[], int n, int quantum) {
    printf("=== Escalonamento: CFS ===\n");
    simular(processos, n, quantum, POLITICA_CFS);
}