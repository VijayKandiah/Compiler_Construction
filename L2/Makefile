CPP_FILES := $(wildcard src/*.cpp)
OBJ_FILES := $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))
CC_FLAGS  := --std=c++14 -I./src -I../lib/PEGTL/include -I../lib -g3 -DDEBUG -pedantic -pedantic-errors -Werror=pedantic
LD_FLAGS  := 
CC        := g++
PL_CLASS  := L2
EXT_CLASS := L2
OPT_LEVEL :=
CC_CLASS  := $(PL_CLASS)c

all: dirs $(PL_CLASS)

dirs: obj bin

obj:
	mkdir -p $@

bin:
	mkdir -p $@

$(PL_CLASS): $(OBJ_FILES)
	$(CC) $(LD_FLAGS) -o ./bin/$@ $^

obj/%.o: src/%.cpp
	$(CC) $(CC_FLAGS) -c -o $@ $<

oracle: $(PL_CLASS)
	../scripts/generateOutput.sh $(EXT_CLASS) $(CC_CLASS)

rm_tests_without_oracle:
	../scripts/rm_tests_without_oracle.sh $(EXT_CLASS)

oracle_liveness: L2
	./scripts/generateOutputLiveness.sh

oracle_spill: L2
	./scripts/generateOutputSpill.sh

oracle_interference: L2
	./scripts/generateOutputInterference.sh

test: dirs $(PL_CLASS)
	../scripts/test.sh $(EXT_CLASS) $(CC_CLASS)

test_liveness: L2
	./scripts/testLiveness.sh

test_spill: L2
	./scripts/testSpill.sh

test_interference: L2
	./scripts/testInterference.sh

test_programs: dirs $(PL_CLASS)
	../scripts/test_programs.sh $(EXT_CLASS) $(CC_CLASS)

performance: dirs $(PL_CLASS)
	if ! test -f ./a.out ; then ./$(CC_CLASS) $(OPT_LEVEL) tests/competition2018.$(EXT_CLASS) ; fi ; /usr/bin/time -f'%E' ./a.out

clean:
	rm -fr bin obj *.out *.L1 *.o *.S core.* tests/*/*.tmp tests/*.tmp
