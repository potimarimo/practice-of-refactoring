//! @file
#include "stdafx.h"

#include <cctype>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <memory>
#include <vector>
#include <list>
#pragma warning(disable:4996)

using namespace std;

//! カレントディレクトリにあるCSVに対し、簡易的なSQLを実行し、結果をファイルに出力します。
//! @param [in] sql 実行するSQLです。
//! @param[in] outputFileName SQLの実行結果をCSVとして出力するファイル名です。拡張子を含みます。
//! @return 実行した結果の状態です。
int ExecuteSQL(const string, const string);

//! ExecuteSQLの戻り値の種類を表します。
enum class ResultValue
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

//! 入力や出力、経過の計算に利用するデータのデータ型の種類を表します。
enum class DataType
{
	STRING,   //!< 文字列型です。
	INTEGER,  //!< 整数型です。
	BOOLEAN   //!< 真偽値型です。
};

//! トークンの種類を表します。
enum class TokenKind
{
	NOT_TOKEN,              //!< トークンを表しません。
	ASC,                    //!< ASCキーワードです。
	AND,                    //!< ANDキーワードです。
	BY,                     //!< BYキーワードです。
	DESC,                   //!< DESCキーワードです。
	FROM,                   //!< FROMキーワードです。
	OR,                     //!< ORキーワードです。
	ORDER,                  //!< ORDERキーワードです。
	SELECT,                 //!< SELECTキーワードです。
	WHERE,                  //!< WHEREキーワードです。
	ASTERISK,               //!< ＊ 記号です。
	COMMA,                  //!< ， 記号です。
	CLOSE_PAREN,            //!< ） 記号です。
	DOT,                    //!< ． 記号です。
	EQUAL,                  //!< ＝ 記号です。
	GREATER_THAN,           //!< ＞ 記号です。
	GREATER_THAN_OR_EQUAL,  //!< ＞＝ 記号です。
	LESS_THAN,              //!< ＜ 記号です。
	LESS_THAN_OR_EQUAL,     //!< ＜＝ 記号です。
	MINUS,                  //!< － 記号です。
	NOT_EQUAL,              //!< ＜＞ 記号です。
	OPEN_PAREN,             //!< （ 記号です。
	PLUS,                   //!< ＋ 記号です。
	SLASH,                  //!< ／ 記号です。
	IDENTIFIER,             //!< 識別子です。
	INT_LITERAL,            //!< 整数リテラルです。
	STRING_LITERAL          //!< 文字列リテラルです。
};

//! 一つの値を持つデータです。
class Data
{
	string m_string; //!< データが文字列型の場合の値です。

	//! 実際のデータを格納する共用体です。
	union
	{
		int integer;                  //!< データが整数型の場合の値です。
		bool boolean;                 //!< データが真偽値型の場合の値です。
	} m_value;
public:
	DataType type = DataType::STRING; //!< データの型です。

	//! Dataクラスの新しいインスタンスを初期化します。
	Data();

	//! Dataクラスの新しいインスタンスを初期化します。
	//! @param [in] value データの値です。
	Data(const string value);

	//! Dataクラスの新しいインスタンスを初期化します。
	//! @param [in] value データの値です。
	Data(const int value);

	//! Dataクラスの新しいインスタンスを初期化します。
	//! @param [in] value データの値です。
	Data(const bool value);

	//! データが文字列型の場合の値を取得します。
	//! @return データが文字列型の場合の値です。
	const string& string() const;

	//! データが整数型の場合の値を取得します。
	//! @return データが整数型の場合の値です。
	const int& integer() const;

	//! データが真偽値型の場合の値を取得します。
	//! @return データが真偽値型の場合の値です。
	const bool& boolean() const;
};

//! WHERE句に指定する演算子の情報を表します。
class Operator
{
public:
	TokenKind kind = TokenKind::NOT_TOKEN; //!< 演算子の種類を、演算子を記述するトークンの種類で表します。
	int order = 0; //!< 演算子の優先順位です。

	//! Operatorクラスの新しいインスタンスを初期化します。
	Operator();

	//! Operatorクラスの新しいインスタンスを初期化します。
	//! @param [in] kind 演算子の種類を、演算子を記述するトークンの種類で表します。
	//! @param [in] order 演算子の優先順位です。
	Operator(const TokenKind kind, const int order);
};

//! トークンを表します。
class Token
{
public:
	TokenKind kind; //!< トークンの種類です。
	string word; //!< 記録されているトークンの文字列です。記録の必要がなければ空白です。

	//! Tokenクラスの新しいインスタンスを初期化します。
	Token();

	//! Tokenクラスの新しいインスタンスを初期化します。
	//! @param [in] kind トークンの種類です。
	Token(const TokenKind kind);

	//! Tokenクラスの新しいインスタンスを初期化します。
	//! @param [in] kind トークンの種類です。
	//! @param [in] word 記録されているトークンの文字列です。記録の必要がなければ空白です。
	Token(const TokenKind kind, const string word);
};

//! 指定された列の情報です。どのテーブルに所属するかの情報も含みます。
class Column
{
public:
	string tableName; //!< 列が所属するテーブル名です。指定されていない場合は空文字列となります。
	string columnName; //!< 指定された列の列名です。

	//! Columnクラスの新しいインスタンスを初期化します。
	Column();

	//! Columnクラスの新しいインスタンスを初期化します。
	//! @param [in] columnName 指定された列の列名です。
	Column(const string columnName);

	//! Columnクラスの新しいインスタンスを初期化します。
	//! @param [in] tableName 列が所属するテーブル名です。指定されていない場合は空文字列となります。
	//! @param [in] columnName 指定された列の列名です。
	Column(const string tableName, const string columnName);
};

//! WHERE句の条件の式木を表します。
class ExtensionTreeNode
{
public:
	shared_ptr<ExtensionTreeNode> parent;//!< 親となるノードです。根の式木の場合はnullptrとなります。
	shared_ptr<ExtensionTreeNode> left;  //!< 左の子となるノードです。自身が末端の葉となる式木の場合はnullptrとなります。
	Operator middleOperator;             //!< 中置される演算子です。自身が末端のとなる式木の場合の種類はNOT_TOKENとなります。
	shared_ptr<ExtensionTreeNode>right;   //!< 右の子となるノードです。自身が末端の葉となる式木の場合はnullptrとなります。
	bool inParen = false;                //!< 自身がかっこにくるまれているかどうかです。
	int parenOpenBeforeClose = 0;        //!< 木の構築中に0以外となり、自身の左にあり、まだ閉じてないカッコの開始の数となります。
	int signCoefficient = 1;             //!< 自身が葉にあり、マイナス単項演算子がついている場合は-1、それ以外は1となります。
	Column column;                       //!< 列場指定されている場合に、その列を表します。列指定ではない場合はcolumnNameが空文字列となります。
	bool calculated = false;             //!< 式の値を計算中に、計算済みかどうかです。
	Data value;                          //!< 指定された、もしくは計算された値です。

	//! ExtensionTreeNodeクラスの新しいインスタンスを初期化します。
	ExtensionTreeNode();
};

