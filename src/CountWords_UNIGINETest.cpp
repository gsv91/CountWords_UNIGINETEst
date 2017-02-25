//============================================================================
// Name        : CountWords_UNIGINETest.cpp
// Author      : Sergey Garkavenko
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <fstream>
#include <locale>
#include <string>
#include "stdint.h"
#include <algorithm>
#include <vector>

unsigned char log_level = 0;

class WordCount {
public:
	WordCount(char* __word)
	{
		this->word = std::string(__word);
		this->count = 1;
	}

	std::string word;
	unsigned int count;
	bool operator<(const WordCount &rhs) const {
		return rhs.word.compare(word) > 0;
	}
};

void addToWordsCounts(char * __candidateWordChars, std::vector<WordCount>* __wordsCounts)
{
	std::string __candidateWord(__candidateWordChars);
	for (std::vector<WordCount>::iterator wordCount = __wordsCounts->begin(); wordCount != __wordsCounts->end(); ++wordCount)
	{
		if (__candidateWord.compare(wordCount->word) == 0)
		{
			wordCount->count++;
			return;
		}
	}
	WordCount newWordCount(&__candidateWordChars[0]);
	__wordsCounts->push_back(newWordCount);
}

void extractWordCountsFromLine(std::vector<WordCount>* __wordsCounts, const char * lineChars, unsigned int length, char utf8)
{
	bool isWord = false;
	bool fittingSymbol = false;
	int wordSize = 0;
	char word[length + 1];
	for (unsigned int i = 0; i < length; i++)
	{
		if (utf8)
		{
			if ((unsigned char)lineChars[i] < 0x80) { // 1 byte symbol?
				if ((lineChars[i] >= 0x41 && lineChars[i] <= 0x5a) || (lineChars[i] >= 0x61 && lineChars[i] <= 0x7a))
				{
					fittingSymbol = true;
					if (lineChars[i] >= 0x41 && lineChars[i] <= 0x5a)
					{
						word[wordSize] = lineChars[i] + 32;
					} else word[wordSize] = lineChars[i];
					wordSize++;
					isWord = true;
				} else {
					fittingSymbol = false;
				}
			} else
			{
				char tmp[2];
				tmp[0] = lineChars[i+1];
				tmp[1] = lineChars[i];
				uint16_t charInt16 = *(uint16_t *)&tmp[0];
				//          rus А      -         rus я
				if ((charInt16 >= 0xd090 && charInt16 <= 0xd18f))
				{//       rus   A           -       rus  П
					if (charInt16 >= 0xd090 && charInt16 <= 0xd09f)
					{
						charInt16 += 32;
						*(uint16_t*) &tmp[0] = charInt16;
						//    rus Р                -           rus  Я
					} else if (charInt16 >= 0xd0a0 && charInt16 <= 0xd0af) {
						charInt16 += 224;
						*(uint16_t*) &tmp[0] = charInt16;
					}
					fittingSymbol = true;
					word[wordSize] = tmp[1];
					word[wordSize+1] = tmp[0];
					wordSize += 2;
					isWord = true;
				} else
				{
					fittingSymbol = false;
				}
				i++;
			}
		} else
		{ //                 eng A   -    eng Z                            end a     -     eng z                               ru А  -   ru я
			if (((unsigned char)lineChars[i] >= 0x41 && (unsigned char)lineChars[i] <= 0x5a) || ((unsigned char)lineChars[i] >= 0x61 && (unsigned char)lineChars[i] <= 0x7a) || ((unsigned char)lineChars[i] >= 0xc0 && (unsigned char)lineChars[i] <= 0xff))
			{
				if ((unsigned char)lineChars[i] >= 0x41 && (unsigned char)lineChars[i] <= 0x5a)
				{
					word[wordSize] = lineChars[i] + 32;
				} else if ((unsigned char)lineChars[i] >= 0xc0 && (unsigned char)lineChars[i] <= 0xdf)
				{
					word[wordSize] = lineChars[i] + 32;
				} else {
					word[wordSize] = lineChars[i];
				}
				fittingSymbol = true;
				wordSize++;
				isWord = true;
			} else
			{
				fittingSymbol = false;
			}
		}
		if (isWord && !fittingSymbol)
		{
			isWord = false;
			word[wordSize] = '\0';
			if (log_level) printf("add word %s \n", word);
			addToWordsCounts(&word[0], __wordsCounts);
			wordSize = 0;
		}
	}
	if (isWord)
	{
		isWord = 0;
		word[wordSize] = '\0';
		if (log_level) printf("add word %s \n", word);
		addToWordsCounts(&word[0], __wordsCounts);
		wordSize = 0;
	}
}

void readFileAndFillInWordsCounts(char *__inFilePath, std::vector<WordCount>* __wordsCounts) {
	std::ifstream ifs(__inFilePath);
	std::string line;
	char isUtf = 0;
	if (ifs.is_open()) {
		if (getline(ifs, line))
		{
			if (line.length() >= 3)
			{ // check whether it is utf-8 file or default ansi
				if (line.c_str()[0] == '\xEF' && line.c_str()[1] == '\xBB' && line.c_str()[2] == '\xBF')
				{
					isUtf = 1;
					line = line.substr(3,line.length() - 3);
				}
				extractWordCountsFromLine(__wordsCounts, line.c_str(), line.length(), isUtf);
			}
			while (getline(ifs, line))
			{
				extractWordCountsFromLine(__wordsCounts, line.c_str(), line.length(), isUtf);
			}
		}
	}
	ifs.close();
}

std::vector<WordCount> *orderWordsCounts(std::vector<WordCount>* __wordsCounts)
{
	std::sort(__wordsCounts->begin(), __wordsCounts->end());
	std::vector<WordCount>* orderedWordsCounts = new std::vector<WordCount>();

	for (std::vector<WordCount>::iterator wordCount = __wordsCounts->begin(); wordCount != __wordsCounts->end(); ++wordCount)
	{
		char inserted = 0;
		for (std::vector<WordCount>::iterator orderedWordsCountsI = orderedWordsCounts->begin(); orderedWordsCountsI != orderedWordsCounts->end(); ++orderedWordsCountsI)
		{
			if (orderedWordsCountsI->count < wordCount->count)
			{
				inserted = 1;
				orderedWordsCounts->insert(orderedWordsCountsI, (*wordCount));
				break;
			} else if (orderedWordsCountsI->count == wordCount->count)
			{
				if (orderedWordsCountsI->word.compare(wordCount->word) > 0)
				{
					inserted = 1;
					orderedWordsCounts->insert(orderedWordsCountsI, (*wordCount));
					break;
				}
			}
		}
		if (!inserted)
		{
			orderedWordsCounts->push_back((*wordCount));
		}
	}

	return orderedWordsCounts;
}

void writeWordsCounts(std::vector<WordCount>* __wordsCounts, char *__outFilePath)
{
	std::ofstream ofs(__outFilePath);
	for (std::vector<WordCount>::iterator wordCount = __wordsCounts->begin(); wordCount != __wordsCounts->end(); ++wordCount)
	{
		ofs << wordCount->word << " " << wordCount->count << std::endl;
	}
	ofs.close();
}

int main(int argc, char* argv[]) {
	//cout << "args = " << argc << " " << argv[0] << " " << argv[1] << '\n';
	if (argc < 3) std::cout << "Need two arguments. 1. Input file path. 2. Output file path.";
	std::vector<WordCount> wordsCounts;
	readFileAndFillInWordsCounts(argv[1], &wordsCounts);
	writeWordsCounts(orderWordsCounts(&wordsCounts), argv[2]);
	return 0;
}
