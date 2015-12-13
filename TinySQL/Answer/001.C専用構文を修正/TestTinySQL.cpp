#include "stdafx.h"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <iterator>

#include "CppUnitTest.h"

int ExecuteSQL(const char*, const char*);


//! ExecuteSQL�̖߂�l�̎�ނ�\���܂��B
enum REAULT_VALUE
{
	OK = 0,                     //!< ���Ȃ��I�����܂����B
	ERR_FILE_OPEN = 1,          //!< �t�@�C�����J�����ƂɎ��s���܂����B
	ERR_FILE_WRITE = 2,         //!< �t�@�C���ɏ������݂��s�����ƂɎ��s���܂����B
	ERR_FILE_CLOSE = 3,         //!< �t�@�C������邱�ƂɎ��s���܂����B
	ERR_TOKEN_CANT_READ = 4,    //!< �g�[�N����͂Ɏ��s���܂����B
	ERR_SQL_SYNTAX = 5,         //!< SQL�̍\����͂����s���܂����B
	ERR_BAD_COLUMN_NAME = 6,    //!< �e�[�u���w����܂ޗ񖼂��K�؂ł͂���܂���B
	ERR_WHERE_OPERAND_TYPE = 7, //!< ���Z�̍��E�̌^���K�؂ł͂���܂���B
	ERR_CSV_SYNTAX = 8,         //!< CSV�̍\����͂����s���܂����B
	ERR_MEMORY_ALLOCATE = 9,    //!< �������̎擾�Ɏ��s���܂����B
	ERR_MEMORY_OVER = 10        //!< �p�ӂ����������̈�̏���𒴂��܂����B
};

