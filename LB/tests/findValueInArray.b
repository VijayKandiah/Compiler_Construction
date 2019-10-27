void main ( ){
	int64[][][] a
	int64 v1, v3, v5, v7, v11, v13, v15, v17

	v1 <- 1
	v3 <- 3
	v5 <- 5
	v7 <- 7
	v11 <- 11
	v13 <- 13
	v15 <- 15
	v17 <- 17

	// create and populate the array
	a <- new Array(2,3,4)
	a[0][0][0] <- v1
	a[0][1][0] <- v3
	a[1][2][2] <- v5
	a[1][0][3] <- v7
	a[1][1][0] <- v11
	a[1][1][0] <- v13
	a[0][0][1] <- v15
	a[1][2][3] <- v17

    // Check values that exist in array
	 printExistance(a, v1)
	 printExistance(a, v3)
	 printExistance(a, v5)
	 printExistance(a, v7)
	 printExistance(a, v13)
	 printExistance(a, v15)
	 printExistance(a, v17)

	// Check values do not exist 
	 printExistance(a, v11)
	 printExistance(a, 9)
	 printExistance(a, 19)
	 printExistance(a, 21)

	return
}

void printExistance (int64[][][] A, int64 v){
	int64 found
	found <-  isValueInArray(A, v)
	 print(found)
	return
}

int64 isValueInArray (int64[][][] A, int64 v){
	int64 l, m, n, found

	found <- 0
	l <- length A 0
	m <- length A 1
	n <- length A 2

	// outer loop initialization
	int64 i
	i <- 0

	while (i < l) :middle_loop :done
	{
		:middle_loop
		int64 j
		j <- 0

		while (j < m) :inner_loop :outer_loop_body
		{
			:inner_loop
			int64 k
			k <- 0
			
			while (k < n) :inner_loop_body :middle_loop_body
			{
				:inner_loop_body
				int64 value
				int64 isequal
				value <- A[i][j][k]
				isequal <- value = v

				if (isequal = 1) :found :body
				:body
				k <- k + 1
				continue
			}

			:middle_loop_body
			j <- j + 1
			continue
		}
		
		:outer_loop_body
		i <- i + 1
		continue
	}

	:found
	found <- 1

	:done
	return found		
}
