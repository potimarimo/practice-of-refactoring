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

class InputTable;
//! 指定された列の情報です。どのテーブルに所属するかの情報も含みます。
class Column
{
public:
	string tableName; //!< 列が所属するテーブル名です。指定されていない場合は空文字列となります。
	string columnName; //!< 指定された列の列名です。
	int allColumnsIndex; //!< 全てのテーブルのすべての列の中で、この列が何番目かです。
	string outputName; //!< この列を出力する時の表示名です。

	//! Columnクラスの新しいインスタンスを初期化します。
	Column();

	//! Columnクラスの新しいインスタンスを初期化します。
	//! @param [in] columnName 指定された列の列名です。
	Column(const string columnName);

	//! Columnクラスの新しいインスタンスを初期化します。
	//! @param [in] tableName 列が所属するテーブル名です。指定されていない場合は空文字列となります。
	//! @param [in] columnName 指定された列の列名です。
	Column(const string tableName, const string columnName);

	//! データの検索に利用するため、全てのテーブルの列の情報を登録します。
	//! @param [in] queryInfo SQLに記述された情報です。
	//! @param [in] inputTables ファイルから読み取ったデータです。
	void Column::SetAllColumns(const vector<const InputTable> &inputTables);
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

//! SqlQueryの構文情報を扱うクラスです。
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

//! CSVとして入力されたファイルの内容を表します。
class InputTable
{
	const string signNum = "+-0123456789"; //!< 全ての符号と数字です。

	const shared_ptr<const vector<const Column>> m_columns; //!< 列の情報です。
	const shared_ptr<vector<const vector<const Data>>> m_data; //! データです。

	//! 全てが数値となる列は数値列に変換します。
	void InputTable::InitializeIntegerColumn();
public:
	//! InputTableクラスの新しいインスタンスを初期化します。
	//! @param [in] columns 読み込んだヘッダ情報です。
	//! @param [in] data 読み込んだデータです。
	InputTable(const shared_ptr<const vector<const Column>> columns, const shared_ptr<vector<const vector<const Data>>> data);

	//! 列の情報を取得します。
	const shared_ptr<const vector<const Column>> columns() const;

	//! データを取得します。
	const shared_ptr<vector<const vector<const Data>>> data() const;
};

class TokenReader
{
protected:
	const string alpahUnder = "_abcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPQRSTUVWXYZ"; //!< 全てのアルファベットの大文字小文字とアンダーバーです。
	const string alpahNumUnder = "_abcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"; //!< 全ての数字とアルファベットの大文字小文字とアンダーバーです。
	const string num = "0123456789"; //!< 全ての数字です。

	//! 実際にトークンを読み込ます。
	//! @param [in] cursol 読み込み開始位置です。
	//! @param [in] end SQL全体の終了位置です。
	//! @return 切り出されたトークンです。読み込みが失敗した場合はnullptrを返します。
	virtual const shared_ptr<const Token> ReadCore(string::const_iterator &cursol, const string::const_iterator& end) const = 0;
public:
	//! トークンを読み込みます。
	//! @param [in] cursol 読み込み開始位置です。
	//! @param [in] end SQL全体の終了位置です。
	//! @return 切り出されたトークンです。読み込みが失敗した場合はnullptrを返します。
	const shared_ptr<const Token> Read(string::const_iterator &cursol, const string::const_iterator& end) const;
};

//! 数値リテラルトークンを読み込む機能を提供します。
class IntLiteralReader : public TokenReader
{
protected:
	//! 実際にトークンを読み込ます。
	//! @param [in] cursol 読み込み開始位置です。
	//! @param [in] end SQL全体の終了位置です。
	//! @return 切り出されたトークンです。読み込みが失敗した場合はnullptrを返します。
	const shared_ptr<const Token> ReadCore(string::const_iterator &cursol, const string::const_iterator& end) const override;
};

//! 文字列リテラルトークンを読み込む機能を提供します。
class StringLiteralReader : public TokenReader
{
protected:
	//! 実際にトークンを読み込みます。
	//! @param [in] cursol 読み込み開始位置です。
	//! @param [in] end SQL全体の終了位置です。
	//! @return 切り出されたトークンです。読み込みが失敗した場合はnullptrを返します。
	const shared_ptr<const Token> ReadCore(string::const_iterator &cursol, const string::const_iterator& end) const override;
};

//! キーワードトークンを読み込む機能を提供します。
class KeywordReader : public TokenReader
{
protected:
	Token keyword; //!< 読み込むキーワードトークンと等しいトークンです。

