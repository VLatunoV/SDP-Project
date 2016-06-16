#include "unit.h"

char* toStr(int in)
{
	int len = 0;
	int copy = in;
	while(copy)
	{
		++len;
		copy /= 10;
	}
	char* res = new char[len + 1];
	res[len] = 0;
	while(len--)
	{
		res[len] = in % 10 + '0';
		in /= 10;
	}
	return res;
}
std::vector<TestFunctionInfo>& Test::GetFuncInfo()
{
	static std::vector<TestFunctionInfo> tfi;
	return tfi;
}
Test::Test(const TFP& in, const char* name)
{
	GetFuncInfo().push_back(TestFunctionInfo(in, name));
}
void Test::RunTests()
{ 
	char* error_msg = new char[2048]();
	std::vector<TestFunctionInfo> handle = Test::GetFuncInfo();
	for(size_t i = 0; i < handle.size(); ++i)
	{
		std::cout<<"Running "<<handle[i].name<<": ";
		try
		{
			if(handle[i].func(error_msg))
			{
				std::cout<<"PASSED.\n";
			}
			else
			{
				std::cout<<"FAILED.\n";
				std::cout<<"  Why: "<<error_msg<<".\n";
			}
		}
		catch (std::exception& e)
		{
			std::cout<<"RAISED EXCEPTION: "<<e.what()<<".\n";;
		}
		catch(...)
		{
			std::cout<<"RAISED EXCEPTION: Unknown.\n";
		}
	}
	delete[] error_msg;
}
