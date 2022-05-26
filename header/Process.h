#pragma once

#include <inttypes.h>
class Process {
	public:
		uint8_t id;
		uint8_t timeCounter;
		uint8_t lineCounter;
		uint8_t timeLimit;
		uint8_t lineLimit;
		uint8_t pageTableBlockPointer;
		uint8_t pageTableWordPointer;
		Process();
};