	//! 実際にトークンを読み込みます。
	//! @param [in] cursol 読み込み開始位置です。
	//! @param [in] end SQL全体の終了位置です。
	//! @return 切り出されたトークンです。読み込みが失敗した場合はnullptrを返します。
	const shared_ptr<const Token> ReadCore(string::const_iterator &cursol, const string::const_iterator& end) const override;

	//! キーワードの次の文字のチェックを行います。
	//! @param [in] next チェック対象となる次の文字のイテレータです。
	//! @param [in] next endイテレータです。
	virtual const bool CheckNextChar(const string::const_iterator& next, const string::const_iterator& end) const;
public:
	//! KeywordReaderクラスの新しいインスタンスを初期化します。
	//! @param [in] kind トークンの種類です。
	//! @param [in] word キーワードの文字列です。
	KeywordReader(const TokenKind kind, const string word);
};

//! 記号トークンを読み込む機能を提供します。
class SignReader : public KeywordReader
{
protected:
	//! キーワードの次の文字のチェックを行います。
	//! @param [in] next チェック対象となる次の文字のイテレータです。
	//! @param [in] next endイテレータです。
	const bool CheckNextChar(const string::const_iterator& next, const string::const_iterator& end) const override;

public:
	//! KeywordReaderクラスの新しいインスタンスを初期化します。
	//! @param [in] kind トークンの種類です。
	//! @param [in] word キーワードの文字列です。
	SignReader(const TokenKind kind, const string word);
};

//! 識別子トークンを読み込む機能を提供します。
class IdentifierReader : public TokenReader
{
protected:
	//! 実際にトークンを読み込みます。
	//! @param [in] cursol 読み込み開始位置です。
	//! @param [in] end SQL全体の終了位置です。
	//! @return 切り出されたトークンです。読み込みが失敗した場合はnullptrを返します。
	const shared_ptr<const Token> ReadCore(string::const_iterator &cursol, const string::const_iterator& end) const override;
};

//! 出力するデータを管理します。
class OutputData
{
	SqlQueryInfo queryInfo; //!< SQLに記述された内容です。
public:

	//! OutputDataクラスの新しいインスタンスを初期化します。
	//! @param [in] queryInfo SQLの情報です。
	OutputData(const SqlQueryInfo queryInfo);

	//! CSVファイルに出力データを書き込みます。
	//! @param [in] outputFileName 結果を出力するファイルのファイル名です。
	//! @param [in] inputTables ファイルから読み取ったデータです。
	void WriteCsv(const string outputFileName, const vector<const InputTable> &inputTables);
};

//! SqlQueryのCsvに対する入出力を扱います。
class Csv
{
	const shared_ptr<const SqlQueryInfo> queryInfo; //!< SQLに記述された内容です。

	//! ファイルストリームからカンマ区切りの一行を読み込みます。
	//! @param [in] inputFile データを読み込むファイルストリームです。
	//! @return ファイルから読み込んだ一行分のデータです。
	const shared_ptr<const vector<const string>> ReadLineData(ifstream &inputFile) const;

	//! 入力ファイルを開きます。
	//! @param [in] filePath 開くファイルのファイルパスです。
	//! @return 入力ファイルを扱うストリームです。
	ifstream OpenInputFile(const string filePath) const;

	//! 入力ファイルを閉じます。
	//! @param [in] inputFile 入力ファイルを扱うストリームです。
	void CloseInputFile(ifstream &inputFile) const;

	//! 入力CSVのヘッダ行を読み込みます。
	//! @param [in] inputFile 入力ファイルを扱うストリームです。開いた後何も読み込んでいません。
	//! @param [in] tableName SQLで指定されたテーブル名です。
	//! @return ファイルから読み取ったヘッダ情報です。
	const shared_ptr<const vector<const Column>> ReadHeader(ifstream &inputFile, const string tableName) const;

	//! 入力CSVのデータ行を読み込みます。
	//! @param [in] inputFile 入力ファイルを扱うストリームです。すでにヘッダのみを読み込んだ後です。
	//! @return ファイルから読み取ったデータです。
	const shared_ptr<vector<const vector<const Data>>> ReadData(ifstream &inputFile) const;
public:

