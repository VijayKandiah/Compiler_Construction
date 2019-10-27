void main() {
	:start
	int64[][][] matrix
	int64 result, test_value

	test_value <- 1234
	matrix <- new Array(5, 5, 5)

	result <-  find(matrix, test_value)
	 print(result)

	matrix[2][1][0] <- test_value

	result <-  find(matrix, test_value)
	 print(result)

	return
}

int64 find(int64[][][] A, int64 v) {
	int64 length1, length2, length3, i, j, k, found, check_value

	length1 <- length A 0
	length2 <- length A 1
	length3 <- length A 2
	
	i <- 0
	found <- 0
	
	while(i < length1) :first_body :end
	:first_body
	j <- 0
	
	while(j < length2) :second_body :first_reset
	:second_body
	k <- 0

	while(k < length3) :third_body :second_reset
	:third_body
	
	check_value <- A[i][j][k]
	if (check_value = v) :did_find :not_found

	:not_found
	k <- k + 1
	continue
	
	:second_reset
	j <- j + 1
	continue

	:first_reset
	i <- i + 1
	continue

	:did_find
	found <- 1

	:end
	return found
}
