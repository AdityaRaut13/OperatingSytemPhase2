#pragma once
#define TIME_LIMIT_EXCEEDED 2

enum SupervisiorInterrupt
{
	getInput = 1,
	putData,
	halt
};
enum ProgramInterrupt
{
	operationError = 1,
	operandError,
	pageFault
};

