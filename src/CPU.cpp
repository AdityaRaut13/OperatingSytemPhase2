#include <cstring>
#include "CPU.h"
CPU::CPU()
{
	memset(this->instructionRegister, 0, 4);
	memset(this->generalRegister, 0, 4);
	instructionCounter = 0;
	toggleRegister = false;
}