//! 行の情報を入力のテーブルインデックス、列インデックスの形で持ちます。
class ColumnIndex
{
public:
	int table;  //!< 列が入力の何テーブル目の列かです。
	int column; //!< 列が入力のテーブルの何列目かです。

	//! Columnクラスの新しいインスタンスを初期化します。
	ColumnIndex();

	//! Columnクラスの新しいインスタンスを初期化します。
	//! @param [in] table 列が入力の何テーブル目の列かです。
	//! @param [in] column 列が入力のテーブルの何列目かです。
	ColumnIndex(const int table, const int column);
};

class SqlQueryInfo
{
public:
	vector<const string> tableNames; //!< FROM句で指定しているテーブル名です。
	vector<Column> selectColumns; //!< SELECT句に指定された列名です。
	vector<Column> orderByColumns; //!< ORDER句に指定された列名です。
	vector<TokenKind> orders; //!< 同じインデックスのorderByColumnsに対応している、昇順、降順の指定です。
	vector<shared_ptr<ExtensionTreeNode>> whereExtensionNodes; //!< WHEREに指定された木のノードを、木構造とは無関係に格納します。
	shared_ptr<ExtensionTreeNode> whereTopNode; //!< 式木の根となるノードです。
};

//! ファイルに対して実行するSQLを表すクラスです。
class SqlQuery
{
	vector<ifstream> inputTableFiles;                       //!< 読み込む入力ファイルの全てのファイルポインタです。
	ofstream outputFile;                                    //!< 書き込むファイルのファイルポインタです。
	bool found = false;                                     //!< 検索時に見つかったかどうかの結果を一時的に保存します。
	vector<vector<vector<Data>>> inputData;                 //!< 入力データです。
	vector<vector<Data>> outputData;                        //!< 出力データです。
	vector<vector<Data>> allColumnOutputData;               //!< 出力するデータに対応するインデックスを持ち、すべての入力データを保管します。

	const string alpahUnder = "_abcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPQRSTUVWXYZ"; //!< 全てのアルファベットの大文字小文字とアンダーバーです。
	const string alpahNumUnder = "_abcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"; //!< 全ての数字とアルファベットの大文字小文字とアンダーバーです。
	const string signNum = "+-0123456789"; //!< 全ての符号と数字です。
	const string num = "0123456789"; //!< 全ての数字です。
	const string space = " \t\r\n"; //!< 全ての空白文字です。

	// keywordConditionsとsignConditionsは先頭から順に検索されるので、前方一致となる二つの項目は順番に気をつけて登録しなくてはいけません。
	const vector<const Token> keywordConditions;//!< キーワードをトークンとして認識するためのキーワード一覧情報です。
	const vector<const Token> signConditions;//!< 記号をトークンとして認識するための記号一覧情報です。
	const vector<const Operator> operators; //!< 演算子の情報です。

	vector<const Token> tokens; //!< SQLを分割したトークンです。

	SqlQueryInfo queryInfo; //! SQLを解析した結果の情報です。

	vector<vector<Column>> inputColumns; //!< 入力されたCSVの行の情報です。

	string m_outputFileName; //!< outputFileName SQLの実行結果をCSVとして出力するファイル名です。拡張子を含みます。

	//! SQLの文字列からトークンを切り出します。
	//! @param [in] sql トークンに分解する元となるSQLです。
	//! @return 切り出されたトークンです。
	const shared_ptr<const vector<const Token>> GetTokens(const string sql) const;

	//! トークンを解析してSQLの構文で指定された情報を取得します。
	void AnalyzeTokens();

	//! CSVファイルから入力データを読み取ります。
	void ReadCsv();

	//! CSVファイルに出力データを書き込みます。
	void WriteCsv();

	//! ファイルのClose処理を行い、正常に行われたか確認します。
	void CheckClosingFiles();
public:
	//! SqlQueryクラスの新しいインスタンスを初期化します。
	SqlQuery();

	//! カレントディレクトリにあるCSVに対し、簡易的なSQLを実行し、結果をファイルに出力します。
	//! @param [in] sql 実行するSQLです。
	//! @param[in] outputFileName SQLの実行結果をCSVとして出力するファイル名です。拡張子を含みます。
	//! @return 実行した結果の状態です。 
	int Execute(const string sql, const string outputFileName);
};

// 以上ヘッダに相当する部分。

//! Dataクラスの新しいインスタンスを初期化します。
Data::Data() :m_value({ 0 })
{
}

//! Dataクラスの新しいインスタンスを初期化します。
//! @param [in] value データの値です。
Data::Data(const std::string value) : m_value({ 0 })
{
	m_string = value;
}

//! Dataクラスの新しいインスタンスを初期化します。
//! @param [in] value データの値です。
Data::Data(const int value) : type(DataType::INTEGER)
{
	m_value.integer = value;
}

//! Dataクラスの新しいインスタンスを初期化します。
//! @param [in] value データの値です。
Data::Data(const bool value) : type(DataType::BOOLEAN)
{
	m_value.boolean = value;
}


//! データが文字列型の場合の値を取得します。
//! @return データが文字列型の場合の値です。
const string& Data::string() const
{
	return m_string;
}

//! データが整数型の場合の値を取得します。
//! @return データが整数型の場合の値です。
const int& Data::integer() const
{
	return m_value.integer;
}

//! データが真偽値型の場合の値を取得します。
//! @return データが真偽値型の場合の値です。
const bool& Data::boolean() const
{
	return m_value.boolean;
}

//! Operatorクラスの新しいインスタンスを初期化します。
Operator::Operator()
{
}

//! Operatorクラスの新しいインスタンスを初期化します。
//! @param [in] kind 演算子の種類を、演算子を記述するトークンの種類で表します。
//! @param [in] order 演算子の優先順位です。
Operator::Operator(const TokenKind kind, const int order) : kind(kind), order(order)
{
}

//! Tokenクラスの新しいインスタンスを初期化します。
Token::Token() : Token(TokenKind::NOT_TOKEN, "")
{
}

//! Tokenクラスの新しいインスタンスを初期化します。
//! @param [in] kind トークンの種類です。
Token::Token(const TokenKind kind) : Token(kind, "")
{
}

//! Tokenクラスの新しいインスタンスを初期化します。
//! @param [in] kind トークンの種類です。
//! @param [in] word 記録されているトークンの文字列です。記録の必要がなければ空白です。
Token::Token(const TokenKind kind, const string word) :kind(kind)
{
	this->word = word;
}

//! Columnクラスの新しいインスタンスを初期化します。
Column::Column() : Column("", "")
{

}

//! Columnクラスの新しいインスタンスを初期化します。
//! @param [in] columnName 指定された列の列名です。
Column::Column(const string columnName) : Column("", columnName)
{
}

//! Columnクラスの新しいインスタンスを初期化します。
//! @param [in] tableName 列が所属するテーブル名です。指定されていない場合は空文字列となります。
//! @param [in] columnName 指定された列の列名です。
Column::Column(const string tableName, const string columnName)
{
	this->tableName = tableName;
	this->columnName = columnName;
}

//! ExtensionTreeNodeクラスの新しいインスタンスを初期化します。
ExtensionTreeNode::ExtensionTreeNode()
{
}

//! Columnクラスの新しいインスタンスを初期化します。
ColumnIndex::ColumnIndex() : ColumnIndex(0, 0)
{
}

