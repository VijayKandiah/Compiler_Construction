void main ( ){

  // Allocate the matrices
  int64[][] m1, m2, m3
  int64 dim
  dim <- 300
  m1 <- new Array(dim, dim)
  m2 <- new Array(dim, dim)
  m3 <- new Array(dim, dim)

  // Compute
  {
    int64 index1, index2, max1, max2
    max1 <- 4
    max2 <- 4

    index1 <- 0
    while (index1 < max1) :body1 :leave
    {
      :body1
      index2 <- 0
      while (index2 < max2) :body2 :endBody1
      {
        :body2
         computeAndPrint(m1, m2, m3, index1, index2)//
        index2 <- index2 + 1
        continue
      }

      :endBody1
      index1 <- index1 + 1
      continue
    }
    :leave
  }

  return
}

void computeAndPrint (int64[][] m1, int64[][] m2, int64[][] m3, int64 v1, int64 v2){

  // Initialize the matrices
   initMatrix(m1, v1)
   initMatrix(m2, v2)

  // Multiple the matrices
   matrixMultiplication(m1, m2, m3)

  // Compute the total sum
  int64 t
  t <-  totalSum(m1)
   print(t)
  t <-  totalSum(m2)
   print(t)
  t <-  totalSum(m3)
   print(t)

  return
}

void initMatrix (int64[][] m, int64 initValue){

  // Fetch the lengths
  int64 l1, l2, index1
  l1 <- length m 0
  l2 <- length m 1

  // Initialize variables
  index1 <- 0

  // Iterate over the rows
  while (index1 < l1) :outerBody :leave
  {
    :outerBody
    int64 index2
    index2 <- 0

    // Iterate over the columns
    while (index2 < l2) :innerBody :endOuterBody
    {
      :innerBody
  
      // Compute the value to store
      int64 valueToStore
      valueToStore <- initValue + index1
      valueToStore <- valueToStore + index2
  
      // Store the value to the current matrix element
      m[index1][index2] <- valueToStore

      index2 <- index2 + 1
      continue
    }

    :endOuterBody
    index1 <- index1 + 1
    continue 
  }

  :leave
  return
}

void matrixMultiplication (int64[][] m1, int64[][] m2, int64[][] m3){

  // Fetch the lengths
  int64 m1_l1, m1_l2, m2_l1, m2_l2, m3_l1, m3_l2
  m1_l1 <- length m1 0
  m1_l2 <- length m1 1
  m2_l1 <- length m2 0
  m2_l2 <- length m2 1
  m3_l1 <- length m3 0
  m3_l2 <- length m3 1

  // Check the length
  if (m1_l2 = m2_l1) :go1 :leave
  :go1
  if (m3_l1 = m1_l1) :go2 :leave
  :go2
  if (m3_l2 = m2_l2) :go :leave
  :go

  // Initialize the result matrix
  {
    int64 i, j, k
    i <- 0
    while (i < m1_l1) :body1_init :done_init
    {
      :body1_init
      j <- 0
      while (j < m2_l2) :body2_init :endBody1_init
      {
        :body2_init
        k <- 0
        while (k < m1_l2) :body3_init :endBody2_init
        {
          :body3_init
          m3[i][j] <- 0
          k <- k + 1
          continue
        }
  
        :endBody2_init
        j <- j + 1
        continue
      }
  
      :endBody1_init
      i <- i + 1
      continue
    }
    :done_init
  }

  // Multiply the matrices
  int64 i, j, k
  i <- 0
  while (i < m1_l1) :body1 :leave
  {
    :body1
    j <- 0
    while (j < m2_l2) :body2 :endBody1
    {
      :body2
      k <- 0
      while (k < m1_l2) :body3 :endBody2
      {
        :body3
        int64 A, B, C, D
        A <- m1[i][k]
        B <- m2[k][j]
        C <- A * B
        D <- m3[i][j]
        D <- D + C
        m3[i][j] <- D

        k <- k + 1
        continue
      }

      :endBody2
      j <- j + 1
      continue
    }

    :endBody1
    i <- i + 1
    continue
  }

  :leave
  return
}

int64 totalSum (int64[][] m){
  int64 sum
 
  // Fetch the lengths
  int64 l1, l2, index1
  l1 <- length m 0
  l2 <- length m 1

  // Initialize variables
  index1 <- 0
  sum <- 0

  // Iterate over the rows
  while (index1 < l1) :outerBody :leave
  {
    :outerBody
    int64 index2
    index2 <- 0

    // Iterate over the columns
    while (index2 < l2) :innerBody :endOuterBody
    {
      :innerBody
  
      // Sum the current value
      int64 temp
      temp <- m[index1][index2]
      sum <- sum + temp

      index2 <- index2 + 1
      continue
    }

    :endOuterBody
    index1 <- index1 + 1
    continue 
  }

  :leave
  return sum
}
