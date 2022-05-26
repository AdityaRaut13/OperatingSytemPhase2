#pragma once
#define TIME_LIMIT_EXCEEDED 2

enum SourceInterrupt
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

