void main ( ){

  int64[][][] A
  int64 v
  int64 var

  A <- new Array(10,8,15)

  A[1][4][5] <- 5
  v <- 5
  var <-  find (A, v)
   print(var)

  v <- 2
  var <-  find (A, v)
   print(var)

  A[8][6][14] <- 4
  v <- 4
  var <-  find (A, v)
   print(var)

  return
}

int64 find (int64[][][] A, int64 v) {
  int64 dim0, dim1, dim2
  int64 i, j, k
  int64 val

  dim0 <- length A 0
  dim1 <- length A 1
  dim2 <- length A 2
  i <- 0
  j <- 0
  k <- 0

  while (i < dim0) :loopi :exiti
  {
    :loopi
    j <- 0
    while (j < dim1) :loopj :exitj
    {
      :loopj
      k <- 0
      while (k < dim2) :loopk :exitk
      {
        :loopk
        val <- A[i][j][k]
        // print(val)
        if (val = v) :found :not_found
        :found
        return 1
        :not_found
        k <- k + 1
        continue
        :exitk
      }
      j <- j + 1
      continue
      :exitj
    }
    i <- i + 1
    continue
    :exiti
  }
  return 0
}
