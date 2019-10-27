void main () {
	:entry
	int64[][][] m
	int64 r
	int64 v

	v <- 10
	m <- new Array(10, 10, 10)

	r <-  search(m, v)
	 print(r)

	m[0][0][0] <- v

	r <-  search(m, v)
	 print(r)

	return
}

int64 search(int64[][][] m, int64 v) {
	:entry
	int64 length1
	int64 length2
	int64 length3
	int64 encLen

	length1 <- length m 0
	 print(length1)

	length2 <- length m 1
	 print(length2)
	
	length3 <- length m 2
	 print(length3)

	:init
	int64 i
	int64 j
	int64 k
	int64 found
	int64 e

	found <- 0
	i <- 0

	:outerHeader
	if (i < length1) :outerBody :return

	:outerBody
	j <- 0

	:middleHeader
	if (j < length2) :middleBody :outerFooter

	:middleBody
	k <- 0

	:innerHeader
	if (k < length3) :innerBody :middleFooter

	:innerBody
	e <- m[i][j][k]
	if (e = v) :setFound :innerFooter

	:innerFooter
	k <- k + 1
	if (0 = 0) :innerHeader :return

	:middleFooter
	j <- j + 1
	if (0 = 0) :middleHeader :return

	:outerFooter
	i <- i + 1
	if (0 = 0) :outerHeader :return

	:setFound
	found <- 1

	:return
	return found
}