//! Columnクラスの新しいインスタンスを初期化します。
//! @param [in] table 列が入力の何テーブル目の列かです。
//! @param [in] column 列が入力のテーブルの何列目かです。
ColumnIndex::ColumnIndex(const int table, const int column) : table(table), column(column)
{
}

//! 二つの文字列を、大文字小文字を区別せずに比較し、等しいかどうかです。
//! @param [in] str1 比較される一つ目の文字列です。
//! @param [in] str2 比較される二つ目の文字列です。
//! @return 比較した結果、等しいかどうかです。
bool Equali(const string str1, const string str2){
	return
		str1.size() == str2.size() &&
		equal(str1.begin(), str1.end(), str2.begin(),
		[](const char &c1, const char &c2){return toupper(c1) == toupper(c2); });
}

//! SQLの文字列からトークンを切り出します。
//! @param [in] sql トークンに分解する元となるSQLです。
//! @return 切り出されたトークンです。
const shared_ptr<const vector<const Token>> SqlQuery::GetTokens(const string sql) const
{
	auto sqlBackPoint = sql.begin(); // SQLをトークンに分割して読み込む時に戻るポイントを記録しておきます。

	auto sqlCursol = sql.begin(); // SQLをトークンに分割して読み込む時に現在読んでいる文字の場所を表します。

	auto sqlEnd = sql.end(); // sqlのendを指します。

	auto tokens = make_shared<vector<const Token>>(); //読み込んだトークンです。

	// SQLをトークンに分割て読み込みます。
	while (sqlCursol != sqlEnd){

		// 空白を読み飛ばします。
		sqlCursol = find_if(sqlCursol, sqlEnd, [&](char c){return space.find(c) == string::npos; });
		if (sqlCursol == sqlEnd){
			break;
		}

		// 数値リテラルを読み込みます。
		sqlBackPoint = sqlCursol;
		sqlCursol = find_if(sqlCursol, sqlEnd, [&](char c){return num.find(c) == string::npos; });
		if (sqlCursol != sqlBackPoint && (
			alpahUnder.find(*sqlCursol) == string::npos || // 数字の後にすぐに識別子が続くのは紛らわしいので数値リテラルとは扱いません。
			sqlCursol == sqlEnd)){
			tokens->push_back(Token(TokenKind::INT_LITERAL, string(sqlBackPoint, sqlCursol)));
			continue;
		}
		else{
			sqlCursol = sqlBackPoint;
		}

		// 文字列リテラルを読み込みます。
		sqlBackPoint = sqlCursol;

		// 文字列リテラルを開始するシングルクォートを判別し、読み込みます。
		if (*sqlCursol == "\'"[0]){
			++sqlCursol;
			// メトリクス測定ツールのccccはシングルクォートの文字リテラル中のエスケープを認識しないため、文字リテラルを使わないことで回避しています。
			sqlCursol = find_if_not(sqlCursol, sqlEnd, [](char c){return c != "\'"[0]; });
			if (sqlCursol == sqlEnd){
				throw ResultValue::ERR_TOKEN_CANT_READ;
			}
			++sqlCursol;
			tokens->push_back(Token(TokenKind::STRING_LITERAL, string(sqlBackPoint, sqlCursol)));
			continue;
		}

		// キーワードを読み込みます。
		auto keyword = find_if(keywordConditions.begin(), keywordConditions.end(),
			[&](Token keyword){
			auto result =
				mismatch(keyword.word.begin(), keyword.word.end(), sqlCursol,
				[](const char keywordChar, const char sqlChar){return keywordChar == toupper(sqlChar); });

			if (result.first == keyword.word.end() && // キーワードの最後の文字まで同じです。
				result.second != sqlEnd && alpahNumUnder.find(*result.second) == string::npos){ //キーワードに識別子が区切りなしに続いていないかを確認します。 
				sqlCursol = result.second;
				return true;
			}
			else{
				return false;
			}
		});
		if (keyword != keywordConditions.end()){
			tokens->push_back(Token(keyword->kind));
			continue;
		}



		// 記号を読み込みます。
		auto sign = find_if(signConditions.begin(), signConditions.end(),
			[&](Token keyword){
			auto result =
				mismatch(keyword.word.begin(), keyword.word.end(), sqlCursol,
				[](const char keywordChar, const char sqlChar){return keywordChar == toupper(sqlChar); });

			if (result.first == keyword.word.end()){
				sqlCursol = result.second;
				return true;
			}
			else{
				return false;
			}
		});
		if (sign != signConditions.end()){
			tokens->push_back(Token(sign->kind));
			continue;
		}

		// 識別子を読み込みます。
		sqlBackPoint = sqlCursol;
		if (alpahUnder.find(*sqlCursol++) != string::npos){
			sqlCursol = find_if(sqlCursol, sqlEnd, [&](const char c){return alpahNumUnder.find(c) == string::npos; });
			tokens->push_back(Token(TokenKind::IDENTIFIER, string(sqlBackPoint, sqlCursol)));
			continue;
		}

		throw ResultValue::ERR_TOKEN_CANT_READ;
	}
	return tokens;
}

