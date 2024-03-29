/*
 * Disciplina: Computacao Concorrente
 * Prof.: Silvana Rossetto
 * Trabalho 1
 * Codigo: Problema da quadratura para integracao numerica
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "timer.h"  //para usar funcoes de medida de tempo

//funcoes cujas integrais serao calculadas
double func1(double x){ return 1 + x; }
double func2(double x){ return sqrt(1 - pow(x, 2)); }
double func3(double x){ return sqrt(1 + pow(x, 4)); }
double func4(double x){ return sin(pow(x,2)); }
double func5(double x){ return cos(exp(-x)); }
double func6(double x){ return cos(exp(-x)) * x;}
double func7(double x){ return cos(exp(-x)) * ((0.005 * pow(x, 3)) + 1); }

//funcao recursiva para aproximacao do valor da integral

double retangulosRecursiva(double (*func)( double), double limiteA, double limiteB, double erro, double integral, double funcEmA, double funcEmB, double funcEmMedio) {

  double valorMedio;                                    //variavel para guardar o ponto médio no intervalo [A, B]
  double dist;                                          //variavel para guardar a distancia de A para B
  double valorMedioEsquerda, valorMedioDireita;         //variaveis para guardarem os pontos médios no intervalo [A, C] e [C, B]
  double funcaoEmEsquerda, funcaoEmDireita;             //variáveis para guardarem os valores da função avaliadas no ponto médio entre [A, C] e [C, B]
  double integralEsquerda, integralDireita;             //variáveis para calcular a integral no lado esquerdo e direito do intervalo [A, C] e [C, B] pelo método dos retangulos
  double novaIntegral;                                  //variável que guarda a nova aproximação da integral da parte dividida;

  valorMedio = (limiteA + limiteB) / 2;                 //calcula o ponto médio entre A e B
  dist = limiteB - limiteA;                             //calcula a distancia entre A e B

  valorMedioEsquerda = (limiteA + valorMedio) / 2;      //calcula o ponto médio entre A e C
  valorMedioDireita = (valorMedio + limiteB) / 2;       //calcula o ponto médio entre C e B

  funcaoEmEsquerda = func(valorMedioEsquerda);          //calcula a função avaliada no ponto medio entre A e C
  funcaoEmDireita = func(valorMedioDireita);            //calcula a função avaliada no ponto medio entre C e B

  integralEsquerda = ( dist / 2 ) * funcaoEmEsquerda;   //calcula a integral da parte da esquerda (de A até C)
  integralDireita = ( dist / 2 ) * funcaoEmDireita;     //calcula a integral da parte da direita (de C até B)
  
  novaIntegral = integralEsquerda + integralDireita;                                                                       
  
  if (fabs(novaIntegral - integral) <= 15 * erro) {  //Calculo do método da quadratura adaptiva para saber se o novo valor da integral satisfaz o 
                                              //erro determinado no inicio do problema, caso satisfaça, esse valor é usado para servir como
                                              //aproximação do valor da integral da função, caso contratário, é chamado mais duas instancias
                                              //da função recursiva, onde irão separar o lado esquerdo e o lado direito e fazer o mesmo processo,
                                              //dividindo em esses lados em 2 novos lados até que o erro minimo seja satisfeito, fazendo assim uma
                                              //série de recursões e divindo cada vez mais a integral em pequenos retangulos nas partes mais cŕticias
    return novaIntegral + (novaIntegral - integral) / 15;
  }                                              
  
  return retangulosRecursiva(func, limiteA, valorMedio, erro/2, integralEsquerda,  funcEmA, funcEmMedio, funcaoEmEsquerda) +                    
         retangulosRecursiva(func, valorMedio, limiteB, erro/2, integralDireita, funcEmMedio, funcEmB, funcaoEmDireita);

}

//funcao que inicializa o calculo da integral e chama a funcao recursiva
double retangulosInicial(double (*func) (double), double limiteA, double limiteB, double erro) {  

  double valorMedio;                      //variável que vai armazenar o ponto médio entre os intervalos A e B
  double dist;                            //variável que vai armazenar a distancia entre o intervalo A e B
  double funcEmA, funcEmB, funcEmMedio;   //variáveis que vão armazenas os valores da função calculada nos pontos A, B e ponto Médio
  double integral;                        //variável que vai armazenar o valor da aproximação da integral da função avaliada pelo método dos retangulos

  valorMedio = (limiteA + limiteB)/2;     //ponto médio entre A e B
  dist = limiteB - limiteA;               //distancia entre A e B                                    
  
  funcEmA = func(limiteA);                //função no ponto A
  funcEmB = func(limiteB);                //função no ponto B
  funcEmMedio = func(valorMedio);         //função no ponto médio

  integral = dist * funcEmMedio;          //utilizando a regra dos retangulos, calcula-se uma aproximação para a integral                                                            
  
  return retangulosRecursiva(func, limiteA, limiteB, erro, integral, funcEmA, funcEmB, funcEmMedio); 

}

//funcao principal
int main(int argc, char *argv[]) {
  double delta, inicio, fim;    //variaveis para medir o tempo de execucao
  int limiteA, limiteB;         //variaveis do intervalo de integracao
  double maxError;              //variavel de erro maximo da aproximacao

  //le e valida os parametros de entrada
  if(argc < 4) {
    fprintf(stderr, "Digite: %s <intervalo de integracao> <erro maximo>.\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  limiteA = atof(argv[1]);
  limiteB = atof(argv[2]);
  maxError = atof(argv[3]);

  //executa todas as funcoes calculando o tempo de execucao de cada uma
  //-----------------------------------------------------------------------------------
  GET_TIME(inicio);

  printf("Resultado para a função 1 = %.8f\n", retangulosInicial(func1, limiteA, limiteB, maxError));

  GET_TIME(fim);

  //calcula e exibe o tempo gasto com a multiplicacao
  delta = fim - inicio;
  printf("Tempo de calculo da integral: %.8lf\n\n", delta);
  //-----------------------------------------------------------------------------------
  

  //-----------------------------------------------------------------------------------

  if (limiteA >= -1 && limiteB <= 1){
    GET_TIME(inicio);

    printf("Resultado para a função 2 = %.8f\n", retangulosInicial(func2, limiteA, limiteB, maxError));
    
    GET_TIME(fim);

    //calcula e exibe o tempo gasto com a multiplicacao
    delta = fim - inicio;
    printf("Tempo de calculo da integral: %.8lf\n\n", delta);
  }
  else
  {
    printf("Funcao 2 fora do intervalo!\n\n");
  }
  //-----------------------------------------------------------------------------------


  //-----------------------------------------------------------------------------------
  GET_TIME(inicio);

  printf("Resultado para a função 3 = %.8f\n", retangulosInicial(func3, limiteA, limiteB, maxError));

  GET_TIME(fim);

  //calcula e exibe o tempo gasto com a multiplicacao
  delta = fim - inicio;
  printf("Tempo de calculo da integral: %.8lf\n\n", delta);
  //-----------------------------------------------------------------------------------


  //-----------------------------------------------------------------------------------

  GET_TIME(inicio);

  printf("Resultado para a função 4 = %.8f\n", retangulosInicial(func4, limiteA, limiteB, maxError));

  GET_TIME(fim);

  //calcula e exibe o tempo gasto com a multiplicacao
  delta = fim - inicio;
  printf("Tempo de calculo da integral: %.8lf\n\n", delta);
  //-----------------------------------------------------------------------------------


  //-----------------------------------------------------------------------------------

  GET_TIME(inicio);

  printf("Resultado para a função 5 = %.8f\n", retangulosInicial(func5, limiteA, limiteB, maxError));

  GET_TIME(fim);

  //calcula e exibe o tempo gasto com a multiplicacao
  delta = fim - inicio;
  printf("Tempo de calculo da integral: %.8lf\n\n", delta);
  //-----------------------------------------------------------------------------------


  //-----------------------------------------------------------------------------------

  GET_TIME(inicio);

  printf("Resultado para a função 6 = %.8f\n", retangulosInicial(func6, limiteA, limiteB, maxError));

  GET_TIME(fim);

  //calcula e exibe o tempo gasto com a multiplicacao
  delta = fim - inicio;
  printf("Tempo de calculo da integral: %.8lf\n\n", delta);
  //-----------------------------------------------------------------------------------


  //-----------------------------------------------------------------------------------
  GET_TIME(inicio);

  printf("Resultado para a função 7 = %.8f\n", retangulosInicial(func7, limiteA, limiteB, maxError));

  GET_TIME(fim);

  //calcula e exibe o tempo gasto com a multiplicacao
  delta = fim - inicio;
  printf("Tempo de calculo da integral: %.8lf\n", delta);
  //-----------------------------------------------------------------------------------

  return 0;
}
