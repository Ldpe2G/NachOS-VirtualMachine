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
unsigned int IntToChar(char* OutputChar, unsigned int InputInt){
	unsigned int CharNum = 0;
	unsigned int digit, i;
	char* pChar = OutputChar + INT_MAX_DIGIT_NUM;
	/* 将整数转化为字符串*/
	do{
		pChar--;
		CharNum++;
		digit = (char)(InputInt % 10) + '0';
		InputInt = InputInt / 10;
		*pChar = digit;
	}while (InputInt > 0);

	/* 将字符串移到数组开头 */
	for (i=0; i<CharNum; i++){
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

//把两个有序的数组组合成一个有序数组
unsigned int* merge(unsigned int* array1, int length1, unsigned int* array2, int length2){
  unsigned int result[length1 + length2];
  int i = 0, j = 0;
  int index = 0;
  while(i < length1 && j < length2){
    if(array1[i] < array2[j])
      result[index ++] = array1[i ++];
    else
      result[index ++] = array2[j ++];
  }
  for(; i < length1; i++)
    result[index ++] = array1[i];
  for(; j < length2; j++)
    result[index ++] = array2[j];
  return result;
}

//根据start和end确定的范围截取array数组
unsigned int* takeN(unsigned int* array, int start, int end){
  unsigned int temp[end - start + 1];
  int i = start;
  for(; i <= end; i++)
    temp[i - start] = array[i];
  return temp;
}

unsigned int* mergeSort(unsigned int* array, int length){
  int n = length / 2;
  if(n == 0)
    return array;
  else{
    //以下两步相当于把array数组分成两部分
    unsigned int* temp1 = takeN(array, 0, n - 1);
    unsigned int* temp2 = takeN(array, n, length - 1);
    return merge(mergeSort(temp1, n), n, mergeSort(temp2, length - n), length - n);
  }
}


int main(){
	int i, j, tmp;
	char IntChar[6];
	unsigned int CharNum;

	Write("归并排序前数字顺序：\n", 17, ConsoleOutput);
	/*按顺序输出数组中的数字*/
	for (i=0; i<SIZE; i++){
		CharNum = IntToChar(IntChar, A[i]);
		Write(IntChar, CharNum, ConsoleOutput);
		Write(" ", 1, ConsoleOutput);
	}
	Write("\n", 1, ConsoleOutput);
	
	/*归并排序*/
	merge_sort();

	Write("归并排序后数字顺序：\n", 17, ConsoleOutput);
	/*按顺序输出数组中的数字*/
	for (i=0; i<SIZE; i++){
		CharNum = IntToChar(IntChar, A[i]);
		Write(IntChar, CharNum, ConsoleOutput);
		Write(" ", 1, ConsoleOutput);
	}
	Write("\n\n", 2, ConsoleOutput);
}
