//! @file
#include "stdafx.h"
#include "ctype.h"

#include <algorithm>
#include <vector>
#pragma warning(disable:4996)

using namespace std;

#define MAX_FILE_LINE_LENGTH 4096          //!< 読み込むファイルの一行の最大長です。
#define MAX_WORD_LENGTH 256                //!< SQLの一語の最大長です。
#define MAX_DATA_LENGTH 256                //!< 入出力されるデータの、各列の最大長です。
#define MAX_COLUMN_COUNT 16                //!< 入出力されるデータに含まれる列の最大数です。
#define MAX_ROW_COUNT 256                  //!< 入出力されるデータに含まれる行の最大数です。
#define MAX_TABLE_COUNT 8                  //!< CSVとして入力されるテーブルの最大数です。
#define MAX_EXTENSION_TREE_NODE_COUNT 256  //!< WHERE句に指定される式木のノードの最大数です。

//! カレントディレクトリにあるCSVに対し、簡易的なSQLを実行し、結果をファイルに出力します。
//! @param [in] sql 実行するSQLです。
//! @param[in] outputFileName SQLの実行結果をCSVとして出力するファイル名です。拡張子を含みます。
//! @return 実行した結果の状態です。
int ExecuteSQL(const char*, const char*);

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
public:
	DataType type = DataType::STRING; //!< データの型です。

	//! 実際のデータを格納する共用体です。
	union
	{
		char string[MAX_DATA_LENGTH]; //!< データが文字列型の場合の値です。
		int integer;                  //!< データが整数型の場合の値です。
		bool boolean;                 //!< データが真偽値型の場合の値です。
	} value;

	//! Dataクラスの新しいインスタンスを初期化します。
	Data();

	//! Dataクラスの新しいインスタンスを初期化します。
	//! @param [in] value データの値です。
	Data(const char* value);

	//! Dataクラスの新しいインスタンスを初期化します。
	//! @param [in] value データの値です。
	Data(const int value);

	//! Dataクラスの新しいインスタンスを初期化します。
	//! @param [in] value データの値です。
	Data(const bool value);
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
	char word[MAX_WORD_LENGTH]; //!< 記録されているトークンの文字列です。記録の必要がなければ空白です。

	//! Tokenクラスの新しいインスタンスを初期化します。
	Token();

	//! Tokenクラスの新しいインスタンスを初期化します。
	//! @param [in] kind トークンの種類です。
	Token(const TokenKind kind);

	//! Tokenクラスの新しいインスタンスを初期化します。
	//! @param [in] kind トークンの種類です。
	//! @param [in] word 記録されているトークンの文字列です。記録の必要がなければ空白です。
	Token(const TokenKind kind, const char *word);
};

//! 指定された列の情報です。どのテーブルに所属するかの情報も含みます。
class Column
{
public:
	char tableName[MAX_WORD_LENGTH]; //!< 列が所属するテーブル名です。指定されていない場合は空文字列となります。
	char columnName[MAX_WORD_LENGTH]; //!< 指定された列の列名です。

	//! Columnクラスの新しいインスタンスを初期化します。
	Column();

	//! Columnクラスの新しいインスタンスを初期化します。
	//! @param [in] columnName 指定された列の列名です。
	Column(const char* columnName);

	//! Columnクラスの新しいインスタンスを初期化します。
	//! @param [in] tableName 列が所属するテーブル名です。指定されていない場合は空文字列となります。
	//! @param [in] columnName 指定された列の列名です。
	Column(const char* tableName, const char* columnName);
};

