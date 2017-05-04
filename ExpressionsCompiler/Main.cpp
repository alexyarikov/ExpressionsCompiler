#include "stdafx.h"
#include "ExpressionsCompiler.h"

int main(int argc, char* argv[])
{
	ExpressionsCompiler cc;
	bool bRes = cc.Parse("SUBSTR(0,5,IN.TID) == \"RBN00\" || IN.BIN_ISSUEING_COUNTRY != \"616\" && IN.DCCELIGIBLE ==\"EG\" && IN.DCCCALCACCPT == \"Y\"");
	if (bRes)
	{
		std::unordered_map<std::string, std::string> msg;
		msg.insert(std::make_pair("IN.TID", "RBN0111"));
		msg.insert(std::make_pair("IN.BIN_ISSUEING_COUNTRY", "617"));
		msg.insert(std::make_pair("IN.DCCELIGIBLE", "EG"));
		msg.insert(std::make_pair("IN.DCCCALCACCPT", "Y1"));

		bool bExpressionRes = false;
		bRes = cc.Evaluate(msg, bExpressionRes);
		int n = 1;
	}
	//((IN.BIN_ISSUEING_COUNTRY != "616") && ((IN.DCCELIGIBLE != "EG") || (IN.DCCCALCACCPT != "Y")) && (IN.MT == 1) && (SUBSTR( 3,1,IN.TID ) == ["9", "2", "L", "V", "U"]))

	_getch();
	return 0;
}