//! トークンを解析してSQLの構文で指定された情報を取得します。
void SqlQuery::AnalyzeTokens()
{
	auto tokenCursol = tokens.begin(); // 現在見ているトークンを指します。

	// SELECT句を読み込みます。
	if (tokenCursol->kind == TokenKind::SELECT){
		++tokenCursol;
	}
	else{
		throw ResultValue::ERR_SQL_SYNTAX;
	}

	if (tokenCursol->kind == TokenKind::ASTERISK){
		++tokenCursol;
	}
	else
	{
		bool first = true; // SELECT句に最初に指定された列名の読み込みかどうかです。
		while (tokenCursol->kind == TokenKind::COMMA || first){
			if (tokenCursol->kind == TokenKind::COMMA){
				++tokenCursol;
			}
			if (tokenCursol->kind == TokenKind::IDENTIFIER){
				// テーブル名が指定されていない場合と仮定して読み込みます。
				queryInfo.selectColumns.push_back(Column(tokenCursol->word));
				++tokenCursol;
				if (tokenCursol->kind == TokenKind::DOT){
					++tokenCursol;
					if (tokenCursol->kind == TokenKind::IDENTIFIER){

						// テーブル名が指定されていることがわかったので読み替えます。
						queryInfo.selectColumns.back() = Column(queryInfo.selectColumns.back().columnName, tokenCursol->word);
						++tokenCursol;
					}
					else{
						throw ResultValue::ERR_SQL_SYNTAX;
					}
				}
			}
			else{
				throw ResultValue::ERR_SQL_SYNTAX;
			}
			first = false;
		}
	}

	// ORDER句とWHERE句を読み込みます。最大各一回ずつ書くことができます。
	bool readOrder = false; // すでにORDER句が読み込み済みかどうかです。
	bool readWhere = false; // すでにWHERE句が読み込み済みかどうかです。
	while (tokenCursol->kind == TokenKind::ORDER || tokenCursol->kind == TokenKind::WHERE){

		// 二度目のORDER句はエラーです。
		if (readOrder && tokenCursol->kind == TokenKind::ORDER){
			throw ResultValue::ERR_SQL_SYNTAX;
		}

		// 二度目のWHERE句はエラーです。
		if (readWhere && tokenCursol->kind == TokenKind::WHERE){
			throw ResultValue::ERR_SQL_SYNTAX;
		}
		// ORDER句を読み込みます。
		if (tokenCursol->kind == TokenKind::ORDER){
			readOrder = true;
			++tokenCursol;
			if (tokenCursol->kind == TokenKind::BY){
				++tokenCursol;
				bool first = true; // ORDER句の最初の列名の読み込みかどうかです。
				while (tokenCursol->kind == TokenKind::COMMA || first){
					if (tokenCursol->kind == TokenKind::COMMA){
						++tokenCursol;
					}
					if (tokenCursol->kind == TokenKind::IDENTIFIER){
						// テーブル名が指定されていない場合と仮定して読み込みます。
						queryInfo.orderByColumns.push_back(Column(tokenCursol->word));
						++tokenCursol;
						if (tokenCursol->kind == TokenKind::DOT){
							++tokenCursol;
							if (tokenCursol->kind == TokenKind::IDENTIFIER){

								// テーブル名が指定されていることがわかったので読み替えます。
								queryInfo.orderByColumns.back() = Column(queryInfo.orderByColumns.back().columnName, tokenCursol->word);
								++tokenCursol;
							}
							else{
								throw ResultValue::ERR_SQL_SYNTAX;
							}
						}

						// 並び替えの昇順、降順を指定します。
						if (tokenCursol->kind == TokenKind::ASC){
							queryInfo.orders.push_back(TokenKind::ASC);
							++tokenCursol;
						}
						else if (tokenCursol->kind == TokenKind::DESC){
							queryInfo.orders.push_back(TokenKind::DESC);
							++tokenCursol;
						}
						else{
							// 指定がない場合は昇順となります。
							queryInfo.orders.push_back(TokenKind::ASC);
						}
					}
					else{
						throw ResultValue::ERR_SQL_SYNTAX;
					}
					first = false;
				}
			}
			else{
				throw ResultValue::ERR_SQL_SYNTAX;
			}
		}

		// WHERE句を読み込みます。
		if (tokenCursol->kind == TokenKind::WHERE){
			readWhere = true;
			++tokenCursol;
			shared_ptr<ExtensionTreeNode> currentNode; // 現在読み込んでいるノードです。
			while (true){
				// オペランドを読み込みます。

				// オペランドのノードを新しく生成します。
				queryInfo.whereExtensionNodes.push_back(make_shared<ExtensionTreeNode>());
				if (currentNode){
					// 現在のノードを右の子にずらし、元の位置に新しいノードを挿入します。
					currentNode->right = queryInfo.whereExtensionNodes.back();
					currentNode->right->parent = currentNode;
					currentNode = currentNode->right;
				}
				else{
					// 最初はカレントノードに新しいノードを入れます。
					currentNode = queryInfo.whereExtensionNodes.back();
				}

				// カッコ開くを読み込みます。
				while (tokenCursol->kind == TokenKind::OPEN_PAREN){
					++currentNode->parenOpenBeforeClose;
					++tokenCursol;
				}

				// オペランドに前置される+か-を読み込みます。
				if (tokenCursol->kind == TokenKind::PLUS || tokenCursol->kind == TokenKind::MINUS){

					// +-を前置するのは列名と数値リテラルのみです。
					if (tokenCursol[1].kind != TokenKind::IDENTIFIER && tokenCursol[1].kind != TokenKind::INT_LITERAL){
						throw ResultValue::ERR_WHERE_OPERAND_TYPE;
					}
					if (tokenCursol->kind == TokenKind::MINUS){
						currentNode->signCoefficient = -1;
					}
					++tokenCursol;
				}

				// 列名、整数リテラル、文字列リテラルのいずれかをオペランドとして読み込みます。
				if (tokenCursol->kind == TokenKind::IDENTIFIER){

					// テーブル名が指定されていない場合と仮定して読み込みます。
					currentNode->column = Column(tokenCursol->word);
					++tokenCursol;
					if (tokenCursol->kind == TokenKind::DOT){
						++tokenCursol;
						if (tokenCursol->kind == TokenKind::IDENTIFIER){

							// テーブル名が指定されていることがわかったので読み替えます。
							currentNode->column = Column(currentNode->column.columnName, tokenCursol->word);
							++tokenCursol;
						}
						else{
							throw ResultValue::ERR_SQL_SYNTAX;
						}
					}
				}
				else if (tokenCursol->kind == TokenKind::INT_LITERAL){
					currentNode->value = Data(stoi(tokenCursol->word));
					++tokenCursol;
				}
				else if (tokenCursol->kind == TokenKind::STRING_LITERAL){
					// 前後のシングルクォートを取り去った文字列をデータとして読み込みます。
					currentNode->value = Data(tokenCursol->word.substr(1, tokenCursol->word.size() - 2));
					++tokenCursol;
				}
				else{
					throw ResultValue::ERR_SQL_SYNTAX;
				}

				// オペランドの右のカッコ閉じるを読み込みます。
				while (tokenCursol->kind == TokenKind::CLOSE_PAREN){
					shared_ptr<ExtensionTreeNode> searchedAncestor = currentNode->parent; // カッコ閉じると対応するカッコ開くを両方含む祖先ノードを探すためのカーソルです。
					while (searchedAncestor){

						// searchedAncestorの左の子に対応するカッコ開くがないかを検索します。
						shared_ptr<ExtensionTreeNode> searched = searchedAncestor; // searchedAncestorの内部からカッコ開くを検索するためのカーソルです。
						while (searched && !searched->parenOpenBeforeClose){
							searched = searched->left;
						}
						if (searched){
							// 対応付けられていないカッコ開くを一つ削除し、ノードがカッコに囲まれていることを記録します。
							--searched->parenOpenBeforeClose;
							searchedAncestor->inParen = true;
							break;
						}
						else{
							searchedAncestor = searchedAncestor->parent;
						}
					}
					++tokenCursol;
				}


				// 演算子(オペレーターを読み込みます。
				auto foundOperator = find_if(operators.begin(), operators.end(), [&](const Operator& op){return op.kind == tokenCursol->kind; }); // 現在読み込んでいる演算子の情報です。

				// 現在見ている演算子の情報を探します。
				if (foundOperator != operators.end()){
					// 見つかった演算子の情報をもとにノードを入れ替えます。
					shared_ptr<ExtensionTreeNode> tmp = currentNode; //ノードを入れ替えるために使う変数です。

					shared_ptr<ExtensionTreeNode> searched = tmp; // 入れ替えるノードを探すためのカーソルです。

					//カッコにくくられていなかった場合に、演算子の優先順位を参考に結合するノードを探します。
					bool first = true; // 演算子の優先順位を検索する最初のループです。
					do{
						if (!first){
							tmp = tmp->parent;
							searched = tmp;
						}
						// 現在の読み込み場所をくくるカッコが開く場所を探します。
						while (searched && !searched->parenOpenBeforeClose){
							searched = searched->left;
						}
						first = false;
					} while (!searched && tmp->parent && (tmp->parent->middleOperator.order <= foundOperator->order || tmp->parent->inParen));

					// 演算子のノードを新しく生成します。
					queryInfo.whereExtensionNodes.push_back(make_shared<ExtensionTreeNode>());
					currentNode = queryInfo.whereExtensionNodes.back();
					currentNode->middleOperator = *foundOperator;

					// 見つかった場所に新しいノードを配置します。これまでその位置にあったノードは左の子となるよう、親ノードと子ノードのポインタをつけかえます。
					currentNode->parent = tmp->parent;
					if (currentNode->parent){
						currentNode->parent->right = currentNode;
					}
					currentNode->left = tmp;
					tmp->parent = currentNode;

					++tokenCursol;
				}
				else{
					// 現在見ている種類が演算子の一覧から見つからなければ、WHERE句は終わります。
					break;
				}
			}

			// 木を根に向かってさかのぼり、根のノードを設定します。
			queryInfo.whereTopNode = currentNode;
			while (queryInfo.whereTopNode->parent){
				queryInfo.whereTopNode = queryInfo.whereTopNode->parent;
			}
		}
	}

	// FROM句を読み込みます。
	if (tokenCursol->kind == TokenKind::FROM){
		++tokenCursol;
	}
	else{
		throw ResultValue::ERR_SQL_SYNTAX;
	}
	bool first = true; // FROM句の最初のテーブル名を読み込み中かどうかです。
	while (tokenCursol != tokens.end() && tokenCursol->kind == TokenKind::COMMA || first){
		if (tokenCursol->kind == TokenKind::COMMA){
			++tokenCursol;
		}
		if (tokenCursol->kind == TokenKind::IDENTIFIER){
			queryInfo.tableNames.push_back(tokenCursol->word);
			++tokenCursol;
		}
		else{
			throw ResultValue::ERR_SQL_SYNTAX;
		}
		first = false;
	}

	// 最後のトークンまで読み込みが進んでいなかったらエラーです。
	if (tokenCursol != tokens.end()){
		throw ResultValue::ERR_SQL_SYNTAX;
	}


}

