#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "L2.h"

namespace L2{
	//just a find & replace function for a string - used in 'test spill'
	void replace_string(std::string & input, std::string find, std::string replace);		
	/* Register sets*/
	const std::string gp_registers[] = {"r10","r11","r8","r9","rax","rcx","rdi","rdx","rsi","r12","r13","r14","r15","rbp","rbx"};
	const std::string arg_registers[] = {"rdi","rsi","rdx","rcx","r8","r9"};
	const std::string callerSav_registers[] = {"r10","r11","r8","r9","rcx","rdi","rdx","rsi"}; // rax not included
	const std::string calleeSav_registers[] = {"r12","r13","r14","r15","rbp","rbx"};

}
