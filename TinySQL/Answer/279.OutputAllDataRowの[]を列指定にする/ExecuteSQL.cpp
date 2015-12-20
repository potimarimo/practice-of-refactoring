//! @file
#include "stdafx.h"

#include <cctype>
#include <fstream>
#include <sstream>
#include <functional>
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
	DataType m_type = DataType::STRING; //!< データの型です。

	const string defaultString = ""; //!< データが文字列を持たない場合にstring()が返す値。
protected:
	//! Dataクラスの新しいインスタンスを初期化します。
	Data();
public:
	//! Data型の具象クラスのインスタンスを返します。
	//! @param [in] value データの実際の値です。
	static shared_ptr<Data> New(const string value);

	//! Data型の具象クラスのインスタンスを返します。
	//! @param [in] value データの実際の値です。
	static shared_ptr<Data> New(const int value);

	//! Data型の具象クラスのインスタンスを返します。
	//! @param [in] value データの実際の値です。
	static shared_ptr<Data> New(const bool value);

	//! データの型を取得します。
	//! @return データの型です。
	virtual const DataType type() const = 0;

	//! データが文字列型の場合の値を取得します。
	//! @return データが文字列型の場合の値です。
	virtual const string& string() const;

	//! データが整数型の場合の値を取得します。
	//! @return データが整数型の場合の値です。
	virtual const int integer() const;

	//! データが真偽値型の場合の値を取得します。
	//! @return データが真偽値型の場合の値です。
	virtual const bool boolean() const;

	//! 加算演算を行います。
	//! @param [in] right 右辺です。
	//! @return 加算した結果です。
	virtual const shared_ptr<const Data> operator+(const shared_ptr<const Data>& right) const = 0;

	//! 減算演算を行います。
	//! @param [in] right 右辺です。
	//! @return 減算した結果です。
	virtual const shared_ptr<const Data> operator-(const shared_ptr<const Data>& right) const = 0;

	//! 乗算演算を行います。
	//! @param [in] right 右辺です。
	//! @return 乗算した結果です。
	virtual const shared_ptr<const Data> operator*(const shared_ptr<const Data>& right) const = 0;

	//! 除算演算を行います。
	//! @param [in] right 右辺です。
	//! @return 除算した結果です。
	virtual const shared_ptr<const Data> operator/(const shared_ptr<const Data>& right) const = 0;

	//! 等値比較を行います。
	//! @param [in] right 右辺です。
	//! @return 比較した結果です。
	virtual const shared_ptr<const Data> operator==(const shared_ptr<const Data>& right) const = 0;

	//! 不等比較を行います。
	//! @param [in] right 右辺です。
	//! @return 比較した結果です。
	virtual const shared_ptr<const Data> operator!=(const shared_ptr<const Data>& right) const = 0;

	//! 以上比較を行います。
	//! @param [in] right 右辺です。
	//! @return 比較した結果です。
	virtual const shared_ptr<const Data> operator>=(const shared_ptr<const Data>& right) const = 0;

	//! 大きい比較を行います。
	//! @param [in] right 右辺です。
	//! @return 比較した結果です。
	virtual const shared_ptr<const Data> operator>(const shared_ptr<const Data>& right) const = 0;

	//! 以下比較を行います。
	//! @param [in] right 右辺です。
	//! @return 比較した結果です。
	virtual const shared_ptr<const Data> operator<=(const shared_ptr<const Data>& right) const = 0;

	//! 小さい比較を行います。
	//! @param [in] right 右辺です。
	//! @return 比較した結果です。
	virtual const shared_ptr<const Data> operator<(const shared_ptr<const Data>& right) const = 0;

	//! AND演算を行います。
	//! @param [in] right 右辺です。
	//! @return 演算した結果です。
	virtual const shared_ptr<const Data> operator&&(const shared_ptr<const Data>& right) const = 0;

	//! OR演算を行います。
	//! @param [in] right 右辺です。
	//! @return 演算した結果です。
	virtual const shared_ptr<const Data> operator||(const shared_ptr<const Data>& right) const = 0;
};

//! 文字列の値を持つDataです。
class StringData : public Data
{
	std::string m_string; //!< データが文字列型の場合の値です。

public:
	//! Dataクラスの新しいインスタンスを初期化します。
	//! @param [in] value データの値です。
	StringData(const std::string value);

	//! データの型を取得します。
	//! @return データ	の型です。
	const DataType type() const override;

	//! データが文字列型の場合の値を取得します。
	//! @return データが文字列型の場合の値です。
	const std::string& string() const override;

	//! 加算演算を行います。
	//! @param [in] right 右辺です。
	//! @return 加算した結果です。
	const shared_ptr<const Data> operator+(const shared_ptr<const Data>& right) const override;

	//! 減算演算を行います。
	//! @param [in] right 右辺です。
	//! @return 減算した結果です。
	const shared_ptr<const Data> operator-(const shared_ptr<const Data>& right) const override;

	//! 乗算演算を行います。
	//! @param [in] right 右辺です。
	//! @return 乗算した結果です。
	const shared_ptr<const Data> operator*(const shared_ptr<const Data>& right) const override;

	//! 除算演算を行います。
	//! @param [in] right 右辺です。
	//! @return 除算した結果です。
	const shared_ptr<const Data> operator/(const shared_ptr<const Data>& right) const override;

	//! 等値比較を行います。
	//! @param [in] right 右辺です。
	//! @return 比較した結果です。
	const shared_ptr<const Data> operator==(const shared_ptr<const Data>& right) const override;

	//! 不等比較を行います。
	//! @param [in] right 右辺です。
	//! @return 比較した結果です。
	const shared_ptr<const Data> operator!=(const shared_ptr<const Data>& right) const override;

	//! 以上比較を行います。
	//! @param [in] right 右辺です。
	//! @return 比較した結果です。
	const shared_ptr<const Data> operator>=(const shared_ptr<const Data>& right) const override;

	//! 大きい比較を行います。
	//! @param [in] right 右辺です。
	//! @return 比較した結果です。
	const shared_ptr<const Data> operator>(const shared_ptr<const Data>& right) const override;

	//! 以下比較を行います。
	//! @param [in] right 右辺です。
	//! @return 比較した結果です。
	const shared_ptr<const Data> operator<=(const shared_ptr<const Data>& right) const override;

	//! 小さい比較を行います。
	//! @param [in] right 右辺です。
	//! @return 比較した結果です。
	const shared_ptr<const Data> operator<(const shared_ptr<const Data>& right) const override;

	//! AND演算を行います。
	//! @param [in] right 右辺です。
	//! @return 演算した結果です。
	const shared_ptr<const Data> operator&&(const shared_ptr<const Data>& right) const override;

	//! OR演算を行います。
	//! @param [in] right 右辺です。
	//! @return 演算した結果です。
	const shared_ptr<const Data> operator||(const shared_ptr<const Data>& right) const override;
};

//! 整数の値を持つDataです。
class IntegerData : public Data
{
	int m_integer;                  //!< データが整数型の場合の値です。

public:
	//! Dataクラスの新しいインスタンスを初期化します。
	//! @param [in] value データの値です。
	IntegerData(const int value);

	//! データの型を取得します。
	//! @return データの型です。
	const DataType type() const override;

	//! データが整数型の場合の値を取得します。
	//! @return データが整数型の場合の値です。
	const int integer() const override;

	//! 加算演算を行います。
	//! @param [in] right 右辺です。
	//! @return 加算した結果です。
	const shared_ptr<const Data> operator+(const shared_ptr<const Data>& right) const override;

	//! 減算演算を行います。
	//! @param [in] right 右辺です。
	//! @return 減算した結果です。
	const shared_ptr<const Data> operator-(const shared_ptr<const Data>& right) const override;

	//! 乗算演算を行います。
	//! @param [in] right 右辺です。
	//! @return 乗算した結果です。
	const shared_ptr<const Data> operator*(const shared_ptr<const Data>& right) const override;

	//! 除算演算を行います。
	//! @param [in] right 右辺です。
	//! @return 除算した結果です。
	const shared_ptr<const Data> operator/(const shared_ptr<const Data>& right) const override;

	//! 等値比較を行います。
	//! @param [in] right 右辺です。
	//! @return 比較した結果です。
	const shared_ptr<const Data> operator==(const shared_ptr<const Data>& right) const override;

	//! 不等比較を行います。
	//! @param [in] right 右辺です。
	//! @return 比較した結果です。
	const shared_ptr<const Data> operator!=(const shared_ptr<const Data>& right) const override;

	//! 以上比較を行います。
	//! @param [in] right 右辺です。
	//! @return 比較した結果です。
	const shared_ptr<const Data> operator>=(const shared_ptr<const Data>& right) const override;

	//! 大きい比較を行います。
	//! @param [in] right 右辺です。
	//! @return 比較した結果です。
	const shared_ptr<const Data> operator>(const shared_ptr<const Data>& right) const override;

	//! 以下比較を行います。
	//! @param [in] right 右辺です。
	//! @return 比較した結果です。
	const shared_ptr<const Data> operator<=(const shared_ptr<const Data>& right) const override;

	//! 小さい比較を行います。
	//! @param [in] right 右辺です。
	//! @return 比較した結果です。
	const shared_ptr<const Data> operator<(const shared_ptr<const Data>& right) const override;

	//! AND演算を行います。
	//! @param [in] right 右辺です。
	//! @return 演算した結果です。
	const shared_ptr<const Data> operator&&(const shared_ptr<const Data>& right) const override;

	//! OR演算を行います。
	//! @param [in] right 右辺です。
	//! @return 演算した結果です。
	const shared_ptr<const Data> operator||(const shared_ptr<const Data>& right) const override;
};

//! 真偽値の値を持つDataです。
class BooleanData : public Data
{
	bool m_boolean;                  //!< データが真偽値型の場合の値です。

public:
	//! Dataクラスの新しいインスタンスを初期化します。
	//! @param [in] value データの値です。
	BooleanData(const bool value);

	//! データの型を取得します。
	//! @return データの型です。
	const DataType type() const override;

	//! データが整数型の場合の値を取得します。
	//! @return データが整数型の場合の値です。
	const bool boolean() const override;

	//! 加算演算を行います。
	//! @param [in] right 右辺です。
	//! @return 加算した結果です。
	const shared_ptr<const Data> operator+(const shared_ptr<const Data>& right) const override;

	//! 減算演算を行います。
	//! @param [in] right 右辺です。
	//! @return 減算した結果です。
	const shared_ptr<const Data> operator-(const shared_ptr<const Data>& right) const override;

	//! 乗算演算を行います。
	//! @param [in] right 右辺です。
	//! @return 乗算した結果です。
	const shared_ptr<const Data> operator*(const shared_ptr<const Data>& right) const override;

	//! 除算演算を行います。
	//! @param [in] right 右辺です。
	//! @return 除算した結果です。
	const shared_ptr<const Data> operator/(const shared_ptr<const Data>& right) const override;

	//! 等値比較を行います。
	//! @param [in] right 右辺です。
	//! @return 比較した結果です。
	const shared_ptr<const Data> operator==(const shared_ptr<const Data>& right) const override;

	//! 不等比較を行います。
	//! @param [in] right 右辺です。
	//! @return 比較した結果です。
	const shared_ptr<const Data> operator!=(const shared_ptr<const Data>& right) const override;

	//! 以上比較を行います。
	//! @param [in] right 右辺です。
	//! @return 比較した結果です。
	const shared_ptr<const Data> operator>=(const shared_ptr<const Data>& right) const override;

	//! 大きい比較を行います。
	//! @param [in] right 右辺です。
	//! @return 比較した結果です。
	const shared_ptr<const Data> operator>(const shared_ptr<const Data>& right) const override;

	//! 以下比較を行います。
	//! @param [in] right 右辺です。
	//! @return 比較した結果です。
	const shared_ptr<const Data> operator<=(const shared_ptr<const Data>& right) const override;

	//! 小さい比較を行います。
	//! @param [in] right 右辺です。
	//! @return 比較した結果です。
	const shared_ptr<const Data> operator<(const shared_ptr<const Data>& right) const override;

	//! AND演算を行います。
	//! @param [in] right 右辺です。
	//! @return 演算した結果です。
	const shared_ptr<const Data> operator&&(const shared_ptr<const Data>& right) const override;

	//! OR演算を行います。
	//! @param [in] right 右辺です。
	//! @return 演算した結果です。
	const shared_ptr<const Data> operator||(const shared_ptr<const Data>& right) const override;
};

//! 加算演算を行います。
//! @param [in] left 左辺です。
//! @param [in] right 右辺です。
//! @return 加算した結果です。
const shared_ptr<const Data> operator+(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right);

