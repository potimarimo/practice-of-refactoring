#include "stdafx.h"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <iterator>

#include "CppUnitTest.h"

extern "C"{
	int ExecuteSQL(const char*, const char*);
}


//! ExecuteSQLの戻り値の種類を表します。
enum REAULT_VALUE
{
	OK = 0,                     //!< 問題なく終了しました。
	ERR_FILE_OPEN = 1,          //!< ファイルを開くことに失敗しました。
	ERR_FILE_WRITE = 2,         //!< ファイルに書き込みを行うことに失敗しました。
	ERR_FILE_CLOSE = 3,         //!< ファイルを閉じることに失敗しました。
	ERR_TOKEN_CANT_READ = 4,    //!< トークン解析に失敗しました。
	ERR_SQL_SYNTAX = 5,         //!< SQLの構文解析が失敗しました。
	ERR_BAD_COLUMN_NAME = 6,    //!< テーブル指定を含む列名が適切ではありません。
	ERR_WHERE_OPERAND_TYPE = 7, //!< 演算の左右の型が適切ではありません。
	ERR_CSV_SYNTAX = 8,         //!< CSVの構文解析が失敗しました。
	ERR_MEMORY_ALLOCATE = 9,    //!< メモリの取得に失敗しました。
	ERR_MEMORY_OVER = 10        //!< 用意したメモリ領域の上限を超えました。
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

		TEST_METHOD_INITIALIZE(初期化)
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

		TEST_METHOD(ExecuteSQLは単純なSQLを実行できます。)
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

		TEST_METHOD(ExecuteSQLは最後に空白があっても正しく動作します。)
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

