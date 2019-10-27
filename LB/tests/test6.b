void main ( ){
  int64[][] m
  m <- new Array(4, 4)
   initMatrix(m, 0)
  return
}

void initMatrix (int64[][] m, int64 initValue){
  int64 l1, l2, index, index2
  l1 <- length m 0
  l2 <- length m 1
  index <- 0

  while (index < l1) :outerBody :leave

  :outerBody
  index2 <- 0
  while (index2 < l2) :innerBody :endOuterBody

  :innerBody
  m[index][index2] <- initValue
  index2 <- index2 + 1
  continue 

  :endOuterBody
  index <- index + 1
  continue 

  :leave
  return
}
