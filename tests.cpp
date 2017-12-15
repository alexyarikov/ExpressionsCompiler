#include <string>
#include <unordered_map>
#include <iostream>
#include "expression_compiler.h"

int main(int argc, char* argv[])
{
	// it's not really a test case but a debugging helper
	ExpressionCompiler ec;
	auto e1 = "(SUBSTR{2, 3, VAR1} == [VAR2, \"123\", \"456\"])";
	std::cout << "expression: " << e1 << std::endl;

	if (!ec.Parse(e1))
	{
		std::cout << "failed to parse the expression" << std::endl;
		return 255;
	}
		
	std::unordered_map<std::string, std::string> values;
	values.insert(std::make_pair("IN.TID", "RBN0111"));
	values.insert(std::make_pair("IN.BIN_ISSUEING_COUNTRY", "617"));
	values.insert(std::make_pair("IN.DCCELIGIBLE", "EG"));
	values.insert(std::make_pair("IN.DCCCALCACCPT", "Y1"));

	bool expression_res = false;
	if (!ec.Evaluate(values, expression_res))
	{
		std::cout << "failed to evaluate the expression" << std::endl;
		return 255;
	}

	std::cout << "expression evaluated successfully, the result is: " << expression_res;

	if (!ec.TransformIntoPrairieRule())
	{
		std::cout << "failed to transform the expression into a prairie rule" << std::endl;
		return 255;
	}

	system("PAUSE");
	return 0;
}