//! 減算演算を行います。
//! @param [in] left 左辺です。
//! @param [in] right 右辺です。
//! @return 減算した結果です。
const shared_ptr<const Data> operator-(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right);


//! 乗算演算を行います。
//! @param [in] left 左辺です。
//! @param [in] right 右辺です。
//! @return 乗算した結果です。
const shared_ptr<const Data> operator*(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right);

//! 除算演算を行います。
//! @param [in] left 左辺です。
//! @param [in] right 右辺です。
//! @return 除算した結果です。
const shared_ptr<const Data> operator/(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right);

//! 等値比較を行います。
//! @param [in] left 左辺です。
//! @param [in] right 右辺です。
//! @return 比較した結果です。
const shared_ptr<const Data> operator==(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right);

//! 不等比較を行います。
//! @param [in] left 左辺です。
//! @param [in] right 右辺です。
//! @return 比較した結果です。
const shared_ptr<const Data> operator!=(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right);

//! 以上比較を行います。
//! @param [in] left 左辺です。
//! @param [in] right 右辺です。
//! @return 比較した結果です。
const shared_ptr<const Data> operator>=(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right);

//! 大きい比較を行います。
//! @param [in] left 左辺です。
//! @param [in] right 右辺です。
//! @return 比較した結果です。
const shared_ptr<const Data> operator>(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right);

//! 以下比較を行います。
//! @param [in] left 左辺です。
//! @param [in] right 右辺です。
//! @return 比較した結果です。
const shared_ptr<const Data> operator<=(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right);

//! 小さい比較を行います。
//! @param [in] left 左辺です。
//! @param [in] right 右辺です。
//! @return 比較した結果です。
const shared_ptr<const Data> operator<(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right);

//! AND演算を行います。
//! @param [in] left 左辺です。
//! @param [in] right 右辺です。
//! @return 演算した結果です。
const shared_ptr<const Data> operator&&(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right);

//! OR演算を行います。
//! @param [in] left 左辺です。
//! @param [in] right 右辺です。
//! @return 演算した結果です。
const shared_ptr<const Data> operator||(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right);
//! WHERE句に指定する演算子の情報を表します。
class Operator
{
	TokenKind m_kind; //!< 演算子の種類を、演算子を記述するトークンの種類で表します。
	int m_order; //!< 演算子の優先順位です。
public:
	//! Operatorクラスの新しいインスタンスを初期化します。
	//! @param [in] kind 演算子の種類を、演算子を記述するトークンの種類で表します。
	//! @param [in] order 演算子の優先順位です。
	Operator(const TokenKind &kind, const int &order);

	//! 演算子の種類を、演算子を記述するトークンの種類で表したものを取得します。
	//! @return 演算子の種類を、演算子を記述するトークンの種類で表したもの。
	const TokenKind& kind() const;

	//! 演算子の優先順位を取得します。
	//! 演算子の優先順位です。
	const int& order() const;
};

//! トークンを表します。
class Token
{
	const TokenKind m_kind; //!< トークンの種類です。
	const string m_word; //!< 記録されているトークンの文字列です。記録の必要がなければ空白です。
public:
	//! Tokenクラスの新しいインスタンスを初期化します。
	//! @param [in] kind トークンの種類です。
	Token(const TokenKind &kind);

	//! Tokenクラスの新しいインスタンスを初期化します。
	//! @param [in] kind トークンの種類です。
	//! @param [in] word 記録されているトークンの文字列です。記録の必要がなければ空白です。
	Token(const TokenKind &kind, const string &word);

	//! トークンの種類を取得します。
	//! @return トークンの種類です。
	const TokenKind& kind() const;

	//! 記録されているトークンの文字列を取得します。記録の必要がなければ空白です。
	//! @return 記録されているトークンの文字列です。
	const string& word() const;
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

//! 出力するすべてのデータを含む行を表します。
class OutputAllDataRow
{
	shared_ptr<const vector<const shared_ptr<const Data>>> data; //!< データを保持します。
public:

	//! OutputAllDataRowクラスの新しいインスタンスを初期化します。
	//! @param [in] data OutputAllDataRowの持つデータです。
	OutputAllDataRow(const shared_ptr<const vector<const shared_ptr<const Data>>> data);

	//! 列を指定し、データを取得します。指定する列は事前にSetAllColumns
	//! @param [in] column 取得したい列。
	//! @return 指定したデータ。
	const shared_ptr<const Data> operator[](const Column &column) const;
};

//! WHERE句の条件の式木を表します。
class ExtensionTreeNode : public enable_shared_from_this<ExtensionTreeNode>
{
	//! カラム名で指定されたデータを持つノードかどうかです。
	//! @return カラム名で指定されたデータを持つノードかどうか。
	bool ExtensionTreeNode::isDataNodeAsColumnName();

	//! 次のノードを指定する関数を指定し、ノードの列を生成します。
	//! @param [in] nextNode 現在のノードから次のノードを指定する関数です。
	//! @param [in] includeSelf 自身を含むかどうか。
	//! @return thisから次のノードをたどっていき、ノードがnullptrになる前までのすべてのノードの列。
	const shared_ptr<vector<const shared_ptr<ExtensionTreeNode>>> ExtensionTreeNode::allNodesOf(function<const shared_ptr<ExtensionTreeNode>(const shared_ptr<const ExtensionTreeNode>)> nextNode, const bool includeSelf = false) const;
public:
	shared_ptr<ExtensionTreeNode> parent;//!< 親となるノードです。根の式木の場合はnullptrとなります。
	shared_ptr<ExtensionTreeNode> left;  //!< 左の子となるノードです。自身が末端の葉となる式木の場合はnullptrとなります。
	Operator middleOperator;             //!< 中置される演算子です。自身が末端のとなる式木の場合の種類はNOT_TOKENとなります。
	shared_ptr<ExtensionTreeNode>right;   //!< 右の子となるノードです。自身が末端の葉となる式木の場合はnullptrとなります。
	bool inParen = false;                //!< 自身がかっこにくるまれているかどうかです。
	int parenOpenBeforeClose = 0;        //!< 木の構築中に0以外となり、自身の左にあり、まだ閉じてないカッコの開始の数となります。
	int signCoefficient = 1;             //!< 自身が葉にあり、マイナス単項演算子がついている場合は-1、それ以外は1となります。
	Column column;                       //!< 列場指定されている場合に、その列を表します。列指定ではない場合はcolumnNameが空文字列となります。
	shared_ptr<const Data> value;                          //!< 指定された、もしくは計算された値です。

	//! ExtensionTreeNodeクラスの新しいインスタンスを初期化します。
	ExtensionTreeNode();

	// leftとrightをmiddleOperatorで演算します。
	void Operate();

	//! 実際に出力する行に合わせて列にデータを設定します。
	//! @param [in] inputTables ファイルから読み取ったデータです。
	//! @param [in] 実際に出力する行です。
	void SetColumnData(const OutputAllDataRow &outputRow);

	//! 自身の祖先ノードを自身に近いほうから順に列挙します。
	//! @return 祖先ノードの一覧。
	const shared_ptr<vector<const shared_ptr<ExtensionTreeNode>>> ancestors() const;

	//! 自身の子孫ノードをずっと左に辿っていき自身に近いほうから順に列挙します。
	//! @return 左に辿った子孫ノードの一覧。
	const shared_ptr<vector<const shared_ptr<ExtensionTreeNode>>> allLeftList() const;

	//! 自身及び自身の祖先ノードを自身に近いほうから順に列挙します。
	//! @return 祖先ノードの一覧。
	const shared_ptr<vector<const shared_ptr<ExtensionTreeNode>>> selfAndAncestors() const;

	//! 自身及び自身の子孫ノードをずっと左に辿っていき自身に近いほうから順に列挙します。
	//! @return 左に辿った子孫ノードの一覧。
	const shared_ptr<vector<const shared_ptr<ExtensionTreeNode>>> selfAndAllLeftList() const;

	//! 自身の位置にノードを挿入し、自身は挿入したノードの左の子となります。
	//! @param [in] inserted 挿入するノードです。
	void InsertAndMoveLeft(const shared_ptr<ExtensionTreeNode> inserted);

	//! 自身の右の子としてノードを挿入します。
	//! @param [in] inserted 挿入するノードです。
	void InsertRight(const shared_ptr<ExtensionTreeNode> inserted);
};

//! 引数として渡したノード及びその子孫のノードを取得します。順序は帰りがけ順です。
//! @param [in] 戻り値のルートとなるノードです。
//! @return 自身及び子孫のノードです。
const shared_ptr<vector<const shared_ptr<ExtensionTreeNode>>> SelfAndDescendants(shared_ptr<ExtensionTreeNode>);

//! Order句の列と順序の指定を表します。
class Order
{
public:
	Column column; //<! ORDER句に指定された列名です。
	const bool isAsc = true; //<! ORDER国指定された順序が昇順かどうかです。

	//! Orderクラスの新しいインスタンスを初期化します。
	//! @param [in] column ORDER句に指定された列名です。
	//! @param [in] isAsc ORDER国指定された順序が昇順かどうかです。
	Order(Column column, const bool isAsc);
};

//! SqlQueryの構文情報を扱うクラスです。
class SqlQueryInfo
{
public:
	vector<const string> tableNames; //!< FROM句で指定しているテーブル名です。
	vector<Column> selectColumns; //!< SELECT句に指定された列名です。
	vector<Order> orders; //!< ORDER句に指定された順序の情報です。
	shared_ptr<ExtensionTreeNode> whereTopNode; //!< 式木の根となるノードです。
};

//! CSVとして入力されたファイルの内容を表します。
class InputTable
{
	const string signNum = "+-0123456789"; //!< 全ての符号と数字です。

	const shared_ptr<const vector<const Column>> m_columns; //!< 列の情報です。
	const shared_ptr<vector<const vector<const shared_ptr<const Data>>>> m_data; //! データです。

	//! 全てが数値となる列は数値列に変換します。
	void InputTable::InitializeIntegerColumn();
public:
	//! InputTableクラスの新しいインスタンスを初期化します。
	//! @param [in] columns 読み込んだヘッダ情報です。
	//! @param [in] data 読み込んだデータです。
	InputTable(const shared_ptr<const vector<const Column>> columns, const shared_ptr<vector<const vector<const shared_ptr<const Data>>>> data);

	//! 列の情報を取得します。
	//! @return 列の情報です。
	const shared_ptr<const vector<const Column>> columns() const;

	//! データを取得します。
	//! @return データです。
	const shared_ptr<vector<const vector<const shared_ptr<const Data>>>> data() const;
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

class SequenceParser;
//! さまざまなパーサーの基底クラスとなる抽象クラスです。
class Parser
{
public:
	//! トークンに対するパースを行います。
	//! @params [in] cursol 現在の読み取り位置を表すカーソルです。
	//! @return パースが成功したかどうかです。
	virtual const bool Parse(vector<const Token>::const_iterator& cursol, vector<const Token>::const_iterator& end) const = 0;
};

//! トークン列の最後を読み取るパーサーです。
class EndParser : public Parser
{
	function<void(void)> m_action; //!< 読み取りが成功したら実行する処理です。
public:
	//! EndParserクラスの新しいインスタンスを初期化します。
	//! @param [in] 読み取りが成功したら実行する処理です。
	EndParser(const function<void(void)> action);

	//! EndParserクラスの新しいインスタンスを初期化します。
	EndParser();

	//! トークンに対するパースを行います。
	//! @params [in] cursol 現在の読み取り位置を表すカーソルです。
	//! @return パースが成功したかどうかです。
	const bool Parse(vector<const Token>::const_iterator& cursol, vector<const Token>::const_iterator& end) const override;

	//! 読み取りが成功したら実行する処理を登録します。
	//! @param [in] 読み取りが成功したら実行する処理です。
	const shared_ptr<const EndParser> Action(const function<void(void)> action) const;
};

//! トークンのパーサーを生成します。
const shared_ptr<EndParser> end();

//! トークンをひとつ読み取るパーサーです。
class TokenParser : public Parser
{
	const vector<const TokenKind> m_kinds; //!< 読み取るトークンの種類です。
	function<void(const Token)> m_action; //!< 読み取りが成功したら実行する処理です。
public:
	//! TokenParserクラスの新しいインスタンスを初期化します。
	//! @param [in] 読み取りが成功したら実行する処理です。
	//! @params [in] kind 読み取るトークンの種類です。
	TokenParser(const function<void(const Token)> action, const vector<const TokenKind> kinds);

	//! TokenParserクラスの新しいインスタンスを初期化します。
	//! @params [in] kind 読み取るトークンの種類です。
	TokenParser(const vector<const TokenKind> kinds);

	//! TokenParserクラスの新しいインスタンスを初期化します。
	//! @params [in] kind 読み取るトークンの種類です。
	TokenParser(const TokenKind kind);

