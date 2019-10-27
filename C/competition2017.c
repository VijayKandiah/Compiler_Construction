#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct {
  int64_t **m;
  int64_t length0;
  int64_t length1;
} C_matrix ;

void computeAndPrint (C_matrix * m1, C_matrix * m2, C_matrix * m3, int64_t v1, int64_t v2);
void initMatrix (C_matrix * m, int64_t initValue);
void matrixMultiplication (C_matrix * m1, C_matrix * m2, C_matrix * m3);
int64_t totalSum (C_matrix * m);

C_matrix * newArray (int64_t dim0, int64_t dim1){
  C_matrix *m = (C_matrix *) malloc(sizeof(C_matrix));

  m->length0 = dim0;
  m->length1 = dim1;

  m->m = (int64_t **) malloc(sizeof(int64_t *) * dim0);
  for (int64_t c=0 ; c < dim0; c++){
    m->m[c] = (int64_t *) malloc(sizeof(int64_t) * dim1);
  }

  return m;
}

void main ( ){

  // Allocate the matrices
  int64_t dim = 300;
  C_matrix * m1 = newArray(dim, dim);
  C_matrix * m2 = newArray(dim, dim);
  C_matrix * m3 = newArray(dim, dim);

  // Compute
  int64_t max = 4;

  for (int64_t index1 = 0; index1 < max; index1 = index1 + 1){
    for (int64_t index2 = 0; index2 < max; index2 = index2 + 1) {
      computeAndPrint(m1, m2, m3, index1, index2);
    }
  }

  return ;
}

void computeAndPrint (C_matrix * m1, C_matrix * m2, C_matrix * m3, int64_t v1, int64_t v2){

  // Initialize the matrices
  initMatrix(m1, v1) ;
  initMatrix(m2, v2) ;

  // Multiple the matrices
  matrixMultiplication(m1, m2, m3) ;

  // Compute the total sum
  int64_t t = totalSum(m1) ;
  printf("%lld\n", t);
  t = totalSum(m2);
  printf("%lld\n", t);
  t = totalSum(m3);
  printf("%lld\n", t);

  return ;
}

void initMatrix (C_matrix * m, int64_t initValue){
  for (int64_t index1 = 0; index1 < m->length0; index1 = index1 + 1) {
    for (int64_t index2 = 0; index2 < m->length1; index2 = index2 + 1) {
      m->m[index1][index2] = initValue + index1 + index2 ;
    }
  }
  return ;
}

void matrixMultiplication (C_matrix * m1, C_matrix * m2, C_matrix * m3){

  // Check the length
  if (m1->length1 != m2->length0) {
    return ;
  }
  if (m3->length0 != m1->length0){
    return ;
  }
  if (m3->length1 != m2->length1) {
    return ;
  }

  // Initialize the result matrix
  for (int64_t i = 0; i < m1->length0; i = i + 1) {
    for (int64_t j = 0; j < m2->length1; j = j + 1) {
      for (int64_t k = 0; k < m1->length1; k = k + 1) {
        m3->m[i][j] = 0 ;
      }
    }
  }

  // Multiply the matrices
  for (int64_t i = 0; i < m1->length0; i = i + 1) {
    for (int64_t j = 0; j < m2->length1; j = j + 1) {
      for (int64_t k = 0; k < m1->length1; k = k + 1) {
        m3->m[i][j] = m3->m[i][j] + (m1->m[i][k] * m2->m[k][j]) ;
      }
    }
  }

  return ;
}

int64_t totalSum (C_matrix * m){
  int64_t sum = 0 ;
  for (int64_t index1 = 0; index1 < m->length0; index1 = index1 + 1) {
    for (int64_t index2 = 0; index2 < m->length1; index2 = index2 + 1) {
      sum = sum + m->m[index1][index2] ;
    }    
  }
  return sum ;
}
