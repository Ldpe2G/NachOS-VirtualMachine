#include "syscall.h"

/* 定义一个整数最多的位数 */
#define	INT_MAX_DIGIT_NUM	5

/**********************************************************
函数名：
	IntToChar
入参：
	InputInt：要转化成字符串的整数
出参：
	OutputChar：转化后的字符串
功能：
	将整数转化为字符串
**********************************************************/
unsigned int IntToChar(char* OutputChar, unsigned int InputInt)
{
	unsigned int CharNum = 0;
	unsigned int digit, i;
	char* pChar = OutputChar + INT_MAX_DIGIT_NUM;
	/* 将整数转化为字符串*/
	do
	{
		pChar--;
		CharNum++;
		digit = (char)(InputInt % 10) + '0';
		InputInt = InputInt / 10;
		*pChar = digit;
	}
	while (InputInt > 0);

	/* 将字符串移到数组开头 */
	for (i=0; i<CharNum; i++)
	{
		*OutputChar = *pChar;
		OutputChar++;
		pChar++;
	}
	*OutputChar = 0;
	return CharNum;
}

#define SIZE (10)

/*需要排序的数组*/
unsigned int A[SIZE] = {89, 65, 789, 985, 3254, 456, 5241, 235, 4521, 3201};

int main()
{
	int i, j, tmp;
	char IntChar[6];
	unsigned int CharNum;

	Write("排序前数字顺序：\n", 17, ConsoleOutput);
	/*按顺序输出数组中的数字*/
	for (i=0; i<SIZE; i++)
	{
		CharNum = IntToChar(IntChar, A[i]);
		Write(IntChar, CharNum, ConsoleOutput);
		Write(" ", 1, ConsoleOutput);
	}
	Write("\n", 1, ConsoleOutput);
	/*排序*/
	for (i=0; i<SIZE; i++)
	{
		for (j=0; j<(SIZE-1); j++)
		{
			if (A[j] > A[j+1])
			{	/* out of order -> need to swap ! */
				tmp = A[j];
				A[j] = A[j+1];
				A[j+1] = tmp;
			}
		}
	}
	Write("排序后数字顺序：\n", 17, ConsoleOutput);
	/*按顺序输出数组中的数字*/
	for (i=0; i<SIZE; i++)
	{
		CharNum = IntToChar(IntChar, A[i]);
		Write(IntChar, CharNum, ConsoleOutput);
		Write(" ", 1, ConsoleOutput);
	}
	Write("\n\n", 2, ConsoleOutput);
}
