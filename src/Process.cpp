#include "Process.h"


Process::Process()
{
	this->id = 0;
	this->timeCounter = 0;
	this->lineCounter = 0;
	this->timeLimit = 0;
	this->lineLimit = 0;
	this->pageTableBlockPointer = 0;
	this->pageTableWordPointer = 0;
}
