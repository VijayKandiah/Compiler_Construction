all: langs
	
langs: L1_lang L2_lang L3_lang IR_lang LA_lang LB_lang

L1_lang:
	cd L1 ; make 

L2_lang:
	cd L2 ; make

L3_lang:
	cd L3 ; make

IR_lang:
	cd IR ; make

LA_lang:
	cd LA ; make

LB_lang:
	cd LB ; make


framework:
	./scripts/framework.sh

homework:
	./scripts/homework.sh

rm_programs: langs
	cd L1 ; make test_programs ; make rm_tests_without_oracle
	cd L2 ; make test_programs ; make rm_tests_without_oracle
	cd L3 ; make test_programs ; make rm_tests_without_oracle
	cd IR ; make test_programs ; make rm_tests_without_oracle
	cd LA ; make test_programs ; make rm_tests_without_oracle
	cd LB ; make test_programs ; make rm_tests_without_oracle

generate_tests: langs
	./scripts/generateTests.sh

clean:
	rm -f *.bz2 ; cd L1 ; make clean ; cd ../L2 ; make clean ; cd ../L3 ; make clean ; cd ../IR ; make clean ; cd ../LA ; make clean ; cd ../LB ; make clean ; cd ../C ; make clean ; 