//! CSVファイルから入力データを読み取ります。
void SqlQuery::ReadCsv()
{
	for (size_t i = 0; i < queryInfo.tableNames.size(); ++i){
		// 入力ファイルを開きます。
		inputTableFiles.push_back(ifstream(queryInfo.tableNames[i] + ".csv"));
		if (!inputTableFiles.back()){
			throw ResultValue::ERR_FILE_OPEN;
		}

		// 入力CSVのヘッダ行を読み込みます。
		inputColumns.push_back(vector<Column>());
		string inputLine; // ファイルから読み込んだ行文字列です。
		if (getline(inputTableFiles.back(), inputLine)){
			auto charactorCursol = inputLine.begin(); // ヘッダ入力行を検索するカーソルです。
			auto lineEnd = inputLine.end(); // ヘッダ入力行のendを指します。

			// 読み込んだ行を最後まで読みます。
			while (charactorCursol != lineEnd){

				// 列名を一つ読みます。
				auto columnStart = charactorCursol; // 現在の列の最初を記録しておきます。
				charactorCursol = find(charactorCursol, lineEnd, ',');
				inputColumns[i].push_back(Column(queryInfo.tableNames[i], string(columnStart, charactorCursol)));

				// 入力行のカンマの分を読み進めます。
				if (charactorCursol != lineEnd){
					++charactorCursol;
				}
			}
		}
		else{
			throw ResultValue::ERR_CSV_SYNTAX;
		}

		// 入力CSVのデータ行を読み込みます。

		inputData.push_back(vector<vector<Data>>());

		while (getline(inputTableFiles.back(), inputLine)){
			inputData[i].push_back(vector<Data>()); // 入力されている一行分のデータです。
			vector<Data> &row = inputData[i].back();

			auto charactorCursol = inputLine.begin(); // データ入力行を検索するカーソルです。
			auto lineEnd = inputLine.end(); // データ入力行のendを指します。

			// 読み込んだ行を最後まで読みます。
			while (charactorCursol != lineEnd){

				// 読み込んだデータを書き込む行のカラムを生成します。
				auto columnStart = charactorCursol; // 現在の列の最初を記録しておきます。
				charactorCursol = find(charactorCursol, lineEnd, ',');

				row.push_back(Data(string(columnStart, charactorCursol)));

				// 入力行のカンマの分を読み進めます。
				if (charactorCursol != lineEnd){
					++charactorCursol;
				}
			}
		}

		// 全てが数値となる列は数値列に変換します。
		for (size_t j = 0; j < inputColumns[i].size(); ++j){

			// 全ての行のある列について、データ文字列から符号と数値以外の文字を探します。
			if (none_of(
				inputData[i].begin(),
				inputData[i].end(),
				[&](const vector<Data> &inputRow){
				return any_of(
					inputRow[j].string().begin(),
					inputRow[j].string().end(),
					[&](const char& c){return signNum.find(c) == string::npos; }); })){

				// 符号と数字以外が見つからない列については、数値列に変換します。
				for (auto& inputRow : inputData[i]){
					inputRow[j] = Data(stoi(inputRow[j].string()));
				}
			}
		}
	}
}