		TEST_METHOD(ExecuteSQLは識別子名に数字を利用できます。)
		{
			char* sql =
				"SELECT * "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)OK, result);
		}

		TEST_METHOD(ExecuteSQLは識別子名に数字で始まる単語は利用できません。)
		{
			char* sql =
				"SELECT * "
				"FROM 1TABLE";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_TOKEN_CANT_READ, result);
		}

		TEST_METHOD(ExecuteSQLは識別子名の２文字目に数字を利用できます。)
		{
			char* sql =
				"SELECT * "
				"FROM T1ABLE";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_FILE_OPEN, result);
		}

		TEST_METHOD(ExecuteSQLは識別子名の先頭にアンダーバーを利用できます。)
		{
			char* sql =
				"SELECT * "
				"FROM _TABLE";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_FILE_OPEN, result);
		}

		TEST_METHOD(ExecuteSQLは識別子名の二文字目以降にアンダーバーを利用できます。)
		{
			char* sql =
				"SELECT * "
				"FROM T_ABLE";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_FILE_OPEN, result);
		}

		TEST_METHOD(ExecuteSQLは複数個続く区切り文字を利用できます。)
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

		TEST_METHOD(ExecuteSQLは区切り文字としてスペースを認識します。)
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

		TEST_METHOD(ExecuteSQLは区切り文字としてタブを認識します。)
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

		TEST_METHOD(ExecuteSQLは区切り文字として改行を認識します。)
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

		TEST_METHOD(ExecuteSQLは認識できないトークンを含む語を指定したときERR_TOKEN_CANT_READエラーとなります。)
		{
			char* sql =
				"?";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_TOKEN_CANT_READ, result);
		}


		TEST_METHOD(ExecuteSQLは指定したテーブル名を取得し、対応するファイルを参照できます。)
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

		TEST_METHOD(ExecuteSQLは二つののテーブルを読み込み、全ての組み合わせを出力します。)
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

		TEST_METHOD(ExecuteSQLは三つ以上のテーブルを読み込み、全ての組み合わせを出力します。)
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

		TEST_METHOD(ExecuteSQLはSQLECT句にテーブルと一緒に指定した列名を指定し、SQLを実行できます。)
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

		TEST_METHOD(ExecuteSQLはSQLECT句に複数のテーブルと一緒に指定した列名を指定し、SQLを実行できます。)
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

		TEST_METHOD(ExecuteSQLはSQLECT句に三つ以上のテーブルと一緒に指定した列名を指定し、SQLを実行できます。)
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

		TEST_METHOD(ExecuteSQLはSELECTの指定にテーブル名も指定できます。)
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

		TEST_METHOD(ExecuteSQLは複数ののテーブルを読み込み、テーブル名で区別してテーブルと一緒に指定した列名を指定することができます。)
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

		TEST_METHOD(ExecuteSQLはSELECT句でテーブル名を二つ目以降のテーブルと一緒に指定した列名に指定することができます。)
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

		TEST_METHOD(ExecuteSQLはSELECTのテーブルと一緒に指定した列名の指定があいまいな場合にERR_BAD_COLUMN_NAMEエラーとなります。)
		{
			char* sql =
				"SELECT Integer "
				"FROM TABLE1, TABLE2";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQLはSELECTで指定したテーブルと一緒に指定した列名の指定の一文字目の違いを見分けます。)
		{
			char* sql =
				"SELECT Ttring "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQLはSELECTで指定したテーブルと一緒に指定した列名の指定の二文字目の違いを見分けます。)
		{
			char* sql =
				"SELECT Suring "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQLはSELECTで指定したテーブルと一緒に指定した列名の指定の最終文字の違いを見分けます。)
		{
			char* sql =
				"SELECT Surinh "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQLはSELECTで指定したテーブルと一緒に指定した列名の指定が一文字多いという違いを見分けます。)
		{
			char* sql =
				"SELECT Suringg "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQLはSELECTで指定したテーブルと一緒に指定した列名の指定の一文字少ないという違いを見分けます。)
		{
			char* sql =
				"SELECT Surin "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQLはSELECTで指定したテーブル名の指定の一文字目の違いを見分けます。)
		{
			char* sql =
				"SELECT UABLE1.Integer "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQLはSELECTで指定したテーブル名の指定の二文字目の違いを見分けます。)
		{
			char* sql =
				"SELECT TBBLE1.Integer "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQLはSELECTで指定したテーブル名の指定の最終文字の違いを見分けます。)
		{
			char* sql =
				"SELECT TABLE2.Integer "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQLはSELECTで指定したテーブル名の指定が一文字多いという違いを見分けます。)
		{
			char* sql =
				"SELECT TABLE1a.Integer "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQLはSELECTで指定したテーブル名の指定の一文字少ないという違いを見分けます。)
		{
			char* sql =
				"SELECT TABLE.Integer "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}


		TEST_METHOD(ExecuteSQLはORDER句で文字列を辞書順で並べ替えます。)
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

		TEST_METHOD(ExecuteSQLはORDER句にSELECTで指定されなかった列を指定することができます。)
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

		TEST_METHOD(ExecuteSQLはORDER句にSELECTで指定されなかった、入力の最後の列を指定することができます。)
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

		TEST_METHOD(ExecuteSQLはORDER句で数字列を大小順で並べ替えます。)
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

		TEST_METHOD(ExecuteSQLはORDER句でマイナスの数値を扱えます。)
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

		TEST_METHOD(ExecuteSQLはORDER句で複数の文字列を条件にして並べ替えます。)
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

		TEST_METHOD(ExecuteSQLはORDER句で複数の数値列を条件にして並べ替えます。)
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

		TEST_METHOD(ExecuteSQLはORDER句で複数の条件を指定した場合に先に指定した条件を優先して並べ替えます。)
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

		TEST_METHOD(ExecuteSQLはORDER句で昇順を指定できます。)
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

		TEST_METHOD(ExecuteSQLはORDER句で降順を指定できます。)
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

		TEST_METHOD(ExecuteSQLはORDER句で二つ目以降の項目に昇順を指定できます。)
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

		TEST_METHOD(ExecuteSQLはORDER句で二つ目以降の項目に降順を指定できます。)
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

		TEST_METHOD(ExecuteSQLはORDER句にテーブル名付のテーブルと一緒に指定した列名を指定することができます。)
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

		TEST_METHOD(ExecuteSQLはORDER句にテーブル名付のテーブルと一緒に指定した列名を指定し、テーブルを選択することができます。)
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

		TEST_METHOD(ExecuteSQLはORDERBY指定したテーブルと一緒に指定した列名の指定の一文字目の違いを見分けます。)
		{
			char* sql =
				"SELECT * "
				"ORDER BY Ttring "
				"FROM UNORDERED";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQLはORDERBY指定したテーブルと一緒に指定した列名の指定の二文字目の違いを見分けます。)
		{
			char* sql =
				"SELECT * "
				"ORDER BY Suring "
				"FROM UNORDERED";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQLはORDERBY指定したテーブルと一緒に指定した列名の指定の最終文字の違いを見分けます。)
		{
			char* sql =
				"SELECT * "
				"ORDER BY Strinh "
				"FROM UNORDERED";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQLはORDERBY指定したテーブルと一緒に指定した列名の指定が一文字多いという違いを見分けます。)
		{
			char* sql =
				"SELECT * "
				"ORDER BY Stringg "
				"FROM UNORDERED";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQLはORDERBY指定したテーブルと一緒に指定した列名の指定の一文字少ないという違いを見分けます。)
		{
			char* sql =
				"SELECT * "
				"ORDER BY Strin "
				"FROM UNORDERED";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQLはORDERBY指定したテーブル名の指定の一文字目の違いを見分けます。)
		{
			char* sql =
				"SELECT * "
				"ORDER BY VNORDERED.String "
				"FROM UNORDERED";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQLはORDERBY指定したテーブル名の指定の二文字目の違いを見分けます。)
		{
			char* sql =
				"SELECT * "
				"ORDER BY UMORDERED.String "
				"FROM UNORDERED";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQLはORDERBY指定したテーブル名の指定の最終文字の違いを見分けます。)
		{
			char* sql =
				"SELECT * "
				"ORDER BY UNORDEREE.String "
				"FROM UNORDERED";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQLはORDERBY指定したテーブル名の指定が一文字多いという違いを見分けます。)
		{
			char* sql =
				"SELECT * "
				"ORDER BY UNORDEREDD.String "
				"FROM UNORDERED";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQLはORDERBY指定したテーブル名の指定の一文字少ないという違いを見分けます。)
		{
			char* sql =
				"SELECT * "
				"ORDER BY UNORDERE.String "
				"FROM UNORDERED";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQLはORDERBYで曖昧なテーブルと一緒に指定した列名を指定した場合にERR_BAD_COLUMN_NAMEエラーとなります。)
		{
			char* sql =
				"SELECT * "
				"ORDER BY String "
				"FROM TABLE1, TABLE2";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句で数値列に対する条件として文字列は指定できません。)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = \'2\' "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}


		TEST_METHOD(ExecuteSQLはWHERE句で数値として等しい条件の指定ができます。)
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

		TEST_METHOD(ExecuteSQLはWHERE句で数値として等しくない条件の指定ができます。)
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

		TEST_METHOD(ExecuteSQLはWHERE句で数値として大きい条件の指定ができます。)
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

		TEST_METHOD(ExecuteSQLはWHERE句で数値として小さい条件の指定ができます。)
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

		TEST_METHOD(ExecuteSQLはWHERE句で数値として以上の条件の指定ができます。)
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


		TEST_METHOD(ExecuteSQLはWHERE句で数値として以下の条件の指定ができます。)
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

		TEST_METHOD(ExecuteSQLはWHERE句でマイナスの数値が扱えます。)
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


		TEST_METHOD(ExecuteSQLはWHERE句でプラスを明示したの数値が扱えます。)
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

		TEST_METHOD(ExecuteSQLはWHERE句でマイナスを指定したの列名が扱えます。)
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


		TEST_METHOD(ExecuteSQLはWHERE句でプラスを明示した列名が扱えます。)
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

		TEST_METHOD(ExecuteSQLはWHERE句で文字列にマイナスの指定はできません。)
		{
			char* sql =
				"SELECT * "
				"WHERE String = -\'B\' "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句で文字列にプラスの指定はできません。)
		{
			char* sql =
				"SELECT * "
				"WHERE String = +\'B\' "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句でSELECT句で指定していない列の条件の指定ができます。)
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

		TEST_METHOD(ExecuteSQLはWHERE句でSELECT句で指定していない、入力の最後の列がテーブル名を指定せずに条件の指定ができます。)
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

		TEST_METHOD(ExecuteSQLはWHERE句でSELECT句で指定していない、入力の最後の列がテーブル名を指定して条件の指定ができます。)
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

		TEST_METHOD(ExecuteSQLはWHERE句で文字列と数値の等しい条件の比較をした場合にERR_WHERE_OPERAND_TYPEエラーとなります。)
		{
			char* sql =
				"SELECT * "
				"WHERE String = 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句で文字列と数値の等しくない条件の比較をした場合にERR_WHERE_OPERAND_TYPEエラーとなります。)
		{
			char* sql =
				"SELECT * "
				"WHERE String <> 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句で文字列と数値の小さい条件の比較をした場合にERR_WHERE_OPERAND_TYPEエラーとなります。)
		{
			char* sql =
				"SELECT * "
				"WHERE String < 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句で文字列と数値の以下条件の比較をした場合にERR_WHERE_OPERAND_TYPEエラーとなります。)
		{
			char* sql =
				"SELECT * "
				"WHERE String <= 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句で文字列と数値の大きい条件の比較をした場合にERR_WHERE_OPERAND_TYPEエラーとなります。)
		{
			char* sql =
				"SELECT * "
				"WHERE String > 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句で文字列と数値の以上条件の比較をした場合にERR_WHERE_OPERAND_TYPEエラーとなります。)
		{
			char* sql =
				"SELECT * "
				"WHERE String >= 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句で数値と文字列の等しい条件の比較をした場合にERR_WHERE_OPERAND_TYPEエラーとなります。)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = \'B\' "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句で数値と文字列の等しくない条件の比較をした場合にERR_WHERE_OPERAND_TYPEエラーとなります。)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer <> \'B\' "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句で数値と文字列の小さい条件の比較をした場合にERR_WHERE_OPERAND_TYPEエラーとなります。)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer < \'B\' "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句で数値と文字列の以下条件の比較をした場合にERR_WHERE_OPERAND_TYPEエラーとなります。)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer <= \'B\' "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句で数値と文字列の大きい条件の比較をした場合にERR_WHERE_OPERAND_TYPEエラーとなります。)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer > \'B\' "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句で数値と文字列の以上条件の比較をした場合にERR_WHERE_OPERAND_TYPEエラーとなります。)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer >= \'B\' "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句で文字列として等しい条件の指定ができます。)
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

		TEST_METHOD(ExecuteSQLはWHERE句で文字列として等しくない条件の指定ができます。)
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

		TEST_METHOD(ExecuteSQLはWHERE句で文字列として大きい条件の指定ができます。)
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

		TEST_METHOD(ExecuteSQLはWHERE句で文字列として小さい条件の指定ができます。)
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

		TEST_METHOD(ExecuteSQLはWHERE句で文字列として以上の条件の指定ができます。)
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


		TEST_METHOD(ExecuteSQLはWHERE句で文字列として以下の条件の指定ができます。)
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

		TEST_METHOD(ExecuteSQLはWHERE指定したテーブルと一緒に指定した列名の指定の一文字目の違いを見分けます。)
		{
			char* sql =
				"SELECT * "
				"WHERE Ttring = \'A\' "
				"FROM UNORDERED";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE指定したテーブルと一緒に指定した列名の指定の二文字目の違いを見分けます。)
		{
			char* sql =
				"SELECT * "
				"WHERE Suring = \'A\' "
				"FROM UNORDERED";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE指定したテーブルと一緒に指定した列名の指定の最終文字の違いを見分けます。)
		{
			char* sql =
				"SELECT * "
				"WHERE Strinh  = \'A\'"
				"FROM UNORDERED";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE指定したテーブルと一緒に指定した列名の指定が一文字多いという違いを見分けます。)
		{
			char* sql =
				"SELECT * "
				"WHERE Stringg  = \'A\'"
				"FROM UNORDERED";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE指定したテーブルと一緒に指定した列名の指定の一文字少ないという違いを見分けます。)
		{
			char* sql =
				"SELECT * "
				"WHERE Strin  = \'A\'"
				"FROM UNORDERED";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句で比較のテーブルと一緒に指定した列名を右辺に持ってくることができます。)
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

		TEST_METHOD(ExecuteSQLはWHERE句で加算演算子が使えます。)
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

		TEST_METHOD(ExecuteSQLはWHERE句の加算演算子の左辺が数値でない場合はERR_WHERE_OPERAND_TYPEエラーとなります。)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = \'A\' + 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句の加算演算子の右辺が数値でない場合はERR_WHERE_OPERAND_TYPEエラーとなります。)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = 1 + \'B\' "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句で減算演算子が使えます。)
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

		TEST_METHOD(ExecuteSQLはWHERE句の減算演算子の左辺が数値でない場合はERR_WHERE_OPERAND_TYPEエラーとなります。)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = \'A\' - 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句の減算演算子の右辺が数値でない場合はERR_WHERE_OPERAND_TYPEエラーとなります。)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = 1 - \'B\' "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句で乗算演算子が使えます。)
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

		TEST_METHOD(ExecuteSQLはWHERE句の乗算演算子の左辺が数値でない場合はERR_WHERE_OPERAND_TYPEエラーとなります。)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = \'A\' * 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句の乗算演算子の右辺が数値でない場合はERR_WHERE_OPERAND_TYPEエラーとなります。)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = 1 * \'B\' "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句で除算演算子が使えます。)
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

		TEST_METHOD(ExecuteSQLはWHERE句の除算演算子の左辺が数値でない場合はERR_WHERE_OPERAND_TYPEエラーとなります。)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = \'A\' / 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句の除算演算子の右辺が数値でない場合はERR_WHERE_OPERAND_TYPEエラーとなります。)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = 1 / \'B\' "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句でAND演算子が使えます。)
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

		TEST_METHOD(ExecuteSQLはWHERE句のAND演算子の左辺が真偽値でない場合はERR_WHERE_OPERAND_TYPEエラーとなります。)
		{
			char* sql =
				"SELECT * "
				"WHERE 2 AND Integer = 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句のAND演算子の右辺が真偽値でない場合はERR_WHERE_OPERAND_TYPEエラーとなります。)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = 1 AND 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句でOR演算子が使えます。)
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

		TEST_METHOD(ExecuteSQLはWHERE句のOR演算子の左辺が真偽値でない場合はERR_WHERE_OPERAND_TYPEエラーとなります。)
		{
			char* sql =
				"SELECT * "
				"WHERE 2 OR Integer = 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句のOR演算子の右辺が真偽値でない場合はERR_WHERE_OPERAND_TYPEエラーとなります。)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = 1 OR 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_WHERE_OPERAND_TYPE, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句で演算子の優先順位が考慮されます。)
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

		TEST_METHOD(ExecuteSQLはWHERE句で加算演算子は減算演算子より強くはない優先順位です。)
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

		TEST_METHOD(ExecuteSQLはWHERE句で乗算演算子は減算演算子より強い優先順位です。)
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

		TEST_METHOD(ExecuteSQLはWHERE句で乗算演算子は加算演算子より強い優先順位です。)
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

		TEST_METHOD(ExecuteSQLはWHERE句で乗算演算子は除算演算子と同じ優先順位です。)
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

		TEST_METHOD(ExecuteSQLはWHERE句で等しい演算子は加算演算子より弱い優先順位です。)
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

		TEST_METHOD(ExecuteSQLはWHERE句で等しくない演算子は加算演算子より弱い優先順位です。)
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

		TEST_METHOD(ExecuteSQLはWHERE句で大きい演算子は加算演算子より弱い優先順位です。)
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

		TEST_METHOD(ExecuteSQLはWHERE句で小さい演算子は加算演算子より弱い優先順位です。)
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

		TEST_METHOD(ExecuteSQLはWHERE句で以上演算子は加算演算子より弱い優先順位です。)
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

		TEST_METHOD(ExecuteSQLはWHERE句で以下演算子は加算演算子より弱い優先順位です。)
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

		TEST_METHOD(ExecuteSQLはWHERE句でAND演算子は比較演算子より弱い優先順位です。)
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

		TEST_METHOD(ExecuteSQLはWHERE句でOR演算子はAND演算子より弱い優先順位です。)
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

		TEST_METHOD(ExecuteSQLはWHERE句でカッコによる優先順位の指定ができます。)
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

		TEST_METHOD(ExecuteSQLはWHERE句でカッコにより左結合を制御することができます。)
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

		TEST_METHOD(ExecuteSQLはWHERE句でネストしたカッコによる優先順位の指定ができます。)
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

		TEST_METHOD(ExecuteSQLはWHERE句でカッコ内部の演算子の優先順位の指定ができます。)
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

		TEST_METHOD(ExecuteSQLはWHERE句でカッコ開くを連続で記述することができます。)
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

		TEST_METHOD(ExecuteSQLはWHERE句でカッコ閉じるを連続で記述することができます。)
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

		TEST_METHOD(ExecuteSQLはWHERE句でテーブル名の指定ができます。)
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

		TEST_METHOD(ExecuteSQLはWHERE句にテーブル名付のテーブルと一緒に指定した列名を指定し、テーブルを選択することができます。)
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

		TEST_METHOD(ExecuteSQLはWHERE句を利用して結合を行うことができます。)
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

		TEST_METHOD(ExecuteSQLはWHERE句のテーブルと一緒に指定した列名の指定があいまいな場合にERR_BAD_COLUMN_NAMEエラーとなります。)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = 2 "
				"FROM TABLE1, TABLE2";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句で指定したテーブルと一緒に指定した列名の指定の一文字目の違いを見分けます。)
		{
			char* sql =
				"SELECT * "
				"WHERE TABLE1.Jnteger = 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句で指定したテーブルと一緒に指定した列名の指定の二文字目の違いを見分けます。)
		{
			char* sql =
				"SELECT * "
				"WHERE TABLE1.Ioteger = 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句で指定したテーブルと一緒に指定した列名の指定の最終文字の違いを見分けます。)
		{
			char* sql =
				"SELECT * "
				"WHERE TABLE1.Integes = 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句で指定したテーブルと一緒に指定した列名の指定が一文字多いという違いを見分けます。)
		{
			char* sql =
				"SELECT * "
				"WHERE TABLE1.Integerr = 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句で指定したテーブルと一緒に指定した列名の指定の一文字少ないという違いを見分けます。)
		{
			char* sql =
				"SELECT * "
				"WHERE TABLE.Intege = 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句で指定したテーブル名の指定の一文字目の違いを見分けます。)
		{
			char* sql =
				"SELECT * "
				"WHERE UABLE1.Integer = 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句で指定したテーブル名の指定の二文字目の違いを見分けます。)
		{
			char* sql =
				"SELECT * "
				"WHERE TBBLE1.Integer = 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句で指定したテーブル名の指定の最終文字の違いを見分けます。)
		{
			char* sql =
				"SELECT * "
				"WHERE TABLE2.Integer = 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句で指定したテーブル名の指定が一文字多いという違いを見分けます。)
		{
			char* sql =
				"SELECT * "
				"WHERE TABLE1a.Integer = 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句で指定したテーブル名の指定の一文字少ないという違いを見分けます。)
		{
			char* sql =
				"SELECT * "
				"WHERE TABLE.Integer = 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_BAD_COLUMN_NAME, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句の後にORDER句を記述することができます。)
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

		TEST_METHOD(ExecuteSQLはORDER句の後にWHERE句を記述することができます。)
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

		TEST_METHOD(ExecuteSQLはFROM句の後にSQLが続いたらERR_SQL_SYNTAXエラーとなります。)
		{
			char* sql =
				"SELECT * "
				"FROM TABLE1 *";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句を二度記述するとERR_SQL_SYNTAXエラーとなります。)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer <> 2 "
				"WHERE Integer <> 2 "
				"FROM TABLE1";


			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQLはORDER句を二度記述するとERR_SQL_SYNTAXエラーとなります。)
		{
			char* sql =
				"SELECT * "
				"ORDER BY Integer DESC "
				"ORDER BY Integer DESC "
				"FROM TABLE1";


			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQLはSELECTキーワードを、大文字でも小文字でも識別します。)
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

		TEST_METHOD(ExecuteSQLはFROMキーワードを、大文字でも小文字でも識別します。)
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

		TEST_METHOD(ExecuteSQLはORDERキーワードを、大文字でも小文字でも識別します。)
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

		TEST_METHOD(ExecuteSQLはBYキーワードを、大文字でも小文字でも識別します。)
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

		TEST_METHOD(ExecuteSQLはASCキーワードを、大文字でも小文字でも識別します。)
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

		TEST_METHOD(ExecuteSQLはDESCキーワードを、大文字でも小文字でも識別します。)
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

		TEST_METHOD(ExecuteSQLはWHEREキーワードを、大文字でも小文字でも識別します。)
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

		TEST_METHOD(ExecuteSQLはANDキーワードを、大文字でも小文字でも識別します。)
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
		TEST_METHOD(ExecuteSQLはORキーワードを、大文字でも小文字でも識別します。)
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


		TEST_METHOD(ExecuteSQLはFrom句のテーブル名を、大文字でも小文字でも識別します。)
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

		TEST_METHOD(ExecuteSQLはSELECT句のテーブルと一緒に指定した列名を、大文字でも小文字でも識別します。)
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

		TEST_METHOD(ExecuteSQLはSELECT句のテーブル名を、大文字でも小文字でも識別します。)
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

		TEST_METHOD(ExecuteSQLはORDER句のテーブルと一緒に指定した列名を、大文字でも小文字でも識別しじます。)
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

		TEST_METHOD(ExecuteSQLはORDER句のテーブル名を、大文字でも小文字でも識別しじます。)
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


		TEST_METHOD(ExecuteSQLは先頭がSELECTではなかった場合にERR_SQL_SYNTAXを返します。)
		{
			char* sql =
				"a SELECT * "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQLはSELECTの次の語が識別子でもアスタリスクでもなかった場合にERR_SQL_SYNTAXを返します。)
		{
			char* sql =
				"SELECT "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQLはSELECT句のカンマの後が識別子でもアスタリスクでもなかった場合にERR_SQL_SYNTAXを返します。)
		{
			char* sql =
				"SELECT String, "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQLはSELECT句のドットの後にテーブルと一緒に指定した列名の記述がなかった場合にERR_SQL_SYNTAXを返します。)
		{
			char* sql =
				"SELECT TABLE1. "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQLはSELECT句のドットの前にテーブル名の記述がなかった場合にERR_SQL_SYNTAXを返します。)
		{
			char* sql =
				"SELECT .String "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQLはORDERの後がBYでなかった場合にERR_SQL_SYNTAXを返します。)
		{
			char* sql =
				"SELECT * "
				"ORDER b String"
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQLはBYの後が識別子でなかった場合にERR_SQL_SYNTAXを返します。)
		{
			char* sql =
				"SELECT * "
				"ORDER BY BY"
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQLはORDER句のドットの後にテーブルと一緒に指定した列名の記述がなかった場合にERR_SQL_SYNTAXを返します。)
		{
			char* sql =
				"SELECT * "
				"ORDER BY TABLE1. "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQLはORDER句のドットの前にテーブル名の記述がなかった場合にERR_SQL_SYNTAXを返します。)
		{
			char* sql =
				"SELECT * "
				"ORDER BY .String "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQLはORDER句のカンマの後がの識別子でなかった場合にERR_SQL_SYNTAXを返します。)
		{
			char* sql =
				"SELECT * "
				"ORDER BY String, "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQLはWHEREの後が識別子でもリテラルでもなかった場合にERR_SQL_SYNTAXを返します。)
		{
			char* sql =
				"SELECT * "
				"WHERE * = 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句のドットの後にテーブルと一緒に指定した列名の記述がなかった場合にERR_SQL_SYNTAXを返します。)
		{
			char* sql =
				"SELECT * "
				"WHERE TABLE1. = 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句のドットの前にテーブル名の記述がなかった場合にERR_SQL_SYNTAXを返します。)
		{
			char* sql =
				"SELECT * "
				"WHERE .Integer = 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句の左辺の後が演算子ではなかった場合にERR_SQL_SYNTAXを返します。)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer WHERE 2 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQLはWHERE句の演算子の後が識別子でもリテラルでもなかった場合にERR_SQL_SYNTAXを返します。)
		{
			char* sql =
				"SELECT * "
				"WHERE Integer = "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQLはFROM句がなかった場合にERR_SQL_SYNTAXを返します。)
		{
			char* sql =
				"SELECT * "
				"TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQLはFROMの後に識別子がなかった場合にERR_SQL_SYNTAXを返します。)
		{
			char* sql =
				"SELECT * "
				"FROM *";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}


		TEST_METHOD(ExecuteSQLはSELECTの後にスペースがなくても問題なく動きます。)
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

		TEST_METHOD(ExecuteSQLはドットの後にスペースがあっても問題なく動きます。)
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

		TEST_METHOD(ExecuteSQLはドットの後にスペースがなくても問題なく動きます。)
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

		TEST_METHOD(ExecuteSQLはカンマの後にスペースがあっても問題なく動きます。)
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

		TEST_METHOD(ExecuteSQLはカンマの後にスペースがなくても問題なく動きます。)
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

		TEST_METHOD(ExecuteSQLはドットの後にスペースを挟まずに文字が続くとキーワードとして読み込まれません。)
		{
			char* sql =
				"SELECTSTRING "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQLはアスタリスクの後にスペースがなくても問題なく動きます。)
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

		TEST_METHOD(ExecuteSQLはORDERの後にスペースを挟まずに文字が続くとキーワードとして読み込まれません。)
		{
			char* sql =
				"SELECT * "
				"ORDERBY Integer "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQLはBYの後にスペースを挟まずに文字が続くとキーワードとして読み込まれません。)
		{
			char* sql =
				"SELECT * "
				"ORDER BYInteger "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}


		TEST_METHOD(ExecuteSQLはASCの後にスペースを挟まずに文字が続くとキーワードとして読み込まれません。)
		{
			char* sql =
				"SELECT * "
				"ORDER BY Integer ASC"
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQLはDESCの後にスペースを挟まずに文字が続くとキーワードとして読み込まれません。)
		{
			char* sql =
				"SELECT * "
				"ORDER BY Integer DESC"
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQLはWHEREの後にスペースがなくても問題なく動きます。)
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

		TEST_METHOD(ExecuteSQLはWHEREの後に文字が続くと整数リテラルとして読み込まれません。)
		{
			char* sql =
				"SELECT *"
				"WHEREInteger = 2"
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_TOKEN_CANT_READ, result);
		}

		TEST_METHOD(ExecuteSQLは識別子の後にスペースがなくても問題なく動きます。)
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

		TEST_METHOD(ExecuteSQLは整数リテラルの後に文字が続くと整数リテラルとして読み込まれません。)
		{
			char* sql =
				"SELECT *"
				"WHERE Integer = 2"
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_TOKEN_CANT_READ, result);
		}

		TEST_METHOD(ExecuteSQLは文字列リテラルの後にスペースがなくても問題なく動きます。)
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

		TEST_METHOD(ExecuteSQLは等しい記号の後にスペースがなくても問題なく動きます。)
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

		TEST_METHOD(ExecuteSQLは等しくない記号の後にスペースがなくても問題なく動きます。)
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

		TEST_METHOD(ExecuteSQLは大なり記号の後にスペースがなくても問題なく動きます。)
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

		TEST_METHOD(ExecuteSQLは小なり記号の後にスペースがなくても問題なく動きます。)
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

		TEST_METHOD(ExecuteSQLは以上記号の後にスペースがなくても問題なく動きます。)
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


		TEST_METHOD(ExecuteSQLは以下記号の後にスペースがなくても問題なく動きます。)
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

		TEST_METHOD(ExecuteSQLは加算記号の後にスペースがなくても問題なく動きます。)
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

		TEST_METHOD(ExecuteSQLは減算記号の後にスペースがなくても問題なく動きます。)
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

		TEST_METHOD(ExecuteSQLは乗算記号の後にスペースがなくても問題なく動きます。)
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

		TEST_METHOD(ExecuteSQLは除算記号の後にスペースがなくても問題なく動きます。)
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

		TEST_METHOD(ExecuteSQLはAND演算子の後にスペースがなくても問題なく動きます。)
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
		TEST_METHOD(ExecuteSQLはAND演算子の後にスペースを挟まずに文字が続くとキーワードとして読み込まれません。)
		{
			char* sql =
				"SELECT *"
				"WHERE Integer < 3 ANDInteger > 1 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQLはOR演算子の後にスペースがなくても問題なく動きます。)
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
		TEST_METHOD(ExecuteSQLはOR演算子の後にスペースを挟まずに文字が続くとキーワードとして読み込まれません。)
		{
			char* sql =
				"SELECT *"
				"WHERE Integer >= 3 ORInteger <= 1 "
				"FROM TABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}

		TEST_METHOD(ExecuteSQLはカッコ開くの後にスペースがなくても問題なく動きます。)
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

		TEST_METHOD(ExecuteSQLはカッコ開くの後にスペースがあっても問題なく動きます。)
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

		TEST_METHOD(ExecuteSQLはカッコ閉じるの後にスペースがなくても問題なく動きます。)
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

		TEST_METHOD(ExecuteSQLはカッコ閉じるの後にスペースがあっても問題なく動きます。)
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

		TEST_METHOD(ExecuteSQLはFROMの後にスペースを挟まずに文字が続くとキーワードとして読み込まれません。)
		{
			char* sql =
				"SELECT *"
				"FROMTABLE1";

			auto result = ExecuteSQL(sql, testOutputPath);

			Assert::AreEqual((int)ERR_SQL_SYNTAX, result);
		}
	};
}