	//! Csvクラスの新しいインスタンスを初期化します。
	//! @param [in] queryInfo SQLに記述された内容です。
	Csv(const shared_ptr<const SqlQueryInfo> queryInfo);

	//! CSVファイルから入力データを読み取ります。
	//! @return ファイルから読み取ったデータです。
	const shared_ptr<const vector<const InputTable>> ReadCsv() const;

	//! CSVファイルに出力データを書き込みます。
	//! @param [in] outputFileName 結果を出力するファイルのファイル名です。
	//! @param [in] inputTables ファイルから読み取ったデータです。
	void WriteCsv(const string outputFileName, const vector<const InputTable> &inputTables) const;
};

//! ファイルに対して実行するSQLを表すクラスです。
class SqlQuery
{
	const string space = " \t\r\n"; //!< 全ての空白文字です。

	const vector<const shared_ptr<const TokenReader>> tokenReaders; //!< トークンの読み込みロジックの集合です。
	const vector<const Operator> operators; //!< 演算子の情報です。

	shared_ptr<Csv> csv; //!< CSV操作を管理します。

	//! SQLの文字列からトークンを切り出します。
	//! @param [in] sql トークンに分解する元となるSQLです。
	//! @return 切り出されたトークンです。
	const shared_ptr<const vector<const Token>> GetTokens(const string sql) const;

	//! トークンを解析してSQLの構文で指定された情報を取得します。
	//! @param [in] tokens 解析の対象となるトークンです。
	//! @return 解析した結果の情報です。
	const shared_ptr<const SqlQueryInfo> AnalyzeTokens(const vector<const Token> &tokens) const;
public:
	//! SqlQueryクラスの新しいインスタンスを初期化します。
	//! @param [in] sql 実行するSQLです。
	SqlQuery(const string sql);

