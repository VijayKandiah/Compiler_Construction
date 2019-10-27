void main ( ){
  int64[][] m
  int64 r, v
  v <- 21
  m <- new Array(4, 4)

   initMatrix(m, 0)
  r <-  checkMatrix(m, v)
   print(r)

   initMatrix(m, 0)
  m[0][0] <- v
  r <-  checkMatrix(m, v)
   print(r)

   initMatrix(m, 0)
  m[0][1] <- v
  r <-  checkMatrix(m, v)
   print(r)

   initMatrix(m, 0)
  m[1][0] <- v
  r <-  checkMatrix(m, v)
   print(r)

   initMatrix(m, 0)
  m[3][0] <- v
  r <-  checkMatrix(m, v)
   print(r)

   initMatrix(m, 0)
  m[3][1] <- v
  r <-  checkMatrix(m, v)
   print(r)

   initMatrix(m, 0)
  m[3][3] <- v
  r <-  checkMatrix(m, v)
   print(r)
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

int64 checkMatrix (int64[][] m, int64 v){
  int64 l1, l2, found, index, index2
  l1 <- length m 0
  l2 <- length m 1
  found <- 0
  index <- 0
   print(m)
   print(v)

  while (index < l1) :outerBody :leave

  :outerBody
  index2 <- 0
  while (index2 < l2) :innerBody :endOuterBody

  :innerBody
  int64 e
  e <- m[index][index2]
  if (e = v) :setAndLeave :endInnerBody

  :endInnerBody
  index2 <- index2 + 1
  continue 

  :endOuterBody
  index <- index + 1
  continue 

  :setAndLeave
  found <- 1

  :leave
  return found
}