//! WHERE句の条件の式木を表します。
class ExtensionTreeNode
{
public:
	ExtensionTreeNode *parent = nullptr; //!< 親となるノードです。根の式木の場合はnullptrとなります。
	ExtensionTreeNode *left = nullptr;   //!< 左の子となるノードです。自身が末端の葉となる式木の場合はnullptrとなります。
	Operator middleOperator;             //!< 中置される演算子です。自身が末端のとなる式木の場合の種類はNOT_TOKENとなります。
	ExtensionTreeNode *right = nullptr;  //!< 右の子となるノードです。自身が末端の葉となる式木の場合はnullptrとなります。
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

// 以上ヘッダに相当する部分。

//! Dataクラスの新しいインスタンスを初期化します。
Data::Data() :value({ "" })
{
}

//! Dataクラスの新しいインスタンスを初期化します。
//! @param [in] value データの値です。
Data::Data(const char* value) : value({ "" })
{
	strncpy(this->value.string, value, max(MAX_DATA_LENGTH, MAX_WORD_LENGTH));
}

//! Dataクラスの新しいインスタンスを初期化します。
//! @param [in] value データの値です。
Data::Data(const int value) : type(DataType::INTEGER)
{
	this->value.integer = value;
}

//! Dataクラスの新しいインスタンスを初期化します。
//! @param [in] value データの値です。
Data::Data(const bool value) : type(DataType::BOOLEAN)
{
	this->value.boolean = value;
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
Token::Token(const TokenKind kind, const char *word) :kind(kind)
{
	strncpy(this->word, word, max(MAX_DATA_LENGTH, MAX_WORD_LENGTH));
}

//! Columnクラスの新しいインスタンスを初期化します。
Column::Column() : Column("", "")
{

}

//! Columnクラスの新しいインスタンスを初期化します。
//! @param [in] columnName 指定された列の列名です。
Column::Column(const char* columnName) : Column("", columnName)
{
}

//! Columnクラスの新しいインスタンスを初期化します。
//! @param [in] tableName 列が所属するテーブル名です。指定されていない場合は空文字列となります。
//! @param [in] columnName 指定された列の列名です。
Column::Column(const char* tableName, const char* columnName)
{
	strncpy(this->tableName, tableName, max(MAX_DATA_LENGTH, MAX_WORD_LENGTH));
	strncpy(this->columnName, columnName, max(MAX_DATA_LENGTH, MAX_WORD_LENGTH));
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
int ExecuteSQL(const char* sql, const char* outputFileName)
{
	vector<FILE*> inputTableFiles;                          // 読み込む入力ファイルの全てのファイルポインタです。
	FILE *outputFile = nullptr;                             // 書き込むファイルのファイルポインタです。
	int result = 0;                                         // 関数の戻り値を一時的に保存します。
	bool found = false;                                     // 検索時に見つかったかどうかの結果を一時的に保存します。
	const char *search = nullptr;                           // 文字列検索に利用するポインタです。
	Data ***currentRow = nullptr;                           // データ検索時に現在見ている行を表します。
	Data **inputData[MAX_TABLE_COUNT][MAX_ROW_COUNT];       // 入力データです。
	Data **outputData[MAX_ROW_COUNT] = { nullptr };         // 出力データです。
	Data **allColumnOutputData[MAX_ROW_COUNT] = { nullptr };// 出力するデータに対応するインデックスを持ち、すべての入力データを保管します。

	const char *alpahUnder = "_abcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPQRSTUVWXYZ"; // 全てのアルファベットの大文字小文字とアンダーバーです。
	const char *alpahNumUnder = "_abcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"; // 全ての数字とアルファベットの大文字小文字とアンダーバーです。
	const char *signNum = "+-0123456789"; // 全ての符号と数字です。
	const char *num = "0123456789"; // 全ての数字です。
	const char* space = " \t\r\n"; // 全ての空白文字です。

	// inputDataを初期化します。
	for (size_t i = 0; i < sizeof(inputData) / sizeof(inputData[0]); i++)
	{
		for (size_t j = 0; j < sizeof(inputData[0]) / sizeof(inputData[0][0]); j++){
			inputData[i][j] = nullptr;
		}
	}

	// SQLからトークンを読み込みます。

	// keywordConditionsとsignConditionsは先頭から順に検索されるので、前方一致となる二つの項目は順番に気をつけて登録しなくてはいけません。

	// キーワードをトークンとして認識するためのキーワード一覧情報です。
	const vector<const Token> keywordConditions =
	{
		{ TokenKind::AND, "AND" },
		{ TokenKind::ASC, "ASC" },
		{ TokenKind::BY, "BY" },
		{ TokenKind::DESC, "DESC" },
		{ TokenKind::FROM, "FROM" },
		{ TokenKind::ORDER, "ORDER" },
		{ TokenKind::OR, "OR" },
		{ TokenKind::SELECT, "SELECT" },
		{ TokenKind::WHERE, "WHERE" }
	};

	// 記号をトークンとして認識するための記号一覧情報です。
	const vector<const Token> signConditions =
	{
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
		{ TokenKind::SLASH, "/" },
	};

	vector<const Token> tokens; // SQLを分割したトークンです。

	// 演算子の情報です。
	const vector<const Operator> operators =
	{
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
		{ TokenKind::OR, 5 },
	};

	const char* charactorBackPoint = nullptr; // SQLをトークンに分割して読み込む時に戻るポイントを記録しておきます。

	const char* charactorCursol = sql; // SQLをトークンに分割して読み込む時に現在読んでいる文字の場所を表します。

	vector<const string> tableNames; // FROM句で指定しているテーブル名です。
	try
	{
		// SQLをトークンに分割て読み込みます。
		while (*charactorCursol){

			// 空白を読み飛ばします。
			for (search = space; *search && *charactorCursol != *search; ++search){}
			if (*search){
				charactorCursol++;
				continue;
			}

			// 数値リテラルを読み込みます。

			// 先頭文字が数字であるかどうかを確認します。
			charactorBackPoint = charactorCursol;
			for (search = num; *search && *charactorCursol != *search; ++search){}
			if (*search){
				Token literal{TokenKind::INT_LITERAL}; // 読み込んだ数値リテラルの情報です。
				int wordLength = 0; // 数値リテラルに現在読み込んでいる文字の数です。

				// 数字が続く間、文字を読み込み続けます。
				do {
					for (search = num; *search && *charactorCursol != *search; ++search){}
					if (*search){
						if (MAX_WORD_LENGTH - 1 <= wordLength){
							throw ResultValue::ERR_MEMORY_OVER;
						}
						literal.word[wordLength++] = *search;
						++charactorCursol;
					}
				} while (*search);

				// 数字の後にすぐに識別子が続くのは紛らわしいので数値リテラルとは扱いません。
				for (search = alpahUnder; *search && *charactorCursol != *search; ++search){}
				if (!*search){
					literal.word[wordLength] = '\0';
					tokens.push_back(literal);
					continue;
				}
				else{
					charactorCursol = charactorBackPoint;
				}
			}

			// 文字列リテラルを読み込みます。

			// 文字列リテラルを開始するシングルクォートを判別し、読み込みます。
			// メトリクス測定ツールのccccはシングルクォートの文字リテラル中のエスケープを認識しないため、文字リテラルを使わないことで回避しています。
			if (*charactorCursol == "\'"[0]){
				++charactorCursol;
				Token literal{TokenKind::STRING_LITERAL, "\'"}; // 読み込んだ文字列リテラルの情報です。
				int wordLength = 1; // 文字列リテラルに現在読み込んでいる文字の数です。初期値の段階で最初のシングルクォートは読み込んでいます。

				// 次のシングルクォートがくるまで文字を読み込み続けます。
				while (*charactorCursol && *charactorCursol != "\'"[0]){
					if (MAX_WORD_LENGTH - 1 <= wordLength){
						throw ResultValue::ERR_MEMORY_OVER;
					}
					literal.word[wordLength++] = *charactorCursol++;
				}
				if (*charactorCursol == "\'"[0]){
					if (MAX_WORD_LENGTH - 1 <= wordLength){
						throw ResultValue::ERR_MEMORY_OVER;
					}
					// 最後のシングルクォートを読み込みます。
					literal.word[wordLength++] = *charactorCursol++;

					// 文字列の終端文字をつけます。
					literal.word[wordLength] = '\0';
					tokens.push_back(literal);
					continue;
				}
				else{
					throw ResultValue::ERR_TOKEN_CANT_READ;
				}
			}

			// キーワードを読み込みます。
			found = false;
			for (auto & keywordCondition : keywordConditions){
				charactorBackPoint = charactorCursol;
				const char *wordCursol = keywordCondition.word; // 確認するキーワードの文字列のうち、現在確認している一文字を指します。

				// キーワードが指定した文字列となっているか確認します。
				while (*wordCursol && toupper(*charactorCursol++) == *wordCursol){
					++wordCursol;
				}

				// キーワードに識別子が区切りなしに続いていないかを確認するため、キーワードの終わった一文字あとを調べます。
				for (search = alpahNumUnder; *search && *charactorCursol != *search; ++search){};

				if (!*wordCursol && !*search){

					// 見つかったキーワードを生成します。
					tokens.push_back(Token(keywordCondition.kind));
					found = true;
				}
				else{
					charactorCursol = charactorBackPoint;
				}
			}
			if (found){
				continue;
			}

			// 記号を読み込みます。
			found = false;
			for (auto &signCondition : signConditions){
				charactorBackPoint = charactorCursol;
				const char *wordCursol = signCondition.word; // 確認する記号の文字列のうち、現在確認している一文字を指します。

				// 記号が指定した文字列となっているか確認します。
				while (*wordCursol && toupper(*charactorCursol++) == *wordCursol){
					++wordCursol;
				}
				if (!*wordCursol){

					// 見つかった記号を生成します。
					tokens.push_back(Token(signCondition.kind));
					found = true;
				}
				else{
					charactorCursol = charactorBackPoint;
				}
			}
			if (found){
				continue;
			}

			// 識別子を読み込みます。

			// 識別子の最初の文字を確認します。
			for (search = alpahUnder; *search && *charactorCursol != *search; ++search){};
			if (*search){
				Token identifier{ TokenKind::IDENTIFIER }; // 読み込んだ識別子の情報です。
				int wordLength = 0; // 識別子に現在読み込んでいる文字の数です。
				do {
					// 二文字目以降は数字も許可して文字の種類を確認します。
					for (search = alpahNumUnder; *search && *charactorCursol != *search; ++search){};
					if (*search){
						if (MAX_WORD_LENGTH - 1 <= wordLength){
							throw ResultValue::ERR_MEMORY_OVER;
						}
						identifier.word[wordLength++] = *search;
						charactorCursol++;
					}
				} while (*search);

				// 識別子の文字列の終端文字を設定します。
				identifier.word[wordLength] = '\0';

				// 読み込んだ識別子を登録します。
				tokens.push_back(identifier);
				continue;
			}
			else{
				charactorCursol = charactorBackPoint;
			}

			throw ResultValue::ERR_TOKEN_CANT_READ;
		}

		// トークン列を解析し、構文を読み取ります。

		Token *tokenCursol = &tokens[0]; // 現在見ているトークンを指します。

		vector<Column> selectColumns; // SELECT句に指定された列名です。

		vector<Column> orderByColumns; // ORDER句に指定された列名です。

		TokenKind orders[MAX_COLUMN_COUNT] = { TokenKind::NOT_TOKEN }; // 同じインデックスのorderByColumnsに対応している、昇順、降順の指定です。

		ExtensionTreeNode whereExtensionNodes[MAX_EXTENSION_TREE_NODE_COUNT]; // WHEREに指定された木のノードを、木構造とは無関係に格納します。
		// whereExtensionNodesを初期化します。
		for (size_t i = 0; i < sizeof(whereExtensionNodes) / sizeof(whereExtensionNodes[0]); i++)
		{
			whereExtensionNodes[i] = ExtensionTreeNode();
		}
		int whereExtensionNodesNum = 0; // 現在読み込まれているのwhereExtensionNodesの数です。

		ExtensionTreeNode *whereTopNode = nullptr; // 式木の根となるノードです。

		// SQLの構文を解析し、必要な情報を取得します。

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
					selectColumns.push_back(Column(tokenCursol->word));
					++tokenCursol;
					if (tokenCursol->kind == TokenKind::DOT){
						++tokenCursol;
						if (tokenCursol->kind == TokenKind::IDENTIFIER){

							// テーブル名が指定されていることがわかったので読み替えます。
							selectColumns.back() = Column(selectColumns.back().columnName, tokenCursol->word);
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
							orderByColumns.push_back(Column(tokenCursol->word));
							++tokenCursol;
							if (tokenCursol->kind == TokenKind::DOT){
								++tokenCursol;
								if (tokenCursol->kind == TokenKind::IDENTIFIER){

									// テーブル名が指定されていることがわかったので読み替えます。
									orderByColumns.back() = Column(orderByColumns.back().columnName, tokenCursol->word);
									++tokenCursol;
								}
								else{
									throw ResultValue::ERR_SQL_SYNTAX;
								}
							}

							// 並び替えの昇順、降順を指定します。
							if (tokenCursol->kind == TokenKind::ASC){
								orders[orderByColumns.size() - 1] = TokenKind::ASC;
								++tokenCursol;
							}
							else if (tokenCursol->kind == TokenKind::DESC){
								orders[orderByColumns.size() - 1] = TokenKind::DESC;
								++tokenCursol;
							}
							else{
								// 指定がない場合は昇順となります。
								orders[orderByColumns.size() - 1] = TokenKind::ASC;
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
				ExtensionTreeNode *currentNode = nullptr; // 現在読み込んでいるノードです。
				while (true){
					// オペランドを読み込みます。

					// オペランドのノードを新しく生成します。
					if (MAX_EXTENSION_TREE_NODE_COUNT <= whereExtensionNodesNum){
						throw ResultValue::ERR_MEMORY_OVER;
					}
					if (currentNode){
						// 現在のノードを右の子にずらし、元の位置に新しいノードを挿入します。
						currentNode->right = &whereExtensionNodes[whereExtensionNodesNum++];
						currentNode->right->parent = currentNode;
						currentNode = currentNode->right;
					}
					else{
						// 最初はカレントノードに新しいノードを入れます。
						currentNode = &whereExtensionNodes[whereExtensionNodesNum++];
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
						currentNode->value = Data(atoi(tokenCursol->word));
						++tokenCursol;
					}
					else if (tokenCursol->kind == TokenKind::STRING_LITERAL){
						currentNode->value = Data();

						// 前後のシングルクォートを取り去った文字列をデータとして読み込みます。
						strncpy(currentNode->value.value.string, tokenCursol->word + 1, std::min(MAX_WORD_LENGTH, MAX_DATA_LENGTH));
						currentNode->value.value.string[MAX_DATA_LENGTH - 1] = '\0';
						currentNode->value.value.string[strlen(currentNode->value.value.string) - 1] = '\0';
						++tokenCursol;
					}
					else{
						throw ResultValue::ERR_SQL_SYNTAX;
					}

					// オペランドの右のカッコ閉じるを読み込みます。
					while (tokenCursol->kind == TokenKind::CLOSE_PAREN){
						ExtensionTreeNode *searchedAncestor = currentNode->parent; // カッコ閉じると対応するカッコ開くを両方含む祖先ノードを探すためのカーソルです。
						while (searchedAncestor){

							// searchedAncestorの左の子に対応するカッコ開くがないかを検索します。
							ExtensionTreeNode *searched = searchedAncestor; // searchedAncestorの内部からカッコ開くを検索するためのカーソルです。
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
					Operator middleOperator; // 現在読み込んでいる演算子の情報です。

					// 現在見ている演算子の情報を探します。
					found = false;
					for (auto &anOperator : operators){
						if (anOperator.kind == tokenCursol->kind){
							middleOperator = anOperator;
							found = true;
							break;
						}
					}
					if (found)
					{
						// 見つかった演算子の情報をもとにノードを入れ替えます。
						ExtensionTreeNode *tmp = currentNode; //ノードを入れ替えるために使う変数です。

						ExtensionTreeNode *searched = tmp; // 入れ替えるノードを探すためのカーソルです。

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
						} while (!searched && tmp->parent && (tmp->parent->middleOperator.order <= middleOperator.order || tmp->parent->inParen));

						// 演算子のノードを新しく生成します。
						if (MAX_EXTENSION_TREE_NODE_COUNT <= whereExtensionNodesNum){
							throw ResultValue::ERR_MEMORY_OVER;
						}
						currentNode = &whereExtensionNodes[whereExtensionNodesNum++];
						currentNode->middleOperator = middleOperator;

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
				whereTopNode = currentNode;
				while (whereTopNode->parent){
					whereTopNode = whereTopNode->parent;
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
		while (tokenCursol->kind == TokenKind::COMMA || first){
			if (tokenCursol->kind == TokenKind::COMMA){
				++tokenCursol;
			}
			if (tokenCursol->kind == TokenKind::IDENTIFIER){
				tableNames.push_back(tokenCursol->word);
				++tokenCursol;
			}
			else{
				throw ResultValue::ERR_SQL_SYNTAX;
			}
			first = false;
		}

		// 最後のトークンまで読み込みが進んでいなかったらエラーです。
		if (tokenCursol <= &tokens.back()){
			throw ResultValue::ERR_SQL_SYNTAX;
		}
		Column inputColumns[MAX_TABLE_COUNT][MAX_COLUMN_COUNT]; // 入力されたCSVの行の情報です。
		// inputColumnsを初期化します。
		for (size_t i = 0; i < sizeof(inputColumns) / sizeof(inputColumns[0]); i++)
		{
			for (size_t j = 0; j < sizeof(inputColumns[0]) / sizeof(inputColumns[0][0]); j++)
			{
				inputColumns[i][j] = Column();
			}
		}
		int inputColumnNums[MAX_TABLE_COUNT] = { 0 }; // 各テーブルごとの列の数です。

		for (size_t i = 0; i < tableNames.size(); ++i){

			// 入力ファイル名を生成します。
			const char csvExtension[] = ".csv"; // csvの拡張子です。
			char fileName[MAX_WORD_LENGTH + sizeof(csvExtension) - 1] = ""; // 拡張子を含む、入力ファイルのファイル名です。
			strncat(fileName, tableNames[i].c_str(), MAX_WORD_LENGTH + sizeof(csvExtension) - 1);
			strncat(fileName, csvExtension, MAX_WORD_LENGTH + sizeof(csvExtension) - 1);

			// 入力ファイルを開きます。
			inputTableFiles.push_back(fopen(fileName, "r"));
			if (!inputTableFiles.back()){
				throw ResultValue::ERR_FILE_OPEN;
			}

			// 入力CSVのヘッダ行を読み込みます。
			char inputLine[MAX_FILE_LINE_LENGTH] = ""; // ファイルから読み込んだ行文字列です。
			if (fgets(inputLine, MAX_FILE_LINE_LENGTH, inputTableFiles.back())){
				charactorCursol = inputLine;

				// 読み込んだ行を最後まで読みます。
				while (*charactorCursol && *charactorCursol != '\r' && *charactorCursol != '\n'){
					if (MAX_COLUMN_COUNT <= inputColumnNums[i]){
						throw ResultValue::ERR_MEMORY_OVER;
					}
					strncpy(inputColumns[i][inputColumnNums[i]].tableName, tableNames[i].c_str(), MAX_WORD_LENGTH);
					char *writeCursol = inputColumns[i][inputColumnNums[i]++].columnName; // 列名の書き込みに利用するカーソルです。

					// 列名を一つ読みます。
					while (*charactorCursol && *charactorCursol != ',' && *charactorCursol != '\r'&& *charactorCursol != '\n'){
						*writeCursol++ = *charactorCursol++;
					}
					// 書き込んでいる列名の文字列に終端文字を書き込みます。
					writeCursol[1] = '\0';

					// 入力行のカンマの分を読み進めます。
					++charactorCursol;
				}
			}
			else{
				throw ResultValue::ERR_CSV_SYNTAX;
			}

			// 入力CSVのデータ行を読み込みます。
			int rowNum = 0;
			while (fgets(inputLine, MAX_FILE_LINE_LENGTH, inputTableFiles.back())){
				if (MAX_ROW_COUNT <= rowNum){
					throw ResultValue::ERR_MEMORY_OVER;
				}
				Data **row = inputData[i][rowNum++] = (Data**)malloc(MAX_COLUMN_COUNT * sizeof(Data*)); // 入力されている一行分のデータです。
				if (!row){
					throw ResultValue::ERR_MEMORY_ALLOCATE;
				}
				// 生成した行を初期化します。
				for (int j = 0; j < MAX_COLUMN_COUNT; ++j){
					row[j] = nullptr;
				}

				charactorCursol = inputLine;
				int columnNum = 0; // いま何列目を読み込んでいるか。0基底の数字となります。

				// 読み込んだ行を最後まで読みます。
				while (*charactorCursol && *charactorCursol != '\r'&& *charactorCursol != '\n'){

					// 読み込んだデータを書き込む行のカラムを生成します。
					if (MAX_COLUMN_COUNT <= columnNum){
						throw ResultValue::ERR_MEMORY_OVER;
					}
					row[columnNum] = (Data*)malloc(sizeof(Data));
					if (!row[columnNum]){
						throw ResultValue::ERR_MEMORY_ALLOCATE;
					}
					*row[columnNum] = Data("");
					char *writeCursol = row[columnNum++]->value.string; // データ文字列の書き込みに利用するカーソルです。

					// データ文字列を一つ読みます。
					while (*charactorCursol && *charactorCursol != ',' && *charactorCursol != '\r'&& *charactorCursol != '\n'){
						*writeCursol++ = *charactorCursol++;
					}
					// 書き込んでいる列名の文字列に終端文字を書き込みます。
					writeCursol[1] = '\0';

					// 入力行のカンマの分を読み進めます。
					++charactorCursol;
				}
			}

			// 全てが数値となる列は数値列に変換します。
			for (int j = 0; j < inputColumnNums[i]; ++j){

				// 全ての行のある列について、データ文字列から符号と数値以外の文字を探します。
				currentRow = inputData[i];
				found = false;
				while (*currentRow){
					char *currentChar = (*currentRow)[j]->value.string;
					while (*currentChar){
						bool isNum = false;
						const char *currentNum = signNum;
						while (*currentNum){
							if (*currentChar == *currentNum){
								isNum = true;
								break;
							}
							++currentNum;
						}
						if (!isNum){
							found = true;
							break;
						}
						++currentChar;
					}
					if (found){
						break;
					}
					++currentRow;
				}

				// 符号と数字以外が見つからない列については、数値列に変換します。
				if (!found){
					currentRow = inputData[i];
					while (*currentRow){
						*(*currentRow)[j] = Data(atoi((*currentRow)[j]->value.string));
						++currentRow;
					}
				}
			}
		}

		Column allInputColumns[MAX_TABLE_COUNT * MAX_COLUMN_COUNT]; // 入力に含まれるすべての列の一覧です。
		// allInputColumnsを初期化します。
		for (size_t i = 0; i < sizeof(allInputColumns) / sizeof(allInputColumns[0]); i++)
		{
			allInputColumns[i] = Column();
		}
		int allInputColumnsNum = 0; // 入力に含まれるすべての列の数です。

		// 入力ファイルに書いてあったすべての列をallInputColumnsに設定します。
		for (size_t i = 0; i < tableNames.size(); ++i){
			for (int j = 0; j < inputColumnNums[i]; ++j){
				allInputColumns[allInputColumnsNum++] = Column(tableNames[i].c_str(), inputColumns[i][j].columnName);
			}
		}

		// SELECT句の列名指定が*だった場合は、入力CSVの列名がすべて選択されます。
		if (selectColumns.empty()){
			for (int i = 0; i < allInputColumnsNum; ++i){
				selectColumns.push_back(allInputColumns[i]);
			}
		}

		Column outputColumns[MAX_TABLE_COUNT * MAX_COLUMN_COUNT]; // 出力するすべての行の情報です。
		int outputColumnNum = 0; // 出力するすべての行の現在の数です。

		// SELECT句で指定された列名が、何個目の入力ファイルの何列目に相当するかを判別します。
		ColumnIndex selectColumnIndexes[MAX_TABLE_COUNT * MAX_COLUMN_COUNT]; // SELECT句で指定された列の、入力ファイルとしてのインデックスです。
		int selectColumnIndexesNum = 0; // selectColumnIndexesの現在の数。
		for (auto &selectColumn : selectColumns){
			found = false;
			for (size_t i = 0; i < tableNames.size(); ++i){
				for (int j = 0; j < inputColumnNums[i]; ++j){
					char* selectTableNameCursol = selectColumn.tableName;
					char* inputTableNameCursol = inputColumns[i][j].tableName;
					while (*selectTableNameCursol && toupper(*selectTableNameCursol) == toupper(*inputTableNameCursol++)){
						++selectTableNameCursol;
					}
					char* selectColumnNameCursol = selectColumn.columnName;
					char* inputColumnNameCursol = inputColumns[i][j].columnName;
					while (*selectColumnNameCursol && toupper(*selectColumnNameCursol) == toupper(*inputColumnNameCursol++)){
						++selectColumnNameCursol;
					}
					if (!*selectColumnNameCursol && !*inputColumnNameCursol &&
						(!*selectColumn.tableName || // テーブル名が設定されている場合のみテーブル名の比較を行います。
						!*selectTableNameCursol && !*inputTableNameCursol)){

						// 既に見つかっているのにもう一つ見つかったらエラーです。
						if (found){
							throw ResultValue::ERR_BAD_COLUMN_NAME;
						}
						found = true;
						// 見つかった値を持つ列のデータを生成します。
						if (MAX_COLUMN_COUNT <= selectColumnIndexesNum){
							throw ResultValue::ERR_MEMORY_OVER;
						}
						selectColumnIndexes[selectColumnIndexesNum++] = ColumnIndex(i, j);
					}
				}
			}
			// 一つも見つからなくてもエラーです。
			if (!found){
				throw ResultValue::ERR_BAD_COLUMN_NAME;
			}
		}

		// 出力する列名を設定します。
		for (size_t i = 0; i < selectColumns.size(); ++i){
			outputColumns[outputColumnNum] = inputColumns[selectColumnIndexes[i].table][selectColumnIndexes[i].column];
			++outputColumnNum;
		}

		if (whereTopNode){
			// 既存数値の符号を計算します。
			for (int i = 0; i < whereExtensionNodesNum; ++i){
				if (whereExtensionNodes[i].middleOperator.kind == TokenKind::NOT_TOKEN &&
					!*whereExtensionNodes[i].column.columnName &&
					whereExtensionNodes[i].value.type == DataType::INTEGER){
					whereExtensionNodes[i].value.value.integer *= whereExtensionNodes[i].signCoefficient;
				}
			}
		}

		int outputRowsNum = 0; // 出力データの現在の行数です。

		Data ***currentRows[MAX_TABLE_COUNT] = { nullptr }; // 入力された各テーブルの、現在出力している行を指すカーソルです。
		for (size_t i = 0; i < tableNames.size(); ++i){
			// 各テーブルの先頭行を設定します。
			currentRows[i] = inputData[i];
		}

		// 出力するデータを設定します。
		while (true){
			if (MAX_ROW_COUNT <= outputRowsNum){
				throw ResultValue::ERR_MEMORY_OVER;
			}
			Data **row = outputData[outputRowsNum] = (Data**)malloc(MAX_COLUMN_COUNT * sizeof(Data*)); // 出力している一行分のデータです。
			if (!row){
				throw ResultValue::ERR_MEMORY_ALLOCATE;
			}

			// 生成した行を初期化します。
			for (int i = 0; i < MAX_COLUMN_COUNT; ++i){
				row[i] = nullptr;
			}

			// 行の各列のデータを入力から持ってきて設定します。
			for (int i = 0; i < selectColumnIndexesNum; ++i){
				row[i] = (Data*)malloc(sizeof(Data));
				if (!row[i]){
					throw ResultValue::ERR_MEMORY_ALLOCATE;
				}
				*row[i] = *(*currentRows[selectColumnIndexes[i].table])[selectColumnIndexes[i].column];
			}

			Data **allColumnsRow = allColumnOutputData[outputRowsNum++] = (Data**)malloc(MAX_TABLE_COUNT * MAX_COLUMN_COUNT * sizeof(Data*)); // WHEREやORDERのためにすべての情報を含む行。rowとインデックスを共有します。
			if (!allColumnsRow){
				throw ResultValue::ERR_MEMORY_ALLOCATE;
			}
			// 生成した行を初期化します。
			for (int i = 0; i < MAX_TABLE_COUNT * MAX_COLUMN_COUNT; ++i){
				allColumnsRow[i] = nullptr;
			}

			// allColumnsRowの列を設定します。
			int allColumnsNum = 0; // allColumnsRowの現在の列数です。
			for (size_t i = 0; i < tableNames.size(); ++i){
				for (int j = 0; j < inputColumnNums[i]; ++j){
					allColumnsRow[allColumnsNum] = (Data*)malloc(sizeof(Data));
					if (!allColumnsRow[allColumnsNum]){
						throw ResultValue::ERR_MEMORY_ALLOCATE;
					}
					*allColumnsRow[allColumnsNum++] = *(*currentRows[i])[j];
				}
			}
			// WHEREの条件となる値を再帰的に計算します。
			if (whereTopNode){
				ExtensionTreeNode *currentNode = whereTopNode; // 現在見ているノードです。
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
						if (*currentNode->column.columnName){
							found = false;
							for (int i = 0; i < allInputColumnsNum; ++i){
								char* whereTableNameCursol = currentNode->column.tableName;
								char* allInputTableNameCursol = allInputColumns[i].tableName;
								while (*whereTableNameCursol && toupper(*whereTableNameCursol) == toupper(*allInputTableNameCursol++)){
									++whereTableNameCursol;
								}
								char* whereColumnNameCursol = currentNode->column.columnName;
								char* allInputColumnNameCursol = allInputColumns[i].columnName;
								while (*whereColumnNameCursol && toupper(*whereColumnNameCursol) == toupper(*allInputColumnNameCursol++)){
									++whereColumnNameCursol;
								}
								if (!*whereColumnNameCursol && !*allInputColumnNameCursol &&
									(!*currentNode->column.tableName || // テーブル名が設定されている場合のみテーブル名の比較を行います。
									!*whereTableNameCursol && !*allInputTableNameCursol)){
									// 既に見つかっているのにもう一つ見つかったらエラーです。
									if (found){
										throw ResultValue::ERR_BAD_COLUMN_NAME;
									}
									found = true;
									currentNode->value = *allColumnsRow[i];
								}
							}
							// 一つも見つからなくてもエラーです。
							if (!found){
								throw ResultValue::ERR_BAD_COLUMN_NAME;
							}
							;
							// 符号を考慮して値を計算します。
							if (currentNode->value.type == DataType::INTEGER){
								currentNode->value.value.integer *= currentNode->signCoefficient;
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
								currentNode->value.value.boolean = currentNode->left->value.value.integer == currentNode->right->value.value.integer;
								break;
							case TokenKind::GREATER_THAN:
								currentNode->value.value.boolean = currentNode->left->value.value.integer > currentNode->right->value.value.integer;
								break;
							case TokenKind::GREATER_THAN_OR_EQUAL:
								currentNode->value.value.boolean = currentNode->left->value.value.integer >= currentNode->right->value.value.integer;
								break;
							case TokenKind::LESS_THAN:
								currentNode->value.value.boolean = currentNode->left->value.value.integer < currentNode->right->value.value.integer;
								break;
							case TokenKind::LESS_THAN_OR_EQUAL:
								currentNode->value.value.boolean = currentNode->left->value.value.integer <= currentNode->right->value.value.integer;
								break;
							case TokenKind::NOT_EQUAL:
								currentNode->value.value.boolean = currentNode->left->value.value.integer != currentNode->right->value.value.integer;
								break;
							}
							break;
						case DataType::STRING:
							switch (currentNode->middleOperator.kind){
							case TokenKind::EQUAL:
								currentNode->value.value.boolean = strcmp(currentNode->left->value.value.string, currentNode->right->value.value.string) == 0;
								break;
							case TokenKind::GREATER_THAN:
								currentNode->value.value.boolean = strcmp(currentNode->left->value.value.string, currentNode->right->value.value.string) > 0;
								break;
							case TokenKind::GREATER_THAN_OR_EQUAL:
								currentNode->value.value.boolean = strcmp(currentNode->left->value.value.string, currentNode->right->value.value.string) >= 0;
								break;
							case TokenKind::LESS_THAN:
								currentNode->value.value.boolean = strcmp(currentNode->left->value.value.string, currentNode->right->value.value.string) < 0;
								break;
							case TokenKind::LESS_THAN_OR_EQUAL:
								currentNode->value.value.boolean = strcmp(currentNode->left->value.value.string, currentNode->right->value.value.string) <= 0;
								break;
							case TokenKind::NOT_EQUAL:
								currentNode->value.value.boolean = strcmp(currentNode->left->value.value.string, currentNode->right->value.value.string) != 0;
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
							currentNode->value.value.integer = currentNode->left->value.value.integer + currentNode->right->value.value.integer;
							break;
						case TokenKind::MINUS:
							currentNode->value.value.integer = currentNode->left->value.value.integer - currentNode->right->value.value.integer;
							break;
						case TokenKind::ASTERISK:
							currentNode->value.value.integer = currentNode->left->value.value.integer * currentNode->right->value.value.integer;
							break;
						case TokenKind::SLASH:
							currentNode->value.value.integer = currentNode->left->value.value.integer / currentNode->right->value.value.integer;
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
							currentNode->value.value.boolean = currentNode->left->value.value.boolean && currentNode->right->value.value.boolean;
							break;
						case TokenKind::OR:
							currentNode->value.value.boolean = currentNode->left->value.value.boolean || currentNode->right->value.value.boolean;
							break;
						}
					}
					currentNode->calculated = true;

					// 自身の計算が終わった後は親の計算に戻ります。
					currentNode = currentNode->parent;
				}

				// 条件に合わない行は出力から削除します。
				if (!whereTopNode->value.value.boolean){
					free(row);
					free(allColumnsRow);
					allColumnOutputData[--outputRowsNum] = nullptr;
					outputData[outputRowsNum] = nullptr;
				}
				// WHERE条件の計算結果をリセットします。
				for (int i = 0; i < whereExtensionNodesNum; ++i){
					whereExtensionNodes[i].calculated = false;
				}
			}

			// 各テーブルの行のすべての組み合わせを出力します。

			// 最後のテーブルのカレント行をインクリメントします。
			++currentRows[tableNames.size() - 1];

			// 最後のテーブルが最終行になっていた場合は先頭に戻し、順に前のテーブルのカレント行をインクリメントします。
			for (int i = tableNames.size() - 1; !*currentRows[i] && 0 < i; --i){
				++currentRows[i - 1];
				currentRows[i] = inputData[i];
			}

			// 最初のテーブルが最後の行を超えたなら出力行の生成は終わりです。
			if (!*currentRows[0]){
				break;
			}
		}

		// ORDER句による並び替えの処理を行います。
		if (!orderByColumns.empty()){
			// ORDER句で指定されている列が、全ての入力行の中のどの行なのかを計算します。
			int orderByColumnIndexes[MAX_COLUMN_COUNT]; // ORDER句で指定された列の、すべての行の中でのインデックスです。
			int orderByColumnIndexesNum = 0; // 現在のorderByColumnIndexesの数です。
			for (auto &orderByColumn : orderByColumns){
				found = false;
				for (int i = 0; i < allInputColumnsNum; ++i){
					char* orderByTableNameCursol = orderByColumn.tableName;
					char* allInputTableNameCursol = allInputColumns[i].tableName;
					while (*orderByTableNameCursol && toupper(*orderByTableNameCursol) == toupper(*allInputTableNameCursol)){
						++orderByTableNameCursol;
						++allInputTableNameCursol;
					}
					char* orderByColumnNameCursol = orderByColumn.columnName;
					char* allInputColumnNameCursol = allInputColumns[i].columnName;
					while (*orderByColumnNameCursol && toupper(*orderByColumnNameCursol) == toupper(*allInputColumnNameCursol)){
						++orderByColumnNameCursol;
						++allInputColumnNameCursol;
					}
					if (!*orderByColumnNameCursol && !*allInputColumnNameCursol &&
						(!*orderByColumn.tableName || // テーブル名が設定されている場合のみテーブル名の比較を行います。
						!*orderByTableNameCursol && !*allInputTableNameCursol)){
						// 既に見つかっているのにもう一つ見つかったらエラーです。
						if (found){
							throw ResultValue::ERR_BAD_COLUMN_NAME;
						}
						found = true;
						if (MAX_COLUMN_COUNT <= orderByColumnIndexesNum){
							throw ResultValue::ERR_MEMORY_OVER;
						}
						orderByColumnIndexes[orderByColumnIndexesNum++] = i;
					}
				}
				// 一つも見つからなくてもエラーです。
				if (!found){
					throw ResultValue::ERR_BAD_COLUMN_NAME;
				}
			}

			// outputDataとallColumnOutputDataのソートを一緒に行います。簡便のため凝ったソートは使わず、選択ソートを利用します。
			for (int i = 0; i < outputRowsNum; ++i){
				int minIndex = i; // 現在までで最小の行のインデックスです。
				for (int j = i + 1; j < outputRowsNum; ++j){
					bool jLessThanMin = false; // インデックスがjの値が、minIndexの値より小さいかどうかです。
					for (int k = 0; k < orderByColumnIndexesNum; ++k){
						Data *mData = allColumnOutputData[minIndex][orderByColumnIndexes[k]]; // インデックスがminIndexのデータです。
						Data *jData = allColumnOutputData[j][orderByColumnIndexes[k]]; // インデックスがjのデータです。
						int cmp = 0; // 比較結果です。等しければ0、インデックスjの行が大きければプラス、インデックスminIndexの行が大きければマイナスとなります。
						switch (mData->type)
						{
						case DataType::INTEGER:
							cmp = jData->value.integer - mData->value.integer;
							break;
						case DataType::STRING:
							cmp = strcmp(jData->value.string, mData->value.string);
							break;
						}

						// 降順ならcmpの大小を入れ替えます。
						if (orders[k] == TokenKind::DESC){
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
				Data** tmp = outputData[minIndex];
				outputData[minIndex] = outputData[i];
				outputData[i] = tmp;

				tmp = allColumnOutputData[minIndex];
				allColumnOutputData[minIndex] = allColumnOutputData[i];
				allColumnOutputData[i] = tmp;
			}
		}

		// 出力ファイルを開きます。
		outputFile = fopen(outputFileName, "w");
		if (outputFile == nullptr){
			throw ResultValue::ERR_FILE_OPEN;
		}

		// 出力ファイルに列名を出力します。
		for (size_t i = 0; i < selectColumns.size(); ++i){
			result = fputs(outputColumns[i].columnName, outputFile);
			if (result == EOF){
				throw ResultValue::ERR_FILE_WRITE;
			}
			if (i < selectColumns.size() - 1){
				result = fputs(",", outputFile);
				if (result == EOF){
					throw ResultValue::ERR_FILE_WRITE;
				}
			}
			else{
				result = fputs("\n", outputFile);
				if (result == EOF){
					throw ResultValue::ERR_FILE_WRITE;
				}
			}
		}

		// 出力ファイルにデータを出力します。
		currentRow = outputData;
		while (*currentRow){
			Data **column = *currentRow;
			for (size_t i = 0; i < selectColumns.size(); ++i){
				char outputString[MAX_DATA_LENGTH] = "";
				switch ((*column)->type){
				case DataType::INTEGER:
					itoa((*column)->value.integer, outputString, 10);
					break;
				case DataType::STRING:
					strcpy(outputString, (*column)->value.string);
					break;
				}
				result = fputs(outputString, outputFile);
				if (result == EOF){
					throw ResultValue::ERR_FILE_WRITE;
				}
				if (i < selectColumns.size() - 1){
					result = fputs(",", outputFile);
					if (result == EOF){
						throw ResultValue::ERR_FILE_WRITE;
					}
				}
				else{
					result = fputs("\n", outputFile);
					if (result == EOF){
						throw ResultValue::ERR_FILE_WRITE;
					}
				}
				++column;
			}
			++currentRow;
		}

		// 正常時の後処理です。

		// ファイルリソースを解放します。
		for (auto &inputTableFile : inputTableFiles){
			if (inputTableFile){
				fclose(inputTableFile);
				if (result == EOF){
					throw ResultValue::ERR_FILE_CLOSE;
				}
			}
		}
		if (outputFile){
			fclose(outputFile);
			if (result == EOF){
				throw ResultValue::ERR_FILE_CLOSE;
			}
		}

		// メモリリソースを解放します。
		for (size_t i = 0; i < tableNames.size(); ++i){
			currentRow = inputData[i];
			while (*currentRow){
				Data **dataCursol = *currentRow;
				while (*dataCursol){
					free(*dataCursol++);
				}
				free(*currentRow);
				currentRow++;
			}
		}
		currentRow = outputData;
		while (*currentRow){
			Data **dataCursol = *currentRow;
			while (*dataCursol){
				free(*dataCursol++);
			}
			free(*currentRow);
			currentRow++;
		}
		currentRow = allColumnOutputData;
		while (*currentRow){
			Data **dataCursol = *currentRow;
			while (*dataCursol){
				free(*dataCursol++);
			}
			free(*currentRow);
			currentRow++;
		}

		return static_cast<int>(ResultValue::OK);
	}
	catch (ResultValue error) // 発生したエラーの種類です。
	{
		// エラー時の処理です。

		// ファイルリソースを解放します。
		for (auto &inputTableFile : inputTableFiles){
			if (inputTableFile){
				fclose(inputTableFile);
			}
		}
		if (outputFile){
			fclose(outputFile);
		}

		// メモリリソースを解放します。
		for (size_t i = 0; i < tableNames.size(); ++i){
			currentRow = inputData[i];
			while (*currentRow){
				Data **dataCursol = *currentRow;
				while (*dataCursol){
					free(*dataCursol++);
				}
				free(*currentRow);
				currentRow++;
			}
		}
		currentRow = outputData;
		while (*currentRow){
			Data **dataCursol = *currentRow;
			while (*dataCursol){
				free(*dataCursol++);
			}
			free(*currentRow);
			currentRow++;
		}
		currentRow = allColumnOutputData;
		while (*currentRow){
			Data **dataCursol = *currentRow;
			while (*dataCursol){
				free(*dataCursol++);
			}
			free(*currentRow);
			currentRow++;
		}
		return static_cast<int>(error);
	}
}