#include "OperatingSystem.h"
#include "CPU.h"
#include "ErrorMessages.h"
#include "Interrupt.h"
#include <fstream>
#include <ios>
#include <iostream>
#include <locale.h>
#include <memory>
#include <stdexcept>
#include <string.h>
#include <string>
OperatingSystem::OperatingSystem(std::string outputFilePath)
{
	CPU();
	this->mode = SLAVE_MODE;
	this->supervisorInterrupt = 0;
	this->timeInterrupt = 0;
	this->programInterrupt = 0;
	this->outputFilePath = outputFilePath;
	memset(memory, '$', 300 * sizeof(Word));
}


void OperatingSystem::load(std::string inputFilePath)
{
	inputFile.open(inputFilePath);
	// i need to switch based on the SI,TI,PI
	std::string line;
	while(std::getline(inputFile, line))
	{
		if(line.substr(0, 4).compare("$AMJ") == 0)
		{
			std::cout << "inside the if\n";
			currentRunning.id = std::stoi(line.substr(4, 4));
			currentRunning.timeLimit = std::stoi(line.substr(8, 4));
			currentRunning.lineLimit = std::stoi(line.substr(12, 4));
			// This is for the allocating the page table
			currentRunning.pageTableBlockPointer = allocate();
			// saving the allocated program card address into the page table
			std::string buffer = saveToBuffer(true);
			std::cout << buffer << "\n";
			// this is saving the program in buffer
			while(buffer.length() != 0)
			{
				uint8_t programCardAddress = allocate();
				// allocating the page for the program card
				std::string blockPointerString = std::to_string(programCardAddress);
				uint8_t pageTableAddress = currentRunning.pageTableBlockPointer * BLOCK_SIZE +
				                           currentRunning.pageTableWordPointer;
				currentRunning.pageTableWordPointer++;
				memory[pageTableAddress].set('$', '$',
				                             blockPointerString.length() == 1 ? '0' : blockPointerString.at(0),
				                             blockPointerString.length() == 1 ? blockPointerString.at(0) : blockPointerString.at(1));
				int length = copyFromBuffer(buffer, programCardAddress);
				// copying the buffer into the memory
				buffer = buffer.substr(length );
			}
			// start the execution
			std::cout << "before execution\n";
			std::getline(inputFile, line); // for taking the $dta from the file
			this->startExecution();
			// this is for control card
			// and next it will be the program car\n\n\nd
		}
	}
	inputFile.close();
}

uint8_t OperatingSystem::allocate()
{
	int blockNumber = std::rand() % NO_OF_BLOCKS;
	while(memory[blockNumber * BLOCK_SIZE].compare("$$$$") == false)
		blockNumber = std::rand() % NO_OF_BLOCKS;
	return blockNumber;
}

std::string OperatingSystem::saveToBuffer( bool programCard)
{
	std::string buffer;
	char end = programCard ? '$' : '\n';
	while(inputFile.peek() != end)
	{
		if(inputFile.peek() == '\n')
			inputFile.get();
		else
			buffer += inputFile.get();
	}
	if(!programCard)
		buffer += inputFile.get();
	return buffer;
}

uint8_t OperatingSystem::copyFromBuffer(std::string &buffer, uint8_t blockPointer)
{
	/*
	 * Returns :
	 *  The number of bytes copied from the buffer
	 * */
	int index = 0;
	while(index < WORD_SIZE * BLOCK_SIZE and index < buffer.length())
	{
		memory[blockPointer * BLOCK_SIZE + index / WORD_SIZE].word[index % 4] = buffer.at(index);
		index++;
	}
	return index;
}
int OperatingSystem::map(uint8_t virtualAddress)
{
	// virtual address must be having the less than 100
	if(virtualAddress >= 100)
	{
		programInterrupt = ProgramInterrupt::operandError;
		return -1;
	}
	uint16_t pageTablePointer = currentRunning.pageTableBlockPointer * BLOCK_SIZE
	                            + virtualAddress / BLOCK_SIZE;
	// calculating the address of the page table for virtual address block
	std::string strBlockPointer = memory[pageTablePointer].toString();
	if(strBlockPointer.substr(2, 2).compare("$$") == 0)
	{
		// check for the page fault conditions
		programInterrupt = ProgramInterrupt::pageFault;
		return -1;
	}
	int blockAddress = std::stoi(strBlockPointer.substr(2, 2));
	return blockAddress * BLOCK_SIZE + virtualAddress % BLOCK_SIZE;
}
int OperatingSystem::map(std::string virtualAddressStr)
{
	if(!isNumber(virtualAddressStr))
		return -1;
	return map(std::stoi(virtualAddressStr));
}

