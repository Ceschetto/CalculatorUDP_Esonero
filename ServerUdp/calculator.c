#include "calculator.h"


double add(double op[NUM_OPERANDS]);
double mult(double op[NUM_OPERANDS]);
double sub(double op[NUM_OPERANDS]);
double division(double op[NUM_OPERANDS]);



double doOperation(double op[NUM_OPERANDS], char operand)
{
	double result;
	switch(operand)
	{
	case '+':
		result = add(op);
		break;
	case '*':
		result = mult(op);
		break;
	case '-':
		result = sub(op);
		break;
	case '/':
		result = division(op);
		break;
	}
	return result;
}


double add(double op[NUM_OPERANDS])
{
	double result = op[0];

	for(int i = 1; i < NUM_OPERANDS; ++i) result += op[i];

	return result;
}


double mult(double op[NUM_OPERANDS])
{
	double result = op[0];

	for(int i = 1; i < NUM_OPERANDS; ++i) result *= op[i];

	return result;
}


double sub(double op[NUM_OPERANDS])
{
	double result = op[0];

	for(int i = 1; i < NUM_OPERANDS; ++i) result -= op[i];
	return result;
}


double division(double op[NUM_OPERANDS])
{
	double result = op[0];

	for(int i = 1; i < NUM_OPERANDS; ++i) result = (op[i] == 0)? 0 : result / op[i];

	return result;
}