//! CSVファイルに出力データを書き込みます。
void SqlQuery::WriteCsv()
{
	vector<Column> allInputColumns; // 入力に含まれるすべての列の一覧です。

	// 入力ファイルに書いてあったすべての列をallInputColumnsに設定します。
	for (size_t i = 0; i < queryInfo.tableNames.size(); ++i){
		transform(
			inputColumns[i].begin(),
			inputColumns[i].end(),
			back_inserter(allInputColumns),
			[&](const Column& column){return Column(queryInfo.tableNames[i], column.columnName); });
	}

	// SELECT句の列名指定が*だった場合は、入力CSVの列名がすべて選択されます。
	if (queryInfo.selectColumns.empty()){
		copy(allInputColumns.begin(), allInputColumns.end(), back_inserter(queryInfo.selectColumns));
	}

	vector<Column> outputColumns; // 出力するすべての行の情報です。

	// SELECT句で指定された列名が、何個目の入力ファイルの何列目に相当するかを判別します。
	vector<ColumnIndex> selectColumnIndexes; // SELECT句で指定された列の、入力ファイルとしてのインデックスです。

	for (auto &selectColumn : queryInfo.selectColumns){
		found = false;
		for (size_t i = 0; i < queryInfo.tableNames.size(); ++i){
			int j = 0;
			for (auto &inputColumn : inputColumns[i]){
				if (Equali(selectColumn.columnName, inputColumn.columnName) &&
					(selectColumn.tableName.empty() || // テーブル名が設定されている場合のみテーブル名の比較を行います。
					Equali(selectColumn.tableName, inputColumn.tableName))){

					// 既に見つかっているのにもう一つ見つかったらエラーです。
					if (found){
						throw ResultValue::ERR_BAD_COLUMN_NAME;
					}
					found = true;
					// 見つかった値を持つ列のデータを生成します。
					selectColumnIndexes.push_back(ColumnIndex(i, j));
				}
				++j;
			}
		}
		// 一つも見つからなくてもエラーです。
		if (!found){
			throw ResultValue::ERR_BAD_COLUMN_NAME;
		}
	}

	// 出力する列名を設定します。
	transform(
		selectColumnIndexes.begin(),
		selectColumnIndexes.end(),
		back_inserter(outputColumns),
		[&](const ColumnIndex& index){return inputColumns[index.table][index.column]; });

	if (queryInfo.whereTopNode){
		// 既存数値の符号を計算します。
		for (auto &whereExtensionNode : queryInfo.whereExtensionNodes){
			if (whereExtensionNode->middleOperator.kind == TokenKind::NOT_TOKEN &&
				whereExtensionNode->column.columnName.empty() &&
				whereExtensionNode->value.type == DataType::INTEGER){
				whereExtensionNode->value = Data(whereExtensionNode->value.integer() * whereExtensionNode->signCoefficient);
			}
		}
	}

	vector<vector<vector<Data>>::iterator> currentRows; // 入力された各テーブルの、現在出力している行を指すカーソルです。
	transform(
		inputData.begin(),
		inputData.end(),
		back_inserter(currentRows),
		[](vector<vector<Data>>& rows){return rows.begin(); });

	// 出力するデータを設定します。
	while (true){
		outputData.push_back(vector<Data>());
		vector<Data> &row = outputData.back(); // 出力している一行分のデータです。

		// 行の各列のデータを入力から持ってきて設定します。
		transform(
			selectColumnIndexes.begin(),
			selectColumnIndexes.end(),
			back_inserter(row),
			[&](const ColumnIndex& index){return (*currentRows[index.table])[index.column]; });

		allColumnOutputData.push_back(vector<Data>());
		vector<Data> &allColumnsRow = allColumnOutputData.back();// WHEREやORDERのためにすべての情報を含む行。rowとインデックスを共有します。

		// allColumnsRowの列を設定します。
		for (auto &currentRow : currentRows){
			copy(
				currentRow->begin(),
				currentRow->end(),
				back_inserter(allColumnsRow));
		}
		// WHEREの条件となる値を再帰的に計算します。
		if (queryInfo.whereTopNode){
			shared_ptr<ExtensionTreeNode> currentNode = queryInfo.whereTopNode; // 現在見ているノードです。
			while (currentNode){
				// 子ノードの計算が終わってない場合は、まずそちらの計算を行います。
				if (currentNode->left && !currentNode->left->calculated){
					currentNode = currentNode->left;
					continue;
				}
				else if (currentNode->right && !currentNode->right->calculated){
					currentNode = currentNode->right;
					continue;
				}

				// 自ノードの値を計算します。
				switch (currentNode->middleOperator.kind){
				case TokenKind::NOT_TOKEN:
					// ノードにデータが設定されている場合です。

					// データが列名で指定されている場合、今扱っている行のデータを設定します。
					if (!currentNode->column.columnName.empty()){
						found = false;
						for (size_t i = 0; i < allInputColumns.size(); ++i){
							if (Equali(currentNode->column.columnName, allInputColumns[i].columnName) &&
								(currentNode->column.tableName.empty() || // テーブル名が設定されている場合のみテーブル名の比較を行います。
								Equali(currentNode->column.tableName, allInputColumns[i].tableName))){
								// 既に見つかっているのにもう一つ見つかったらエラーです。
								if (found){
									throw ResultValue::ERR_BAD_COLUMN_NAME;
								}
								found = true;
								currentNode->value = allColumnsRow[i];
							}
						}
						// 一つも見つからなくてもエラーです。
						if (!found){
							throw ResultValue::ERR_BAD_COLUMN_NAME;
						}
						;
						// 符号を考慮して値を計算します。
						if (currentNode->value.type == DataType::INTEGER){
							currentNode->value = Data(currentNode->value.integer() * currentNode->signCoefficient);
						}
					}
					break;
				case TokenKind::EQUAL:
				case TokenKind::GREATER_THAN:
				case TokenKind::GREATER_THAN_OR_EQUAL:
				case TokenKind::LESS_THAN:
				case TokenKind::LESS_THAN_OR_EQUAL:
				case TokenKind::NOT_EQUAL:
					// 比較演算子の場合です。

					// 比較できるのは文字列型か整数型で、かつ左右の型が同じ場合です。
					if (currentNode->left->value.type != DataType::INTEGER && currentNode->left->value.type != DataType::STRING ||
						currentNode->left->value.type != currentNode->right->value.type){
						throw ResultValue::ERR_WHERE_OPERAND_TYPE;
					}
					currentNode->value.type = DataType::BOOLEAN;

					// 比較結果を型と演算子によって計算方法を変えて、計算します。
					switch (currentNode->left->value.type){
					case DataType::INTEGER:
						switch (currentNode->middleOperator.kind){
						case TokenKind::EQUAL:
							currentNode->value = Data(currentNode->left->value.integer() == currentNode->right->value.integer());
							break;
						case TokenKind::GREATER_THAN:
							currentNode->value = Data(currentNode->left->value.integer() > currentNode->right->value.integer());
							break;
						case TokenKind::GREATER_THAN_OR_EQUAL:
							currentNode->value = Data(currentNode->left->value.integer() >= currentNode->right->value.integer());
							break;
						case TokenKind::LESS_THAN:
							currentNode->value = Data(currentNode->left->value.integer() < currentNode->right->value.integer());
							break;
						case TokenKind::LESS_THAN_OR_EQUAL:
							currentNode->value = Data(currentNode->left->value.integer() <= currentNode->right->value.integer());
							break;
						case TokenKind::NOT_EQUAL:
							currentNode->value = Data(currentNode->left->value.integer() != currentNode->right->value.integer());
							break;
						}
						break;
					case DataType::STRING:
						switch (currentNode->middleOperator.kind){
						case TokenKind::EQUAL:
							currentNode->value = Data(currentNode->left->value.string() == currentNode->right->value.string());
							break;
						case TokenKind::GREATER_THAN:
							currentNode->value = Data(currentNode->left->value.string() > currentNode->right->value.string());
							break;
						case TokenKind::GREATER_THAN_OR_EQUAL:
							currentNode->value = Data(currentNode->left->value.string() >= currentNode->right->value.string());
							break;
						case TokenKind::LESS_THAN:
							currentNode->value = Data(currentNode->left->value.string() < currentNode->right->value.string());
							break;
						case TokenKind::LESS_THAN_OR_EQUAL:
							currentNode->value = Data(currentNode->left->value.string() <= currentNode->right->value.string());
							break;
						case TokenKind::NOT_EQUAL:
							currentNode->value = Data(currentNode->left->value.string() != currentNode->right->value.string());
							break;
						}
						break;
					}
					break;
				case TokenKind::PLUS:
				case TokenKind::MINUS:
				case TokenKind::ASTERISK:
				case TokenKind::SLASH:
					// 四則演算の場合です。

					// 演算できるのは整数型同士の場合のみです。
					if (currentNode->left->value.type != DataType::INTEGER || currentNode->right->value.type != DataType::INTEGER){
						throw ResultValue::ERR_WHERE_OPERAND_TYPE;
					}
					currentNode->value.type = DataType::INTEGER;

					// 比較結果を演算子によって計算方法を変えて、計算します。
					switch (currentNode->middleOperator.kind){
					case TokenKind::PLUS:
						currentNode->value = Data(currentNode->left->value.integer() + currentNode->right->value.integer());
						break;
					case TokenKind::MINUS:
						currentNode->value = Data(currentNode->left->value.integer() - currentNode->right->value.integer());
						break;
					case TokenKind::ASTERISK:
						currentNode->value = Data(currentNode->left->value.integer() * currentNode->right->value.integer());
						break;
					case TokenKind::SLASH:
						currentNode->value = Data(currentNode->left->value.integer() / currentNode->right->value.integer());
						break;
					}
					break;
				case TokenKind::AND:
				case TokenKind::OR:
					// 論理演算の場合です。

					// 演算できるのは真偽値型同士の場合のみです。
					if (currentNode->left->value.type != DataType::BOOLEAN || currentNode->right->value.type != DataType::BOOLEAN){
						throw ResultValue::ERR_WHERE_OPERAND_TYPE;
					}
					currentNode->value.type = DataType::BOOLEAN;

					// 比較結果を演算子によって計算方法を変えて、計算します。
					switch (currentNode->middleOperator.kind){
					case TokenKind::AND:
						currentNode->value = Data(currentNode->left->value.boolean() && currentNode->right->value.boolean());
						break;
					case TokenKind::OR:
						currentNode->value = Data(currentNode->left->value.boolean() || currentNode->right->value.boolean());
						break;
					}
				}
				currentNode->calculated = true;

				// 自身の計算が終わった後は親の計算に戻ります。
				currentNode = currentNode->parent;
			}

			// 条件に合わない行は出力から削除します。
			if (!queryInfo.whereTopNode->value.boolean()){
				allColumnOutputData.pop_back();
				outputData.pop_back();
			}
			// WHERE条件の計算結果をリセットします。
			for (auto &whereExtensionNode : queryInfo.whereExtensionNodes){
				whereExtensionNode->calculated = false;
			}
		}

		// 各テーブルの行のすべての組み合わせを出力します。

		// 最後のテーブルのカレント行をインクリメントします。
		++currentRows[queryInfo.tableNames.size() - 1];

		// 最後のテーブルが最終行になっていた場合は先頭に戻し、順に前のテーブルのカレント行をインクリメントします。
		for (int i = queryInfo.tableNames.size() - 1; currentRows[i] == inputData[i].end() && 0 < i; --i){
			++currentRows[i - 1];
			currentRows[i] = inputData[i].begin();
		}

		// 最初のテーブルが最後の行を超えたなら出力行の生成は終わりです。
		if (currentRows[0] == inputData[0].end()){
			break;
		}
	}

	// ORDER句による並び替えの処理を行います。
	if (!queryInfo.orderByColumns.empty()){
		// ORDER句で指定されている列が、全ての入力行の中のどの行なのかを計算します。
		vector<int> orderByColumnIndexes; // ORDER句で指定された列の、すべての行の中でのインデックスです。

		for (auto &orderByColumn : queryInfo.orderByColumns){
			found = false;
			for (size_t i = 0; i < allInputColumns.size(); ++i){
				if (Equali(orderByColumn.columnName, allInputColumns[i].columnName) &&
					(orderByColumn.tableName.empty() || // テーブル名が設定されている場合のみテーブル名の比較を行います。
					Equali(orderByColumn.tableName, allInputColumns[i].tableName))){
					// 既に見つかっているのにもう一つ見つかったらエラーです。
					if (found){
						throw ResultValue::ERR_BAD_COLUMN_NAME;
					}
					found = true;
					orderByColumnIndexes.push_back(i);
				}
			}
			// 一つも見つからなくてもエラーです。
			if (!found){
				throw ResultValue::ERR_BAD_COLUMN_NAME;
			}
		}

		// outputDataとallColumnOutputDataのソートを一緒に行います。簡便のため凝ったソートは使わず、選択ソートを利用します。
		for (size_t i = 0; i < outputData.size(); ++i){
			int minIndex = i; // 現在までで最小の行のインデックスです。
			for (size_t j = i + 1; j < outputData.size(); ++j){
				bool jLessThanMin = false; // インデックスがjの値が、minIndexの値より小さいかどうかです。
				for (size_t k = 0; k < orderByColumnIndexes.size(); ++k){
					const Data &mData = allColumnOutputData[minIndex][orderByColumnIndexes[k]]; // インデックスがminIndexのデータです。
					const Data &jData = allColumnOutputData[j][orderByColumnIndexes[k]]; // インデックスがjのデータです。
					int cmp = 0; // 比較結果です。等しければ0、インデックスjの行が大きければプラス、インデックスminIndexの行が大きければマイナスとなります。
					switch (mData.type)
					{
					case DataType::INTEGER:
						cmp = jData.integer() - mData.integer();
						break;
					case DataType::STRING:
						cmp = strcmp(jData.string().c_str(), mData.string().c_str());
						break;
					}

					// 降順ならcmpの大小を入れ替えます。
					if (queryInfo.orders[k] == TokenKind::DESC){
						cmp *= -1;
					}
					if (cmp < 0){
						jLessThanMin = true;
						break;
					}
					else if (0 < cmp){
						break;
					}
				}
				if (jLessThanMin){
					minIndex = j;
				}
			}
			vector<Data> tmp = outputData[minIndex];
			outputData[minIndex] = outputData[i];
			outputData[i] = tmp;

			tmp = allColumnOutputData[minIndex];
			allColumnOutputData[minIndex] = allColumnOutputData[i];
			allColumnOutputData[i] = tmp;
		}
	}

	// 出力ファイルを開きます。
	outputFile = ofstream(m_outputFileName);
	if (outputFile.bad()){
		throw ResultValue::ERR_FILE_OPEN;
	}

	// 出力ファイルに列名を出力します。
	for (size_t i = 0; i < queryInfo.selectColumns.size(); ++i){
		outputFile << outputColumns[i].columnName;
		if (i < queryInfo.selectColumns.size() - 1){
			outputFile << ",";
		}
		else{
			outputFile << "\n";
		}
	}

	// 出力ファイルにデータを出力します。
	for (auto& outputRow : outputData){
		size_t i = 0;
		for (const auto &column : outputRow){
			switch (column.type){
			case DataType::INTEGER:
				outputFile << column.integer();
				break;
			case DataType::STRING:
				outputFile << column.string();
				break;
			}
			if (i++ < queryInfo.selectColumns.size() - 1){
				outputFile << ",";
			}
			else{
				outputFile << "\n";
			}
		}
	}
	if (outputFile.bad()){
		throw ResultValue::ERR_FILE_WRITE;
	}
}

