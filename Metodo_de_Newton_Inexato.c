/*Alunos:
Anderson Aparecido do Carmo Frasão
GRR 20204069

Erick Eckermann Cardoso
GRR20186075
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "Metodo_de_Newton_Inexato.h"
#include "SistLinear.h"

//Recebe os independentes da iteração e retorna os valores convergindo para a raiz
double * calcula_independentes(SistLinear_t *SL, double **m_aux, double *grad, double * res)
{
  double result = 0.0;
  //Vetor que guarda os valores x1 .. xn
  double *indep = (double*) calloc(SL->num_v, sizeof(double));
  if (!(indep)){
    free(SL);
    printf("ERRO");
    return NULL;
  }
  for (int i = 0; i < SL->num_v; i++)
    indep [i] = res[i];

  for(int i = 0; i < SL->num_v; i++){     //Alterna os valores x1 .. xn para cálculo dos seus resultados
    for(int j = 0; j < SL->num_v; j++)
      result += m_aux[i][j] * indep[j];   //Calcula xi, avaliando os outros termos do poinômio de acordo com indep[]
    result -= m_aux[i][i] * indep[i];
    result = grad[i] - result;           
    result = result / m_aux[i][i];
    if (isnan(m_aux[i][i]))               //Confere se não é divisão por zero
      printf("ERRO: %g\n", m_aux[i][i]);  
    indep[i] = result;                    //Guarda o resulado de xi para o calculo de xi+1
    result = 0.0; 
  }
  return indep;
}

// Método Gauss-Seidel para resolução de sistemas lineares
double * gaussSeidel(SistLinear_t *SL, double **m_aux, double *grad)
{
  double *res = (double*) calloc(SL->num_v, sizeof(double));
  if (!(res)){
    free(SL);
    printf("ERRO");
    return NULL;
  }
  double *prox_res;

  int i = 0;
  double num_maior = 0.0;
  double sub = 0.0;

  do{
    //Função que calcula valores de x1 .. xn da próxima iteração
    prox_res = calcula_independentes(SL, m_aux, grad, res);
    for(int j = 0; j < SL->num_v; j++){
      sub = prox_res[j] - res[j];
      if(sub >= num_maior){
        num_maior = sub;
      }
    }
    free(res);
    res = prox_res;  
    
    i++;
  //Critério de parada, vai no máximo até 50 iterações
  }while((i <= 50) && (num_maior > SL->epsilon));

  return res;
}

//função principal para o metodo newton Inexato
double ** Newton_Inexato(SistLinear_t *SL, double *TderivadasGS, double * TlsGS, double ** m_aux)
{
  double ** m_res = (double**) calloc(SL->max_iter+1, sizeof(double*));
  if (!(m_res)){
    free(SL);
    printf("ERRO");
    return NULL;
  }
  for(int i = 0; i < SL->max_iter+1; i++)
  {
    m_res[i] = (double*) calloc(SL->num_v, sizeof(double));
    if (!(m_res[i])){
      free(SL);
      printf("ERRO");
      return NULL;
    }
  }

  //iteração zero
  for(int z = 0; z < SL->num_v; z++)
    m_res[0][z] = SL->Xgs[z];

  //for principal
  for (int i = 0; i < SL->max_iter; i++)
  {
    double aux = 0.0;

    //atualiza o vetor gradiente e a matriz hessiana
    double * grad = calc_grad(SL, SL->Xgs, TderivadasGS);
    calc_hes(SL, SL->Xgs, TderivadasGS, m_aux);
  
    for (int i = 0; i < SL->num_v; i++)
      aux += grad[i]*grad[i];
    aux = sqrt(aux);

    //verifica se o gradiente é menor que epsolon
    if(aux < SL->epsilon)
    {
      free(grad);
      for (int l = i+1; l < SL->max_iter+1; l++)
        for(int z = 0; z < SL->num_v; z++)
          m_res[l][z] = NAN;
      return m_res;
    }
    
    double * delta;
    
    //calcula o vetor delta e o tempo de execução do calculo
    double tTotal = timestamp();
    delta = gaussSeidel(SL, m_aux, grad);
    *TlsGS += timestamp() - tTotal;

    for (int l = 0; l < SL->num_v; l++)
      SL->Xgs[l] += delta[l];

    aux = 0.0;

    for (int i = 0; i < SL->num_v; i++)
      aux += delta[i]*delta[i];
    aux = sqrt(aux);

    //iteração i+1
    for(int z = 0; z < SL->num_v; z++)
      m_res[i+1][z] = SL->Xgs[z];

    free(delta);

    //verifica se delta é menor que epsolon
    if(aux < SL->epsilon)
    {
      free(grad);
      for (int l = i+1; l < SL->max_iter; l++)
        for(int z = 0; z < SL->num_v; z++)
          m_res[l+1][z] = NAN;
      return m_res;
    }
    free(grad);
  }
  return m_res;
}