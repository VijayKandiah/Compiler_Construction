void main (){

  int64[][][] A
  int64 test

  // create test values
  int64 test1
  int64 test2
  int64 test3
  int64 test4
  int64 test5
  int64 test6

  test1 <- 1
  test2 <- 2
  test3 <- 3
  test4 <- 4
  test5 <- 5
  test6 <- 6

  // initialize 3D array
  A <- new Array(5, 4, 3) // 5 x 4 x 3
  A[1][0][1] <- test1
  A[3][0][2] <- test2
  A[0][1][0] <- test3
  A[0][2][2] <- test4
  A[1][3][1] <- test5
  A[4][1][0] <- test6

  // test program
  test <-  findArrayValue(A, test1)
   print(test) // print 1
  test <-  findArrayValue(A, test2)
   print(test) // print 1
  test <-  findArrayValue(A, 8)
   print(test) // print 0
  test <-  findArrayValue(A, test3)
   print(test) // print 1
  test <-  findArrayValue(A, test4)
   print(test) // print 1
  test <-  findArrayValue(A, test5)
   print(test) // print 1
  test <-  findArrayValue(A, 9)
   print(test) // print 0
  test <-  findArrayValue(A, 12)
   print(test) // print 0
  test <-  findArrayValue(A, test6)
   print(test) // print 1
  return
}

int64 findArrayValue (int64[][][] A, int64 v){

  // get length of all 3 dimensions of array
  int64 first_dimension_length
  int64 second_dimension_length
  int64 third_dimension_length

  first_dimension_length <- length A 0
  second_dimension_length <- length A 1
  third_dimension_length <- length A 2

  int64 i
  i <- 0

  // iterate through first dimension
  while (i < first_dimension_length) :loop_1_body :leave
  {
    :loop_1_body

    int64 j
    j <- 0

    // iterate through second dimension
    while (j < second_dimension_length) :loop_2_body :continue_loop1
    {
      :loop_2_body

      int64 k
      k <- 0

      // iterate through third dimension
      while (k < third_dimension_length) :loop_3_body :continue_loop2
      {
        :loop_3_body

        // if(A[i][j][k] == v)
        int64 current_value
        current_value <- A[i][j][k]
        if (current_value = v) :found_value :continue_search
        
        :continue_search
         k <- k + 1
         continue
      }
      :continue_loop2
       j <- j + 1
       continue
    }
    :continue_loop1
     i <- i + 1
     continue
  }

  // value found
  :found_value
  return 1

  // value not found
  :leave
  return 0
}
