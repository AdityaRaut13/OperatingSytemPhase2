#pragma once
#include <inttypes.h>
#include <string>
#include <vector>
#include <fstream>
#include "CPU.h"
#include "Interrupt.h"
#include "Process.h"
#include "ErrorMessages.h"
#define KERNEL_MODE 1
#define SLAVE_MODE 0
#define NO_OF_BLOCKS 30
#define BLOCK_SIZE 10


class OperatingSystem : private CPU {
	private:
		bool mode;
		Process currentRunning;
		Word memory[300];
		uint8_t supervisorInterrupt;
		uint8_t programInterrupt;
		uint8_t timeInterrupt;
		std::string outputFilePath;
		std::vector<uint8_t> errorVector;
		std::fstream inputFile;
		void read(uint16_t realAddress);
		void write(uint16_t realAddress);
		void terminate(void);
		uint8_t allocate();
		void startExecution(void);
		int map(uint8_t virtualAddress);
		int map(std::string virtualAddressStr);
		void masterModeForInterrupt(int realAddress, int virtualAddress);
		std::string saveToBuffer(bool programCard);
		uint8_t copyFromBuffer(std::string &buffer, uint8_t blockPointer);
		void fatal(std::string const &message );
		void displayMemory(void);
	public:
		OperatingSystem(std::string outputFilePath);
		void load(std::string inputFilePath);
		static bool isNumber(std::string str);
};

