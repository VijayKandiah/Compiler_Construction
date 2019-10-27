void main ( ){
  int64[] v

  v <- new Array(5)
   initVector(v)
   print(v)

  return

}

void initVector (int64[] v) {

  // Fetch the length
  int64 l, index
  l <- length v 0

  // Iterate over the elements
  index <- 0

  :body
   print(index)
  index <- index + 1
  while (index < l) :body :exit

  :exit
  return
}
