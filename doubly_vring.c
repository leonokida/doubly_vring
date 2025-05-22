/* Autor: Leon A. Okida Gonçalves.
   Data da Última Modificação: 22/maio/2025
   Descrição: VRing baseado em heartbeats */

// Bibliotecas
#include <stdio.h>
#include <stdlib.h>
#include "smpl.h"

// Eventos
#define HEARTBEAT 1
#define FAULT 2
#define PROCESSAMENTO 3

// Status
#define UNKNOWN -1
#define CORRETO 0
#define SUSPEITO 1


typedef struct{ 
    int id; // identificador de facility do SMPL
    int *State; // vetor State[] do processo
    int contador; // quantas rodadas já executou
    int antecessor_horario; // de quem recebe heartbeat no sentido horario
    int sucessor_horario; // para quem envia hearbeat no sentido horario
    int antecessor_antihorario; // de quem recebe heartbeat no sentido anti-horario
    int sucessor_antihorario; // para quem envia heartbeat no sentido anti-horario
    int *Heartbeats; // processos de quem recebeu heartbeat
    int *ultima_atualizacao; // última rodada em que o valor foi atualizado no vetor State
} TipoProcesso;
TipoProcesso *processo;

int main (int argc, char *argv[]) {
    static int N;
    int token, event, req;
    int MaxTempoSimulac = 155; // Para sistemas distribuídos menores
    //int MaxTempoSimulac = 200; // Para sistemas distribuídos maiores
    
    static char fa_name[5];

    if (argc != 2) {
        printf("Uso correto do programa: ./doubly_vring <numero de processos>\n");
        exit(1);
    }

    N = atoi(argv[1]);

    // Inicializa SMPL
    smpl(0, "Doubly VRing");
    reset();
    stream(1);

    // Inicializa processos
    processo = (TipoProcesso *) malloc(sizeof(TipoProcesso)*N);
    for (int i = 0; i < N; i++) {
        memset(fa_name, '\0', 5);
        sprintf(fa_name, "%d", i);
        processo[i].id = facility(fa_name,1);

        // Inicializa vetor State com valor -1 para todos os processos exceto i
        // Inicializa vetor Hearbeats
        // Inicializa vetor de última atualização
        processo[i].State = (int *)malloc(sizeof(int)*N);
        processo[i].Heartbeats = (int *)malloc(sizeof(int)*N);
        processo[i].ultima_atualizacao = (int *)malloc(sizeof(int)*N);
        for (int j = 0; j < N; j++) {
            if (j != i) {
                processo[i].State[j] = UNKNOWN;
                processo[i].Heartbeats[j] = 0;
                processo[i].ultima_atualizacao[j] = 0;
            }
        }
        processo[i].State[i] = CORRETO;
        processo[i].Heartbeats[i] = 1;

        // Inicializa contador e sucessores/antecessores
        processo[i].contador = 0;
        processo[i].antecessor_antihorario = ((i + 1) % N);
        processo[i].sucessor_horario = processo[i].antecessor_antihorario;
        processo[i].antecessor_horario = ((i - 1 + N) % N);
        processo[i].sucessor_antihorario = processo[i].antecessor_horario;
    }

    // Escalonamento inicial de eventos
    for (int i = 0; i < N; i++) {
        schedule(HEARTBEAT, 15.0, i); 
    }

    /* Programando falhas para experimentos */
    // Experimento 1: 3 falhas em um sistema distribuído com 8 processos
    //schedule(FAULT, 31.0, 3);
    //schedule(FAULT, 61.0, 4);
    //schedule(FAULT, 91.0, 6);

    // Experimento 2: 2 falhas em um sistema distribuído com 4 processos
    //schedule(FAULT, 31.0, 0);
    //schedule(FAULT, 31.0, 2);

    // Experimento 3: 8 falhas em um sistema distribuído com 16 processos
    //schedule(FAULT, 8.0, 8);
    //schedule(FAULT, 16.0, 9);
    //schedule(FAULT, 31.0, 10);
    //schedule(FAULT, 51.0, 11);
    //schedule(FAULT, 61.0, 12);
    //schedule(FAULT, 71.0, 13);
    //schedule(FAULT, 91.0, 14);
    //schedule(FAULT, 101.0, 15);

    // Experimento 4: 12 falhas em um sistema distribuído com 32 processos
    //schedule(FAULT, 31.0, 1);
    //schedule(FAULT, 31.0, 3);
    //schedule(FAULT, 46.0, 6);
    //schedule(FAULT, 46.0, 8);
    //schedule(FAULT, 61.0, 12);
    //schedule(FAULT, 61.0, 13);
    //schedule(FAULT, 76.0, 15);
    //schedule(FAULT, 76.0, 17);
    //schedule(FAULT, 101.0, 20);
    //schedule(FAULT, 101.0, 22);
    //schedule(FAULT, 116.0, 28);
    //schedule(FAULT, 116.0, 31);

    // Experimento 5: 7 falhas em um sistema distribuído com 8 processos
    //schedule(FAULT, 8.0, 7);
    //schedule(FAULT, 16.0, 5);
    //schedule(FAULT, 31.0, 3);
    //schedule(FAULT, 51.0, 6);
    //schedule(FAULT, 61.0, 4);
    //schedule(FAULT, 71.0, 1);
    //schedule(FAULT, 91.0, 2);

    /* Laço principal do programa */
    puts("===============================================================");
    puts("Autor: Leon Augusto Okida Gonçalves");
    puts("Log do Trabalho - VRing Baseado em Push - Doubly VRing");
    printf("Este programa foi executado para: N=%d processos.\n", N); 
    printf("Tempo Total de Simulação = %d\n", MaxTempoSimulac);
    puts("===============================================================");

    while(time() <= MaxTempoSimulac) {
        int j;
        cause(&event, &token);
        switch(event) {
            case HEARTBEAT: 
                if (status(processo[token].id) !=0) break; // se o processo está falho, não envia heartbeat!
                //Sentido Horário
                if ((processo[token].contador % 2) == 0)
                    j = processo[token].sucessor_horario;
                // Sentido Anti-Horário
                else
                    j = processo[token].sucessor_antihorario;

                printf("Processo %d: estou enviando heartbeat para o processo %d no tempo %4.1f\n", token, j, time());   
                if (status(processo[j].id) == 0) {
                    processo[j].Heartbeats[token] = 1;
                }

                // Evento PROCESSAMENTO verifica se o processo recebeu algum heartbeat
                // Escalona nova rodada em 15 unidades de tempo
                schedule(PROCESSAMENTO, 5.0, token);
                schedule(HEARTBEAT, 15.0, token);
                break;

            case PROCESSAMENTO:
                if (status(processo[token].id) !=0) break;

                // Sentido Horário
                if ((processo[token].contador % 2) == 0) {
                    // Verifica se recebeu heartbeat do processo esperado
                    j = processo[token].antecessor_horario;
                    if (processo[token].Heartbeats[j]) {
                        printf("Processo %d: processei heartbeat do processo %d no tempo %4.1f\n", token, j, time());
                        processo[token].State[j] = CORRETO;
                        processo[token].ultima_atualizacao[j] = processo[token].contador;
                    }                        

                    // Não recebeu heartbeat do processo esperado
                    else {
                        processo[token].State[j] = SUSPEITO;
                        processo[token].ultima_atualizacao[j] = processo[token].contador;
                        do {
                            j = ((j - 1 + N) % N);
                        } while ((!processo[token].Heartbeats[j]) && (j != token));
                        if ((processo[token].Heartbeats[j]) && (j != token)) {
                            printf("Processo %d: processei heartbeat do processo %d no tempo %4.1f\n", token, j, time());
                            processo[token].State[j] = CORRETO;
                            processo[token].ultima_atualizacao[j] = processo[token].contador;
                        }
                    }

                    // Copia vetor State de quem recebeu o heartbeat (se valores forem mais atualizados)
                    // Altera antecessor no sentido horário e sucessor no sentido anti-horário para quem recebeu o heartbeat
                    if ((processo[token].Heartbeats[j]) && (j != token)) {
                        for (int i = 0; i < N; i++) {
                            if ((!processo[token].Heartbeats[i]) && (processo[j].ultima_atualizacao[i] > processo[token].ultima_atualizacao[i])) {
                                processo[token].State[i] = processo[j].State[i];
                                processo[token].ultima_atualizacao[i] = processo[j].ultima_atualizacao[i];
                            }
                            if (i != token)
                                processo[token].Heartbeats[i] = 0;
                        }

                        processo[token].antecessor_horario = j;
                        processo[token].sucessor_antihorario = j;
                    }
                    else {
                        printf("Processo %d: não recebi heartbeat algum no tempo %4.1f\n", token, time());

                        // Testa o sucessor do sucessor (que não estiver sabidamente falho) na próxima rodada
                        do {
                            processo[token].antecessor_horario = ((processo[token].antecessor_horario - 1 + N) % N);
                            processo[token].sucessor_antihorario = processo[token].antecessor_horario;
                        } while((processo[token].State[processo[token].antecessor_horario] == SUSPEITO) && (processo[token].antecessor_horario != token));

                        // Se não encontrar, é o único processo funcional do sistema distribuído
                        if (processo[token].antecessor_horario == token) {
                            printf("Processo %d: sou o único processo correto restante no tempo %4.1f\n", token, time());
                            printf("Processo %d: State = [", token);
                            for (int k = 0; k < N-1; k++) {
                                printf(" %d,", processo[token].State[k]);
                            }
                            printf(" %d ]\n", processo[token].State[N-1]);
                            printf("Encerrando simulação...\n");
                            exit(0);
                        }
                    }
                }

                // Sentido Anti-Horário
                else {
                    // Verifica se recebeu heartbeat do processo esperado
                    j = processo[token].antecessor_antihorario;
                    if (processo[token].Heartbeats[j]) {
                        printf("Processo %d: processei heartbeat do processo %d no tempo %4.1f\n", token, j, time());
                        processo[token].State[j] = CORRETO;
                        processo[token].ultima_atualizacao[j] = processo[token].contador;
                    }

                    // Não recebeu heartbeat do processo esperado
                    else {
                        processo[token].State[j] = SUSPEITO;
                        processo[token].ultima_atualizacao[j] = processo[token].contador;
                        do {
                            j = ((j + 1) % N);
                        } while ((!processo[token].Heartbeats[j]) && (j != token));
                        if ((processo[token].Heartbeats[j]) && (j != token)) {
                            printf("Processo %d: processei heartbeat do processo %d no tempo %4.1f\n", token, j, time());
                            processo[token].State[j] = CORRETO;
                            processo[token].ultima_atualizacao[j] = processo[token].contador;
                        }
                    }

                    // Copia vetor State de quem recebeu o heartbeat (se valores forem mais atualizados)
                    // Altera antecessor no sentido anti-horário e sucessor no sentido horário para quem recebeu o heartbeat
                    if ((processo[token].Heartbeats[j]) && (j != token)) {
                        for (int i = 0; i < N; i++) {
                            if ((!processo[token].Heartbeats[i]) && (processo[j].ultima_atualizacao[i] > processo[token].ultima_atualizacao[i])) {
                                processo[token].State[i] = processo[j].State[i];
                                processo[token].ultima_atualizacao[i] = processo[j].ultima_atualizacao[i];
                            }
                            if (i != token)
                                processo[token].Heartbeats[i] = 0;
                        }

                        processo[token].antecessor_antihorario = j;
                        processo[token].sucessor_horario = j;
                    }
                    else {
                        printf("Processo %d: não recebi heartbeat algum no tempo %4.1f\n", token, time());

                        // Testa o sucessor do sucessor (que não estiver sabidamente falho) na próxima rodada
                        do {
                            processo[token].antecessor_antihorario = ((processo[token].antecessor_antihorario + 1) % N);
                            processo[token].sucessor_horario = processo[token].antecessor_antihorario;
                        } while((processo[token].State[processo[token].antecessor_antihorario] == SUSPEITO) && (processo[token].antecessor_antihorario != token));

                        // Se não encontrar, é o único processo funcional do sistema distribuído
                        if (processo[token].antecessor_antihorario == token) {
                            printf("Processo %d: sou o único processo correto restante no tempo %4.1f\n", token, time());
                            printf("Processo %d: State = [", token);
                            for (int k = 0; k < N-1; k++) {
                                printf(" %d,", processo[token].State[k]);
                            }
                            printf(" %d ]\n", processo[token].State[N-1]);
                            printf("Encerrando simulação...\n");
                            exit(0);
                        }
                    }
                }

                // Imprime State[]
                printf("Processo %d: State = [", token);
                    for (int k = 0; k < N-1; k++) {
                        printf(" %d,", processo[token].State[k]);
                    }
                printf(" %d ]\n", processo[token].State[N-1]);

                // Atualiza contador de rodadas para inverter o sentido da próxima rodada
                processo[token].contador++;
                break;

            case FAULT:
                req = request(processo[token].id, token, 0);
                printf("Socooorro!!! Sou o processo %d  e estou falhando no tempo %4.1f\n", token, time());

                // Esvazia heartbeats recebidos
                for (int i = 0; i < N; i++) {
                    if (i != token)
                        processo[token].Heartbeats[i] = 0;
                }

                break;
        }
   }
}