//! ファイルのClose処理を行い、正常に行われたか確認します。
void SqlQuery::CheckClosingFiles()
{
	// ファイルリソースを解放します。
	for (auto &inputTableFile : inputTableFiles){
		if (inputTableFile){
			inputTableFile.close();
			if (inputTableFile.bad()){
				throw ResultValue::ERR_FILE_CLOSE;
			}
		}
	}
	if (outputFile){
		outputFile.close();
		if (outputFile.bad()){
			throw ResultValue::ERR_FILE_CLOSE;
		}
	}
}

//! SqlQueryクラスの新しいインスタンスを初期化します。
SqlQuery::SqlQuery() :
	keywordConditions({
		{ TokenKind::AND, "AND" },
		{ TokenKind::ASC, "ASC" },
		{ TokenKind::BY, "BY" },
		{ TokenKind::DESC, "DESC" },
		{ TokenKind::FROM, "FROM" },
		{ TokenKind::ORDER, "ORDER" },
		{ TokenKind::OR, "OR" },
		{ TokenKind::SELECT, "SELECT" },
		{ TokenKind::WHERE, "WHERE" }}),
	signConditions({
		{ TokenKind::GREATER_THAN_OR_EQUAL, ">=" },
		{ TokenKind::LESS_THAN_OR_EQUAL, "<=" },
		{ TokenKind::NOT_EQUAL, "<>" },
		{ TokenKind::ASTERISK, "*" },
		{ TokenKind::COMMA, "," },
		{ TokenKind::CLOSE_PAREN, ")" },
		{ TokenKind::DOT, "." },
		{ TokenKind::EQUAL, "=" },
		{ TokenKind::GREATER_THAN, ">" },
		{ TokenKind::LESS_THAN, "<" },
		{ TokenKind::MINUS, "-" },
		{ TokenKind::OPEN_PAREN, "(" },
		{ TokenKind::PLUS, "+" },
		{ TokenKind::SLASH, "/" } }),
	operators({
		{ TokenKind::ASTERISK, 1 },
		{ TokenKind::SLASH, 1 },
		{ TokenKind::PLUS, 2 },
		{ TokenKind::MINUS, 2 },
		{ TokenKind::EQUAL, 3 },
		{ TokenKind::GREATER_THAN, 3 },
		{ TokenKind::GREATER_THAN_OR_EQUAL, 3 },
		{ TokenKind::LESS_THAN, 3 },
		{ TokenKind::LESS_THAN_OR_EQUAL, 3 },
		{ TokenKind::NOT_EQUAL, 3 },
		{ TokenKind::AND, 4 },
		{ TokenKind::OR, 5 }}){}