	//! トークンに対するパースを行います。
	//! @params [in] cursol 現在の読み取り位置を表すカーソルです。
	//! @return パースが成功したかどうかです。
	const bool Parse(vector<const Token>::const_iterator& cursol, vector<const Token>::const_iterator& end) const override;

	//! 読み取りが成功したら実行する処理を登録します。
	//! @param [in] 読み取りが成功したら実行する処理です。
	const shared_ptr<const TokenParser> Action(const function<void(const Token)> action) const;

	//! 二つのTokenPerserを元に複数の種類をとるTokenParserクラスの新しいインスタンスを初期化します。
	//! @params [in] parser 追加する種類ののParserです。
	const shared_ptr<const TokenParser> or(const shared_ptr<const TokenParser> parser) const;
};

//! トークンのパーサーを生成します。
//! @params [in] kind 読み取るトークンの種類です。
const shared_ptr<TokenParser> token(TokenKind kind);

//! 何も読み取らずにアクションだけ実行するパーサーです。
class NoTokenParser : public Parser
{
	function<void(void)> m_action; //!< 読み取りが成功したら実行する処理です。
public:
	//! NoTokenParserクラスの新しいインスタンスを初期化します。
	//! @param [in] 読み取りが成功したら実行する処理です。
	NoTokenParser(const function<void(void)> action);

	//! トークンに対するパースを行います。
	//! @params [in] cursol 現在の読み取り位置を表すカーソルです。
	//! @return パースが成功したかどうかです。
	const bool Parse(vector<const Token>::const_iterator& cursol, vector<const Token>::const_iterator& end) const override;

	//! 読み取りが成功したら実行する処理を登録します。
	//! @param [in] 読み取りが成功したら実行する処理です。
	const shared_ptr<const NoTokenParser> Action(const function<void(void)> action) const;
};

//! 何も読み取らずにアクションだけ実行するパーサーを生成します。
const shared_ptr<NoTokenParser> action(function<void(void)> action);

//! 二つの規則を順番に組み合わせた規則を順に読み取るパーサーです。
class SequenceParser : public Parser
{
	const shared_ptr<const Parser> m_parser1; //!< 一つ目のパーサーです。
	const shared_ptr<const Parser> m_parser2; //!< 二つ目のパーサーです。
	const function<void(void)> m_action; //!< 読み取りが成功したら実行する処理です。
public:
	//! SequenceParserクラスの新しいインスタンスを初期化します。
	//! @param [in] 読み取りが成功したら実行する処理です。
	//! @params [in] parser1 一つ目のParserです。
	//! @params [in] parser2 二つ目目のParserです。
	SequenceParser(const function<void(void)> action, const shared_ptr<const Parser> parser1, const shared_ptr<const Parser> parser2);

	//! SequenceParserクラスの新しいインスタンスを初期化します。
	//! @params [in] parser1 一つ目のParserです。
	//! @params [in] parser2 二つ目のParserです。
	SequenceParser(const shared_ptr<const Parser> parser1, const shared_ptr<const Parser> parser2);

	//! 読み取りが成功したら実行する処理を登録します。
	//! @param [in] 読み取りが成功したら実行する処理です。
	const shared_ptr<const SequenceParser> Action(const function<void(void)> action) const;

	//! 二つの規則に対するパースを行います。
	//! @params [in] cursol 現在の読み取り位置を表すカーソルです。
	//! @return パースが成功したかどうかです。
	const bool Parse(vector<const Token>::const_iterator& cursol, vector<const Token>::const_iterator& end) const override;
};

//! SequenceParserクラスの新しいインスタンスを生成します。
//! @params [in] parser1 一つ目のParserです。
//! @params [in] parser2 二つ目のParserです。
const shared_ptr<const SequenceParser> operator>>(const shared_ptr<const Parser> parser1, const shared_ptr<const Parser> parser2);

//! 二つの規則のどちらかを読み取るパーサーです。
class OrderedChoiceParser : public Parser
{
	const shared_ptr<const Parser> m_parser1; //!< 一つ目のパーサーです。
	const shared_ptr<const Parser> m_parser2; //!< 二つ目のパーサーです。
	const function<void(void)> m_action; //!< 読み取りが成功したら実行する処理です。
public:
	//! OrderedChoiceParserクラスの新しいインスタンスを初期化します。
	//! @param [in] 読み取りが成功したら実行する処理です。
	//! @params [in] parser1 一つ目のParserです。
	//! @params [in] parser2 二つ目目のParserです。
	OrderedChoiceParser(const function<void(void)> action, const shared_ptr<const Parser> parser1, const shared_ptr<const Parser> parser2);

	//! OrderedChoiceParserクラスの新しいインスタンスを初期化します。
	//! @params [in] parser1 一つ目のParserです。
	//! @params [in] parser2 二つ目のParserです。
	OrderedChoiceParser(const shared_ptr<const Parser> parser1, const shared_ptr<const Parser> parser2);

	//! 読み取りが成功したら実行する処理を登録します。
	//! @param [in] 読み取りが成功したら実行する処理です。
	const shared_ptr<const OrderedChoiceParser> Action(const function<void(void)> action) const;

	//! 二つの規則に対するパースを行います。
	//! @params [in] cursol 現在の読み取り位置を表すカーソルです。
	//! @return パースが成功したかどうかです。
	const bool Parse(vector<const Token>::const_iterator& cursol, vector<const Token>::const_iterator& end) const override;
};

//! OrderedChoiceParserクラスの新しいインスタンスを生成します。
//! @params [in] parser1 一つ目のParserです。
//! @params [in] parser2 二つ目のParserです。
const shared_ptr<const OrderedChoiceParser> operator|(const shared_ptr<const Parser> parser1, const shared_ptr<const Parser> parser2);

//! 存在しなくても失敗とならない規則を読み取るパーサーです。
class OptionalParser : public Parser
{
	const shared_ptr<const Parser> m_optional; //!< 存在してもしなくてもよい規則です。
	const function<void(void)> m_action; //!< 読み取りが成功したら実行する処理です。
public:
	//! OptionalParserクラスの新しいインスタンスを初期化します。
	//! @param [in] 読み取りが成功したら実行する処理です。
	//! @params [in] optional 存在してもしなくてもよい規則です。
	OptionalParser(const function<void(void)> action, const shared_ptr<const Parser> optional);

	//! OptionalParserクラスの新しいインスタンスを初期化します。
	//! @params [in] optional 存在してもしなくてもよい規則です。
	OptionalParser(const shared_ptr<const Parser> optional);

	//! 読み取りが成功したら実行する処理を登録します。
	//! @param [in] 読み取りが成功したら実行する処理です。
	const shared_ptr<const OptionalParser> Action(const function<void(void)> action) const;

	//! オプショナルなパースを行います。
	//! @params [in] cursol 現在の読み取り位置を表すカーソルです。
	//! @return パースが成功したかどうかです。
	const bool Parse(vector<const Token>::const_iterator& cursol, vector<const Token>::const_iterator& end) const override;
};

//! OptionalParserクラスの新しいインスタンスを生成します。
//! @params [in] optional 存在してもしなくてもよい規則です。
const shared_ptr<const OptionalParser> operator-(const shared_ptr<const Parser> optional);

//! 0回以上の繰り返しを読み取るパーサーです。
class ZeroOrMoreParser : public Parser
{
	const shared_ptr<const Parser> m_once; //!< 繰り返しの一回分となる規則です。
	const function<void(void)> m_action; //!< 読み取りが成功したら実行する処理です。
public:
	//! ZeroOrMoreParserクラスの新しいインスタンスを初期化します。
	//! @param [in] 読み取りが成功したら実行する処理です。
	//! @params [in] once 繰り返しの一回分となる規則です。
	ZeroOrMoreParser(const function<void(void)> action, const shared_ptr<const Parser> once);

	//! ZeroOrMoreParserクラスの新しいインスタンスを初期化します。
	//! @params [in] once 繰り返しの一回分となる規則です。
	ZeroOrMoreParser(const shared_ptr<const Parser> once);

	//! 読み取りが成功したら実行する処理を登録します。
	//! @param [in] 読み取りが成功したら実行する処理です。
	const shared_ptr<const ZeroOrMoreParser> Action(const function<void(void)> action) const;

	//! 繰り返しのパースを行います。
	//! @params [in] cursol 現在の読み取り位置を表すカーソルです。
	//! @return パースが成功したかどうかです。
	const bool Parse(vector<const Token>::const_iterator& cursol, vector<const Token>::const_iterator& end) const override;
};

////! ZeroOrMoreParserクラスの新しいインスタンスを生成します。
////! @params [in] once 繰り返しの一回分となる規則です。
const shared_ptr<const Parser> operator~(const shared_ptr<const Parser> once);

//! 先読みを行い、カーソルを進めず先にその規則の文法が存在するかどうかを返すパーサーです。
class AndPredicateParser : public Parser
{
	const shared_ptr<const Parser> m_parser; //!< 先読みする規則です。
	const function<void(bool)> m_action; //!< 先読みを実行したら実行する処理です。先読みが成功したかどうかを受け取ります。
public:
	//! AndPredicateParserクラスの新しいインスタンスを初期化します。
	//! @param [in] 先読みを実行したら実行する処理です。先読みが成功したかどうかを受け取ります。
	//! @params [in] parser 先読みする規則です。
	AndPredicateParser(const function<void(bool)> action, const shared_ptr<const Parser> parser);

	//! AndPredicateParserクラスの新しいインスタンスを初期化します。
	//! @params [in] parser 先読みする規則です。
	AndPredicateParser(const shared_ptr<const Parser> parser);

	//! 読み取りが成功したら実行する処理を登録します。
	//! @param [in] 先読みを実行したら実行する処理です。先読みが成功したかどうかを受け取ります。
	const shared_ptr<const AndPredicateParser> Action(const function<void(bool)> action) const;

	//! 繰り返しのパースを行います。
	//! @params [in] cursol 現在の読み取り位置を表すカーソルです。
	//! @return パースが成功したかどうかです。
	const bool Parse(vector<const Token>::const_iterator& cursol, vector<const Token>::const_iterator& end) const;
};

//! AndPredicateParserクラスの新しいインスタンスを初期化します。
//! @params [in] parser 先読みする規則です。
const shared_ptr<const AndPredicateParser> operator&(const shared_ptr<const Parser> parser);

//! 出力するデータを管理します。
class OutputData
{
	SqlQueryInfo queryInfo; //!< SQLに記述された内容です。
	vector<Column> allInputColumns; //!< 入力に含まれるすべての列の一覧です。

	const vector<const InputTable> &inputTables; //!< ファイルから読み取ったデータです。

	//! 入力ファイルに書いてあったすべての列をallInputColumnsに設定します。
	void InitializeAllInputColumns();

	//! SELECT句の列名指定が*だった場合は、入力CSVの列名がすべて選択されます。
	void OpenSelectAsterisk();

	//! SELECT句で指定された列名が、何個目の入力ファイルの何列目に相当するかを判別します。
	void SetAllColumns();

	//! 入力された各テーブルの、現在出力している行を指すカーソルを、初期化された状態で取得します。
	//! @return 初期化されたカーソルです。
	const shared_ptr<vector<vector<const vector<const shared_ptr<const Data>>>::const_iterator>> OutputData::GetInitializedCurrentRows() const;

	//! WHEREやORDER BYを適用していないすべての行を取得します。
	//! @return すべてのデータ行。入力されたすべての入力データを保管します。
	const shared_ptr<vector<const OutputAllDataRow>> GetAllRows() const;

	//! データに対してWHERE句を適用します。
	//! @params [in] outputRows 適用されるデータ。
	void ApplyWhere(vector<const OutputAllDataRow> &outputRows) const;

	//! データに対してORDER BY句を適用します。
	//! @params [in] outputRows 適用されるデータ。
	void ApplyOrderBy(vector<const OutputAllDataRow> &outputRows) const;
public:

	//! OutputDataクラスの新しいインスタンスを初期化します。
	//! @param [in] queryInfo SQLの情報です。
	//! @param [in] inputTables ファイルから読み取ったデータです。
	OutputData(const SqlQueryInfo queryInfo, const vector<const InputTable> &inputTables);

	//! 出力するカラムを取得します。
	//! @return 出力するカラムです。
	const vector<Column> columns() const;

	//! 出力するすべてのデータ行を取得します。
	//! @return 出力するすべてのデータ行。入力されたすべての入力データを保管します。
	const shared_ptr<const vector<const OutputAllDataRow>> outputRows() const;
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
	const shared_ptr<vector<const vector<const shared_ptr<const Data>>>> ReadData(ifstream &inputFile) const;

