#pragma once

#include <vector>
#include <exception>
#include <iostream>
#include <string.h>

char* toStr(int);
typedef bool (*TFP)(char*);
class TestFunctionInfo
{
	TestFunctionInfo();
public:
	TestFunctionInfo(const TFP& in, const char* n): func(0), name(0)
	{
		func = in;
		name = new char[strlen(n) + 1];
		strcpy(name, n);
	}
	TestFunctionInfo(const TestFunctionInfo& other): func(0), name(0)
	{
		*this = other;
	}
	~TestFunctionInfo()
	{
		delete[] name;
	}
	TestFunctionInfo& operator= (const TestFunctionInfo& other)
	{
		func = other.func;
		delete[] name;
		name = 0;
		if(other.name)
		{
			name = new char [strlen(other.name) + 1];
			strcpy(name, other.name);
		}
		return *this;
	}
	TFP func;
	char* name;
};
class Test
{
	Test();
	Test(const Test&);
	Test& operator=(const Test&);
public:
	Test(const TFP&, const char*);
	~Test(){}
	static std::vector<TestFunctionInfo>& GetFuncInfo();
	static void RunTests();
};

#define UTest(func) \
        bool func(char*); \
        Test test_obj_##func(&func, #func); \
        bool func(char* __msg)

// In <filename>, line <line>: <expression> failled assertion
#define add(str) strcat(__msg, str);
#define addexpr(str) add("("); add(str); add(")");
#define DebugMessage \
			strcpy(__msg, "In "); \
			add(__FILE__); \
			add(", line "); \
			char* l = toStr(__LINE__); \
			add(l); \
			delete[] l; \
			add(": ");
#define AssertFail() {DebugMessage; add("Assert fail"); return false;}
#define AssertTrue(expr) if(!(expr)) {DebugMessage; add(#expr); add(" not true"); return false;}
#define AssertFalse(expr) if((expr)) {DebugMessage; add(#expr); add(" not false"); return false;}
#define AssertEqual(expr1, expr2) if((expr1) != (expr2)) {DebugMessage; addexpr(#expr1) ;add(" == "); addexpr(#expr2); add(" not true"); return false;}
#define AssertNotEq(expr1, expr2) if((expr1) == (expr2)) {DebugMessage; addexpr(#expr1) ;add(" != "); addexpr(#expr2); add(" not true"); return false;}
#define AssertClose(expr1, expr2, err) if((expr1) - (expr2) > (err) || (expr1) - (expr2) < -(err)) {DebugMessage; addexpr(#expr1);add(" not close to ");addexpr(#expr2);return false;}
#define AssertGreater(expr1, expr2) if((expr1) <= (expr2)) {DebugMessage; addexpr(#expr1); add(" > "); addexpr(#expr2);add(" not true"); return false;}
#define AssertGreaterEq(expr1, expr2) if((expr1) < (expr2)) {DebugMessage; addexpr(#expr1); add(" >= "); addexpr(#expr2);add(" not true"); return false;}
#define AssertLess(expr1, expr2) if((expr1) >= (expr2)) {DebugMessage; addexpr(#expr1); add(" < "); addexpr(#expr2);add(" not true"); return false;}
#define AssertLessEq(expr1, expr2) if((expr1) > (expr2)) {DebugMessage; addexpr(#expr1); add(" <= "); addexpr(#expr2);add(" not true"); return false;}