	//! カレントディレクトリにあるCSVに対し、簡易的なSQLを実行し、結果をファイルに出力します。
	//! @param[in] outputFileName SQLの実行結果をCSVとして出力するファイル名です。拡張子を含みます。
	void Execute(const string outputFileName);
};

// 以上ヘッダに相当する部分。

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

//! データの検索に利用するため、全てのテーブルの列の情報を登録します。
//! @param [in] queryInfo SQLに記述された情報です。
//! @param [in] inputTables ファイルから読み取ったデータです。
void Column::SetAllColumns(const vector<const InputTable> &inputTables)
{
	bool found = false;
	int i = 0;
	for (auto &inputTable : inputTables){
		for (auto &inputColumn : *inputTable.columns()){
			if (Equali(columnName, inputColumn.columnName) &&
				(tableName.empty() || // テーブル名が設定されている場合のみテーブル名の比較を行います。
				Equali(tableName, inputColumn.tableName))){

				// 既に見つかっているのにもう一つ見つかったらエラーです。
				if (found){
					throw ResultValue::ERR_BAD_COLUMN_NAME;
				}
				found = true;
				// 見つかった値を持つ列のデータを生成します。
				allColumnsIndex = i;
				outputName = inputColumn.columnName;
			}
			++i;
		}
	}
	// 一つも見つからなくてもエラーです。
	if (!found){
		throw ResultValue::ERR_BAD_COLUMN_NAME;
	}
}

//! ExtensionTreeNodeクラスの新しいインスタンスを初期化します。
ExtensionTreeNode::ExtensionTreeNode()
{
}

//! 全てが数値となる列は数値列に変換します。
void InputTable::InitializeIntegerColumn()
{
	for (size_t i = 0; i < columns()->size(); ++i){

		// 全ての行のある列について、データ文字列から符号と数値以外の文字を探します。
		if (none_of(
			data()->begin(),
			data()->end(),
			[&](const vector<const Data> &inputRow){
			return
				inputRow[i].type == DataType::STRING &&
				any_of(
				inputRow[i].string().begin(),
				inputRow[i].string().end(),
				[&](const char& c){return signNum.find(c) == string::npos; }); })){

			// 符号と数字以外が見つからない列については、数値列に変換します。
			for (auto& inputRow : *data()){
				inputRow[i] = Data(stoi(inputRow[i].string()));
			}
		}
	}
}

//! InputTableクラスの新しいインスタンスを初期化します。
//! @param [in] columns 読み込んだヘッダ情報です。
//! @param [in] data 読み込んだデータです。
InputTable::InputTable(const shared_ptr<const vector<const Column>> columns, const shared_ptr<vector<const vector<const Data>>> data) : m_columns(columns), m_data(data)
{
	InitializeIntegerColumn();
}

//! 列の情報を取得します。
const shared_ptr<const vector<const Column>> InputTable::columns() const
{
	return m_columns;
}

//! データを取得します。
const shared_ptr<vector<const vector<const Data>>> InputTable::data() const
{
	return m_data;
}

//! トークンを読み込みます。
//! @param [in] cursol 読み込み開始位置です。
//! @param [in] end SQL全体の終了位置です。
//! @return 切り出されたトークンです。読み込みが失敗した場合はnullptrを返します。
const shared_ptr<const Token> TokenReader::Read(string::const_iterator &cursol, const string::const_iterator &end) const
{
	auto backPoint = cursol;
	auto token = ReadCore(cursol, end);
	if (!token){
		cursol = backPoint;
	}
	return token;
}

//! 実際にトークンを読み込みます。
//! @param [in] cursol 読み込み開始位置です。
//! @param [in] end SQL全体の終了位置です。
//! @return 切り出されたトークンです。読み込みが失敗した場合はnullptrを返します。
const shared_ptr<const Token> IntLiteralReader::ReadCore(string::const_iterator &cursol, const string::const_iterator &end) const
{
	auto start = cursol;
	cursol = find_if(cursol, end, [&](char c){return num.find(c) == string::npos; });
	if (start != cursol && (
		alpahUnder.find(*cursol) == string::npos || // 数字の後にすぐに識別子が続くのは紛らわしいので数値リテラルとは扱いません。
		cursol == end)){
		return make_shared<Token>(TokenKind::INT_LITERAL, string(start, cursol));
	}
	else{
		return nullptr;
	}
}

//! 実際にトークンを読み込みます。
//! @param [in] cursol 読み込み開始位置です。
//! @param [in] end SQL全体の終了位置です。
//! @return 切り出されたトークンです。読み込みが失敗した場合はnullptrを返します。
const shared_ptr<const Token> StringLiteralReader::ReadCore(string::const_iterator &cursol, const string::const_iterator &end) const
{
	auto start = cursol;
	// 文字列リテラルを開始するシングルクォートを判別し、読み込みます。
	if (*cursol == "\'"[0]){
		++cursol;
		// メトリクス測定ツールのccccはシングルクォートの文字リテラル中のエスケープを認識しないため、文字リテラルを使わないことで回避しています。
		cursol = find_if_not(cursol, end, [](char c){return c != "\'"[0]; });
		if (cursol == end){
			throw ResultValue::ERR_TOKEN_CANT_READ;
		}
		++cursol;
		return make_shared<Token>(TokenKind::STRING_LITERAL, string(start, cursol));
	}
	else{
		return nullptr;
	}
}

//! 実際にトークンを読み込みます。
//! @param [in] cursol 読み込み開始位置です。
//! @param [in] end SQL全体の終了位置です。
//! @return 切り出されたトークンです。読み込みが失敗した場合はnullptrを返します。
const shared_ptr<const Token> KeywordReader::ReadCore(string::const_iterator &cursol, const string::const_iterator &end) const
{
	auto result =
		mismatch(keyword.word.begin(), keyword.word.end(), cursol,
		[](const char keywordChar, const char sqlChar){return keywordChar == toupper(sqlChar); });

	if (result.first == keyword.word.end() && // キーワードの最後の文字まで同じです。
		CheckNextChar(result.second, end)){ 
		cursol = result.second;
		return make_shared<Token>(keyword);
	}
	else{
		return nullptr;
	}
}

//! KeywordReaderクラスの新しいインスタンスを初期化します。
//! @param [in] kind トークンの種類です。
//! @param [in] word キーワードの文字列です。
KeywordReader::KeywordReader(const TokenKind kind, const string word) : keyword(Token(kind, word)){}

//! キーワードの次の文字のチェックを行います。
//! @param [in] next チェック対象となる次の文字のイテレータです。
//! @param [in] next endイテレータです。
const bool KeywordReader::CheckNextChar(const string::const_iterator& next, const string::const_iterator& end) const
{
	//キーワードに識別子が区切りなしに続いていないかを確認します。 
	return next != end && alpahNumUnder.find(*next) == string::npos;
}

//! KeywordReaderクラスの新しいインスタンスを初期化します。
//! @param [in] kind トークンの種類です。
//! @param [in] word キーワードの文字列です。
SignReader::SignReader(const TokenKind kind, const string word) : KeywordReader(kind, word){}

//! キーワードの次の文字のチェックを行います。
//! @param [in] next チェック対象となる次の文字のイテレータです。
//! @param [in] next endイテレータです。
const bool SignReader::CheckNextChar(const string::const_iterator& next, const string::const_iterator& end) const
{
	// 次の文字はチェックせずに必ずOKとなります。
	return true;
}

//! 実際にトークンを読み込みます。
//! @param [in] cursol 読み込み開始位置です。
//! @param [in] end SQL全体の終了位置です。
//! @return 切り出されたトークンです。読み込みが失敗した場合はnullptrを返します。
const shared_ptr<const Token> IdentifierReader::ReadCore(string::const_iterator &cursol, const string::const_iterator &end) const
{
	auto start = cursol;
	if (alpahUnder.find(*cursol++) != string::npos){
		cursol = find_if(cursol, end, [&](const char c){return alpahNumUnder.find(c) == string::npos; });
		return make_shared<Token>(TokenKind::IDENTIFIER, string(start, cursol));
	}
	else{
		return nullptr;
	}
}

//! OutputDataクラスの新しいインスタンスを初期化します。
//! @param [in] queryInfo SQLの情報です。
OutputData::OutputData(const SqlQueryInfo queryInfo) : queryInfo(queryInfo)
{
}

//! CSVファイルに出力データを書き込みます。
//! @param [in] outputFileName 結果を出力するファイルのファイル名です。
//! @param [in] inputTables ファイルから読み取ったデータです。
void OutputData::WriteCsv(const string outputFileName, const vector<const InputTable> &inputTables)
{
	ofstream outputFile; // 書き込むファイルのファイルポインタです。

	vector<Column> allInputColumns; // 入力に含まれるすべての列の一覧です。

	vector<vector<Data>> outputData; // 出力データです。
	vector<vector<Data>> allColumnOutputData; // 出力するデータに対応するインデックスを持ち、すべての入力データを保管します。

	// 入力ファイルに書いてあったすべての列をallInputColumnsに設定します。
	for (size_t i = 0; i < queryInfo.tableNames.size(); ++i){
		transform(
			inputTables[i].columns()->begin(),
			inputTables[i].columns()->end(),
			back_inserter(allInputColumns),
			[&](const Column& column){return Column(queryInfo.tableNames[i], column.columnName); });
	}

	// SELECT句の列名指定が*だった場合は、入力CSVの列名がすべて選択されます。
	if (queryInfo.selectColumns.empty()){
		copy(allInputColumns.begin(), allInputColumns.end(), back_inserter(queryInfo.selectColumns));
	}

	// SELECT句で指定された列名が、何個目の入力ファイルの何列目に相当するかを判別します。
	for (auto &selectColumn : queryInfo.selectColumns){
		selectColumn.SetAllColumns(inputTables);
	}

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

	vector<vector<const vector<const Data>>::const_iterator> currentRows; // 入力された各テーブルの、現在出力している行を指すカーソルです。
	transform(
		inputTables.begin(),
		inputTables.end(),
		back_inserter(currentRows),
		[](const InputTable& table){return table.data()->begin(); });

	// 出力するデータを設定します。
	while (true){
		outputData.push_back(vector<Data>());
		vector<Data> &row = outputData.back(); // 出力している一行分のデータです。

		allColumnOutputData.push_back(vector<Data>());
		vector<Data> &allColumnsRow = allColumnOutputData.back();// WHEREやORDERのためにすべての情報を含む行。rowとインデックスを共有します。

		// allColumnsRowの列を設定します。
		for (auto &currentRow : currentRows){
			copy(
				currentRow->begin(),
				currentRow->end(),
				back_inserter(allColumnsRow));
		}

		// 行の各列のデータを設定します。
		transform(
			queryInfo.selectColumns.begin(),
			queryInfo.selectColumns.end(),
			back_inserter(row),
			[&](const Column& column){return allColumnsRow[column.allColumnsIndex]; });

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
						bool found = false;
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
		for (int i = queryInfo.tableNames.size() - 1; currentRows[i] == inputTables[i].data()->end() && 0 < i; --i){
			++currentRows[i - 1];
			currentRows[i] = inputTables[i].data()->begin();
		}

		// 最初のテーブルが最後の行を超えたなら出力行の生成は終わりです。
		if (currentRows[0] == inputTables[0].data()->end()){
			break;
		}
	}

	// ORDER句による並び替えの処理を行います。
	if (!queryInfo.orderByColumns.empty()){
		// ORDER句で指定されている列が、全ての入力行の中のどの行なのかを計算します。
		vector<int> orderByColumnIndexes; // ORDER句で指定された列の、すべての行の中でのインデックスです。

		for (auto &orderByColumn : queryInfo.orderByColumns){
			bool found = false;
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
	outputFile = ofstream(outputFileName);
	if (outputFile.bad()){
		throw ResultValue::ERR_FILE_OPEN;
	}

	// 出力ファイルに列名を出力します。
	for (size_t i = 0; i < queryInfo.selectColumns.size(); ++i){
		outputFile << queryInfo.selectColumns[i].outputName;
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
	if (outputFile){
		outputFile.close();
		if (outputFile.bad()){
			throw ResultValue::ERR_FILE_CLOSE;
		}
	}
}

//! CSVファイルに出力データを書き込みます。
//! @param [in] outputFileName 結果を出力するファイルのファイル名です。
//! @param [in] queryInfo SQLの情報です。
//! @param [in] inputTables ファイルから読み取ったデータです。
void Csv::WriteCsv(const string outputFileName, const vector<const InputTable> &inputTables) const
{
	OutputData output(*queryInfo);
	output.WriteCsv(outputFileName, inputTables);
}

//! ファイルストリームからカンマ区切りの一行を読み込みます。
//! @param [in] inputFile データを読み込むファイルストリームです。
//! @return ファイルから読み込んだ一行分のデータです。
const shared_ptr<const vector<const string>> Csv::ReadLineData(ifstream &inputFile) const
{
	string inputLine; // ファイルから読み込んだ行文字列です。
	if (getline(inputFile, inputLine)){
		auto lineData = make_shared<vector<const string>>(); // 一行分のデータです。

		auto charactorCursol = inputLine.begin(); // ヘッダ入力行を検索するカーソルです。
		auto lineEnd = inputLine.end(); // ヘッダ入力行のendを指します。

		// 読み込んだ行を最後まで読みます。
		while (charactorCursol != lineEnd){

			// 列名を一つ読みます。
			auto columnStart = charactorCursol; // 現在の列の最初を記録しておきます。
			charactorCursol = find(charactorCursol, lineEnd, ',');
			lineData->push_back(string(columnStart, charactorCursol));

			// 入力行のカンマの分を読み進めます。
			if (charactorCursol != lineEnd){
				++charactorCursol;
			}
		}
		return lineData;
	}
	else{
		return nullptr;
	}
}

//! 入力CSVのヘッダ行を読み込みます。
//! @param [in] inputFile 入力ファイルを扱うストリームです。開いた後何も読み込んでいません。
//! @param [in] tableName SQLで指定されたテーブル名です。
//! @return ファイルから読み取ったヘッダ情報です。
const shared_ptr<const vector<const Column>> Csv::ReadHeader(ifstream &inputFile, const string tableName) const
{
	auto columns = make_shared<vector<const Column>>(); // 読み込んだ列の一覧。

	if (auto lineData = ReadLineData(inputFile)){
		transform(
			lineData->begin(),
			lineData->end(),
			back_inserter(*columns),
			[&](const string& column){return Column(tableName, column); });
		return columns;
	}
	else{
		throw ResultValue::ERR_CSV_SYNTAX;
	}
}
//! 入力CSVのデータ行を読み込みます。
//! @param [in] inputFile 入力ファイルを扱うストリームです。すでにヘッダのみを読み込んだ後です。
//! @return ファイルから読み取ったデータです。
const shared_ptr<vector<const vector<const Data>>> Csv::ReadData(ifstream &inputFile) const
{
	auto data = make_shared<vector<const vector<const Data>>>(); // 読み込んだデータの一覧。

	while (auto lineData = ReadLineData(inputFile)){
		vector<const Data> row;
		transform(
			lineData->begin(),
			lineData->end(),
			back_inserter(row),
			[&](const string& column){return Data(column); });
		data->push_back(row);
	}
	return data;
}

//! 入力ファイルを開きます。
//! @param [in] filePath 開くファイルのファイルパスです。
//! @return 入力ファイルを扱うストリームです。
ifstream Csv::OpenInputFile(const string filePath) const
{
	auto inputFile = ifstream(filePath); //入力するCSVファイルを扱うストリームです。
	if (!inputFile){
		throw ResultValue::ERR_FILE_OPEN;
	}
	return inputFile;
}

//! 入力ファイルを閉じます。
//! @param [in] inputFile 入力ファイルを扱うストリームです。
void Csv::CloseInputFile(ifstream &inputFile) const
{
	inputFile.close();
	if (inputFile.bad()){
		throw ResultValue::ERR_FILE_CLOSE;
	}
}

//! Csvクラスの新しいインスタンスを初期化します。
//! @param [in] queryInfo SQLに記述された内容です。
Csv::Csv(const shared_ptr<const SqlQueryInfo> queryInfo) : queryInfo(queryInfo){}

//! CSVファイルから入力データを読み取ります。
//! @return ファイルから読み取ったデータです。
const shared_ptr<const vector<const InputTable>> Csv::ReadCsv() const
{
	auto tables = make_shared<vector<const InputTable>>();

	for (auto &tableName : queryInfo->tableNames){
		
		auto inputFile = OpenInputFile(tableName + ".csv");
		
		auto header = ReadHeader(inputFile, tableName);
		auto data = ReadData(inputFile);
		tables->push_back(InputTable(InputTable(header, data)));

		CloseInputFile(inputFile);
	}
	return tables;
}

//! SQLの文字列からトークンを切り出します。
//! @param [in] sql トークンに分解する元となるSQLです。
//! @return 切り出されたトークンです。
const shared_ptr<const vector<const Token>> SqlQuery::GetTokens(const string sql) const
{
	auto cursol = sql.begin(); // SQLをトークンに分割して読み込む時に現在読んでいる文字の場所を表します。
	auto end = sql.end(); // sqlのendを指します。
	auto tokens = make_shared<vector<const Token>>(); //読み込んだトークンです。

	// SQLをトークンに分割て読み込みます。
	while (cursol != end){

		// 空白を読み飛ばします。
		cursol = find_if(cursol, end, [&](char c){return space.find(c) == string::npos; });
		if (cursol == end){
			break;
		}
		// 各種トークンを読み込みます。
		shared_ptr<const Token> token;
		if (any_of(
			tokenReaders.begin(),
			tokenReaders.end(),
			[&](const shared_ptr<const TokenReader>& reader){
				return token = reader->Read(cursol, end); 
			})){
			tokens->push_back(*token);
		}
		else{
			throw ResultValue::ERR_TOKEN_CANT_READ;
		}
	}
	return tokens;
}

//! トークンを解析してSQLの構文で指定された情報を取得します。
//! @param [in] tokens 解析の対象となるトークンです。
//! @return 解析した結果の情報です。
const shared_ptr<const SqlQueryInfo> SqlQuery::AnalyzeTokens(const vector<const Token> &tokens) const
{
	auto queryInfo = make_shared<SqlQueryInfo>();
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
				queryInfo->selectColumns.push_back(Column(tokenCursol->word));
				++tokenCursol;
				if (tokenCursol->kind == TokenKind::DOT){
					++tokenCursol;
					if (tokenCursol->kind == TokenKind::IDENTIFIER){

						// テーブル名が指定されていることがわかったので読み替えます。
						queryInfo->selectColumns.back() = Column(queryInfo->selectColumns.back().columnName, tokenCursol->word);
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
						queryInfo->orderByColumns.push_back(Column(tokenCursol->word));
						++tokenCursol;
						if (tokenCursol->kind == TokenKind::DOT){
							++tokenCursol;
							if (tokenCursol->kind == TokenKind::IDENTIFIER){

								// テーブル名が指定されていることがわかったので読み替えます。
								queryInfo->orderByColumns.back() = Column(queryInfo->orderByColumns.back().columnName, tokenCursol->word);
								++tokenCursol;
							}
							else{
								throw ResultValue::ERR_SQL_SYNTAX;
							}
						}

						// 並び替えの昇順、降順を指定します。
						if (tokenCursol->kind == TokenKind::ASC){
							queryInfo->orders.push_back(TokenKind::ASC);
							++tokenCursol;
						}
						else if (tokenCursol->kind == TokenKind::DESC){
							queryInfo->orders.push_back(TokenKind::DESC);
							++tokenCursol;
						}
						else{
							// 指定がない場合は昇順となります。
							queryInfo->orders.push_back(TokenKind::ASC);
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
				queryInfo->whereExtensionNodes.push_back(make_shared<ExtensionTreeNode>());
				if (currentNode){
					// 現在のノードを右の子にずらし、元の位置に新しいノードを挿入します。
					currentNode->right = queryInfo->whereExtensionNodes.back();
					currentNode->right->parent = currentNode;
					currentNode = currentNode->right;
				}
				else{
					// 最初はカレントノードに新しいノードを入れます。
					currentNode = queryInfo->whereExtensionNodes.back();
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
					queryInfo->whereExtensionNodes.push_back(make_shared<ExtensionTreeNode>());
					currentNode = queryInfo->whereExtensionNodes.back();
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
			queryInfo->whereTopNode = currentNode;
			while (queryInfo->whereTopNode->parent){
				queryInfo->whereTopNode = queryInfo->whereTopNode->parent;
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
			queryInfo->tableNames.push_back(tokenCursol->word);
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

	return queryInfo;
}

//! SqlQueryクラスの新しいインスタンスを初期化します。
//! @param [in] sql 実行するSQLです。
SqlQuery::SqlQuery(const string sql) :
// 先頭から順に検索されるので、前方一致となる二つの項目は順番に気をつけて登録しなくてはいけません。
	tokenReaders({
		make_shared<IntLiteralReader>(),
		make_shared<StringLiteralReader>(),
		make_shared<KeywordReader>(TokenKind::AND, "AND"),
		make_shared<KeywordReader>(TokenKind::ASC, "ASC"),
		make_shared<KeywordReader>(TokenKind::BY, "BY"),
		make_shared<KeywordReader>(TokenKind::DESC, "DESC"),
		make_shared<KeywordReader>(TokenKind::FROM, "FROM"),
		make_shared<KeywordReader>(TokenKind::ORDER, "ORDER"),
		make_shared<KeywordReader>(TokenKind::OR, "OR"),
		make_shared<KeywordReader>(TokenKind::SELECT, "SELECT"),
		make_shared<KeywordReader>(TokenKind::WHERE, "WHERE"),
		make_shared<SignReader>(TokenKind::GREATER_THAN_OR_EQUAL, ">="),
		make_shared<SignReader>(TokenKind::LESS_THAN_OR_EQUAL, "<="),
		make_shared<SignReader>(TokenKind::NOT_EQUAL, "<>"),
		make_shared<SignReader>(TokenKind::ASTERISK, "*"),
		make_shared<SignReader>(TokenKind::COMMA, ","),
		make_shared<SignReader>(TokenKind::CLOSE_PAREN, ")"),
		make_shared<SignReader>(TokenKind::DOT, "."),
		make_shared<SignReader>(TokenKind::EQUAL, "="),
		make_shared<SignReader>(TokenKind::GREATER_THAN, ">"),
		make_shared<SignReader>(TokenKind::LESS_THAN, "<"),
		make_shared<SignReader>(TokenKind::MINUS, "-"),
		make_shared<SignReader>(TokenKind::OPEN_PAREN, "("),
		make_shared<SignReader>(TokenKind::PLUS, "+"),
		make_shared<SignReader>(TokenKind::SLASH, "/"),
		make_shared<IdentifierReader>(),}),
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
		{ TokenKind::OR, 5 }})
{
	csv = make_shared<Csv>(AnalyzeTokens(*GetTokens(sql)));
}

//! カレントディレクトリにあるCSVに対し、簡易的なSQLを実行し、結果をファイルに出力します。
//! @param[in] outputFileName SQLの実行結果をCSVとして出力するファイル名です。拡張子を含みます。
void SqlQuery::Execute(const string outputFileName)
{
	csv->WriteCsv(outputFileName, *csv->ReadCsv());
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
	try
	{
		SqlQuery(sql).Execute(outputFileName);
		return static_cast<int>(ResultValue::OK);
	}
	catch (ResultValue error) // 発生したエラーの種類です。
	{
		return static_cast<int>(error);
	}
}