using namespace std;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace Test
{
	TEST_CLASS(ExecuteSQLTest)
	{
		char* testOutputPath = "output.csv";

		string ReadOutput()
		{
			return	string(istreambuf_iterator<char>(ifstream(testOutputPath, ios::in | ios::binary)), istreambuf_iterator<char>());
		}
	public:

		TEST_METHOD_INITIALIZE(������)
		{
			ofstream o;
			remove(testOutputPath);
			o = ofstream("TABLE1.csv");
			o
				<< "Integer,String" << endl
				<< "1,A" << endl
				<< "2,B" << endl
				<< "3,C" << endl;
			o = ofstream("TABLE2.csv");
			o
				<< "Integer,String" << endl
				<< "4,D" << endl
				<< "5,E" << endl
				<< "6,F" << endl;
			o = ofstream("TABLE3.csv");
			o
				<< "Integer,String" << endl
				<< "7,G" << endl
				<< "8,H" << endl;
			o = ofstream("UNORDERED.csv");
			o
				<< "Integer,String" << endl
				<< "21,BA" << endl
				<< "2,B" << endl
				<< "12,AB" << endl
				<< "11,AA" << endl
				<< "22,BB" << endl
				<< "1,A" << endl;
			o = ofstream("UNORDERED2.csv");
			o
				<< "Integer1,Integer2,String1,String2" << endl
				<< "1,2,A,B" << endl
				<< "2,2,B,B" << endl
				<< "2,1,B,A" << endl
				<< "1,1,A,A" << endl;
			o = ofstream("PARENTS.csv");
			o
				<< "Id,Name" << endl
				<< "1,Parent1" << endl
				<< "2,Parent2" << endl
				<< "3,Parent3" << endl;
			o = ofstream("CHILDREN.csv");
			o
				<< "Id,Name,ParentId" << endl
				<< "1,Child1,1" << endl
				<< "2,Child2,1" << endl
				<< "3,Child3,2" << endl
				<< "4,Child4,2" << endl
				<< "5,Child5,3" << endl
				<< "6,Child6,3" << endl
				<< "7,Child7,3" << endl;
			o = ofstream("MINUS.csv");
			o
				<< "Integer" << endl
				<< "-1" << endl
				<< "-2" << endl
				<< "-3" << endl
				<< "-4" << endl
				<< "-5" << endl
				<< "-6" << endl;
		}

		TEST_METHOD(ExecuteSQL�͒P����SQL�����s�ł��܂��B)
		{
			char* sql =
				"SELECT * "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"1,A"				"\r\n"
				"2,B"				"\r\n"
				"3,C"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL�͍Ō�ɋ󔒂������Ă����������삵�܂��B)
		{
			char* sql =
				"SELECT * "
				"FROM TABLE1 ";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"1,A"				"\r\n"
				"2,B"				"\r\n"
				"3,C"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL�͎��ʎq���ɐ����𗘗p�ł��܂��B)
		{
			char* sql =
				"SELECT * "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
		}

		TEST_METHOD(ExecuteSQL�͎��ʎq���ɐ����Ŏn�܂�P��͗��p�ł��܂���B)
		{
			char* sql =
				"SELECT * "
				"FROM 1TABLE";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_TOKEN_CANT_READ, result);
		}

		TEST_METHOD(ExecuteSQL�͎��ʎq���̂Q�����ڂɐ����𗘗p�ł��܂��B)
		{
			char* sql =
				"SELECT * "
				"FROM T1ABLE";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_FILE_OPEN, result);
		}

		TEST_METHOD(ExecuteSQL�͎��ʎq���̐擪�ɃA���_�[�o�[�𗘗p�ł��܂��B)
		{
			char* sql =
				"SELECT * "
				"FROM _TABLE";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_FILE_OPEN, result);
		}

		TEST_METHOD(ExecuteSQL�͎��ʎq���̓񕶎��ڈȍ~�ɃA���_�[�o�[�𗘗p�ł��܂��B)
		{
			char* sql =
				"SELECT * "
				"FROM T_ABLE";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_FILE_OPEN, result);
		}

		TEST_METHOD(ExecuteSQL�͕���������؂蕶���𗘗p�ł��܂��B)
		{
			char* sql =
				"SELECT  *  "
				"FROM  TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"1,A"				"\r\n"
				"2,B"				"\r\n"
				"3,C"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL�͋�؂蕶���Ƃ��ăX�y�[�X��F�����܂��B)
		{
			char* sql =
				"SELECT * "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"1,A"				"\r\n"
				"2,B"				"\r\n"
				"3,C"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL�͋�؂蕶���Ƃ��ă^�u��F�����܂��B)
		{
			char* sql =
				"SELECT\t*\t"
				"FROM\tTABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"1,A"				"\r\n"
				"2,B"				"\r\n"
				"3,C"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL�͋�؂蕶���Ƃ��ĉ��s��F�����܂��B)
		{
			char* sql =
				"SELECT\n*\r\n"
				"FROM\rTABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"1,A"				"\r\n"
				"2,B"				"\r\n"
				"3,C"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL�͔F���ł��Ȃ��g�[�N�����܂ތ���w�肵���Ƃ�ERR_TOKEN_CANT_READ�G���[�ƂȂ�܂��B)
		{
			char* sql =
				"?";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_TOKEN_CANT_READ, result);
		}


		TEST_METHOD(ExecuteSQL�͎w�肵���e�[�u�������擾���A�Ή�����t�@�C�����Q�Ƃł��܂��B)
		{
			char* sql =
				"SELECT * "
				"FROM TABLE2";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"4,D"				"\r\n"
				"5,E"				"\r\n"
				"6,F"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL�͓�̂̃e�[�u����ǂݍ��݁A�S�Ă̑g�ݍ��킹���o�͂��܂��B)
		{
			char* sql =
				"SELECT * "
				"FROM TABLE1, TABLE2";

			string expectedCsv =
				"Integer,String,Integer,String"	"\r\n"
				"1,A,4,D"						"\r\n"
				"1,A,5,E"						"\r\n"
				"1,A,6,F"						"\r\n"
				"2,B,4,D"						"\r\n"
				"2,B,5,E"						"\r\n"
				"2,B,6,F"						"\r\n"
				"3,C,4,D"						"\r\n"
				"3,C,5,E"						"\r\n"
				"3,C,6,F"						"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL�͎O�ȏ�̃e�[�u����ǂݍ��݁A�S�Ă̑g�ݍ��킹���o�͂��܂��B)
		{
			char* sql =
				"SELECT * "
				"FROM TABLE1, TABLE2, TABLE3";

			string expectedCsv =
				"Integer,String,Integer,String,Integer,String"	"\r\n"
				"1,A,4,D,7,G"									"\r\n"
				"1,A,4,D,8,H"									"\r\n"
				"1,A,5,E,7,G"									"\r\n"
				"1,A,5,E,8,H"									"\r\n"
				"1,A,6,F,7,G"									"\r\n"
				"1,A,6,F,8,H"									"\r\n"
				"2,B,4,D,7,G"									"\r\n"
				"2,B,4,D,8,H"									"\r\n"
				"2,B,5,E,7,G"									"\r\n"
				"2,B,5,E,8,H"									"\r\n"
				"2,B,6,F,7,G"									"\r\n"
				"2,B,6,F,8,H"									"\r\n"
				"3,C,4,D,7,G"									"\r\n"
				"3,C,4,D,8,H"									"\r\n"
				"3,C,5,E,7,G"									"\r\n"
				"3,C,5,E,8,H"									"\r\n"
				"3,C,6,F,7,G"									"\r\n"
				"3,C,6,F,8,H"									"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��SQLECT��Ƀe�[�u���ƈꏏ�Ɏw�肵���񖼂��w�肵�ASQL�����s�ł��܂��B)
		{
			char* sql =
				"SELECT String "
				"FROM TABLE1";

			string expectedCsv =
				"String"	"\r\n"
				"A"			"\r\n"
				"B"			"\r\n"
				"C"			"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��SQLECT��ɕ����̃e�[�u���ƈꏏ�Ɏw�肵���񖼂��w�肵�ASQL�����s�ł��܂��B)
		{
			char* sql =
				"SELECT String,Integer "
				"FROM TABLE1";

			string expectedCsv =
				"String,Integer"	"\r\n"
				"A,1"				"\r\n"
				"B,2"				"\r\n"
				"C,3"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��SQLECT��ɎO�ȏ�̃e�[�u���ƈꏏ�Ɏw�肵���񖼂��w�肵�ASQL�����s�ł��܂��B)
		{
			char* sql =
				"SELECT String,Integer,String,Integer "
				"FROM TABLE1";

			string expectedCsv =
				"String,Integer,String,Integer"	"\r\n"
				"A,1,A,1"						"\r\n"
				"B,2,B,2"						"\r\n"
				"C,3,C,3"						"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��SELECT�̎w��Ƀe�[�u�������w��ł��܂��B)
		{
			char* sql =
				"SELECT TABLE1.Integer "
				"FROM TABLE1";

			string expectedCsv =
				"Integer"	"\r\n"
				"1"			"\r\n"
				"2"			"\r\n"
				"3"			"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL�͕����̂̃e�[�u����ǂݍ��݁A�e�[�u�����ŋ�ʂ��ăe�[�u���ƈꏏ�Ɏw�肵���񖼂��w�肷�邱�Ƃ��ł��܂��B)
		{
			char* sql =
				"SELECT Table1.Integer "
				"FROM TABLE1, TABLE2";

			string expectedCsv =
				"Integer"	"\r\n"
				"1"			"\r\n"
				"1"			"\r\n"
				"1"			"\r\n"
				"2"			"\r\n"
				"2"			"\r\n"
				"2"			"\r\n"
				"3"			"\r\n"
				"3"			"\r\n"
				"3"			"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��SELECT��Ńe�[�u�������ڈȍ~�̃e�[�u���ƈꏏ�Ɏw�肵���񖼂Ɏw�肷�邱�Ƃ��ł��܂��B)
		{
			char* sql =
				"SELECT Table1.Integer, Table2.String "
				"FROM TABLE1, TABLE2";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"1,D"				"\r\n"
				"1,E"				"\r\n"
				"1,F"				"\r\n"
				"2,D"				"\r\n"
				"2,E"				"\r\n"
				"2,F"				"\r\n"
				"3,D"				"\r\n"
				"3,E"				"\r\n"
				"3,F"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��SELECT�̃e�[�u���ƈꏏ�Ɏw�肵���񖼂̎w�肪�����܂��ȏꍇ��ERR_BAD_COLUMN_NAME�G���[�ƂȂ�܂��B)
		{
			char* sql =
				"SELECT Integer "
				"FROM TABLE1, TABLE2";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQL��SELECT�Ŏw�肵���e�[�u���ƈꏏ�Ɏw�肵���񖼂̎w��̈ꕶ���ڂ̈Ⴂ���������܂��B)
		{
			char* sql =
				"SELECT Ttring "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQL��SELECT�Ŏw�肵���e�[�u���ƈꏏ�Ɏw�肵���񖼂̎w��̓񕶎��ڂ̈Ⴂ���������܂��B)
		{
			char* sql =
				"SELECT Suring "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQL��SELECT�Ŏw�肵���e�[�u���ƈꏏ�Ɏw�肵���񖼂̎w��̍ŏI�����̈Ⴂ���������܂��B)
		{
			char* sql =
				"SELECT Surinh "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQL��SELECT�Ŏw�肵���e�[�u���ƈꏏ�Ɏw�肵���񖼂̎w�肪�ꕶ�������Ƃ����Ⴂ���������܂��B)
		{
			char* sql =
				"SELECT Suringg "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQL��SELECT�Ŏw�肵���e�[�u���ƈꏏ�Ɏw�肵���񖼂̎w��̈ꕶ�����Ȃ��Ƃ����Ⴂ���������܂��B)
		{
			char* sql =
				"SELECT Surin "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQL��SELECT�Ŏw�肵���e�[�u�����̎w��̈ꕶ���ڂ̈Ⴂ���������܂��B)
		{
			char* sql =
				"SELECT UABLE1.Integer "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQL��SELECT�Ŏw�肵���e�[�u�����̎w��̓񕶎��ڂ̈Ⴂ���������܂��B)
		{
			char* sql =
				"SELECT TBBLE1.Integer "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQL��SELECT�Ŏw�肵���e�[�u�����̎w��̍ŏI�����̈Ⴂ���������܂��B)
		{
			char* sql =
				"SELECT TABLE2.Integer "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQL��SELECT�Ŏw�肵���e�[�u�����̎w�肪�ꕶ�������Ƃ����Ⴂ���������܂��B)
		{
			char* sql =
				"SELECT TABLE1a.Integer "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQL��SELECT�Ŏw�肵���e�[�u�����̎w��̈ꕶ�����Ȃ��Ƃ����Ⴂ���������܂��B)
		{
			char* sql =
				"SELECT TABLE.Integer "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}


		TEST_METHOD(ExecuteSQL��ORDER��ŕ�������������ŕ��בւ��܂��B)
		{
			char* sql =
				"SELECT * "
				"ORDER BY String "
				"FROM UNORDERED";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"1,A"				"\r\n"
				"11,AA"				"\r\n"
				"12,AB"				"\r\n"
				"2,B"				"\r\n"
				"21,BA"				"\r\n"
				"22,BB"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��ORDER���SELECT�Ŏw�肳��Ȃ���������w�肷�邱�Ƃ��ł��܂��B)
		{
			char* sql =
				"SELECT String "
				"ORDER BY Integer "
				"FROM UNORDERED";

			string expectedCsv =
				"String"	"\r\n"
				"A"			"\r\n"
				"B"			"\r\n"
				"AA"		"\r\n"
				"AB"		"\r\n"
				"BA"		"\r\n"
				"BB"		"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��ORDER���SELECT�Ŏw�肳��Ȃ������A���͂̍Ō�̗���w�肷�邱�Ƃ��ł��܂��B)
		{
			char* sql =
				"SELECT Integer "
				"ORDER BY String "
				"FROM UNORDERED";

			string expectedCsv =
				"Integer"	"\r\n"
				"1"			"\r\n"
				"11"		"\r\n"
				"12"		"\r\n"
				"2"			"\r\n"
				"21"		"\r\n"
				"22"		"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��ORDER��Ő������召���ŕ��בւ��܂��B)
		{
			char* sql =
				"SELECT * "
				"ORDER BY Integer "
				"FROM UNORDERED";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"1,A"				"\r\n"
				"2,B"				"\r\n"
				"11,AA"				"\r\n"
				"12,AB"				"\r\n"
				"21,BA"				"\r\n"
				"22,BB"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��ORDER��Ń}�C�i�X�̐��l�������܂��B)
		{
			char* sql =
				"SELECT * "
				"ORDER BY Integer "
				"FROM MINUS";

			string expectedCsv =
				"Integer"	"\r\n"
				"-6"		"\r\n"
				"-5"		"\r\n"
				"-4"		"\r\n"
				"-3"		"\r\n"
				"-2"		"\r\n"
				"-1"		"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��ORDER��ŕ����̕�����������ɂ��ĕ��בւ��܂��B)
		{
			char* sql =
				"SELECT String1, String2 "
				"ORDER BY String1, String2 "
				"FROM UNORDERED2";

			string expectedCsv =
				"String1,String2"	"\r\n"
				"A,A"				"\r\n"
				"A,B"				"\r\n"
				"B,A"				"\r\n"
				"B,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��ORDER��ŕ����̐��l��������ɂ��ĕ��בւ��܂��B)
		{
			char* sql =
				"SELECT Integer1, Integer2 "
				"ORDER BY Integer1, Integer2 "
				"FROM UNORDERED2";

			string expectedCsv =
				"Integer1,Integer2"	"\r\n"
				"1,1"				"\r\n"
				"1,2"				"\r\n"
				"2,1"				"\r\n"
				"2,2"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��ORDER��ŕ����̏������w�肵���ꍇ�ɐ�Ɏw�肵��������D�悵�ĕ��בւ��܂��B)
		{
			char* sql =
				"SELECT String1, String2 "
				"ORDER BY String2, String1 "
				"FROM UNORDERED2";

			string expectedCsv =
				"String1,String2"	"\r\n"
				"A,A"				"\r\n"
				"B,A"				"\r\n"
				"A,B"				"\r\n"
				"B,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��ORDER��ŏ������w��ł��܂��B)
		{
			char* sql =
				"SELECT String1, String2 "
				"ORDER BY String1 ASC, String2 "
				"FROM UNORDERED2";

			string expectedCsv =
				"String1,String2"	"\r\n"
				"A,A"				"\r\n"
				"A,B"				"\r\n"
				"B,A"				"\r\n"
				"B,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��ORDER��ō~�����w��ł��܂��B)
		{
			char* sql =
				"SELECT String1, String2 "
				"ORDER BY String1 DESC, String2 "
				"FROM UNORDERED2";

			string expectedCsv =
				"String1,String2"	"\r\n"
				"B,A"				"\r\n"
				"B,B"				"\r\n"
				"A,A"				"\r\n"
				"A,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��ORDER��œ�ڈȍ~�̍��ڂɏ������w��ł��܂��B)
		{
			char* sql =
				"SELECT String1, String2 "
				"ORDER BY String1 , String2 ASC "
				"FROM UNORDERED2";

			string expectedCsv =
				"String1,String2"	"\r\n"
				"A,A"				"\r\n"
				"A,B"				"\r\n"
				"B,A"				"\r\n"
				"B,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��ORDER��œ�ڈȍ~�̍��ڂɍ~�����w��ł��܂��B)
		{
			char* sql =
				"SELECT String1, String2 "
				"ORDER BY String1, String2 DESC "
				"FROM UNORDERED2";

			string expectedCsv =
				"String1,String2"	"\r\n"
				"A,B"				"\r\n"
				"A,A"				"\r\n"
				"B,B"				"\r\n"
				"B,A"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��ORDER��Ƀe�[�u�����t�̃e�[�u���ƈꏏ�Ɏw�肵���񖼂��w�肷�邱�Ƃ��ł��܂��B)
		{
			char* sql =
				"SELECT String "
				"ORDER BY UNORDERED.String "
				"FROM UNORDERED";

			string expectedCsv =
				"String"	"\r\n"
				"A"			"\r\n"
				"AA"		"\r\n"
				"AB"		"\r\n"
				"B"			"\r\n"
				"BA"		"\r\n"
				"BB"		"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��ORDER��Ƀe�[�u�����t�̃e�[�u���ƈꏏ�Ɏw�肵���񖼂��w�肵�A�e�[�u����I�����邱�Ƃ��ł��܂��B)
		{
			char* sql =
				"SELECT *"
				"ORDER BY Table2.String "
				"FROM TABLE1, TABLE2";

			string expectedCsv =
				"Integer,String,Integer,String"	"\r\n"
				"1,A,4,D"				"\r\n"
				"2,B,4,D"				"\r\n"
				"3,C,4,D"				"\r\n"
				"1,A,5,E"				"\r\n"
				"2,B,5,E"				"\r\n"
				"3,C,5,E"				"\r\n"
				"1,A,6,F"				"\r\n"
				"2,B,6,F"				"\r\n"
				"3,C,6,F"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��ORDERBY�w�肵���e�[�u���ƈꏏ�Ɏw�肵���񖼂̎w��̈ꕶ���ڂ̈Ⴂ���������܂��B)
		{
			char* sql =
				"SELECT * "
				"ORDER BY Ttring "
				"FROM UNORDERED";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQL��ORDERBY�w�肵���e�[�u���ƈꏏ�Ɏw�肵���񖼂̎w��̓񕶎��ڂ̈Ⴂ���������܂��B)
		{
			char* sql =
				"SELECT * "
				"ORDER BY Suring "
				"FROM UNORDERED";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQL��ORDERBY�w�肵���e�[�u���ƈꏏ�Ɏw�肵���񖼂̎w��̍ŏI�����̈Ⴂ���������܂��B)
		{
			char* sql =
				"SELECT * "
				"ORDER BY Strinh "
				"FROM UNORDERED";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQL��ORDERBY�w�肵���e�[�u���ƈꏏ�Ɏw�肵���񖼂̎w�肪�ꕶ�������Ƃ����Ⴂ���������܂��B)
		{
			char* sql =
				"SELECT * "
				"ORDER BY Stringg "
				"FROM UNORDERED";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQL��ORDERBY�w�肵���e�[�u���ƈꏏ�Ɏw�肵���񖼂̎w��̈ꕶ�����Ȃ��Ƃ����Ⴂ���������܂��B)
		{
			char* sql =
				"SELECT * "
				"ORDER BY Strin "
				"FROM UNORDERED";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQL��ORDERBY�w�肵���e�[�u�����̎w��̈ꕶ���ڂ̈Ⴂ���������܂��B)
		{
			char* sql =
				"SELECT * "
				"ORDER BY VNORDERED.String "
				"FROM UNORDERED";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQL��ORDERBY�w�肵���e�[�u�����̎w��̓񕶎��ڂ̈Ⴂ���������܂��B)
		{
			char* sql =
				"SELECT * "
				"ORDER BY UMORDERED.String "
				"FROM UNORDERED";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQL��ORDERBY�w�肵���e�[�u�����̎w��̍ŏI�����̈Ⴂ���������܂��B)
		{
			char* sql =
				"SELECT * "
				"ORDER BY UNORDEREE.String "
				"FROM UNORDERED";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQL��ORDERBY�w�肵���e�[�u�����̎w�肪�ꕶ�������Ƃ����Ⴂ���������܂��B)
		{
			char* sql =
				"SELECT * "
				"ORDER BY UNORDEREDD.String "
				"FROM UNORDERED";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQL��ORDERBY�w�肵���e�[�u�����̎w��̈ꕶ�����Ȃ��Ƃ����Ⴂ���������܂��B)
		{
			char* sql =
				"SELECT * "
				"ORDER BY UNORDERE.String "
				"FROM UNORDERED";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQL��ORDERBY�ŞB���ȃe�[�u���ƈꏏ�Ɏw�肵���񖼂��w�肵���ꍇ��ERR_BAD_COLUMN_NAME�G���[�ƂȂ�܂��B)
		{
			char* sql =
				"SELECT * "
				"ORDER BY String "
				"FROM TABLE1, TABLE2";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE��Ő��l��ɑ΂�������Ƃ��ĕ�����͎w��ł��܂���B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = \'2\' "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}


		TEST_METHOD(ExecuteSQL��WHERE��Ő��l�Ƃ��ē����������̎w�肪�ł��܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = 2 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"2,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE��Ő��l�Ƃ��ē������Ȃ������̎w�肪�ł��܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer <> 2 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"1,A"				"\r\n"
				"3,C"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE��Ő��l�Ƃ��đ傫�������̎w�肪�ł��܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer > 2 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"3,C"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE��Ő��l�Ƃ��ď����������̎w�肪�ł��܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer < 2 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"1,A"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE��Ő��l�Ƃ��Ĉȏ�̏����̎w�肪�ł��܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer >= 2 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"2,B"				"\r\n"
				"3,C"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}


		TEST_METHOD(ExecuteSQL��WHERE��Ő��l�Ƃ��Ĉȉ��̏����̎w�肪�ł��܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer <= 2 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"1,A"				"\r\n"
				"2,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE��Ń}�C�i�X�̐��l�������܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer < -3 "
				"FROM MINUS";

			string expectedCsv =
				"Integer"	"\r\n"
				"-4"		"\r\n"
				"-5"		"\r\n"
				"-6"		"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}


		TEST_METHOD(ExecuteSQL��WHERE��Ńv���X�𖾎������̐��l�������܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer <= +2 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"1,A"				"\r\n"
				"2,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE��Ń}�C�i�X���w�肵���̗񖼂������܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE -Integer > 3 "
				"FROM MINUS";

			string expectedCsv =
				"Integer"	"\r\n"
				"-4"		"\r\n"
				"-5"		"\r\n"
				"-6"		"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}


		TEST_METHOD(ExecuteSQL��WHERE��Ńv���X�𖾎������񖼂������܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE +Integer <= 2 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"1,A"				"\r\n"
				"2,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE��ŕ�����Ƀ}�C�i�X�̎w��͂ł��܂���B)
		{
			char* sql =
				"SELECT * "
				"WHERE String = -\'B\' "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE��ŕ�����Ƀv���X�̎w��͂ł��܂���B)
		{
			char* sql =
				"SELECT * "
				"WHERE String = +\'B\' "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE���SELECT��Ŏw�肵�Ă��Ȃ���̏����̎w�肪�ł��܂��B)
		{
			char* sql =
				"SELECT String "
				"WHERE Integer = 2 "
				"FROM TABLE1";

			string expectedCsv =
				"String"	"\r\n"
				"B"			"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE���SELECT��Ŏw�肵�Ă��Ȃ��A���͂̍Ō�̗񂪃e�[�u�������w�肹���ɏ����̎w�肪�ł��܂��B)
		{
			char* sql =
				"SELECT Integer "
				"WHERE String = \'B\' "
				"FROM TABLE1";

			string expectedCsv =
				"Integer"	"\r\n"
				"2"			"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE���SELECT��Ŏw�肵�Ă��Ȃ��A���͂̍Ō�̗񂪃e�[�u�������w�肵�ď����̎w�肪�ł��܂��B)
		{
			char* sql =
				"SELECT Integer "
				"WHERE TABLE1.String = \'B\' "
				"FROM TABLE1";

			string expectedCsv =
				"Integer"	"\r\n"
				"2"			"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE��ŕ�����Ɛ��l�̓����������̔�r�������ꍇ��ERR_WHERE_OPERAND_TYPE�G���[�ƂȂ�܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE String = 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE��ŕ�����Ɛ��l�̓������Ȃ������̔�r�������ꍇ��ERR_WHERE_OPERAND_TYPE�G���[�ƂȂ�܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE String <> 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE��ŕ�����Ɛ��l�̏����������̔�r�������ꍇ��ERR_WHERE_OPERAND_TYPE�G���[�ƂȂ�܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE String < 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE��ŕ�����Ɛ��l�̈ȉ������̔�r�������ꍇ��ERR_WHERE_OPERAND_TYPE�G���[�ƂȂ�܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE String <= 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE��ŕ�����Ɛ��l�̑傫�������̔�r�������ꍇ��ERR_WHERE_OPERAND_TYPE�G���[�ƂȂ�܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE String > 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE��ŕ�����Ɛ��l�̈ȏ�����̔�r�������ꍇ��ERR_WHERE_OPERAND_TYPE�G���[�ƂȂ�܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE String >= 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE��Ő��l�ƕ�����̓����������̔�r�������ꍇ��ERR_WHERE_OPERAND_TYPE�G���[�ƂȂ�܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = \'B\' "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE��Ő��l�ƕ�����̓������Ȃ������̔�r�������ꍇ��ERR_WHERE_OPERAND_TYPE�G���[�ƂȂ�܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer <> \'B\' "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE��Ő��l�ƕ�����̏����������̔�r�������ꍇ��ERR_WHERE_OPERAND_TYPE�G���[�ƂȂ�܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer < \'B\' "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE��Ő��l�ƕ�����̈ȉ������̔�r�������ꍇ��ERR_WHERE_OPERAND_TYPE�G���[�ƂȂ�܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer <= \'B\' "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE��Ő��l�ƕ�����̑傫�������̔�r�������ꍇ��ERR_WHERE_OPERAND_TYPE�G���[�ƂȂ�܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer > \'B\' "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE��Ő��l�ƕ�����̈ȏ�����̔�r�������ꍇ��ERR_WHERE_OPERAND_TYPE�G���[�ƂȂ�܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer >= \'B\' "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE��ŕ�����Ƃ��ē����������̎w�肪�ł��܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE String = \'B\' "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"2,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE��ŕ�����Ƃ��ē������Ȃ������̎w�肪�ł��܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE String <> \'B\' "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"1,A"				"\r\n"
				"3,C"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE��ŕ�����Ƃ��đ傫�������̎w�肪�ł��܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE String > \'B\' "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"3,C"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE��ŕ�����Ƃ��ď����������̎w�肪�ł��܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE String < \'B\' "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"1,A"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE��ŕ�����Ƃ��Ĉȏ�̏����̎w�肪�ł��܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE String >= \'B\' "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"2,B"				"\r\n"
				"3,C"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}


		TEST_METHOD(ExecuteSQL��WHERE��ŕ�����Ƃ��Ĉȉ��̏����̎w�肪�ł��܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE String <= \'B\' "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"1,A"				"\r\n"
				"2,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE�w�肵���e�[�u���ƈꏏ�Ɏw�肵���񖼂̎w��̈ꕶ���ڂ̈Ⴂ���������܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Ttring = \'A\' "
				"FROM UNORDERED";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE�w�肵���e�[�u���ƈꏏ�Ɏw�肵���񖼂̎w��̓񕶎��ڂ̈Ⴂ���������܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Suring = \'A\' "
				"FROM UNORDERED";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE�w�肵���e�[�u���ƈꏏ�Ɏw�肵���񖼂̎w��̍ŏI�����̈Ⴂ���������܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Strinh  = \'A\'"
				"FROM UNORDERED";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE�w�肵���e�[�u���ƈꏏ�Ɏw�肵���񖼂̎w�肪�ꕶ�������Ƃ����Ⴂ���������܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Stringg  = \'A\'"
				"FROM UNORDERED";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE�w�肵���e�[�u���ƈꏏ�Ɏw�肵���񖼂̎w��̈ꕶ�����Ȃ��Ƃ����Ⴂ���������܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Strin  = \'A\'"
				"FROM UNORDERED";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE��Ŕ�r�̃e�[�u���ƈꏏ�Ɏw�肵���񖼂��E�ӂɎ����Ă��邱�Ƃ��ł��܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE 2 = Integer "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"2,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE��ŉ��Z���Z�q���g���܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = 1 + 2 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"3,C"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE��̉��Z���Z�q�̍��ӂ����l�łȂ��ꍇ��ERR_WHERE_OPERAND_TYPE�G���[�ƂȂ�܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = \'A\' + 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE��̉��Z���Z�q�̉E�ӂ����l�łȂ��ꍇ��ERR_WHERE_OPERAND_TYPE�G���[�ƂȂ�܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = 1 + \'B\' "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE��Ō��Z���Z�q���g���܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = 3 - 1 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"2,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE��̌��Z���Z�q�̍��ӂ����l�łȂ��ꍇ��ERR_WHERE_OPERAND_TYPE�G���[�ƂȂ�܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = \'A\' - 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE��̌��Z���Z�q�̉E�ӂ����l�łȂ��ꍇ��ERR_WHERE_OPERAND_TYPE�G���[�ƂȂ�܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = 1 - \'B\' "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE��ŏ�Z���Z�q���g���܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = 1 * 2 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"2,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE��̏�Z���Z�q�̍��ӂ����l�łȂ��ꍇ��ERR_WHERE_OPERAND_TYPE�G���[�ƂȂ�܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = \'A\' * 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE��̏�Z���Z�q�̉E�ӂ����l�łȂ��ꍇ��ERR_WHERE_OPERAND_TYPE�G���[�ƂȂ�܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = 1 * \'B\' "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE��ŏ��Z���Z�q���g���܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = 5 / 2 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"2,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE��̏��Z���Z�q�̍��ӂ����l�łȂ��ꍇ��ERR_WHERE_OPERAND_TYPE�G���[�ƂȂ�܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = \'A\' / 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE��̏��Z���Z�q�̉E�ӂ����l�łȂ��ꍇ��ERR_WHERE_OPERAND_TYPE�G���[�ƂȂ�܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = 1 / \'B\' "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE���AND���Z�q���g���܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE 1 < Integer AND Integer < 3 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"2,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE���AND���Z�q�̍��ӂ��^�U�l�łȂ��ꍇ��ERR_WHERE_OPERAND_TYPE�G���[�ƂȂ�܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE 2 AND Integer = 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE���AND���Z�q�̉E�ӂ��^�U�l�łȂ��ꍇ��ERR_WHERE_OPERAND_TYPE�G���[�ƂȂ�܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = 1 AND 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE���OR���Z�q���g���܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer < 2 OR 2 < Integer "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"1,A"				"\r\n"
				"3,C"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE���OR���Z�q�̍��ӂ��^�U�l�łȂ��ꍇ��ERR_WHERE_OPERAND_TYPE�G���[�ƂȂ�܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE 2 OR Integer = 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE���OR���Z�q�̉E�ӂ��^�U�l�łȂ��ꍇ��ERR_WHERE_OPERAND_TYPE�G���[�ƂȂ�܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = 1 OR 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE��ŉ��Z�q�̗D�揇�ʂ��l������܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = 2 * 1 + 1 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"3,C"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE��ŉ��Z���Z�q�͌��Z���Z�q��苭���͂Ȃ��D�揇�ʂł��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = 2 - 1 + 1 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"2,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE��ŏ�Z���Z�q�͌��Z���Z�q��苭���D�揇�ʂł��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = 8 - 3 * 2 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"2,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE��ŏ�Z���Z�q�͉��Z���Z�q��苭���D�揇�ʂł��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = 1 + 1 * 2 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"3,C"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE��ŏ�Z���Z�q�͏��Z���Z�q�Ɠ����D�揇�ʂł��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = 2 * 5 / 3 * 2 - 4 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"2,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE��œ��������Z�q�͉��Z���Z�q���ア�D�揇�ʂł��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = 1 + 1 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"2,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE��œ������Ȃ����Z�q�͉��Z���Z�q���ア�D�揇�ʂł��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer <> 1 + 1 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"1,A"				"\r\n"
				"3,C"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE��ő傫�����Z�q�͉��Z���Z�q���ア�D�揇�ʂł��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer > 1 + 1 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"3,C"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE��ŏ��������Z�q�͉��Z���Z�q���ア�D�揇�ʂł��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer < 1 + 1 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"1,A"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE��ňȏ㉉�Z�q�͉��Z���Z�q���ア�D�揇�ʂł��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer >= 1 + 1 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"2,B"				"\r\n"
				"3,C"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE��ňȉ����Z�q�͉��Z���Z�q���ア�D�揇�ʂł��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer <= 1 + 1 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"1,A"				"\r\n"
				"2,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE���AND���Z�q�͔�r���Z�q���ア�D�揇�ʂł��B)
		{
			char* sql =
				"SELECT * "
				"WHERE 1 < Integer AND Integer < 3 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"2,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE���OR���Z�q��AND���Z�q���ア�D�揇�ʂł��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = 1 OR Integer <= 2 AND 2 <= Integer "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"1,A"				"\r\n"
				"2,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE��ŃJ�b�R�ɂ��D�揇�ʂ̎w�肪�ł��܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = (1 + 2) * 3 - 7 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"2,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE��ŃJ�b�R�ɂ�荶�����𐧌䂷�邱�Ƃ��ł��܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = 1 - (2 - 3) "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"2,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE��Ńl�X�g�����J�b�R�ɂ��D�揇�ʂ̎w�肪�ł��܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = (2 * (2 + 1) + 2) / 3 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"2,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE��ŃJ�b�R�����̉��Z�q�̗D�揇�ʂ̎w�肪�ł��܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = (3 * 2 - 2 * 2) "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"2,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE��ŃJ�b�R�J����A���ŋL�q���邱�Ƃ��ł��܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = ((3 - 2) * 2) "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"2,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE��ŃJ�b�R�����A���ŋL�q���邱�Ƃ��ł��܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = (2 * (3 - 2))"
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"2,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE��Ńe�[�u�����̎w�肪�ł��܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE TABLE1.Integer = 2 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"2,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE��Ƀe�[�u�����t�̃e�[�u���ƈꏏ�Ɏw�肵���񖼂��w�肵�A�e�[�u����I�����邱�Ƃ��ł��܂��B)
		{
			char* sql =
				"SELECT *"
				"WHERE Table2.Integer = 5 "
				"FROM TABLE1, TABLE2";

			string expectedCsv =
				"Integer,String,Integer,String"	"\r\n"
				"1,A,5,E"				"\r\n"
				"2,B,5,E"				"\r\n"
				"3,C,5,E"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE��𗘗p���Č������s�����Ƃ��ł��܂��B)
		{
			char* sql =
				"SELECT PARENTS.Name, CHILDREN.Name "
				"WHERE PARENTS.Id = CHILDREN.ParentId "
				"FROM PARENTS, CHILDREN";

			string expectedCsv =
				"Name,Name"			"\r\n"
				"Parent1,Child1"	"\r\n"
				"Parent1,Child2"	"\r\n"
				"Parent2,Child3"	"\r\n"
				"Parent2,Child4"	"\r\n"
				"Parent3,Child5"	"\r\n"
				"Parent3,Child6"	"\r\n"
				"Parent3,Child7"	"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE��̃e�[�u���ƈꏏ�Ɏw�肵���񖼂̎w�肪�����܂��ȏꍇ��ERR_BAD_COLUMN_NAME�G���[�ƂȂ�܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = 2 "
				"FROM TABLE1, TABLE2";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE��Ŏw�肵���e�[�u���ƈꏏ�Ɏw�肵���񖼂̎w��̈ꕶ���ڂ̈Ⴂ���������܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE TABLE1.Jnteger = 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE��Ŏw�肵���e�[�u���ƈꏏ�Ɏw�肵���񖼂̎w��̓񕶎��ڂ̈Ⴂ���������܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE TABLE1.Ioteger = 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE��Ŏw�肵���e�[�u���ƈꏏ�Ɏw�肵���񖼂̎w��̍ŏI�����̈Ⴂ���������܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE TABLE1.Integes = 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE��Ŏw�肵���e�[�u���ƈꏏ�Ɏw�肵���񖼂̎w�肪�ꕶ�������Ƃ����Ⴂ���������܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE TABLE1.Integerr = 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE��Ŏw�肵���e�[�u���ƈꏏ�Ɏw�肵���񖼂̎w��̈ꕶ�����Ȃ��Ƃ����Ⴂ���������܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE TABLE.Intege = 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE��Ŏw�肵���e�[�u�����̎w��̈ꕶ���ڂ̈Ⴂ���������܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE UABLE1.Integer = 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE��Ŏw�肵���e�[�u�����̎w��̓񕶎��ڂ̈Ⴂ���������܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE TBBLE1.Integer = 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE��Ŏw�肵���e�[�u�����̎w��̍ŏI�����̈Ⴂ���������܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE TABLE2.Integer = 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE��Ŏw�肵���e�[�u�����̎w�肪�ꕶ�������Ƃ����Ⴂ���������܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE TABLE1a.Integer = 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE��Ŏw�肵���e�[�u�����̎w��̈ꕶ�����Ȃ��Ƃ����Ⴂ���������܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE TABLE.Integer = 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE��̌��ORDER����L�q���邱�Ƃ��ł��܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer <> 2 "
				"ORDER BY Integer DESC "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"3,C"				"\r\n"
				"1,A"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��ORDER��̌��WHERE����L�q���邱�Ƃ��ł��܂��B)
		{
			char* sql =
				"SELECT * "
				"ORDER BY Integer DESC "
				"WHERE Integer <> 2 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"3,C"				"\r\n"
				"1,A"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��FROM��̌��SQL����������ERR_SQL_SYNTAX�G���[�ƂȂ�܂��B)
		{
			char* sql =
				"SELECT * "
				"FROM TABLE1 *";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE����x�L�q�����ERR_SQL_SYNTAX�G���[�ƂȂ�܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer <> 2 "
				"WHERE Integer <> 2 "
				"FROM TABLE1";


			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQL��ORDER����x�L�q�����ERR_SQL_SYNTAX�G���[�ƂȂ�܂��B)
		{
			char* sql =
				"SELECT * "
				"ORDER BY Integer DESC "
				"ORDER BY Integer DESC "
				"FROM TABLE1";


			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQL��SELECT�L�[���[�h���A�啶���ł��������ł����ʂ��܂��B)
		{
			char* sql =
				"select * "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"1,A"				"\r\n"
				"2,B"				"\r\n"
				"3,C"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��FROM�L�[���[�h���A�啶���ł��������ł����ʂ��܂��B)
		{
			char* sql =
				"SELECT * "
				"from TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"1,A"				"\r\n"
				"2,B"				"\r\n"
				"3,C"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��ORDER�L�[���[�h���A�啶���ł��������ł����ʂ��܂��B)
		{
			char* sql =
				"SELECT * "
				"order BY String "
				"FROM UNORDERED";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"1,A"				"\r\n"
				"11,AA"				"\r\n"
				"12,AB"				"\r\n"
				"2,B"				"\r\n"
				"21,BA"				"\r\n"
				"22,BB"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��BY�L�[���[�h���A�啶���ł��������ł����ʂ��܂��B)
		{
			char* sql =
				"SELECT * "
				"ORDER by String "
				"FROM UNORDERED";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"1,A"				"\r\n"
				"11,AA"				"\r\n"
				"12,AB"				"\r\n"
				"2,B"				"\r\n"
				"21,BA"				"\r\n"
				"22,BB"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��ASC�L�[���[�h���A�啶���ł��������ł����ʂ��܂��B)
		{
			char* sql =
				"SELECT String1, String2 "
				"ORDER BY String1 asc, String2 "
				"FROM UNORDERED2";

			string expectedCsv =
				"String1,String2"	"\r\n"
				"A,A"				"\r\n"
				"A,B"				"\r\n"
				"B,A"				"\r\n"
				"B,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��DESC�L�[���[�h���A�啶���ł��������ł����ʂ��܂��B)
		{
			char* sql =
				"SELECT String1, String2 "
				"ORDER BY String1 desc, String2 "
				"FROM UNORDERED2";

			string expectedCsv =
				"String1,String2"	"\r\n"
				"B,A"				"\r\n"
				"B,B"				"\r\n"
				"A,A"				"\r\n"
				"A,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE�L�[���[�h���A�啶���ł��������ł����ʂ��܂��B)
		{
			char* sql =
				"SELECT * "
				"where Integer = 2 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"2,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��AND�L�[���[�h���A�啶���ł��������ł����ʂ��܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE 1 < Integer and Integer < 3 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"2,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}
		TEST_METHOD(ExecuteSQL��OR�L�[���[�h���A�啶���ł��������ł����ʂ��܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer < 2 or 2 < Integer "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"1,A"				"\r\n"
				"3,C"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}


		TEST_METHOD(ExecuteSQL��From��̃e�[�u�������A�啶���ł��������ł����ʂ��܂��B)
		{
			char* sql =
				"SELECT * "
				"FROM table1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"1,A"				"\r\n"
				"2,B"				"\r\n"
				"3,C"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��SELECT��̃e�[�u���ƈꏏ�Ɏw�肵���񖼂��A�啶���ł��������ł����ʂ��܂��B)
		{
			char* sql =
				"SELECT sTRING "
				"FROM table1";

			string expectedCsv =
				"String"	"\r\n"
				"A"			"\r\n"
				"B"			"\r\n"
				"C"			"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��SELECT��̃e�[�u�������A�啶���ł��������ł����ʂ��܂��B)
		{
			char* sql =
				"SELECT table1.String "
				"FROM TABLE1";

			string expectedCsv =
				"String"	"\r\n"
				"A"			"\r\n"
				"B"			"\r\n"
				"C"			"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��ORDER��̃e�[�u���ƈꏏ�Ɏw�肵���񖼂��A�啶���ł��������ł����ʂ����܂��B)
		{
			char* sql =
				"SELECT * "
				"ORDER BY sTRING "
				"FROM UNORDERED";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"1,A"				"\r\n"
				"11,AA"				"\r\n"
				"12,AB"				"\r\n"
				"2,B"				"\r\n"
				"21,BA"				"\r\n"
				"22,BB"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��ORDER��̃e�[�u�������A�啶���ł��������ł����ʂ����܂��B)
		{
			char* sql =
				"SELECT * "
				"ORDER BY unordered.String "
				"FROM UNORDERED";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"1,A"				"\r\n"
				"11,AA"				"\r\n"
				"12,AB"				"\r\n"
				"2,B"				"\r\n"
				"21,BA"				"\r\n"
				"22,BB"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}


		TEST_METHOD(ExecuteSQL�͐擪��SELECT�ł͂Ȃ������ꍇ��ERR_SQL_SYNTAX��Ԃ��܂��B)
		{
			char* sql =
				"a SELECT * "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQL��SELECT�̎��̌ꂪ���ʎq�ł��A�X�^���X�N�ł��Ȃ������ꍇ��ERR_SQL_SYNTAX��Ԃ��܂��B)
		{
			char* sql =
				"SELECT "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQL��SELECT��̃J���}�̌オ���ʎq�ł��A�X�^���X�N�ł��Ȃ������ꍇ��ERR_SQL_SYNTAX��Ԃ��܂��B)
		{
			char* sql =
				"SELECT String, "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQL��SELECT��̃h�b�g�̌�Ƀe�[�u���ƈꏏ�Ɏw�肵���񖼂̋L�q���Ȃ������ꍇ��ERR_SQL_SYNTAX��Ԃ��܂��B)
		{
			char* sql =
				"SELECT TABLE1. "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQL��SELECT��̃h�b�g�̑O�Ƀe�[�u�����̋L�q���Ȃ������ꍇ��ERR_SQL_SYNTAX��Ԃ��܂��B)
		{
			char* sql =
				"SELECT .String "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQL��ORDER�̌オBY�łȂ������ꍇ��ERR_SQL_SYNTAX��Ԃ��܂��B)
		{
			char* sql =
				"SELECT * "
				"ORDER b String"
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQL��BY�̌オ���ʎq�łȂ������ꍇ��ERR_SQL_SYNTAX��Ԃ��܂��B)
		{
			char* sql =
				"SELECT * "
				"ORDER BY BY"
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQL��ORDER��̃h�b�g�̌�Ƀe�[�u���ƈꏏ�Ɏw�肵���񖼂̋L�q���Ȃ������ꍇ��ERR_SQL_SYNTAX��Ԃ��܂��B)
		{
			char* sql =
				"SELECT * "
				"ORDER BY TABLE1. "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQL��ORDER��̃h�b�g�̑O�Ƀe�[�u�����̋L�q���Ȃ������ꍇ��ERR_SQL_SYNTAX��Ԃ��܂��B)
		{
			char* sql =
				"SELECT * "
				"ORDER BY .String "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQL��ORDER��̃J���}�̌オ�̎��ʎq�łȂ������ꍇ��ERR_SQL_SYNTAX��Ԃ��܂��B)
		{
			char* sql =
				"SELECT * "
				"ORDER BY String, "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE�̌オ���ʎq�ł����e�����ł��Ȃ������ꍇ��ERR_SQL_SYNTAX��Ԃ��܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE * = 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE��̃h�b�g�̌�Ƀe�[�u���ƈꏏ�Ɏw�肵���񖼂̋L�q���Ȃ������ꍇ��ERR_SQL_SYNTAX��Ԃ��܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE TABLE1. = 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE��̃h�b�g�̑O�Ƀe�[�u�����̋L�q���Ȃ������ꍇ��ERR_SQL_SYNTAX��Ԃ��܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE .Integer = 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE��̍��ӂ̌オ���Z�q�ł͂Ȃ������ꍇ��ERR_SQL_SYNTAX��Ԃ��܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer WHERE 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE��̉��Z�q�̌オ���ʎq�ł����e�����ł��Ȃ������ꍇ��ERR_SQL_SYNTAX��Ԃ��܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQL��FROM�傪�Ȃ������ꍇ��ERR_SQL_SYNTAX��Ԃ��܂��B)
		{
			char* sql =
				"SELECT * "
				"TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQL��FROM�̌�Ɏ��ʎq���Ȃ������ꍇ��ERR_SQL_SYNTAX��Ԃ��܂��B)
		{
			char* sql =
				"SELECT * "
				"FROM *";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}


		TEST_METHOD(ExecuteSQL��SELECT�̌�ɃX�y�[�X���Ȃ��Ă����Ȃ������܂��B)
		{
			char* sql =
				"SELECT* "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"1,A"				"\r\n"
				"2,B"				"\r\n"
				"3,C"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL�̓h�b�g�̌�ɃX�y�[�X�������Ă����Ȃ������܂��B)
		{
			char* sql =
				"SELECT TABLE1. Integer "
				"FROM TABLE1";

			string expectedCsv =
				"Integer"	"\r\n"
				"1"			"\r\n"
				"2"			"\r\n"
				"3"			"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL�̓h�b�g�̌�ɃX�y�[�X���Ȃ��Ă����Ȃ������܂��B)
		{
			char* sql =
				"SELECT TABLE1.Integer "
				"FROM TABLE1";

			string expectedCsv =
				"Integer"	"\r\n"
				"1"			"\r\n"
				"2"			"\r\n"
				"3"			"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL�̓J���}�̌�ɃX�y�[�X�������Ă����Ȃ������܂��B)
		{
			char* sql =
				"SELECT Integer, String "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"1,A"				"\r\n"
				"2,B"				"\r\n"
				"3,C"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL�̓J���}�̌�ɃX�y�[�X���Ȃ��Ă����Ȃ������܂��B)
		{
			char* sql =
				"SELECT Integer,String "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"1,A"				"\r\n"
				"2,B"				"\r\n"
				"3,C"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL�̓h�b�g�̌�ɃX�y�[�X�����܂��ɕ����������ƃL�[���[�h�Ƃ��ēǂݍ��܂�܂���B)
		{
			char* sql =
				"SELECTSTRING "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQL�̓A�X�^���X�N�̌�ɃX�y�[�X���Ȃ��Ă����Ȃ������܂��B)
		{
			char* sql =
				"SELECT *"
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"1,A"				"\r\n"
				"2,B"				"\r\n"
				"3,C"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��ORDER�̌�ɃX�y�[�X�����܂��ɕ����������ƃL�[���[�h�Ƃ��ēǂݍ��܂�܂���B)
		{
			char* sql =
				"SELECT * "
				"ORDERBY Integer "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQL��BY�̌�ɃX�y�[�X�����܂��ɕ����������ƃL�[���[�h�Ƃ��ēǂݍ��܂�܂���B)
		{
			char* sql =
				"SELECT * "
				"ORDER BYInteger "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}


		TEST_METHOD(ExecuteSQL��ASC�̌�ɃX�y�[�X�����܂��ɕ����������ƃL�[���[�h�Ƃ��ēǂݍ��܂�܂���B)
		{
			char* sql =
				"SELECT * "
				"ORDER BY Integer ASC"
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQL��DESC�̌�ɃX�y�[�X�����܂��ɕ����������ƃL�[���[�h�Ƃ��ēǂݍ��܂�܂���B)
		{
			char* sql =
				"SELECT * "
				"ORDER BY Integer DESC"
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQL��WHERE�̌�ɃX�y�[�X���Ȃ��Ă����Ȃ������܂��B)
		{
			char* sql =
				"SELECT *"
				"WHERE\'B\' = String "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"2,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��WHERE�̌�ɕ����������Ɛ������e�����Ƃ��ēǂݍ��܂�܂���B)
		{
			char* sql =
				"SELECT *"
				"WHEREInteger = 2"
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_TOKEN_CANT_READ, result);
		}

		TEST_METHOD(ExecuteSQL�͎��ʎq�̌�ɃX�y�[�X���Ȃ��Ă����Ȃ������܂��B)
		{
			char* sql =
				"SELECT *"
				"WHERE Integer= 2 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"2,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL�͐������e�����̌�ɕ����������Ɛ������e�����Ƃ��ēǂݍ��܂�܂���B)
		{
			char* sql =
				"SELECT *"
				"WHERE Integer = 2"
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_TOKEN_CANT_READ, result);
		}

		TEST_METHOD(ExecuteSQL�͕����񃊃e�����̌�ɃX�y�[�X���Ȃ��Ă����Ȃ������܂��B)
		{
			char* sql =
				"SELECT *"
				"WHERE String = \'B\'"
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"2,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL�͓������L���̌�ɃX�y�[�X���Ȃ��Ă����Ȃ������܂��B)
		{
			char* sql =
				"SELECT *"
				"WHERE Integer =2 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"2,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL�͓������Ȃ��L���̌�ɃX�y�[�X���Ȃ��Ă����Ȃ������܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer <>2 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"1,A"				"\r\n"
				"3,C"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL�͑�Ȃ�L���̌�ɃX�y�[�X���Ȃ��Ă����Ȃ������܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer >2 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"3,C"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL�͏��Ȃ�L���̌�ɃX�y�[�X���Ȃ��Ă����Ȃ������܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer <2 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"1,A"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL�͈ȏ�L���̌�ɃX�y�[�X���Ȃ��Ă����Ȃ������܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer >=2 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"2,B"				"\r\n"
				"3,C"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}


		TEST_METHOD(ExecuteSQL�͈ȉ��L���̌�ɃX�y�[�X���Ȃ��Ă����Ȃ������܂��B)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer <=2 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"1,A"				"\r\n"
				"2,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL�͉��Z�L���̌�ɃX�y�[�X���Ȃ��Ă����Ȃ������܂��B)
		{
			char* sql =
				"SELECT *"
				"WHERE Integer +1 = 3 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"2,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL�͌��Z�L���̌�ɃX�y�[�X���Ȃ��Ă����Ȃ������܂��B)
		{
			char* sql =
				"SELECT *"
				"WHERE Integer = 3 -1 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"2,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL�͏�Z�L���̌�ɃX�y�[�X���Ȃ��Ă����Ȃ������܂��B)
		{
			char* sql =
				"SELECT *"
				"WHERE Integer = 2 *1 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"2,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL�͏��Z�L���̌�ɃX�y�[�X���Ȃ��Ă����Ȃ������܂��B)
		{
			char* sql =
				"SELECT *"
				"WHERE Integer = 2 /1 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"2,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��AND���Z�q�̌�ɃX�y�[�X���Ȃ��Ă����Ȃ������܂��B)
		{
			char* sql =
				"SELECT *"
				"WHERE Integer < 3 AND\'A\' < String "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"2,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}
		TEST_METHOD(ExecuteSQL��AND���Z�q�̌�ɃX�y�[�X�����܂��ɕ����������ƃL�[���[�h�Ƃ��ēǂݍ��܂�܂���B)
		{
			char* sql =
				"SELECT *"
				"WHERE Integer < 3 ANDInteger > 1 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQL��OR���Z�q�̌�ɃX�y�[�X���Ȃ��Ă����Ȃ������܂��B)
		{
			char* sql =
				"SELECT *"
				"WHERE Integer >= 3  OR\'A\' >= String "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"1,A"				"\r\n"
				"3,C"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}
		TEST_METHOD(ExecuteSQL��OR���Z�q�̌�ɃX�y�[�X�����܂��ɕ����������ƃL�[���[�h�Ƃ��ēǂݍ��܂�܂���B)
		{
			char* sql =
				"SELECT *"
				"WHERE Integer >= 3 ORInteger <= 1 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQL�̓J�b�R�J���̌�ɃX�y�[�X���Ȃ��Ă����Ȃ������܂��B)
		{
			char* sql =
				"SELECT *"
				"WHERE (Integer = 2) "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"2,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL�̓J�b�R�J���̌�ɃX�y�[�X�������Ă����Ȃ������܂��B)
		{
			char* sql =
				"SELECT *"
				"WHERE ( Integer = 2) "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"2,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL�̓J�b�R����̌�ɃX�y�[�X���Ȃ��Ă����Ȃ������܂��B)
		{
			char* sql =
				"SELECT *"
				"WHERE Integer = (2 - 1)* 2 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"2,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL�̓J�b�R����̌�ɃX�y�[�X�������Ă����Ȃ������܂��B)
		{
			char* sql =
				"SELECT *"
				"WHERE Integer = (2 - 1) * 2 "
				"FROM TABLE1";

			string expectedCsv =
				"Integer,String"	"\r\n"
				"2,B"				"\r\n";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
			Assert::AreEqual(expectedCsv, ReadOutput());
		}

		TEST_METHOD(ExecuteSQL��FROM�̌�ɃX�y�[�X�����܂��ɕ����������ƃL�[���[�h�Ƃ��ēǂݍ��܂�܂���B)
		{
			char* sql =
				"SELECT *"
				"FROMTABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}
	};
}