	//! 出力ファイルを開きます。
	//! @param [in] filePath 開くファイルのファイルパスです。
	//! @return 出力ファイルを扱うストリームです。
	ofstream OpenOutputFile(const string filePath) const;

	//! 出力ファイルを閉じます。
	//! @param [in] OutputFile 入力ファイルを扱うストリームです。
	void CloseOutputFile(ofstream &outputFile) const;

	//! 入力CSVのヘッダ行を読み込みます。
	//! @param [in] OutputFile 出力ファイルを扱うストリームです。開いた後何も読み込んでいません。
	//! @param [in] columns 出力するヘッダ情報です。
	void WriteHeader(ofstream &outputFile, const vector<Column> &columns) const;

	//! 入力CSVのデータ行を読み込みます。
	//! @param [in] OutputFile 出力ファイルを扱うストリームです。すでにヘッダのみを読み込んだ後です。
	//! columns [in] 出力するデータです。
	void WriteData(ofstream &outputFile, const OutputData &data) const;
public:

	//! Csvクラスの新しいインスタンスを初期化します。
	//! @param [in] queryInfo SQLに記述された内容です。
	Csv(const shared_ptr<const SqlQueryInfo> queryInfo);

	//! CSVファイルから入力データを読み取ります。
	//! @return ファイルから読み取ったデータです。
	const shared_ptr<const vector<const InputTable>> Read() const;