bool OperatingSystem::isNumber(std::string str)
{
	for(auto i : str)
		if(!std::isdigit(i))
			return false;
	return true;
}



void  OperatingSystem::fatal(std::string const &message)
{
	std::cout << "[ERROR]:" << message << "\n";
	exit(0);
}


void OperatingSystem::startExecution(void)
{
	instructionCounter = 0;
	while(true)
	{
		std::cout << "inside the start execution\n";
		std::cout << instructionCounter << "\n";
		int realAddress = this->map(instructionCounter);
		if(programInterrupt != 0)
		{
			mode = KERNEL_MODE;
			this->masterModeForInterrupt(realAddress, instructionCounter);
			mode = SLAVE_MODE;
			continue;
		}
		instructionRegister = memory[realAddress];
		instructionCounter++;
		std::string instruction = instructionRegister.toString();
		std::cout << instruction << "\n";
		realAddress = map(instruction.substr(2, 2));
		int virtualAddress = isNumber(instruction.substr(2, 2))
		                     ? std::stoi(instruction.substr(2, 2))
		                     : 0 ;
		if(programInterrupt != 0)
		{
			mode = KERNEL_MODE;
			this->masterModeForInterrupt(realAddress, virtualAddress);
			mode = SLAVE_MODE;
		}
		if(instruction.substr(0, 2).compare("LR") == 0)
			generalRegister = memory[realAddress];
		else if (instruction.substr(0, 2).compare("SR") == 0)
		{
			realAddress = map(virtualAddress);
			memory[realAddress] = generalRegister;
		}
		else if (instruction.substr(0, 2).compare("GD") == 0)
		{
			if(virtualAddress % 10 != 0)
				fatal("GD Must have block address");
			supervisorInterrupt = SupervisiorInterrupt::getInput;
			realAddress = map(instruction.substr(2, 2));
		}
		else if (instruction.substr(0, 2).compare("PD") == 0)
			supervisorInterrupt = SupervisiorInterrupt::putData;
		else if (instruction.substr(0, 2).compare("CR") == 0)
			toggleRegister = generalRegister.compare(memory[realAddress]);
		else if (instruction.substr(0, 2).compare("BT") == 0)
			instructionCounter = toggleRegister == true ? std::stoi(instruction.substr(2, 2)) : instructionCounter;
		else if (instruction[0] == 'H')
			supervisorInterrupt = SupervisiorInterrupt::halt;
		else
			programInterrupt = ProgramInterrupt::operationError;
		currentRunning.timeCounter++;
		if(currentRunning.timeLimit == currentRunning.timeCounter)
			timeInterrupt = TIME_LIMIT_EXCEEDED;
		if(programInterrupt != 0 or supervisorInterrupt != 0 or timeInterrupt != 0)
		{
			mode = KERNEL_MODE;
			this->masterModeForInterrupt(realAddress, virtualAddress);
			mode = SLAVE_MODE;
		}
	}
}


