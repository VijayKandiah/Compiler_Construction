void main ( ){
  int64 a
  a <- 2

  if (a = 2) :TRUE :FALSE
  :TRUE
    print(1)
    br :EXIT

  :FALSE
    print(2)

  :EXIT
  print(3)

  return
}
