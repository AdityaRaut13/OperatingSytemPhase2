#pragma once
#include <inttypes.h>
class CPU {
	public :
		char instructionRegister[4];
		char generalRegister[4];
		uint8_t instructionCounter;
		bool toggleRegister;
		CPU();
};

