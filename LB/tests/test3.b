void main ( ){
  int64 v1, v2, v3

  v1 <- 1
  v2 <- 2
  v3 <- v1 + v2

  if (v3 <= 1) :false :true

  :true
   print(v3)

  :false
  return

}
