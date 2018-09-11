#include "gtest/gtest.h"
#include "../expression_evaluator.h"

int main(int args, char* argv[])
{
	::testing::InitGoogleTest(&args, argv);
	return RUN_ALL_TESTS();
}

using namespace Renaissance;

void DoTest(const std::string& expression,
            const std::unordered_map<std::string, std::string>& variables = {{}},
            const bool expect_evaluation = true, const bool expected_result = true)
{
	ExpressionEvaluator e;
	bool result = false;
	EXPECT_EQ(e.Evaluate(expression, variables, result), expect_evaluation);
	EXPECT_EQ(result, expected_result);
}

TEST(ExpressionCompiler, EmptyExpressionTest1)
{
	DoTest("");
}

TEST(ExpressionCompiler, EmptyExpressionTest2)
{
	DoTest(" ()   \t   ");
}

TEST(ExpressionCompiler, ParsingError)
{
	DoTest("(5 == 6) && (6 == 7", {{}}, false, false);
}

TEST(ExpressionCompiler, SubstringTest)
{
	DoTest("SUBSTR{VAR, 2, 3} == \"cde\"", {{"VAR", "abcdef"}});
}

TEST(ExpressionCompiler, SubstringOutOfBoundTest1)
{
	DoTest("SUBSTR{VAR, 2, 45} == \"cd\"", {{"VAR", "abcd"}});
}

TEST(ExpressionCompiler, SubstringOutOfBoundTest2)
{
	DoTest("SUBSTR{VAR, 2, 3} == \"\"", {{"VAR", ""}});
}

TEST(ExpressionCompiler, ArrayTest1)
{
	DoTest("VAR == [1, 3, 5, 7, 9]", {{"VAR", "3"}});
}

TEST(ExpressionCompiler, ArrayTest2)
{
	DoTest("VAR != [1, 3, 5, 7, 9]", {{"VAR", "4"}});
}

TEST(ExpressionCompiler, ArrayTest3)
{
	DoTest("9 == [1, 3, 5, 7, 9]");
}

TEST(ExpressionCompiler, BigExpressionTest1)
{
	DoTest("(IN.VAR1 == 6011 && IN.VAR2 == [1,2,4,5] && IN.VAR3 != [\"abc\",\"def\",\"ghi\"] && (IN.VAR4 != \"EG\" || IN.VAR5 != \"Y\" && SUBSTR{IN.VAR6,3,9} == \"SOMETHING\"))",
          {{"IN.VAR1", "6011"}, {"IN.VAR2", "4"}, {"IN.VAR3", "ikl"}, {"IN.VAR4", "NOT EG"}, {"IN.VAR5", "N"}, {"IN.VAR6", "___SOMETHING"}});
}

TEST(ExpressionCompiler, BigExpressionTest2)
{
	DoTest("(IN.VAR1 == 6011 && IN.VAR2 == [1,2,4,5] && IN.VAR3 != [\"abc\",\"def\",\"ghi\"] && (IN.VAR4 != \"EG\" || IN.VAR5 != \"Y\" && SUBSTR{IN.VAR6,3,9} == \"SOMETHING\"))",
			 {{"IN.VAR1", "6011"}, {"IN.VAR2", "4"}, {"IN.VAR3", "ikl"}, {"IN.VAR4", "EG"}, {"IN.VAR5", "Y"}, {"IN.VAR6", "___SOMETHING"}},
			 true,
			 false);
}

