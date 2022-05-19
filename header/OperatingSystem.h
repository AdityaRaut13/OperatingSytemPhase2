#pragma once
#include <inttypes.h>
#include "CPU.h"
#include "Interrupt.h"
#include "Process.h"
#define KERNEL_MODE 1
#define SLAVE_MODE 0


class OperatingSystem : private CPU {
	private:
		bool mode;
	public:
		uint8_t sourceInterrupt;
		uint8_t programInterrupt;
		uint8_t timeInterrupt;
		OperatingSystem()
		{
			CPU();
			this->mode = SLAVE_MODE;
			this->sourceInterrupt = 0;
			this->timeInterrupt = 0;
			this->programInterrupt = 0;
		}
};

