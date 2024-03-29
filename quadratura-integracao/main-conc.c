/*
 * Disciplina: Computacao Concorrente
 * Prof.: Silvana Rossetto
 * Trabalho 1
 * Codigo: Problema da quadratura para integracao numerica
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include "timer.h"  //para usar funcoes de medida de tempo
#include "fila.h"

int threadsExecutando = 0;
int threadsProntas = 0;
int nthreads;
int * iteracoes;

pthread_mutex_t mutex;
pthread_cond_t cond;

fila * nbuffer;

double * resultado;

double func1(double x){ return 1 + x; }
double func2(double x){ return sqrt(1 - pow(x, 2)); }
double func3(double x){ return sqrt(1 + pow(x, 4)); }
double func4(double x){ return sin(pow(x,2)); }
double func5(double x){ return cos(exp(-x)); }
double func6(double x){ return cos(exp(-x)) * x;}
double func7(double x){ return cos(exp(-x)) * ((0.005 * pow(x, 3)) + 1); }

 void * retangulosRecursiva(void * tid) {
  
  struct Values * valores;
  int idThread = * (int *) tid;

  double valorMedio;                              //variavel para guardar o ponto médio no intervalo [A, B]
  double dist;                                    //variavel para guardar a distancia de A para B
  double valorMedioEsquerda, valorMedioDireita;   //variaveis para guardarem os pontos médios no intervalo [A, C] e [C, B]
  double funcaoEmEsquerda, funcaoEmDireita;       //variáveis para guardarem os valores da função avaliadas no ponto médio entre [A, C] e [C, B]
  double integralEsquerda, integralDireita;       //variáveis para calcular a integral no lado esquerdo e direito do intervalo [A, C] e [C, B] pelo método dos retangulos
  double novaIntegral;                            //variável que guarda a nova aproximação da integral da parte dividida;
    
  while(nbuffer->size > 0 || threadsExecutando > 0 || iteracoes[idThread] < 1) {

    iteracoes[idThread]++;
    
    pthread_mutex_lock(&mutex);

    //printf("threads executando = %d e tamanho = %lld\n", threadsExecutando, nbuffer->size);
    if(nbuffer->size == 0){ threadsExecutando--; pthread_mutex_unlock(&mutex); break;}

    while(nbuffer->size == 0) {   }
    threadsExecutando++;
    valores = defila(nbuffer);

    pthread_mutex_unlock(&mutex);
    
    valorMedio = (valores->limiteA + valores->limiteB) / 2; //calcula o ponto médio entre A e B
    dist = valores->limiteB - valores->limiteA; //calcula a distancia entre A e B

    valorMedioEsquerda = (valores->limiteA + valorMedio) / 2;  //calcula o ponto médio entre A e C
    valorMedioDireita = (valorMedio + valores->limiteB) / 2;   //calcula o ponto médio entre C e B

    funcaoEmEsquerda = valores->func(valorMedioEsquerda);  //calcula a função avaliada no ponto medio entre A e C
    funcaoEmDireita = valores->func(valorMedioDireita);    //calcula a função avaliada no ponto medio entre C e B

    integralEsquerda = ( dist / 2 ) * funcaoEmEsquerda; //calcula a integral da parte da esquerda (de A até C)
    integralDireita = ( dist / 2 ) * funcaoEmDireita; //calcula a integral da parte da direita (de C até B)
    
    novaIntegral = integralEsquerda + integralDireita;                                                                       

    if (fabs(novaIntegral - valores->integral) <= 15 * valores->erro) { //Calculo do método da quadratura adaptiva para saber se o novo valor da integral satisfaz o 
                                                                //erro determinado no inicio do problema, caso satisfaça, esse valor é usado para servir como
                                                                //aproximação do valor da integral da função, caso contratário, é chamado mais duas instancias
                                                                //da função recursiva, onde irão separar o lado esquerdo e o lado direito e fazer o mesmo processo,
                                                                //dividindo em esses lados em 2 novos lados até que o erro minimo seja satisfeito, fazendo assim uma
                                                                //série de recursões e divindo cada vez mais a integral em pequenos retangulos nas partes mais cŕticias
      resultado[idThread] += novaIntegral + (novaIntegral - valores->integral) / 15;
    }

    else{
          pthread_mutex_lock(&mutex);

          enfila(nbuffer, valores->func, valores->limiteA, valorMedio, valores->erro/2, integralEsquerda, valores->funcEmA, valores->funcEmMedio, funcaoEmEsquerda);
          enfila(nbuffer, valores->func, valorMedio, valores->limiteB, valores->erro/2, integralDireita, valores->funcEmMedio, valores->funcEmB, funcaoEmDireita);
          free(valores);
          pthread_cond_broadcast(&cond);

          pthread_mutex_unlock(&mutex);
    }

    pthread_mutex_lock(&mutex);
    threadsExecutando--;
    if (threadsExecutando != 0){
      while (nbuffer->size == 0 && threadsExecutando > 0) { pthread_cond_wait(&cond, &mutex); }
    }
    pthread_mutex_unlock(&mutex);
  
  }

  pthread_mutex_lock(&mutex);
  pthread_cond_broadcast(&cond);
  pthread_mutex_unlock(&mutex);

  pthread_exit(NULL);
}

void retangulosInicial(double (*func) (double), double limiteA, double limiteB, double erro) {  

  double valorMedio;                      //variável que vai armazenar o ponto médio entre os intervalos A e B
  double dist;                            //variável que vai armazenar a distancia entre o intervalo A e B
  double funcEmA, funcEmB, funcEmMedio;   //variáveis que vão armazenas os valores da função calculada nos pontos A, B e ponto Médio
  double integral;                        //variável que vai armazenar o valor da aproximação da integral da função avaliada pelo método de simpson

  valorMedio = (limiteA + limiteB)/2; //ponto médio entre A e B

  dist = limiteB - limiteA; //distancia entre A e B                                    
  
  funcEmA = func(limiteA);          //função no ponto A
  funcEmB = func(limiteB);          //função no ponto B
  funcEmMedio = func(valorMedio);   //função no ponto médio
  
  integral = dist * funcEmMedio; //utilizando a regra dos retangulos, calcula-se uma aproximação para a integral                                                            
  
  pthread_mutex_lock(&mutex);

  nbuffer = criaFila();
  enfila(nbuffer, func, limiteA, limiteB, erro, integral, funcEmA, funcEmB, funcEmMedio);
  
  pthread_mutex_unlock(&mutex);
}

//funcao principal
int main(int argc, char *argv[]) {
  
  double inicio, fim, delta;
  pthread_t *tid_sistema;     //vetor de identificadores das threads no sistema
  int *tid;                   //identificadores das threads no programa
  int t, h;                   //variavel contadora
  double limiteA, limiteB, erro;
  double finalResult;

  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&cond, NULL);
  
  if(argc < 5) {

    fprintf(stderr, "Digite: %s <intervalo de integracao> <erro maximo> <numero de threads>.\n", argv[0]);
    exit(EXIT_FAILURE);
  
  }

  limiteA = atof(argv[1]);
  limiteB = atof(argv[2]);
  erro = atof(argv[3]);
  nthreads = atoi(argv[4]);

  iteracoes = malloc(sizeof(long int) * nthreads);
  resultado = malloc(sizeof(double));

  for(h = 0 ; h < 7 ; h++){

    for(t = 0 ; t < nthreads ; t++){
      iteracoes[t] = 0;
    }

    //aloca espaco para o vetor de identificadores das threads no sistema
    tid_sistema = (pthread_t *) malloc (sizeof(pthread_t) * nthreads);
    if(tid_sistema==NULL) { printf("--ERRO: malloc()\n"); exit(-1); }

    if(h == 0) { retangulosInicial(func1, limiteA, limiteB, erro); }
    if(h == 1) { 
      if (limiteA >= -1 && limiteB <= 1){
        retangulosInicial(func2, limiteA, limiteB, erro);
      }
    }
    if(h == 2) { retangulosInicial(func3, limiteA, limiteB, erro); }
    if(h == 3) { retangulosInicial(func4, limiteA, limiteB, erro); }
    if(h == 4) { retangulosInicial(func5, limiteA, limiteB, erro); }
    if(h == 5) { retangulosInicial(func6, limiteA, limiteB, erro); }
    if(h == 6) { retangulosInicial(func7, limiteA, limiteB, erro); }

    for(t = 0 ; t < nthreads ; t++){
      resultado[t] = 0;
    }

    GET_TIME(inicio);

    for(t=0; t<nthreads; t++) {

      tid = malloc (sizeof(int));
      *tid = t;

      if (pthread_create(&tid_sistema[t], NULL, retangulosRecursiva, (void *) tid)) {
        printf("--ERRO: pthread_create()\n"); exit(-1);
      }

    }

    for(t=0; t<nthreads; t++) {
      if (pthread_join(tid_sistema[t], NULL)) {
        printf("--ERRO: pthread_join()\n"); exit(-1);
      }
    }

    for(t = 0 ; t<nthreads ; t++){
      finalResult += resultado[t];
    }

    GET_TIME(fim);

    delta = fim - inicio;

    if(h == 0){ printf("função 1 + x : \n\n"); }
    if(h == 1){ printf("função sqrt(1 - pow(x, 2)) : \n\n"); }
    if(h == 2){ printf("função sqrt(1 + pow(x, 4)) : \n\n"); }
    if(h == 3){ printf("função sin(pow(x,2)) : \n\n"); }
    if(h == 4){ printf("função cos(exp(-x)) : \n\n"); }
    if(h == 5){ printf("função cos(exp(-x)) * x : \n\n"); }
    if(h == 6){ printf("função cos(exp(-x)) * ((0.005 * pow(x, 3)) + 1) : \n\n"); }
    printf("numero de iterações de cada thread no cálculo desta integral : \n");
    for(t = 0 ; t < nthreads ; t++){
      printf("thread %d : %d\n", t, iteracoes[t]);
    }

    if(h == 2 && limiteA <= -1 && limiteB >= 1) {
      printf("Funcao fora do intervalo -1 < x < 1");
    }
    else {
      printf("Valor da integral: %.8lf\n", finalResult);
      printf("Tempo = %lf\n\n", delta);
    }

    free(tid_sistema);
    finalResult = 0;
    threadsProntas = 0;

  }

  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&cond);

  return 0;
}
