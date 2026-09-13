# Simulador de Escalonamento de Processos (Sistemas Operacionais)
Este projeto implementa um simulador de escalonamento de processos preemptivo e gerenciamento de memória virtual em C, desenvolvido como requisito para a disciplina de Sistemas Operacionais. O simulador processa um arquivo de configuração e simula a execução na CPU utilizando quatro algoritmos de escalonamento, comparando simultaneamente quatro políticas de substituição de página.

## Integrantes do Grupo
* [Lara Letittja Sague Lopez Guardiola Velloso/150886]
* [Gustavo do Amaral Gimenes/155468]

## Algoritmos Implementados
### Escalonamento de Processos
1. **Alternância Circular (Round Robin)**
2. **Prioridade**
3. **Loteria**
4. **CFS (Completely Fair Scheduler)**

### Substituição de Páginas (Memória)
1. **FIFO** (First-In, First-Out)
2. **LRU** (Least Recently Used)
3. **NFU** (Not Frequently Used)
4. **Ótimo**

## Estrutura de Arquivos
* `processos.h`: Definição da estrutura do Bloco de Controle de Processo (PCB).
* `processos.c`: Parser de entrada, validação e inicialização do sistema.
* `algoritmoDeEscalonamento.c`: Implementação da lógica de escalonamento e loop principal (relogio).
* `memoria.c` / `memoria.h`: Implementação dos algoritmos de substituição de página.
* `interface.h` / `interface.c`: Módulos da interface gráfica animada via CLI.
* `gerenciador.txt`: Arquivo de entrada com as configurações globais e lista de processos.

## Como Compilar e Executar (WSL / Linux / Unix)
   O sistema foi desenvolvido utilizando bibliotecas padrão da linguagem C (ANSI C), garantindo alta portabilidade.

   1. Abra o terminal e navegue até a pasta do projeto utilizando o caminho do diretório:

      ```bash
      cd /mnt/c/Users/"Lara Letittja"/Documents/GitHub/GerenciadorDeProcessos

      1.1.  Certifique-se de que o arquivo de entrada está com o nome correto (gerenciador.txt). Caso esteja usando o arquivo de exemplo, renomeie-o:

      ```bash
      mv entrada_ES.txt gerenciador.txt

   2. Para compilar todos os módulos juntos, execute o comando:

      ```bash
      gcc *.c -o simulador

   3. Para executar o simulador, digite:

      ```bash
      ./simulador

## Como configurar o arquivo gerenciador.txt
O simulador lê os dados estritamente do arquivo gerenciador.txt localizado no mesmo diretório do executável. O formato deve seguir estritamente o padrão abaixo:

Linha 1: Algoritmo|FatiaDeCPU|PolíticaMemória|TamanhoMemória|TamanhoPáginasMolduras|PercentualAlocação|NumDispositivosES
Linhas de Dispositivos: IdDispositivo|NumUsosSimultaneos|TempoOperação (uma linha para cada dispositivo)
Linhas de Processos: TempoCriação|PID|TempoDeExecução|Prioridade (ou bilhetes)|QtdeMemoria|SequênciaAcessoPaginasProcesso|ChanceRequisitarES

Por exemplo:
alternancia|10|local|65536|512|50|4
device-0|1|3
device-1|2|5
device-2|2|2
device-3|1|6
0|1|20|59|4096|1 2 2 2 3 4 3 4 5 5 6 1 5 3 2 6 7 7 7 8|32
0|2|24|32|2048|1 2 2 2 3 4 3 4 4 4 2 3 2 1 3 2 1 2 2 3 4 3 2 2|12
0|3|32|32|4096|1 2 3 4 5 6 7 8 4 3 2 1 1 6 7 5 6 8 3 2 2 1 2 2 4 4 5 3 2 1 7 8|88

Projeto desenvolvido para a disciplina de Sistemas Operacionais - 2026.