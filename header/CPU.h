#pragma once
#include <inttypes.h>
#include <string>
#include <string.h>
#define WORD_SIZE 4
struct Word
{
	char word[4];
	Word();
	Word(Word &other);
	Word(std::string const &other);
	void set(char a, char b, char c, char d);
	std::string toString(void);
	bool compare(Word other);
	bool compare(std::string const &str);
};
class CPU {
	public :
		Word instructionRegister;
		Word generalRegister;
		uint8_t instructionCounter;
		bool toggleRegister;
		CPU();
};