	//! CSVファイルに出力データを書き込みます。
	//! @param [in] outputFileName 結果を出力するファイルのファイル名です。
	//! @param [in] inputTables ファイルから読み取ったデータです。
	void Write(const string outputFileName, const vector<const InputTable> &inputTables) const;
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
Data::Data(){}

//! Data型の具象クラスのインスタンスを返します。
//! @param [in] value データの実際の値です。
shared_ptr<Data> Data::New(const std::string value)
{
	return make_shared<StringData>(value);
}

//! Data型の具象クラスのインスタンスを返します。
//! @param [in] value データの実際の値です。
shared_ptr<Data> Data::New(const int value)
{
	return make_shared<IntegerData>(value);
}

//! Data型の具象クラスのインスタンスを返します。
//! @param [in] value データの実際の値です。
shared_ptr<Data> Data::New(const bool value)
{
	return make_shared<BooleanData>(value);
}

//! データが文字列型の場合の値を取得します。
//! @return データが文字列型の場合の値です。
const DataType Data::type() const
{
	return m_type;
}

//! データが文字列型の場合の値を取得します。
//! @return データが文字列型の場合の値です。
const string& Data::string() const
{
	return defaultString;
}

//! データが整数型の場合の値を取得します。
//! @return データが整数型の場合の値です。
const int Data::integer() const
{
	return 0;
}

//! データが真偽値型の場合の値を取得します。
//! @return データが真偽値型の場合の値です。
const bool Data::boolean() const
{
	return false;
}

//! Dataクラスの新しいインスタンスを初期化します。
//! @param [in] value データの値です。
StringData::StringData(const std::string value) : m_string(value){}

//! データの型を取得します。
//! @return データの型です。
const DataType StringData::type() const
{
	return DataType::STRING;
}

//! データが文字列型の場合の値を取得します。
//! @return データが文字列型の場合の値です。
const string& StringData::string() const
{
	return m_string;
}

//! 加算演算を行います。
//! @param [in] right 右辺です。
//! @return 加算した結果です。
const shared_ptr<const Data> StringData::operator+(const shared_ptr<const Data>& right) const
{
	throw ResultValue::ERR_WHERE_OPERAND_TYPE;
}

//! 減算演算を行います。
//! @param [in] right 右辺です。
//! @return 減算した結果です。
const shared_ptr<const Data> StringData::operator-(const shared_ptr<const Data>& right) const
{
	throw ResultValue::ERR_WHERE_OPERAND_TYPE;
}

//! 乗算演算を行います。
//! @param [in] right 右辺です。
//! @return 乗算した結果です。
const shared_ptr<const Data> StringData::operator*(const shared_ptr<const Data>& right) const
{
	throw ResultValue::ERR_WHERE_OPERAND_TYPE;
}

//! 除算演算を行います。
//! @param [in] right 右辺です。
//! @return 除算した結果です。
const shared_ptr<const Data> StringData::operator/(const shared_ptr<const Data>& right) const
{
	throw ResultValue::ERR_WHERE_OPERAND_TYPE;
}

//! 等値比較を行います。
//! @param [in] right 右辺です。
//! @return 比較した結果です。
const shared_ptr<const Data> StringData::operator==(const shared_ptr<const Data>& right) const
{
	if (right->type() == DataType::STRING){
		return Data::New(string() == right->string());
	}
	else{
		throw ResultValue::ERR_WHERE_OPERAND_TYPE;
	}
}

//! 不等比較を行います。
//! @param [in] right 右辺です。
//! @return 比較した結果です。
const shared_ptr<const Data> StringData::operator!=(const shared_ptr<const Data>& right) const
{
	if (right->type() == DataType::STRING){
		return Data::New(string() != right->string());
	}
	else{
		throw ResultValue::ERR_WHERE_OPERAND_TYPE;
	}
}

//! 以上比較を行います。
//! @param [in] right 右辺です。
//! @return 比較した結果です。
const shared_ptr<const Data> StringData::operator>=(const shared_ptr<const Data>& right) const
{
	if (right->type() == DataType::STRING){
		return Data::New(string() >= right->string());
	}
	else{
		throw ResultValue::ERR_WHERE_OPERAND_TYPE;
	}
}

//! 大きい比較を行います。
//! @param [in] right 右辺です。
//! @return 比較した結果です。
const shared_ptr<const Data> StringData::operator>(const shared_ptr<const Data>& right) const
{
	if (right->type() == DataType::STRING){
		return Data::New(string() > right->string());
	}
	else{
		throw ResultValue::ERR_WHERE_OPERAND_TYPE;
	}
}

//! 以下比較を行います。
//! @param [in] right 右辺です。
//! @return 比較した結果です。
const shared_ptr<const Data> StringData::operator<=(const shared_ptr<const Data>& right) const
{
	if (right->type() == DataType::STRING){
		return Data::New(string() <= right->string());
	}
	else{
		throw ResultValue::ERR_WHERE_OPERAND_TYPE;
	}
}

//! 小さい比較を行います。
//! @param [in] right 右辺です。
//! @return 比較した結果です。
const shared_ptr<const Data> StringData::operator<(const shared_ptr<const Data>& right) const
{
	if (right->type() == DataType::STRING){
		return Data::New(string() < right->string());
	}
	else{
		throw ResultValue::ERR_WHERE_OPERAND_TYPE;
	}
}

//! AND演算を行います。
//! @param [in] right 右辺です。
//! @return 演算した結果です。
const shared_ptr<const Data> StringData::operator&&(const shared_ptr<const Data>& right) const
{
	throw ResultValue::ERR_WHERE_OPERAND_TYPE;
}

//! OR演算を行います。
//! @param [in] right 右辺です。
//! @return 演算した結果です。
const shared_ptr<const Data> StringData::operator||(const shared_ptr<const Data>& right) const
{
	throw ResultValue::ERR_WHERE_OPERAND_TYPE;
}

//! Dataクラスの新しいインスタンスを初期化します。
//! @param [in] value データの値です。
IntegerData::IntegerData(const int value) : m_integer(value){}

//! データの型を取得します。
//! @return データの型です。
const DataType IntegerData::type() const
{
	return DataType::INTEGER;
}

//! データが整数型の場合の値を取得します。
//! @return データが整数型の場合の値です。
const int IntegerData::integer() const
{
	return m_integer;
}

//! 加算演算を行います。
//! @param [in] right 右辺です。
//! @return 加算した結果です。
const shared_ptr<const Data> IntegerData::operator+(const shared_ptr<const Data>& right) const
{
	if (right->type() == DataType::INTEGER){
		return Data::New(integer() + right->integer());
	}
	else{
		throw ResultValue::ERR_WHERE_OPERAND_TYPE;
	}
}

//! 減算演算を行います。
//! @param [in] right 右辺です。
//! @return 減算した結果です。
const shared_ptr<const Data> IntegerData::operator-(const shared_ptr<const Data>& right) const
{
	if (right->type() == DataType::INTEGER){
		return Data::New(integer() - right->integer());
	}
	else{
		throw ResultValue::ERR_WHERE_OPERAND_TYPE;
	}
}

//! 乗算演算を行います。
//! @param [in] right 右辺です。
//! @return 乗算した結果です。
const shared_ptr<const Data> IntegerData::operator*(const shared_ptr<const Data>& right) const
{
	if (right->type() == DataType::INTEGER){
		return Data::New(integer() * right->integer());
	}
	else{
		throw ResultValue::ERR_WHERE_OPERAND_TYPE;
	}
}

//! 除算演算を行います。
//! @param [in] right 右辺です。
//! @return 除算した結果です。
const shared_ptr<const Data> IntegerData::operator/(const shared_ptr<const Data>& right) const
{
	if (right->type() == DataType::INTEGER){
		return Data::New(integer() / right->integer());
	}
	else{
		throw ResultValue::ERR_WHERE_OPERAND_TYPE;
	}
}

//! 等値比較を行います。
//! @param [in] right 右辺です。
//! @return 比較した結果です。
const shared_ptr<const Data> IntegerData::operator==(const shared_ptr<const Data>& right) const
{
	if (right->type() == DataType::INTEGER){
		return Data::New(integer() == right->integer());
	}
	else{
		throw ResultValue::ERR_WHERE_OPERAND_TYPE;
	}
}

//! 不等比較を行います。
//! @param [in] right 右辺です。
//! @return 比較した結果です。
const shared_ptr<const Data> IntegerData::operator!=(const shared_ptr<const Data>& right) const
{
	if (right->type() == DataType::INTEGER){
		return Data::New(integer() != right->integer());
	}
	else{
		throw ResultValue::ERR_WHERE_OPERAND_TYPE;
	}
}

//! 以上比較を行います。
//! @param [in] right 右辺です。
//! @return 比較した結果です。
const shared_ptr<const Data> IntegerData::operator>=(const shared_ptr<const Data>& right) const
{
	if (right->type() == DataType::INTEGER){
		return Data::New(integer() >= right->integer());
	}
	else{
		throw ResultValue::ERR_WHERE_OPERAND_TYPE;
	}
}

//! 大きい比較を行います。
//! @param [in] right 右辺です。
//! @return 比較した結果です。
const shared_ptr<const Data> IntegerData::operator>(const shared_ptr<const Data>& right) const
{
	if (right->type() == DataType::INTEGER){
		return Data::New(integer() > right->integer());
	}
	else{
		throw ResultValue::ERR_WHERE_OPERAND_TYPE;
	}
}

//! 以下比較を行います。
//! @param [in] right 右辺です。
//! @return 比較した結果です。
const shared_ptr<const Data> IntegerData::operator<=(const shared_ptr<const Data>& right) const
{
	if (right->type() == DataType::INTEGER){
		return Data::New(integer() <= right->integer());
	}
	else{
		throw ResultValue::ERR_WHERE_OPERAND_TYPE;
	}
}

//! 小さい比較を行います。
//! @param [in] right 右辺です。
//! @return 比較した結果です。
const shared_ptr<const Data> IntegerData::operator<(const shared_ptr<const Data>& right) const
{
	if (right->type() == DataType::INTEGER){
		return Data::New(integer() < right->integer());
	}
	else{
		throw ResultValue::ERR_WHERE_OPERAND_TYPE;
	}
}

//! AND演算を行います。
//! @param [in] right 右辺です。
//! @return 演算した結果です。
const shared_ptr<const Data> IntegerData::operator&&(const shared_ptr<const Data>& right) const
{
	throw ResultValue::ERR_WHERE_OPERAND_TYPE;
}

//! OR演算を行います。
//! @param [in] right 右辺です。
//! @return 演算した結果です。
const shared_ptr<const Data> IntegerData::operator||(const shared_ptr<const Data>& right) const
{
	throw ResultValue::ERR_WHERE_OPERAND_TYPE;
}

//! Dataクラスの新しいインスタンスを初期化します。
//! @param [in] value データの値です。
BooleanData::BooleanData(const bool value) : m_boolean(value){}

//! データの型を取得します。
//! @return データの型です。
const DataType BooleanData::type() const
{
	return DataType::BOOLEAN;
}

//! データが整数型の場合の値を取得します。
//! @return データが整数型の場合の値です。
const bool BooleanData::boolean() const
{
	return m_boolean;
}

//! 加算演算を行います。
//! @param [in] right 右辺です。
//! @return 加算した結果です。
const shared_ptr<const Data> BooleanData::operator+(const shared_ptr<const Data>& right) const
{
	throw ResultValue::ERR_WHERE_OPERAND_TYPE;
}

//! 減算演算を行います。
//! @param [in] right 右辺です。
//! @return 減算した結果です。
const shared_ptr<const Data> BooleanData::operator-(const shared_ptr<const Data>& right) const
{
	throw ResultValue::ERR_WHERE_OPERAND_TYPE;
}

//! 乗算演算を行います。
//! @param [in] right 右辺です。
//! @return 乗算した結果です。
const shared_ptr<const Data> BooleanData::operator*(const shared_ptr<const Data>& right) const
{
	throw ResultValue::ERR_WHERE_OPERAND_TYPE;
}

//! 除算演算を行います。
//! @param [in] right 右辺です。
//! @return 除算した結果です。
const shared_ptr<const Data> BooleanData::operator/(const shared_ptr<const Data>& right) const
{
	throw ResultValue::ERR_WHERE_OPERAND_TYPE;
}

//! 等値比較を行います。
//! @param [in] right 右辺です。
//! @return 比較した結果です。
const shared_ptr<const Data> BooleanData::operator==(const shared_ptr<const Data>& right) const
{
	throw ResultValue::ERR_WHERE_OPERAND_TYPE;
}

//! 不等比較を行います。
//! @param [in] right 右辺です。
//! @return 比較した結果です。
const shared_ptr<const Data> BooleanData::operator!=(const shared_ptr<const Data>& right) const
{
	throw ResultValue::ERR_WHERE_OPERAND_TYPE;
}

//! 以上比較を行います。
//! @param [in] right 右辺です。
//! @return 比較した結果です。
const shared_ptr<const Data> BooleanData::operator>=(const shared_ptr<const Data>& right) const
{
	throw ResultValue::ERR_WHERE_OPERAND_TYPE;
}

//! 大きい比較を行います。
//! @param [in] right 右辺です。
//! @return 比較した結果です。
const shared_ptr<const Data> BooleanData::operator>(const shared_ptr<const Data>& right) const
{
	throw ResultValue::ERR_WHERE_OPERAND_TYPE;
}

//! 以下比較を行います。
//! @param [in] right 右辺です。
//! @return 比較した結果です。
const shared_ptr<const Data> BooleanData::operator<=(const shared_ptr<const Data>& right) const
{
	throw ResultValue::ERR_WHERE_OPERAND_TYPE;
}

//! 小さい比較を行います。
//! @param [in] right 右辺です。
//! @return 比較した結果です。
const shared_ptr<const Data> BooleanData::operator<(const shared_ptr<const Data>& right) const
{
	throw ResultValue::ERR_WHERE_OPERAND_TYPE;
}

//! AND演算を行います。
//! @param [in] right 右辺です。
//! @return 演算した結果です。
const shared_ptr<const Data> BooleanData::operator&&(const shared_ptr<const Data>& right) const
{
	if (right->type() == DataType::BOOLEAN){
		return Data::New(boolean() && right->boolean());
	}
	else{
		throw ResultValue::ERR_WHERE_OPERAND_TYPE;
	}
}

//! OR演算を行います。
//! @param [in] right 右辺です。
//! @return 演算した結果です。
const shared_ptr<const Data> BooleanData::operator||(const shared_ptr<const Data>& right) const
{
	if (right->type() == DataType::BOOLEAN){
		return Data::New(boolean() || right->boolean());
	}
	else{
		throw ResultValue::ERR_WHERE_OPERAND_TYPE;
	}
}

//! 加算演算を行います。
//! @param [in] left 左辺です。
//! @param [in] right 右辺です。
//! @return 加算した結果です。
const shared_ptr<const Data> operator+(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right)
{
	return *left + right;
}

//! 減算演算を行います。
//! @param [in] left 左辺です。
//! @param [in] right 右辺です。
//! @return 減算した結果です。
const shared_ptr<const Data> operator-(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right)
{
	return *left - right;
}


//! 乗算演算を行います。
//! @param [in] left 左辺です。
//! @param [in] right 右辺です。
//! @return 乗算した結果です。
const shared_ptr<const Data> operator*(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right)
{
	return *left * right;
}

//! 除算演算を行います。
//! @param [in] left 左辺です。
//! @param [in] right 右辺です。
//! @return 除算した結果です。
const shared_ptr<const Data> operator/(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right)
{
	return *left / right;
}

//! 等値比較を行います。
//! @param [in] left 左辺です。
//! @param [in] right 右辺です。
//! @return 比較した結果です。
const shared_ptr<const Data> operator==(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right)
{
	return *left == right;
}

//! 不等比較を行います。
//! @param [in] left 左辺です。
//! @param [in] right 右辺です。
//! @return 比較した結果です。
const shared_ptr<const Data> operator!=(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right)
{
	return *left != right;
}

//! 以上比較を行います。
//! @param [in] left 左辺です。
//! @param [in] right 右辺です。
//! @return 比較した結果です。
const shared_ptr<const Data> operator>=(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right)
{
	return *left >= right;
}

//! 大きい比較を行います。
//! @param [in] left 左辺です。
//! @param [in] right 右辺です。
//! @return 比較した結果です。
const shared_ptr<const Data> operator>(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right)
{
	return *left > right;
}

//! 以下比較を行います。
//! @param [in] left 左辺です。
//! @param [in] right 右辺です。
//! @return 比較した結果です。
const shared_ptr<const Data> operator<=(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right)
{
	return *left <= right;
}

//! 小さい比較を行います。
//! @param [in] left 左辺です。
//! @param [in] right 右辺です。
//! @return 比較した結果です。
const shared_ptr<const Data> operator<(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right)
{
	return *left < right;
}

//! AND演算を行います。
//! @param [in] left 左辺です。
//! @param [in] right 右辺です。
//! @return 演算した結果です。
const shared_ptr<const Data> operator&&(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right)
{
	return *left && right;
}

//! OR演算を行います。
//! @param [in] left 左辺です。
//! @param [in] right 右辺です。
//! @return 演算した結果です。
const shared_ptr<const Data> operator||(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right)
{
	return *left || right;
}

//! Operatorクラスの新しいインスタンスを初期化します。
//! @param [in] kind 演算子の種類を、演算子を記述するトークンの種類で表します。
//! @param [in] order 演算子の優先順位です。
Operator::Operator(const TokenKind &kind, const int &order) : m_kind(kind), m_order(order)
{
}

//! 演算子の種類を、演算子を記述するトークンの種類で表したものを取得します。
//! @return 演算子の種類を、演算子を記述するトークンの種類で表したもの。
const TokenKind& Operator::kind() const
{
	return m_kind;
}

//! 演算子の優先順位を取得します。
//! 演算子の優先順位です。
const int& Operator::order() const
{
	return m_order;
}

//! Tokenクラスの新しいインスタンスを初期化します。
//! @param [in] kind トークンの種類です。
Token::Token(const TokenKind &kind) : Token(kind, "")
{
}

//! Tokenクラスの新しいインスタンスを初期化します。
//! @param [in] kind トークンの種類です。
//! @param [in] word 記録されているトークンの文字列です。記録の必要がなければ空白です。
Token::Token(const TokenKind& kind, const string& word) :m_kind(kind), m_word(word)
{
}

//! トークンの種類を取得します。
//! @return トークンの種類です。
const TokenKind& Token::kind() const
{
	return m_kind;
}

//! 記録されているトークンの文字列を取得します。記録の必要がなければ空白です。
//! @return 記録されているトークンの文字列です。
const string& Token::word() const
{
	return m_word;
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

//! OutputAllDataRowクラスの新しいインスタンスを初期化します。
//! @param [in] data OutputAllDataRowの持つデータです。
OutputAllDataRow::OutputAllDataRow(const shared_ptr<const vector<const shared_ptr<const Data>>> data) : data(data)
{}

//! 列を指定し、データを取得します。事前にSetAllColumnsを実行しておく必要があります。
//! @param [in] column 取得したい列。
//! @return 指定したデータ。
const shared_ptr<const Data> OutputAllDataRow::operator[](const Column &column) const
{
	return (*data)[column.allColumnsIndex];
}

//! ExtensionTreeNodeクラスの新しいインスタンスを初期化します。
ExtensionTreeNode::ExtensionTreeNode() : middleOperator(TokenKind::NOT_TOKEN, 0)
{
}

// leftとrightをmiddleOperatorで演算します。
void ExtensionTreeNode::Operate()
{
	// 自ノードより前に子ノードを演算しておきます。
	if (left){
		left->Operate();
	}
	if (right){
		right->Operate();
	}

	// 自ノードの値を計算します。
	switch (middleOperator.kind()){
	case TokenKind::PLUS:
		value = left->value + right->value;
		break;
	case TokenKind::MINUS:
		value = left->value - right->value;
		break;
	case TokenKind::ASTERISK:
		value = left->value * right->value;
		break;
	case TokenKind::SLASH:
		value = left->value / right->value;
		break;
	case TokenKind::EQUAL:
		value = left->value == right->value;
		break;
	case TokenKind::GREATER_THAN:
		value = left->value > right->value;
		break;
	case TokenKind::GREATER_THAN_OR_EQUAL:
		value = left->value >= right->value;
		break;
	case TokenKind::LESS_THAN:
		value = left->value < right->value;
		break;
	case TokenKind::LESS_THAN_OR_EQUAL:
		value = left->value <= right->value;
		break;
	case TokenKind::NOT_EQUAL:
		value = left->value != right->value;
		break;
	case TokenKind::AND:
		value = left->value && right->value;
		break;
	case TokenKind::OR:
		value = left->value || right->value;
		break;
	}
}

//! カラム名で指定されたデータを持つノードかどうかです。
//! @return カラム名で指定されたデータを持つノードかどうか。
bool ExtensionTreeNode::isDataNodeAsColumnName()
{
	return middleOperator.kind() == TokenKind::NOT_TOKEN && !column.columnName.empty();
}

//! 実際に出力する行に合わせて列にデータを設定します。
//! @param [in] 実際に出力する行です。
void ExtensionTreeNode::SetColumnData(const OutputAllDataRow &outputRow)
{
	if (isDataNodeAsColumnName()){
		value = outputRow[column];

		// 符号を考慮して値を計算します。
		if (value->type() == DataType::INTEGER){
			value = Data::New(value->integer() * signCoefficient);
		}
	}
}

//! 次のノードを指定する関数を指定し、ノードの列を生成します。
//! @param [in] nextNode 現在のノードから次のノードを指定する関数です。
//! @return thisから次のノードをたどっていき、ノードがnullptrになる前までのすべてのノードの列。
const shared_ptr<vector<const shared_ptr<ExtensionTreeNode>>> ExtensionTreeNode::allNodesOf(function<const shared_ptr<ExtensionTreeNode>(const shared_ptr<const ExtensionTreeNode>)> nextNode, bool includeSelf) const
{
	auto returnValue = make_shared<vector<const shared_ptr<ExtensionTreeNode>>>();
	if (includeSelf){
		returnValue->push_back(const_pointer_cast<ExtensionTreeNode>(shared_from_this()));
	}
	for (auto current = nextNode(shared_from_this()); current; current = nextNode(current)){
		returnValue->push_back(current);
	}
	return returnValue;
}

//! 自身の祖先ノードを自身に近いほうから順に列挙します。
//! @return 祖先ノードの一覧。
const shared_ptr<vector<const shared_ptr<ExtensionTreeNode>>> ExtensionTreeNode::ancestors() const
{
	return allNodesOf([](const shared_ptr<const ExtensionTreeNode> thisNode){return thisNode->parent; });
}

//! 自身の子孫ノードをずっと左に辿っていき自身に近いほうから順に列挙します。
//! @return 左に辿った子孫ノードの一覧。
const shared_ptr<vector<const shared_ptr<ExtensionTreeNode>>> ExtensionTreeNode::allLeftList() const
{
	return allNodesOf([](const shared_ptr<const ExtensionTreeNode> thisNode){return thisNode->left; });
}

//! 自身及び自身の祖先ノードを自身に近いほうから順に列挙します。
//! @return 祖先ノードの一覧。
const shared_ptr<vector<const shared_ptr<ExtensionTreeNode>>> ExtensionTreeNode::selfAndAncestors() const
{
	return allNodesOf([](const shared_ptr<const ExtensionTreeNode> thisNode){return thisNode->parent; }, true);
}

//! 自身及び自身の子孫ノードをずっと左に辿っていき自身に近いほうから順に列挙します。
//! @return 左に辿った子孫ノードの一覧。
const shared_ptr<vector<const shared_ptr<ExtensionTreeNode>>> ExtensionTreeNode::selfAndAllLeftList() const
{
	return allNodesOf([](const shared_ptr<const ExtensionTreeNode> thisNode){return thisNode->left; }, true);
}

//! 自身の位置にノードを挿入し、自身は挿入したノードの左の子となります。
//! @param [in] inserted 挿入するノードです。
void ExtensionTreeNode::InsertAndMoveLeft(const shared_ptr<ExtensionTreeNode> inserted)
{
	inserted->parent = parent;
	if (inserted->parent){
		inserted->parent->right = inserted;
	}
	inserted->left = shared_from_this();
	parent = inserted;
}

//! 自身の右の子としてノードを挿入します。
//! @param [in] inserted 挿入するノードです。
void ExtensionTreeNode::InsertRight(const shared_ptr<ExtensionTreeNode> inserted)
{
	right = inserted;
	right->parent = shared_from_this();
}

//! 引数として渡したノード及びその子孫のノードを取得します。
//! @param [in] 戻り値のルートとなるノードです。順序は帰りがけ順です。
//! @return 自身及び子孫のノードです。
const shared_ptr<vector<const shared_ptr<ExtensionTreeNode>>> SelfAndDescendants(shared_ptr<ExtensionTreeNode> self)
{
	auto selfAndDescendants = make_shared<vector<const shared_ptr<ExtensionTreeNode>>>();
	if (self->left){
		auto descendants = SelfAndDescendants(self->left);
		copy(
			descendants->begin(),
			descendants->end(),
			back_inserter(*selfAndDescendants));
	}
	if (self->right){
		auto descendants = SelfAndDescendants(self->right);
		copy(
			descendants->begin(),
			descendants->end(),
			back_inserter(*selfAndDescendants));
	}
	selfAndDescendants->push_back(self);
	return selfAndDescendants;
}

//! Orderクラスの新しいインスタンスを初期化します。
//! @param [in] column ORDER句に指定された列名です。
//! @param [in] isAsc ORDER国指定された順序が昇順かどうかです。
Order::Order(Column column, const bool isAsc) : column(column), isAsc(isAsc){}

//! 全てが数値となる列は数値列に変換します。
void InputTable::InitializeIntegerColumn()
{
	for (size_t i = 0; i < columns()->size(); ++i){

		// 全ての行のある列について、データ文字列から符号と数値以外の文字を探します。
		if (none_of(
			data()->begin(),
			data()->end(),
			[&](const vector<const shared_ptr<const Data>> &inputRow){
			return
				inputRow[i]->type() == DataType::STRING &&
				any_of(
				inputRow[i]->string().begin(),
				inputRow[i]->string().end(),
				[&](const char& c){return signNum.find(c) == string::npos; }); })){

			// 符号と数字以外が見つからない列については、数値列に変換します。
			for (auto& inputRow : *data()){
				inputRow[i] = Data::New(stoi(inputRow[i]->string()));
			}
		}
	}
}

//! InputTableクラスの新しいインスタンスを初期化します。
//! @param [in] columns 読み込んだヘッダ情報です。
//! @param [in] data 読み込んだデータです。
InputTable::InputTable(const shared_ptr<const vector<const Column>> columns, const shared_ptr<vector<const vector<const shared_ptr<const Data>>>> data) : m_columns(columns), m_data(data)
{
	InitializeIntegerColumn();
}

//! 列の情報を取得します。
//! @return データです。
const shared_ptr<const vector<const Column>> InputTable::columns() const
{
	return m_columns;
}

//! データを取得します。
//! @return 列の情報です。
const shared_ptr<vector<const vector<const shared_ptr<const Data>>>> InputTable::data() const
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
		mismatch(keyword.word().begin(), keyword.word().end(), cursol,
		[](const char keywordChar, const char sqlChar){return keywordChar == toupper(sqlChar); });

	if (result.first == keyword.word().end() && // キーワードの最後の文字まで同じです。
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

//! TokenParserクラスの新しいインスタンスを初期化します。
//! @param [in] 読み取りが成功したら実行する処理です。
//! @params [in] kind 読み取るトークンの種類です。
TokenParser::TokenParser(function<void(const Token)> action, const vector<const TokenKind> kinds) : m_action(action), m_kinds(kinds){}

//! TokenParserクラスの新しいインスタンスを初期化します。
//! @params [in] kind 読み取るトークンの種類です。
TokenParser::TokenParser(const vector<const TokenKind> kinds) : m_kinds(kinds){}

//! TokenParserクラスの新しいインスタンスを初期化します。
//! @params [in] kind 読み取るトークンの種類です。
TokenParser::TokenParser(TokenKind kind) : m_kinds({ kind }){}

//! トークンに対するパースを行います。
//! @params [in] cursol 現在の読み取り位置を表すカーソルです。
//! @return パースが成功したかどうかです。
const bool TokenParser::Parse(vector<const Token>::const_iterator& cursol, vector<const Token>::const_iterator& end) const
{
	if (cursol == end){
		return false;
	}
	return any_of(m_kinds.begin(), m_kinds.end(),[&](const TokenKind &kind){
		if (cursol->kind() == kind){
			if (m_action){
				m_action(*cursol);
			}
			++cursol;
			return true;
		}
		else{
			return false;
		}
	});
}
//! 読み取りが成功したら実行する処理を登録します。
//! @param [in] 読み取りが成功したら実行する処理です。
const shared_ptr<const TokenParser> TokenParser::Action(const function<void(const Token)> action) const
{
	return make_shared<TokenParser>(action, m_kinds);
}

//! トークンのパーサーを生成します。
//! @params [in] kind 読み取るトークンの種類です。
const shared_ptr<TokenParser> token(TokenKind kind)
{
	return make_shared<TokenParser>(kind);
}

//! 二つのTokenPerserを元に複数の種類をとるTokenParserクラスの新しいインスタンスを初期化します。
//! @params [in] parser1 元のParserです。
//! @params [in] parser2 追加する種類ののParserです。
const shared_ptr<const TokenParser> TokenParser::or(const shared_ptr<const TokenParser> parser) const
{
	vector<const TokenKind> newKinds(m_kinds);
	copy(parser->m_kinds.begin(), parser->m_kinds.end(), back_inserter(newKinds));
	return make_shared<TokenParser>(newKinds);
}

//! EndParserクラスの新しいインスタンスを初期化します。
//! @param [in] 読み取りが成功したら実行する処理です。
EndParser::EndParser(const function<void(void)> action) : m_action(action){}

//! EndParserクラスの新しいインスタンスを初期化します。
EndParser::EndParser(){}

//! トークンに対するパースを行います。
//! @params [in] cursol 現在の読み取り位置を表すカーソルです。
//! @return パースが成功したかどうかです。
const bool EndParser::Parse(vector<const Token>::const_iterator& cursol, vector<const Token>::const_iterator& end) const
{
	if (cursol == end){
		if (m_action){
			m_action();
		}
		return true;
	}
	return false;
}

//! 読み取りが成功したら実行する処理を登録します。
//! @param [in] 読み取りが成功したら実行する処理です。
const shared_ptr<const EndParser> EndParser::Action(const function<void()> action) const
{
	return make_shared<EndParser>(action);
}

//! トークンのパーサーを生成します。
const shared_ptr<EndParser> end()
{
	return make_shared<EndParser>();
}

//! NoTokenParserクラスの新しいインスタンスを初期化します。
//! @param [in] 読み取りが成功したら実行する処理です。
NoTokenParser::NoTokenParser(const function<void(void)> action) : m_action(action){}

//! トークンに対するパースを行います。
//! @params [in] cursol 現在の読み取り位置を表すカーソルです。
//! @return パースが成功したかどうかです。
const bool NoTokenParser::Parse(vector<const Token>::const_iterator& cursol, vector<const Token>::const_iterator& end) const
{
	if (m_action){
		m_action();
	}
	return true;
}

//! 読み取りが成功したら実行する処理を登録します。
//! @param [in] 読み取りが成功したら実行する処理です。
const shared_ptr<const NoTokenParser> NoTokenParser::Action(const function<void(void)> action) const
{
	return make_shared<NoTokenParser>(action);
}

//! 何も読み取らずにアクションだけ実行するパーサーを生成します。
const shared_ptr<NoTokenParser> action(function<void(void)> action)
{
	return make_shared<NoTokenParser>(action);
}

//! SequenceParserクラスの新しいインスタンスを初期化します。
//! @param [in] 読み取りが成功したら実行する処理です。
//! @params [in] parser1 一つ目のParserです。
//! @params [in] parser1 一つ目のParserです。
SequenceParser::SequenceParser(const function<void(void)> action, const shared_ptr<const Parser> parser1, const shared_ptr<const Parser> parser2) : m_action(action), m_parser1(parser1), m_parser2(parser2){}

//! SequenceParserクラスの新しいインスタンスを初期化します。
//! @params [in] parser1 一つ目のParserです。
//! @params [in] parser1 一つ目のParserです。
SequenceParser::SequenceParser(const shared_ptr<const Parser> parser1, const shared_ptr<const Parser> parser2) : m_parser1(parser1), m_parser2(parser2){}

//! トークンに対するパースを行います。
//! @params [in] cursol 現在の読み取り位置を表すカーソルです。
//! @return パースが成功したかどうかです。
const bool SequenceParser::Parse(vector<const Token>::const_iterator& cursol, vector<const Token>::const_iterator& end) const
{
	auto beforeParse = cursol;
	if (m_parser1->Parse(cursol, end) && m_parser2->Parse(cursol, end)){
		if (m_action){
			m_action();
		}
		return true;
	}
	cursol = beforeParse;
	return false;
}

//! 読み取りが成功したら実行する処理を登録します。
//! @param [in] 読み取りが成功したら実行する処理です。
const shared_ptr<const SequenceParser> SequenceParser::Action(const function<void(void)> action) const
{
	return make_shared<SequenceParser>(action, m_parser1, m_parser2);
}

//! SequenceParserクラスの新しいインスタンスを生成します。
//! @params [in] parser1 一つ目のParserです。
//! @params [in] parser2 二つ目のParserです。
const shared_ptr<const SequenceParser> operator>>(const shared_ptr<const Parser> parser1, const shared_ptr<const Parser> parser2)
{
	return make_shared<SequenceParser>(parser1, parser2);
}

//! SequenceParserクラスの新しいインスタンスを初期化します。
//! @param [in] 読み取りが成功したら実行する処理です。
//! @params [in] parser1 一つ目のParserです。
//! @params [in] parser2 二つ目目のParserです。
OrderedChoiceParser::OrderedChoiceParser(const function<void(void)> action, const shared_ptr<const Parser> parser1, const shared_ptr<const Parser> parser2) : m_action(action), m_parser1(parser1), m_parser2(parser2){}


//! SequenceParserクラスの新しいインスタンスを初期化します。
//! @params [in] parser1 一つ目のParserです。
//! @params [in] parser2 二つ目のParserです。
OrderedChoiceParser::OrderedChoiceParser(const shared_ptr<const Parser> parser1, const shared_ptr<const Parser> parser2) : m_parser1(parser1), m_parser2(parser2){}

//! 読み取りが成功したら実行する処理を登録します。
//! @param [in] 読み取りが成功したら実行する処理です。
const shared_ptr<const OrderedChoiceParser> OrderedChoiceParser::Action(const function<void(void)> action) const
{
	return make_shared<OrderedChoiceParser>(action, m_parser1, m_parser2);
}

//! 二つの規則に対するパースを行います。
//! @params [in] cursol 現在の読み取り位置を表すカーソルです。
//! @return パースが成功したかどうかです。
const bool OrderedChoiceParser::Parse(vector<const Token>::const_iterator& cursol, vector<const Token>::const_iterator& end) const
{
	if (m_parser1->Parse(cursol, end) || m_parser2->Parse(cursol, end)){
		if (m_action){
			m_action();
		}
		return true;
	}
	return false;
}

//! OrderedChoiceParserクラスの新しいインスタンスを生成します。
//! @params [in] parser1 一つ目のParserです。
//! @params [in] parser2 二つ目のParserです。
const shared_ptr<const OrderedChoiceParser> operator|(const shared_ptr<const Parser> parser1, const shared_ptr<const Parser> parser2)
{
	return make_shared<OrderedChoiceParser>(parser1, parser2);
}

//! OptionalParserクラスの新しいインスタンスを初期化します。
//! @param [in] 読み取りが成功したら実行する処理です。
//! @params [in] optional 存在してもしなくてもよい規則です。
OptionalParser::OptionalParser(const function<void(void)> action, const shared_ptr<const Parser> optional) :m_action(action), m_optional(optional) {}

//! OptionalParserクラスの新しいインスタンスを初期化します。
//! @params [in] optional 存在してもしなくてもよい規則です。
OptionalParser::OptionalParser(const shared_ptr<const Parser> optional) : m_optional(optional){}

//! 読み取りが成功したら実行する処理を登録します。
//! @param [in] 読み取りが成功したら実行する処理です。
const shared_ptr<const OptionalParser> OptionalParser::Action(const function<void(void)> action) const
{
	return make_shared<OptionalParser>(action, m_optional);
}

//! オプショナルなパースを行います。
//! @params [in] cursol 現在の読み取り位置を表すカーソルです。
//! @return パースが成功したかどうかです。
const bool OptionalParser::Parse(vector<const Token>::const_iterator& cursol, vector<const Token>::const_iterator& end) const
{
	auto beforeParse = cursol;
	if (m_optional->Parse(cursol, end)){
		if (m_action){
			m_action();
		}
	}
	else{
		cursol = beforeParse;
	}
	return true;
}
//! OptionalParserクラスの新しいインスタンスを生成します。
//! @params [in] optional 存在してもしなくてもよい規則です。
const shared_ptr<const OptionalParser> operator-(const shared_ptr<const Parser> optional)
{
	return make_shared<OptionalParser>(optional);
}

//! ZeroOrMoreParserクラスの新しいインスタンスを初期化します。
//! @param [in] 読み取りが成功したら実行する処理です。
//! @params [in] once 繰り返しの一回分となる規則です。
ZeroOrMoreParser::ZeroOrMoreParser(const function<void(void)> action, const shared_ptr<const Parser> once) :m_action(action), m_once(once){}

//! ZeroOrMoreParserクラスの新しいインスタンスを初期化します。
//! @params [in] once 繰り返しの一回分となる規則です。
ZeroOrMoreParser::ZeroOrMoreParser(const shared_ptr<const Parser> once): m_once(once){}

//! 読み取りが成功したら実行する処理を登録します。
//! @param [in] 読み取りが成功したら実行する処理です。
const shared_ptr<const ZeroOrMoreParser> ZeroOrMoreParser::Action(const function<void(void)> action) const
{
	return make_shared<ZeroOrMoreParser>(action, m_once);
}

//! 繰り返しのパースを行います。
//! @params [in] cursol 現在の読み取り位置を表すカーソルです。
//! @return パースが成功したかどうかです。
const bool ZeroOrMoreParser::Parse(vector<const Token>::const_iterator& cursol, vector<const Token>::const_iterator& end) const
{
	while (m_once->Parse(cursol, end)){}
	if (m_action){
		m_action();
	}
	return true;
}

//! ZeroOrMoreParserクラスの新しいインスタンスを生成します。
//! @params [in] once 繰り返しの一回分となる規則です。
const shared_ptr<const Parser> operator~(const shared_ptr<const Parser> once)
{
	return make_shared<ZeroOrMoreParser>(once);
}

//! AndPredicateParserクラスの新しいインスタンスを初期化します。
//! @param [in] 先読みを実行したら実行する処理です。先読みが成功したかどうかを受け取ります。
//! @params [in] parser 先読みする規則です。
AndPredicateParser::AndPredicateParser(const function<void(bool)> action, const shared_ptr<const Parser> parser) :m_action(action), m_parser(parser){}

//! ZeroOrMoreParserクラスの新しいインスタンスを初期化します。
//! @params [in] parser 先読みする規則です。
AndPredicateParser::AndPredicateParser(const shared_ptr<const Parser> parser) : m_parser(parser){}

//! 読み取りが成功したら実行する処理を登録します。
//! @param [in] 先読みを実行したら実行する処理です。先読みが成功したかどうかを受け取ります。
const shared_ptr<const AndPredicateParser> AndPredicateParser::Action(const function<void(bool)> action) const
{
	return make_shared<AndPredicateParser>(action, m_parser);
}

//! 繰り返しのパースを行います。
//! @params [in] cursol 現在の読み取り位置を表すカーソルです。
//! @return パースが成功したかどうかです。
const bool AndPredicateParser::Parse(vector<const Token>::const_iterator& cursol, vector<const Token>::const_iterator& end) const
{
	auto beforeParse = cursol;
	if (m_parser->Parse(cursol, end)){
		if (m_action){
			m_action(true);
		}
		cursol = beforeParse;
		return true;
	}
	else{
		if (m_action){
			m_action(false);
		}
		cursol = beforeParse;
		return false;
	}
}

//! AndPredicateParserクラスの新しいインスタンスを初期化します。
//! @params [in] parser 先読みする規則です。
const shared_ptr<const AndPredicateParser> operator&(const shared_ptr<const Parser> parser)
{
	return make_shared<AndPredicateParser>(parser);
}

//! 入力ファイルに書いてあったすべての列をallInputColumnsに設定します。
void OutputData::InitializeAllInputColumns()
{
	for (auto &inputTable : inputTables){
		copy(
			inputTable.columns()->begin(),
			inputTable.columns()->end(),
			back_inserter(allInputColumns));
	}
}

//! WHEREやORDER BYを適用していないすべての行を取得します。
//! @return すべてのデータ行。入力されたすべての入力データを保管します。
const shared_ptr<vector<const OutputAllDataRow>> OutputData::GetAllRows() const
{
	auto outputRows = make_shared<vector<const OutputAllDataRow>>();
	auto currentRowsPtr = GetInitializedCurrentRows();
	auto &currentRows = *currentRowsPtr;

	// 出力するデータを設定します。
	while (true){
		auto outputRow = make_shared<vector<const shared_ptr<const Data>>>();// WHEREやORDERのためにすべての情報を含む行。rowとインデックスを共有します。

		// outputRowの列を設定します。
		for (auto &currentRow : currentRows){
			copy(
				currentRow->begin(),
				currentRow->end(),
				back_inserter(*outputRow));
		}
		outputRows->push_back(OutputAllDataRow(outputRow));

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
	return outputRows;
}

//! データに対してWHERE句を適用します。
//! @params [in] outputRows 適用されるデータ。
void OutputData::ApplyWhere(vector<const OutputAllDataRow> &outputRows) const
{
	// WHERE条件を適用します。
	if (queryInfo.whereTopNode){
		auto & newEnd = copy_if(
			outputRows.begin(),
			outputRows.end(),
			outputRows.begin(),
			[&](const OutputAllDataRow row){
			auto allNodes = SelfAndDescendants(queryInfo.whereTopNode);
			for (auto& node : *allNodes){
				node->SetColumnData(row);
			}
			queryInfo.whereTopNode->Operate();
			return queryInfo.whereTopNode->value->boolean();
		});
		outputRows.erase(newEnd, outputRows.end());
	}
}

//! データに対してORDER BY句を適用します。
//! @params [in] outputRows 適用されるデータ。
void OutputData::ApplyOrderBy(vector<const OutputAllDataRow> &outputRows) const
{
	// ORDER句による並び替えの処理を行います。
	if (!queryInfo.orders.empty()){
		sort(
			outputRows.begin(),
			outputRows.end(),
			[&](const OutputAllDataRow& lRow, const OutputAllDataRow& rRow){
			for (auto &order : queryInfo.orders){
				auto &lData = lRow[order.column]; // インデックスがminIndexのデータです。
				auto &rData = rRow[order.column]; // インデックスがjのデータです。
				int cmp = 0; // 比較結果です。等しければ0、インデックスjの行が大きければプラス、インデックスminIndexの行が大きければマイナスとなります。
				switch (lData->type())
				{
				case DataType::INTEGER:
					cmp = lData->integer() - rData->integer();
					break;
				case DataType::STRING:
					cmp = strcmp(lData->string().c_str(), rData->string().c_str());
					break;
				}

				// 降順ならcmpの大小を入れ替えます。
				if (!order.isAsc){
					cmp *= -1;
				}
				if (cmp != 0){
					return cmp < 0;
				}
			}
			return false;
		});
	}
}


//! SELECT句の列名指定が*だった場合は、入力CSVの列名がすべて選択されます。
void OutputData::OpenSelectAsterisk()
{
	if (queryInfo.selectColumns.empty()){
		copy(allInputColumns.begin(), allInputColumns.end(), back_inserter(queryInfo.selectColumns));
	}
}

//! 利用する列名が、何個目の入力ファイルの何列目に相当するかを判別します。
void OutputData::SetAllColumns()
{
	for (auto &selectColumn : queryInfo.selectColumns){
		selectColumn.SetAllColumns(inputTables);
	}
	if (queryInfo.whereTopNode){
		auto allWhereNode = SelfAndDescendants(queryInfo.whereTopNode);
		for (auto& node : *allWhereNode){
			if (!node->column.columnName.empty()){
				node->column.SetAllColumns(inputTables);
			}
		}
	}
	for (auto &order : queryInfo.orders){
		order.column.SetAllColumns(inputTables);
	}
}

//! OutputDataクラスの新しいインスタンスを初期化します。
//! @param [in] queryInfo SQLの情報です。
OutputData::OutputData(const SqlQueryInfo queryInfo, const vector<const InputTable> &inputTables) : queryInfo(queryInfo), inputTables(inputTables)
{
	InitializeAllInputColumns();
	OpenSelectAsterisk();
	SetAllColumns();
}

//! 入力された各テーブルの、現在出力している行を指すカーソルを、初期化された状態で取得します。
//! @return 初期化されたカーソルです。
const shared_ptr<vector<vector<const vector<const shared_ptr<const Data>>>::const_iterator>> OutputData::GetInitializedCurrentRows() const
{
	auto currentRows = make_shared<vector<vector<const vector<const shared_ptr<const Data>>>::const_iterator>>();
	transform(
		inputTables.begin(),
		inputTables.end(),
		back_inserter(*currentRows),
		[](const InputTable& table){return table.data()->begin(); });

	return currentRows;
}

//! 出力するカラム名を取得します。
//! @return 出力するカラム名です。
const vector<Column> OutputData::columns() const
{
	return queryInfo.selectColumns;
}

//! 出力するすべてのデータ行を取得します。
//! @return 出力するすべてのデータ行。入力されたすべての入力データを保管します。
const shared_ptr<const vector<const OutputAllDataRow>> OutputData::outputRows() const
{
	auto outputRows = GetAllRows();
	ApplyWhere(*outputRows);

	ApplyOrderBy(*outputRows);

	return outputRows;
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
const shared_ptr<vector<const vector<const shared_ptr<const Data>>>> Csv::ReadData(ifstream &inputFile) const
{
	auto data = make_shared<vector<const vector<const shared_ptr<const Data>>>>(); // 読み込んだデータの一覧。

	while (auto lineData = ReadLineData(inputFile)){
		vector<const shared_ptr<const Data>> row;
		transform(
			lineData->begin(),
			lineData->end(),
			back_inserter(row),
			[&](const string& column){return Data::New(column); });
		data->push_back(row);
	}
	return data;
}

//! 出力ファイルを開きます。
//! @param [in] outputFileName 開くファイルのファイルパスです。
//! @return 出力ファイルを扱うストリームです。
ofstream Csv::OpenOutputFile(const string outputFileName) const
{
	ofstream outputFile(outputFileName);
	if (outputFile.bad()){
		throw ResultValue::ERR_FILE_OPEN;
	}
	return outputFile;
}

//! 出力ファイルを閉じます。
//! @param [in] OutputFile 入力ファイルを扱うストリームです。
void Csv::CloseOutputFile(ofstream &outputFile) const
{
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

//! 入力CSVのヘッダ行を読み込みます。
//! @param [in] OutputFile 出力ファイルを扱うストリームです。開いた後何も読み込んでいません。
//! @param [in] columns 出力するヘッダ情報です。
void Csv::WriteHeader(ofstream &outputFile, const vector<Column> &columns) const
{
	for (auto it = columns.begin(); it != columns.end(); ++it){
		outputFile << it->outputName;
		if (it != columns.end() - 1){
			outputFile << ",";
		}
		else{
			outputFile << "\n";
		}
	}
}

//! 入力CSVのデータ行を読み込みます。
//! @param [in] OutputFile 出力ファイルを扱うストリームです。すでにヘッダのみを読み込んだ後です。
//! columns [in] 出力するデータです。
void Csv::WriteData(ofstream &outputFile, const OutputData &data) const
{
	auto outputRows = data.outputRows();

	auto a = outputRows;
	for (auto& outputRow : *a){
		size_t i = 0;
		for (const auto &column : data.columns()){
			switch (outputRow[column]->type()){
			case DataType::INTEGER:
				outputFile << outputRow[column]->integer();
				break;
			case DataType::STRING:
				outputFile << outputRow[column]->string();
				break;
			}
			if (i++ < data.columns().size() - 1){
				outputFile << ",";
			}
			else{
				outputFile << "\n";
			}
		}
	}
}

//! Csvクラスの新しいインスタンスを初期化します。
//! @param [in] queryInfo SQLに記述された内容です。
Csv::Csv(const shared_ptr<const SqlQueryInfo> queryInfo) : queryInfo(queryInfo){}

//! CSVファイルから入力データを読み取ります。
//! @return ファイルから読み取ったデータです。
const shared_ptr<const vector<const InputTable>> Csv::Read() const
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

//! CSVファイルに出力データを書き込みます。
//! @param [in] outputFileName 結果を出力するファイルのファイル名です。
//! @param [in] queryInfo SQLの情報です。
//! @param [in] inputTables ファイルから読み取ったデータです。
void Csv::Write(const string outputFileName, const vector<const InputTable> &inputTables) const
{
	OutputData outputData(*queryInfo, inputTables); // 出力するデータです。

	auto outputFile = OpenOutputFile(outputFileName); // 書き込むファイルのファイルストリームです。
	
	WriteHeader(outputFile, outputData.columns());

	WriteData(outputFile, outputData);

	CloseOutputFile(outputFile);
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

	auto AND = token(TokenKind::AND);// ANDキーワードトークンのパーサーです。
	auto ASC = token(TokenKind::ASC);// ASCキーワードトークンのパーサーです。
	auto BY = token(TokenKind::BY);// BYキーワードトークンのパーサーです。
	auto DESC = token(TokenKind::DESC);// DESCキーワードトークンのパーサーです。
	auto FROM = token(TokenKind::FROM);// FROMキーワードトークンのパーサーです。
	auto OR = token(TokenKind::OR);// ORキーワードトークンのパーサーです。
	auto ORDER = token(TokenKind::ORDER);// ORDERキーワードトークンのパーサーです。
	auto SELECT = token(TokenKind::SELECT);// SELECTキーワードトークンのパーサーです。
	auto WHERE = token(TokenKind::WHERE);// WHEREキーワードトークンのパーサーです。
	auto ASTERISK = token(TokenKind::ASTERISK); // ＊ 記号トークンのパーサーです。
	auto COMMA = token(TokenKind::COMMA); // ， 記号ですトークンのパーサーです。
	auto CLOSE_PAREN = token(TokenKind::CLOSE_PAREN); // ） 記号トークンのパーサーです。
	auto DOT = token(TokenKind::DOT); // ． 記号トークンのパーサーです。
	auto EQUAL = token(TokenKind::EQUAL); // ＝ 記号トークンのパーサーです。
	auto GREATER_THAN = token(TokenKind::GREATER_THAN); // ＞ 記号トークンのパーサーです。
	auto GREATER_THAN_OR_EQUAL = token(TokenKind::GREATER_THAN_OR_EQUAL); // ＞＝ 記号トークンのパーサーです。
	auto LESS_THAN = token(TokenKind::LESS_THAN); // ＜ 記号トークンのパーサーです。
	auto LESS_THAN_OR_EQUAL = token(TokenKind::LESS_THAN_OR_EQUAL); // ＜＝ 記号トークンのパーサーです。
	auto MINUS = token(TokenKind::MINUS); // － 記号トークンのパーサーです。
	auto NOT_EQUAL = token(TokenKind::NOT_EQUAL); // ＜＞ 記号トークンのパーサーです。
	auto OPEN_PAREN = token(TokenKind::OPEN_PAREN); // （ 記号トークンのパーサーです。
	auto PLUS = token(TokenKind::PLUS); // ＋ 記号トークンのパーサーです。
	auto SLASH = token(TokenKind::SLASH); // ／ 記号トークンのパーサーです。
	auto IDENTIFIER = token(TokenKind::IDENTIFIER); // 識別子トークンのパーサーです。
	auto INT_LITERAL = token(TokenKind::INT_LITERAL); // 整数リテラルトークンのパーサーです。
	auto STRING_LITERAL = token(TokenKind::STRING_LITERAL); // 文字列リテラルトークンのパーサーです。

	Column column; // 現在読み込んでいるORDER BY句での列を保持します。

	// 列指定の一つ目の識別子のパーサーです。
	auto FIRST_COLUMN_NAME = IDENTIFIER->Action([&](const Token token){
		// テーブル名が指定されていない場合と仮定して読み込みます。
		column = Column(token.word());
	});

	// 列指定の二つ目の識別子のパーサーです。
	auto SECOND_COLUMN_NAME = IDENTIFIER->Action([&](const Token token){
		// テーブル名が指定されていることがわかったので読み替えます。
		column = Column(column.columnName, token.word());
	});

	auto COLUMN = FIRST_COLUMN_NAME >> -(DOT >> SECOND_COLUMN_NAME); // 列指定一つのパーサーです。

	bool isAsc = true; // 現在読み込んでいるORDER BY句での列が昇順であるかどうかです。

	// 昇順降順を指定するためのDESCトークンのパーサーです。
	auto SET_DESC = DESC->Action([&](const Token token){
		isAsc = false;
	});

	shared_ptr<ExtensionTreeNode> currentNode; // 現在読み込んでいるノードです。
	auto WHERE_OPEN_PAREN = OPEN_PAREN->Action([&](const Token token){
		++currentNode->parenOpenBeforeClose;
	});

	auto WHERE_UNIARY_MINUS = MINUS->Action([&](const Token token){
		currentNode->signCoefficient = -1;
	});

	auto WHERE_COLUMN = COLUMN->Action([&]{
		currentNode->column = column;
	});

	auto WHERE_INT_LITERAL = INT_LITERAL->Action([&](const Token token){
		currentNode->value = Data::New(stoi(token.word()));
	});

	auto WHERE_STRING_LITERAL = STRING_LITERAL->Action([&](const Token token){
		currentNode->value = Data::New(token.word().substr(1, token.word().size() - 2));
	});

	// 記号の意味
	// A >> B		:Aの後にBが続く
	// A | B		:AもしくはB
	// A->or(B)		:トークンAもしくはトークンB
	// -A			:Aが任意
	// ~A			:Aが0回以上続く
	// A >> &B		:後にBが存在するA

	// SELECT句の列指定一つのパーサーです。
	auto SELECT_COLUMN = COLUMN->Action([&]{
		queryInfo->selectColumns.push_back(column);
	});

	auto SELECT_COLUMNS = SELECT_COLUMN >> ~(COMMA >> SELECT_COLUMN); // SELECT句の一つ以上のの列指定のパーサーです。

	auto SELECT_CLAUSE = SELECT >> (ASTERISK | SELECT_COLUMNS); // SELECT句のパーサーです。

	auto PRE_ORDERBY_COLUMN = action([&]{
		isAsc = true;
	});

	auto ORDER_BY_COLUMN = PRE_ORDERBY_COLUMN >> COLUMN >> -(ASC | SET_DESC); // ORDER BY句の列指定一つのパーサーです。

	ORDER_BY_COLUMN = ORDER_BY_COLUMN->Action([&]{
		queryInfo->orders.push_back(Order(column, isAsc));
	});

	auto ORDER_BY_COLUMNS = ORDER_BY_COLUMN >> ~(COMMA >> ORDER_BY_COLUMN); // ORDER BY句の一つ以上の列指定のパーサーです。

	auto ORDER_BY_CLAUSE = ORDER >> BY >> ORDER_BY_COLUMNS; // ORDER BY句のパーサーです。

	// オペランドに前置される + か -の次のトークンを先読みし判別するパーサーです。
	auto WHERE_UNIALY_NEXT = (&(IDENTIFIER | INT_LITERAL))->Action([&](const bool success){
		if (!success){
			throw ResultValue::ERR_WHERE_OPERAND_TYPE;
		}
	});

	// オペランドに前置される + か - を読み込むパーサーです。
	auto WHERE_UNIAEY_PLUS_MINUS = (PLUS | WHERE_UNIARY_MINUS) >> WHERE_UNIALY_NEXT;

	auto WHERE_CLOSE_PAREN = CLOSE_PAREN->Action([&](const Token token){
		auto ancestors = currentNode->ancestors();
		any_of(ancestors->begin(), ancestors->end(), [](shared_ptr<ExtensionTreeNode> ancestor){
			auto leftNodes = ancestor->allLeftList();
			auto node = find_if(leftNodes->begin(), leftNodes->end(),
				[](const shared_ptr<ExtensionTreeNode> node){return node->parenOpenBeforeClose; });

			if (node != leftNodes->end()){
				// 対応付けられていないカッコ開くを一つ削除し、ノードがカッコに囲まれていることを記録します。
				--(*node)->parenOpenBeforeClose;
				ancestor->inParen = true;
				return true;
			}
			return false;
		});
	});

	auto OPERAND = WHERE_COLUMN | WHERE_INT_LITERAL | WHERE_STRING_LITERAL;

	auto WHERE_OPERAND = -~WHERE_OPEN_PAREN >> -WHERE_UNIAEY_PLUS_MINUS >> OPERAND >> -~WHERE_CLOSE_PAREN;

	auto OPERATOR = ASTERISK->or(SLASH)->or(PLUS)->or(MINUS)->or(EQUAL)->or(GREATER_THAN)->or(GREATER_THAN_OR_EQUAL)->or(LESS_THAN)->or(LESS_THAN_OR_EQUAL)->or(NOT_EQUAL)->or(AND)->or(AND)->or(OR);
	
	OPERATOR = OPERATOR->Action([&](const Token token){
		// 演算子(オペレーターを読み込みます。
		auto foundOperator = find_if(operators.begin(), operators.end(), [&](const Operator& op){return op.kind() == token.kind(); }); // 現在読み込んでいる演算子の情報です。

		// 現在見ている演算子の情報を探します。
		// 見つかった演算子の情報をもとにノードを入れ替えます。

		//カッコにくくられていなかった場合に、演算子の優先順位を参考に結合するノードを探します。
		auto ancestors = currentNode->selfAndAncestors();

		auto found = find_if(ancestors->begin(), ancestors->end(),
			[&](const shared_ptr<ExtensionTreeNode> ancestor)
		{
			auto allLefts = ancestor->selfAndAllLeftList();
			return 
				any_of(allLefts->begin(), allLefts->end(),
					[](shared_ptr<ExtensionTreeNode> node){return node->parenOpenBeforeClose; }) 
				|| !ancestor->parent 
				|| !(ancestor->parent->middleOperator.order() <= foundOperator->order() || ancestor->parent->inParen);
		});

		// 演算子のノードを新しく生成します。
		currentNode = make_shared<ExtensionTreeNode>();
		currentNode->middleOperator = *foundOperator;

		(*found)->InsertAndMoveLeft(currentNode);
	});

	auto PRE_WHERE_OPERAND = action([&]{
		// オペランドのノードを新しく生成します。
		auto newNode = make_shared<ExtensionTreeNode>();
		if (currentNode){
			// 現在のノードを右の子にずらし、元の位置に新しいノードを挿入します。
			currentNode->InsertRight(newNode);
			currentNode = newNode;
		}
		else{
			// 最初はカレントノードに新しいノードを入れます。
			currentNode = newNode;
		}
	});

	WHERE_OPERAND = PRE_WHERE_OPERAND >> WHERE_OPERAND;

	auto WHERE_EXTENSION = WHERE_OPERAND >> ~(OPERATOR >> WHERE_OPERAND);

	auto WHERE_CLAUSE = WHERE >> WHERE_EXTENSION;

	WHERE_CLAUSE = WHERE_CLAUSE->Action([&]{
		queryInfo->whereTopNode = currentNode;
		// 木を根に向かってさかのぼり、根のノードを設定します。
		while (queryInfo->whereTopNode->parent){
			queryInfo->whereTopNode = queryInfo->whereTopNode->parent;
		}
	});

	auto WHERE_ORDER = (ORDER_BY_CLAUSE >> -WHERE_CLAUSE) | (WHERE_CLAUSE >> -ORDER_BY_CLAUSE);

	auto TABLE_NAME = IDENTIFIER->Action([&](const Token &token){
		queryInfo->tableNames.push_back(token.word());
	});

	auto FROM_CLAUSE = FROM >> TABLE_NAME >> ~(COMMA >> TABLE_NAME);

	auto TINY_SQL =
		SELECT_CLAUSE >>
		-WHERE_ORDER >>
		FROM_CLAUSE >>
		end();

	TINY_SQL = TINY_SQL->Action([&]{
		// 構文エラーがないことを前提とした処理なので最後に実行しています。
		if (queryInfo->whereTopNode){
			// 既存数値の符号を計算します。
			auto whereNodes = SelfAndDescendants(queryInfo->whereTopNode);
			for (auto &whereNode : *whereNodes){
				if (whereNode->middleOperator.kind() == TokenKind::NOT_TOKEN &&
					whereNode->column.columnName.empty() &&
					whereNode->value->type() == DataType::INTEGER){
					whereNode->value = Data::New(whereNode->value->integer() * whereNode->signCoefficient);
				}
			}
		}
	});

	if (!TINY_SQL->Parse(tokens.begin(), tokens.end())){
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
	csv->Write(outputFileName, *csv->Read());
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