void OperatingSystem::masterModeForInterrupt(int realAddress, int virtualAddress)
{
	if(timeInterrupt == 0 and supervisorInterrupt == SupervisiorInterrupt::getInput)
	{
		read(realAddress);
		supervisorInterrupt = 0;
	}
	else if (timeInterrupt == 0 and supervisorInterrupt == SupervisiorInterrupt::putData)
	{
		write(realAddress);
		supervisorInterrupt = 0;
	}
	else if(supervisorInterrupt == SupervisiorInterrupt::halt )
	{
		errorVector.push_back(Error::noError);
		terminate();
	}
	else if (timeInterrupt == TIME_LIMIT_EXCEEDED and supervisorInterrupt == SupervisiorInterrupt::getInput)
	{
		errorVector.push_back(Error::timeLimitExceeded);
		terminate();
	}
	else if (timeInterrupt == TIME_LIMIT_EXCEEDED and supervisorInterrupt == SupervisiorInterrupt::putData)
	{
		write(realAddress);
		supervisorInterrupt = 0;
		errorVector.push_back(Error::timeLimitExceeded);
	}
	supervisorInterrupt = 0;
	// checking the time Interrupt and the program Interrupt
	if(timeInterrupt == 0 and programInterrupt == ProgramInterrupt::operationError)
	{
		errorVector.push_back(Error::operationCodeError);
		terminate();
	}
	else  if (timeInterrupt == 0 and  programInterrupt == ProgramInterrupt::operandError)
	{
		errorVector.push_back(Error::operandError);
		terminate();
	}
	else if (timeInterrupt == 0 and programInterrupt == ProgramInterrupt::pageFault)
	{
		if(instructionRegister.toString().substr(0, 2).compare("GD") == 0
		        or instructionRegister.toString().substr(0, 2).compare("SR") == 0
		  )
		{
			// valid page fault
			int blockPointer = allocate();
			int pageTableEntry = currentRunning.pageTableBlockPointer * BLOCK_SIZE +
			                     virtualAddress / BLOCK_SIZE;
			std::string blockPointerString = std::to_string(blockPointer);
			memory[pageTableEntry].set('$', '$',
			                           blockPointerString.length() == 1 ? '0' : blockPointerString.at(0),
			                           blockPointerString.length() == 1 ? blockPointerString.at(0) : blockPointerString.at(1));
		}
		else
		{
			errorVector.push_back(Error::invalidPageFault);
			terminate();
		}
	}
	else if (timeInterrupt == TIME_LIMIT_EXCEEDED and programInterrupt == ProgramInterrupt::operationError)
	{
		errorVector.push_back(Error::operationCodeError);
		errorVector.push_back(Error::timeLimitExceeded);
		terminate();
	}
	else if (timeInterrupt == TIME_LIMIT_EXCEEDED and programInterrupt == ProgramInterrupt::operandError)
	{
		errorVector.push_back(Error::operandError);
		errorVector.push_back(Error::timeLimitExceeded);
		terminate();
	}
	else if (timeInterrupt == TIME_LIMIT_EXCEEDED and programInterrupt == ProgramInterrupt::pageFault)
	{
		errorVector.push_back(Error::timeLimitExceeded);
		terminate();
	}
	programInterrupt = 0;
	if(timeInterrupt == TIME_LIMIT_EXCEEDED)
	{
		errorVector.push_back(Error::timeLimitExceeded);
		terminate();
	}
	timeInterrupt = 0;
}

void OperatingSystem::write(uint16_t realAddress)
{
	currentRunning.lineCounter++;
	if(currentRunning.lineCounter > currentRunning.lineLimit )
	{
		errorVector.push_back(Error::lineLimitExceeded);
		terminate();
	}
	std::fstream outputFile;
	outputFile.open(outputFilePath, std::ios_base::app);
	if(!outputFile.is_open())
		fatal("Can't Open output file");
	int index = 0;
	while(memory[realAddress + index / WORD_SIZE].word[index % WORD_SIZE] != '\n' and index < BLOCK_SIZE * WORD_SIZE)
	{
		outputFile.put(memory[realAddress + index / WORD_SIZE].word[index % WORD_SIZE]);
		index++;
	}
	outputFile << "\n";
	outputFile.close();
}



void OperatingSystem::read(uint16_t realAddress)
{
	std::string line = saveToBuffer(false);
	if(line.substr(0, 4).compare("$END") == 0 or line.empty())
	{
		errorVector.push_back(Error::outOfData);
		terminate();
	}
	uint16_t index = 0;
	while(index < WORD_SIZE * BLOCK_SIZE and index < line.length())
	{
		memory[realAddress + index / WORD_SIZE].word[index % WORD_SIZE] = line.at(index);
		index++;
	}
}


void OperatingSystem::terminate(void)
{
	std::fstream outputFile(outputFilePath, std::ios_base::app);
	if(!outputFile.is_open())
		fatal("The Output File doesn't exists");
	for(int i : errorVector)
	{
		switch(i)
		{
			case Error::noError:
				{
					outputFile << "\n\n";
					break;
				}
			case Error::outOfData:
				{
					outputFile << "\nOut Of Data\n\n";
					break;
				}
			case Error::lineLimitExceeded:
				{
					outputFile << "\nThe line limit exceeded\n\n";
					break;
				}
			case  Error::invalidPageFault:
				{
					outputFile << "\nInvalid Page Fault\n\n";
					break;
				}
			case Error::operandError:
				{
					outputFile << "\nThe Operand is not correct\n\n";
					break;
				}
			case Error::operationCodeError:
				{
					outputFile << "\nThe Operation is not recognizied\n\n";
					break;
				}
			case Error::timeLimitExceeded:
				{
					outputFile << "\nThe Time is exceeded\n\n";
					break;
				}
		}
	}
	outputFile << "Process ID :" << static_cast<int>(currentRunning.id ) << "\n";
	outputFile.close();
	inputFile.close();
	exit(1);
}


void OperatingSystem::displayMemory(void)
{
	for(int i = 0; i < NO_OF_BLOCKS * BLOCK_SIZE; i++)
		std::cout << i << " " << memory[i].toString() << "\n";
	std::cout << "\n";
}

