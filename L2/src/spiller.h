#pragma once

#include "L2.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <vector>

namespace L2{
	void spill(Function* f);
	void set_spilltype(Function * f);
	void spill_print(Function * f);
	void replace_inst(Instruction* i, int count);
	void read_inst(Instruction* i, int locals, int count);
	void write_inst(Instruction* i, int locals, int count);
}
