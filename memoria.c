#include <stdio.h>
#include "memoria.h"

//Variável declarada como global e static para saber qual o tempo global da simulação,
//para atualizar os tempos de carregamento das páginas
//RELÓGIO GLOBAL PARA A SIMULAÇÃO DE LRU
static int tempo_global = 0;

int gerenciar_acesso(Processo *P,int pagina_acessada,int politica,int* sequencia_futura,int tamanho_futuro){
    //Incrementando o tempo global a cada acesso para o LRU
    tempo_global++; 

    //Primeira passo: Verificar se está na memória
    //percorrendo o array de páginas do processo
    for (int i = 0; i < P -> n_paginas_ocupadas; i++) {
        //printf("Verificando página %d do processo %s (tempo de carregamento: %d).\n", P->paginas[i], P->pid, P->tempo_carregamento[i]);
        //Verificando se a página está na memória
            if (P->paginas[i] == pagina_acessada) {
                //printf("Página %d já está na memória do processo %s.\n",pagina_acessada, P->pid);

                if (politica == POLITICA_LRU) {
                    P->tempo_carregamento[i] = tempo_global;
                    //printf("Política LRU, tempo de carregamento da página %d do processo %s atualizado para %d.\n",pagina_acessada, P->pid, tempo_global);
                }

                if (politica == POLITICA_NFU) {
                    P->contador_nfu[i]++;
                }

                //Se for FIFO, não precisa atualizar o tempo de carregamento, pois a ordem é fixa.
                //Tanto para o FIFO quanto para o LRU, se a página já estiver na memória, 
                //não há troca, então retorna 0
                return 0;
            }
    }

    //Segundo passo: Verificar se está cheio
    //Se cheio ele chama a respectiva politica
    if (P->n_paginas_ocupadas == P->limite_molduras) {
        if (politica == POLITICA_FIFO) {
            return simular_fifo(P, pagina_acessada);
        } else if (politica == POLITICA_LRU) {
            return simular_lru(P, pagina_acessada);
        } else if (politica == POLITICA_NFU) {
            return simular_nfu(P, pagina_acessada);
        } else if (politica == POLITICA_OTIMO) {
            return simular_otimo(P, pagina_acessada, sequencia_futura, tamanho_futuro);
        }
    }

    //Terceiro passo: memória livre, adiciona na pagina    
    else {
        int pos = P->n_paginas_ocupadas;
        P->paginas[pos] = pagina_acessada;
        
        //Atualiza o tempo de carregamento da página inserida,
        //que é o tempo global atual para o LRU, enquanto o FIFO é usada para manter a consistência dos dados
        P->tempo_carregamento[pos] = tempo_global;
        
        // Garante que o NFU comece a contar assim que a página entra na memória livre
        if (politica == POLITICA_NFU) {
            P->contador_nfu[pos] = 1; 
        }
        P->n_paginas_ocupadas++;
        return 0; // Inserido, sem troca
    }    
    return 0;
}

// Função FIFO: usa o ponteiro circular da struct
int simular_fifo(Processo *P, int pagina_acessada){
    //printf("Memória cheia (FIFO). Substituindo página %d por %d.\n", P->paginas[P->ponteiro_fifo], pagina_acessada);
    P->paginas[P->ponteiro_fifo] = pagina_acessada;
    P->ponteiro_fifo = (P->ponteiro_fifo + 1) % P->limite_molduras; 
    return 1; // Página substituída, houve troca
}

// Função LRU: procura a página com menor timestamp
int simular_lru(Processo *P, int pagina_acessada) {
    int indice_mais_antigo = 0;
    int menor_tempo = P->tempo_carregamento[0];

    // Procura a página que não é usada há mais tempo
    for (int i = 1; i < P->limite_molduras; i++) {
        if (P->tempo_carregamento[i] < menor_tempo) {
            menor_tempo = P->tempo_carregamento[i];
            indice_mais_antigo = i;
        }
    }
    
    P->paginas[indice_mais_antigo] = pagina_acessada;
    P->tempo_carregamento[indice_mais_antigo] = tempo_global;
    
    return 1; // Página substituída, houve troca
}

int simular_nfu(
    Processo *P,
    int pagina_acessada) 
    {
    int indice_remover = 0;
    int menor_frequencia = P->contador_nfu[0];
    // Escolhe a página com menor contador de uso
    // Em caso de empate, escolhe a página de menor ID
    for (int i = 1; i < P->limite_molduras; i++) {
        // Lógica de desempate e menor frequência usando P->contador_nfu[i]
        if (P->contador_nfu[i] < menor_frequencia || 
           (P->contador_nfu[i] == menor_frequencia && P->paginas[i] < P->paginas[indice_remover])) {
            menor_frequencia = P->contador_nfu[i];
            indice_remover = i;
        }
    }

    P->paginas[indice_remover] = pagina_acessada;
    P->contador_nfu[indice_remover] = 1;

    return 1;
}

int simular_otimo(
    Processo *P,
    int pagina_acessada,
    int* sequencia_futura,
    int tamanho_futuro)
    {
    int indice_remover = -1;
    int maior_distancia = -1;

    for (int i = 0; i < P->limite_molduras; i++) {
        int distancia = -1;

        for (int j = 0; j < tamanho_futuro; j++) {
            if (sequencia_futura[j] == P->paginas[i]) {
                distancia = j;
                break;
            }
        }

        // Se a página não será mais usada, ela é a melhor escolha
        if (distancia == -1) {
            indice_remover = i;
            break;
        }

        // Caso contrário, remove a que será usada mais tarde
        if (distancia > maior_distancia) {
            maior_distancia = distancia;
            indice_remover = i;
        }
    }

    if (indice_remover == -1) {
        indice_remover = 0;
    }

    P->paginas[indice_remover] = pagina_acessada;

    return 1;
}