//! カレントディレクトリにあるCSVに対し、簡易的なSQLを実行し、結果をファイルに出力します。
//! @param [in] sql 実行するSQLです。
//! @param[in] outputFileName SQLの実行結果をCSVとして出力するファイル名です。拡張子を含みます。
//! @return 実行した結果の状態です。 
int SqlQuery::Execute(const string sql, const string outputFileName)
{
	m_outputFileName = outputFileName;
	try
	{
		tokens = *GetTokens(sql);
		AnalyzeTokens();
		ReadCsv();
		WriteCsv();
		CheckClosingFiles();

		return static_cast<int>(ResultValue::OK);
	}
	catch (ResultValue error) // 発生したエラーの種類です。
	{
		return static_cast<int>(error);
	}
}

//! カレントディレクトリにあるCSVに対し、簡易的なSQLを実行し、結果をファイルに出力します。
//! @param [in] sql 実行するSQLです。
//! @param[in] outputFileName SQLの実行結果をCSVとして出力するファイル名です。拡張子を含みます。
//! @return 実行した結果の状態です。
//! @retval OK=0                      問題なく終了しました。
//! @retval ERR_FILE_OPEN=1           ファイルを開くことに失敗しました。
//! @retval ERR_FILE_WRITE=2          ファイルに書き込みを行うことに失敗しました。
//! @retval ERR_FILE_CLOSE=3          ファイルを閉じることに失敗しました。
//! @retval ERR_TOKEN_CANT_READ=4     トークン解析に失敗しました。
//! @retval ERR_SQL_SYNTAX=5          SQLの構文解析が失敗しました。
//! @retval ERR_BAD_COLUMN_NAME=6     テーブル指定を含む列名が適切ではありません。
//! @retval ERR_WHERE_OPERAND_TYPE=7  演算の左右の型が適切ではありません。
//! @retval ERR_CSV_SYNTAX=8          CSVの構文解析が失敗しました。
//! @retval ERR_MEMORY_ALLOCATE=9     メモリの取得に失敗しました。
//! @retval ERR_MEMORY_OVER=10        用意したメモリ領域の上限を超えました。
//! @details 
//! 参照するテーブルは、テーブル名.csvの形で作成します。                                                     @n
//! 一行目はヘッダ行で、その行に列名を書きます。                                                             @n
//! 前後のスペース読み飛ばしやダブルクォーテーションでくくるなどの機能はありません。                         @n
//! 列の型の定義はできないので、列のすべてのデータの値が数値として解釈できる列のデータを整数として扱います。 @n
//! 実行するSQLで使える機能を以下に例としてあげます。                                                        @n
//! 例1:                                                                                                     @n
//! SELECT *                                                                                                 @n
//! FROM USERS                                                                                               @n
//!                                                                                                          @n
//! 例2: 大文字小文字は区別しません。                                                                        @n
//! select *                                                                                                 @n
//! from users                                                                                               @n
//!                                                                                                          @n
//! 例3: 列の指定ができます。                                                                                @n
//! SELECT Id, Name                                                                                          @n
//! FROM USERS                                                                                               @n
//!                                                                                                          @n
//! 例4: テーブル名を指定して列の指定ができます。                                                            @n
//! SELECT USERS.Id                                                                                          @n
//! FROM USERS                                                                                               @n
//!                                                                                                          @n
//! 例5: ORDER句が使えます。                                                                                 @n
//! SELECT *                                                                                                 @n
//! ORDER BY NAME                                                                                            @n
//! FROM USERS                                                                                               @n
//!                                                                                                          @n
//! 例6: ORDER句に複数列や昇順、降順の指定ができます。                                                       @n
//! SELECT *                                                                                                 @n
//! ORDER BY AGE DESC, Name ASC                                                                              @n
//!                                                                                                          @n
//! 例7: WHERE句が使えます。                                                                                 @n
//! SELECT *                                                                                                 @n
//! WHERE AGE >= 20                                                                                          @n
//! FROM USERS                                                                                               @n
//!                                                                                                          @n
//! 例8: WHERE句では文字列の比較も使えます。                                                                 @n
//! SELECT *                                                                                                 @n
//! WHERE NAME >= 'N'                                                                                        @n
//! FROM USERS                                                                                               @n
//!                                                                                                          @n
//! 例9: WHERE句には四則演算、カッコ、AND、ORなどを含む複雑な式が利用できます。                              @n
//! SELECT *                                                                                                 @n
//! WHERE AGE >= 20 AND (AGE <= 40 || WEIGHT < 100)                                                          @n
//! FROM USERS                                                                                               @n
//!                                                                                                          @n
//! 例10: FROM句に複数のテーブルが指定できます。その場合はクロスで結合します。                               @n
//! SELECT *                                                                                                 @n
//! FROM USERS, CHILDREN                                                                                     @n
//!                                                                                                          @n
//! 例11: WHEREで条件をつけることにより、テーブルの結合ができます。                                          @n
//! SELECT USERS.NAME, CHILDREN.NAME                                                                         @n
//! WHERE USERS.ID = CHILDREN.PARENTID                                                                       @n
//! FROM USERS, CHILDREN                                                                                     @n
int ExecuteSQL(const string sql, const string outputFileName)
{
	return SqlQuery().Execute(sql, outputFileName);
}