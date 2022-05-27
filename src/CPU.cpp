#include <cstring>
#include "CPU.h"
CPU::CPU()
{
	instructionCounter = 0;
	toggleRegister = false;
}
std::string Word::toString(void)
{
	std::string result;
	result += word[0];
	result += word[1];
	result += word[2];
	result += word[3];
	return result;
}
bool Word::compare(std::string const &str)
{
	for(int i = 0; i < WORD_SIZE; i++)
		if(word[i] != str.at(i))
			return false;
	return true;
}



void Word::set(char a, char b, char c, char d)
{
	word[0] = a;
	word[1] = b;
	word[2] = c;
	word[3] = d;
}
Word::Word()
{
	memset(word, 0, WORD_SIZE);
}
Word::Word(Word &other)
{
	memcpy(word, other.word, WORD_SIZE);
}
Word::Word(std::string const &other)
{
	for(int i = 0; i < WORD_SIZE; i++)
		word[i] = other.at(i);
}



bool Word::compare(Word other)
{
	for(int i = 0; i < WORD_SIZE; i++)
		if(word[i] != other.word[i])
			return false;
	return true;
}


