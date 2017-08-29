//! @file
#include "ExecuteSQL.h"
#import <Foundation/NSArray.h>
#import <Foundation/NSException.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma warning(disable : 4996)

#define MAX_FILE_LINE_LENGTH 4096 //!< 読み込むファイルの一行の最大長です。
#define MAX_WORD_LENGTH 256 //!< SQLの一語の最大長です。
#define MAX_DATA_LENGTH 256 //!< 入出力されるデータの、各列の最大長です。
#define MAX_COLUMN_COUNT 16 //!< 入出力されるデータに含まれる列の最大数です。
#define MAX_ROW_COUNT 256 //!< 入出力されるデータに含まれる行の最大数です。
#define MAX_TABLE_COUNT 8 //!< CSVとして入力されるテーブルの最大数です。
#define MAX_EXTENSION_TREE_NODE_COUNT                                          \
  256 //!< WHERE句に指定される式木のノードの最大数です。

//! カレントディレクトリにあるCSVに対し、簡易的なSQLを実行し、結果をファイルに出力します。
//! @param [in] sql 実行するSQLです。
//! @param[in] outputFileName
//! SQLの実行結果をCSVとして出力するファイル名です。拡張子を含みます。
//! @return 実行した結果の状態です。
int ExecuteSQL(const char *sql, const char *outputFileName);

//! ExecuteSQLの戻り値の種類を表します。
typedef NS_ENUM(NSUInteger, RESULT_VALUE) {
  OK = 0,            //!< 問題なく終了しました。
  ERR_FILE_OPEN = 1, //!< ファイルを開くことに失敗しました。
  ERR_FILE_WRITE = 2, //!< ファイルに書き込みを行うことに失敗しました。
  ERR_FILE_CLOSE = 3, //!< ファイルを閉じることに失敗しました。
  ERR_TOKEN_CANT_READ = 4, //!< トークン解析に失敗しました。
  ERR_SQL_SYNTAX = 5,      //!< SQLの構文解析が失敗しました。
  ERR_BAD_COLUMN_NAME = 6, //!< テーブル指定を含む列名が適切ではありません。
  ERR_WHERE_OPERAND_TYPE = 7, //!< 演算の左右の型が適切ではありません。
  ERR_CSV_SYNTAX = 8,      //!< CSVの構文解析が失敗しました。
  ERR_MEMORY_ALLOCATE = 9, //!< メモリの取得に失敗しました。
  ERR_MEMORY_OVER = 10 //!< 用意したメモリ領域の上限を超えました。
};

//! 入力や出力、経過の計算に利用するデータのデータ型の種類を表します。
typedef NS_ENUM(NSUInteger, DATA_TYPE) {
  STRING,  //!< 文字列型です。
  INTEGER, //!< 整数型です。
  BOOLEAN  //!< 真偽値型です。
};

//! トークンの種類を表します。
typedef NS_ENUM(NSUInteger, TOKEN_KIND) {
  NOT_TOKEN,             //!< トークンを表しません。
  ASC,                   //!< ASCキーワードです。
  AND,                   //!< ANDキーワードです。
  BY,                    //!< BYキーワードです。
  DESC,                  //!< DESCキーワードです。
  FROM,                  //!< FROMキーワードです。
  OR,                    //!< ORキーワードです。
  ORDER,                 //!< ORDERキーワードです。
  SELECT,                //!< SELECTキーワードです。
  WHERE,                 //!< WHEREキーワードです。
  ASTERISK,              //!< ＊ 記号です。
  COMMA,                 //!< ， 記号です。
  CLOSE_PAREN,           //!< ） 記号です。
  DOT,                   //!< ． 記号です。
  EQUAL,                 //!< ＝ 記号です。
  GREATER_THAN,          //!< ＞ 記号です。
  GREATER_THAN_OR_EQUAL, //!< ＞＝ 記号です。
  LESS_THAN,             //!< ＜ 記号です。
  LESS_THAN_OR_EQUAL,    //!< ＜＝ 記号です。
  MINUS,                 //!< － 記号です。
  NOT_EQUAL,             //!< ＜＞ 記号です。
  OPEN_PAREN,            //!< （ 記号です。
  PLUS,                  //!< ＋ 記号です。
  SLASH,                 //!< ／ 記号です。
  IDENTIFIER,            //!< 識別子です。
  INT_LITERAL,           //!< 整数リテラルです。
  STRING_LITERAL         //!< 文字列リテラルです。
};

//! 一つの値を持つデータです。
typedef struct {
  enum DATA_TYPE type; //!< データの型です。

  //! 実際のデータを格納する共用体です。
  union {
    char string[MAX_DATA_LENGTH]; //!< データが文字列型の場合の値です。
    int integer;  //!< データが整数型の場合の値です。
    bool boolean; //!< データが真偽値型の場合の値です。
  } value;
} Data;

//! WHERE句に指定する演算子の情報を表します。
@interface Operator : NSObject
@property enum TOKEN_KIND
    kind;            //!< 演算子の種類を、演算子を記述するトークンの種類で表します。
@property int order; //!< 演算子の優先順位です。
- (Operator *)init;
- (Operator *)initWithKind:(enum TOKEN_KIND)kind Order:(int)order;
@end

//! トークンを表します。
@interface Token : NSObject {
  char __word[MAX_WORD_LENGTH];
}
@property enum TOKEN_KIND kind; //!< トークンの種類です。
@property char *word;           //!<
//!記録されているトークンの文字列です。記録の必要がなければ空白です。
- (Token *)init;
- (Token *)initWithKind:(enum TOKEN_KIND)kind Word:(char *)word;

@end

//! 指定された列の情報です。どのテーブルに所属するかの情報も含みます。
@interface Column : NSObject {
  char __tableName[MAX_WORD_LENGTH];
  char __columnName[MAX_WORD_LENGTH];
}
@property char *tableName; //!<
//!列が所属するテーブル名です。指定されていない場合は空文字列となります。
@property char *columnName; //!< 指定された列の列名です。
- (Column *)init;
@end

//! WHERE句の条件の式木を表します。
@interface ExtensionTreeNode : NSObject {
  Data __value;
}
- (ExtensionTreeNode *)init;
@property __weak ExtensionTreeNode
    *parent;                        //!< 親となるノードです。根の式木の場合はNULLとなります。
@property ExtensionTreeNode *left;  //!<
                                    //!左の子となるノードです。自身が末端の葉となる式木の場合はNULLとなります。
@property Operator *operator;       //!<
                                    //!中置される演算子です。自身が末端のとなる式木の場合の種類はNOT_TOKENとなります。
@property ExtensionTreeNode *right; //!<
                                    //!右の子となるノードです。自身が末端の葉となる式木の場合はNULLとなります。
@property bool inParen; //!< 自身がかっこにくるまれているかどうかです。
@property int parenOpenBeforeClose; //!<
                                    //!木の構築中に0以外となり、自身の左にあり、まだ閉じてないカッコの開始の数となります。
@property int signCoefficient;      //!<
                                    //!自身が葉にあり、マイナス単項演算子がついている場合は-1、それ以外は1となります。
@property Column *column;           //!<
                                    //!列場指定されている場合に、その列を表します。列指定ではない場合はcolumnNameが空文字列となります。
@property bool calculated; //!< 式の値を計算中に、計算済みかどうかです。
@property(nonatomic) Data *value; //!< 指定された、もしくは計算された値です。

@end

//! 行の情報を入力のテーブルインデックス、列インデックスの形で持ちます。
@interface ColumnIndex : NSObject
- (ColumnIndex *)initWithTable:(int)table Column:(int)column;
@property int table;  //!< 列が入力の何テーブル目の列かです。
@property int column; //!< 列が入力のテーブルの何列目かです。
@end

@interface TynySQLException : NSException
- (TynySQLException *)initWithErrorCode:(enum RESULT_VALUE)code;
@property enum RESULT_VALUE errorCode;
@end

// 以上ヘッダに相当する部分。

@implementation Operator
- (Operator *)init {
  _kind = NOT_TOKEN;
  _order = 0;
  return self;
}
- (Operator *)initWithKind:(enum TOKEN_KIND)kind Order:(int)order {
  _kind = kind;
  _order = order;
  return self;
}
@end
@implementation Token

- (Token *)init {
  _kind = NOT_TOKEN;
  strcpy(__word, "");
  _word = __word;
  return self;
}
- (Token *)initWithKind:(enum TOKEN_KIND)kind Word:(char *)word {
  _kind = kind;
  strncpy(__word, word, MAX_WORD_LENGTH);
  _word = __word;
  return self;
}

@end

@implementation Column

- (Column *)init {
  strcpy(__tableName, "");
  _tableName = __tableName;
  strcpy(__columnName, "");
  _columnName = __columnName;
  return self;
}

@end

@implementation ExtensionTreeNode

- (ExtensionTreeNode *)init {
  _parent = nil;
  _left = nil;
  _operator = [[Operator alloc] init];
  _right = nil;
  _inParen = false;
  _parenOpenBeforeClose = 0;
  _signCoefficient = 1;
  _column = [[Column alloc] init];
  _calculated = false;
  __value = (Data){.type = STRING, .value = {.string = ""}};
  _value = &__value;
  return self;
}
- (void)setValue:(Data *)value {
  __value = *value;
}
@end

@implementation ColumnIndex
- (ColumnIndex *)initWithTable:(int)table Column:(int)column {
  _table = table;
  _column = column;
  return self;
}
@end

@implementation TynySQLException
- (TynySQLException *)initWithErrorCode:(enum RESULT_VALUE)code {
  _errorCode = code;
  return self;
}

@end

//! カレントディレクトリにあるCSVに対し、簡易的なSQLを実行し、結果をファイルに出力します。
//! @param [in] sql 実行するSQLです。
//! @param[in] outputFileName
//! SQLの実行結果をCSVとして出力するファイル名です。拡張子を含みます。
//! @return 実行した結果の状態です。
//! @retval OK=0                      問題なく終了しました。
//! @retval ERR_FILE_OPEN=1           ファイルを開くことに失敗しました。
//! @retval ERR_FILE_WRITE=2 ファイルに書き込みを行うことに失敗しました。
//! @retval ERR_FILE_CLOSE=3          ファイルを閉じることに失敗しました。
//! @retval ERR_TOKEN_CANT_READ=4     トークン解析に失敗しました。
//! @retval ERR_SQL_SYNTAX=5          SQLの構文解析が失敗しました。
//! @retval ERR_BAD_COLUMN_NAME=6 テーブル指定を含む列名が適切ではありません。
//! @retval ERR_WHERE_OPERAND_TYPE=7  演算の左右の型が適切ではありません。
//! @retval ERR_CSV_SYNTAX=8          CSVの構文解析が失敗しました。
//! @retval ERR_MEMORY_ALLOCATE=9     メモリの取得に失敗しました。
//! @retval ERR_MEMORY_OVER=10        用意したメモリ領域の上限を超えました。
//! @details
//! 参照するテーブルは、テーブル名.csvの形で作成します。 @n
//! 一行目はヘッダ行で、その行に列名を書きます。 @n
//! 前後のスペース読み飛ばしやダブルクォーテーションでくくるなどの機能はありません。
//! @n
//! 列の型の定義はできないので、列のすべてのデータの値が数値として解釈できる列のデータを整数として扱います。
//! @n
//! 実行するSQLで使える機能を以下に例としてあげます。 @n
//! 例1: @n
//! SELECT * @n
//! FROM USERS @n
//!                                                                                                          @n
//! 例2: 大文字小文字は区別しません。 @n
//! select * @n
//! from users @n
//!                                                                                                          @n
//! 例3: 列の指定ができます。 @n
//! SELECT Id, Name @n
//! FROM USERS @n
//!                                                                                                          @n
//! 例4: テーブル名を指定して列の指定ができます。 @n
//! SELECT USERS.Id @n
//! FROM USERS @n
//!                                                                                                          @n
//! 例5: ORDER句が使えます。 @n
//! SELECT * @n
//! ORDER BY NAME @n
//! FROM USERS @n
//!                                                                                                          @n
//! 例6: ORDER句に複数列や昇順、降順の指定ができます。 @n
//! SELECT * @n
//! ORDER BY AGE DESC, Name ASC @n
//!                                                                                                          @n
//! 例7: WHERE句が使えます。 @n
//! SELECT * @n
//! WHERE AGE >= 20 @n
//! FROM USERS @n
//!                                                                                                          @n
//! 例8: WHERE句では文字列の比較も使えます。 @n
//! SELECT * @n
//! WHERE NAME >= 'N' @n
//! FROM USERS @n
//!                                                                                                          @n
//! 例9: WHERE句には四則演算、カッコ、AND、ORなどを含む複雑な式が利用できます。
//! @n
//! SELECT * @n
//! WHERE AGE >= 20 AND (AGE <= 40 || WEIGHT < 100) @n
//! FROM USERS @n
//!                                                                                                          @n
//! 例10: FROM句に複数のテーブルが指定できます。その場合はクロスで結合します。
//! @n
//! SELECT * @n
//! FROM USERS, CHILDREN @n
//!                                                                                                          @n
//! 例11: WHEREで条件をつけることにより、テーブルの結合ができます。 @n
//! SELECT USERS.NAME, CHILDREN.NAME @n
//! WHERE USERS.ID = CHILDREN.PARENTID @n
//! FROM USERS, CHILDREN @n
int ExecuteSQL(const char *sql, const char *outputFileName) {
  enum RESULT_VALUE error = OK; // 発生したエラーの種類です。
  FILE *inputTableFiles[MAX_TABLE_COUNT] = {
      NULL}; // 読み込む入力ファイルの全てのファイルポインタです。
  FILE *outputFile = NULL; // 書き込むファイルのファイルポインタです。
  Data ***currentRow = NULL; // データ検索時に現在見ている行を表します。
  Data **inputData[MAX_TABLE_COUNT][MAX_ROW_COUNT]; // 入力データです。
  Data **outputData[MAX_ROW_COUNT] = {NULL};        // 出力データです。
  Data **allColumnOutputData[MAX_ROW_COUNT] = {
      NULL};             // 出力するデータに対応するインデックスを持ち、すべての入力データを保管します。
  int tableNamesNum = 0; // 現在読み込まれているテーブル名の数です。
  @try {

    int result = 0; // 関数の戻り値を一時的に保存します。
    bool found =
        false;                 // 検索時に見つかったかどうかの結果を一時的に保存します。
    const char *search = NULL; // 文字列検索に利用するポインタです。

    const char *alpahUnder =
        "_abcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPQRSTUVWXY"
        "Z"; // 全てのアルファベットの大文字小文字とアンダーバーです。
    const char *alpahNumUnder =
        "_abcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPQRSTUVWXYZ0123"
        "456789";                         // 全ての数字とアルファベットの大文字小文字とアンダーバーです。
    const char *signNum = "+-0123456789"; // 全ての符号と数字です。
    const char *num = "0123456789";       // 全ての数字です。
    const char *space = " \t\r\n";        // 全ての空白文字です。

    // inputDataを初期化します。
    for (size_t i = 0; i < sizeof(inputData) / sizeof(inputData[0]); i++) {
      for (size_t j = 0; j < sizeof(inputData[0]) / sizeof(inputData[0][0]);
           j++) {
        inputData[i][j] = NULL;
      }
    }

    // SQLからトークンを読み込みます。

    // keywordConditionsとsignConditionsは先頭から順に検索されるので、前方一致となる二つの項目は順番に気をつけて登録しなくてはいけません。

    // キーワードをトークンとして認識するためのキーワード一覧情報です。
    NSArray *keywordConditions = @[
      [[Token alloc] initWithKind:AND Word:"AND"],
      [[Token alloc] initWithKind:ASC Word:"ASC"],
      [[Token alloc] initWithKind:BY Word:"BY"],
      [[Token alloc] initWithKind:DESC Word:"DESC"],
      [[Token alloc] initWithKind:FROM Word:"FROM"],
      [[Token alloc] initWithKind:ORDER Word:"ORDER"],
      [[Token alloc] initWithKind:OR Word:"OR"],
      [[Token alloc] initWithKind:SELECT Word:"SELECT"],
      [[Token alloc] initWithKind:WHERE Word:"WHERE"]
    ];

    // 記号をトークンとして認識するための記号一覧情報です。
    NSArray *signConditions = @[
      [[Token alloc] initWithKind:GREATER_THAN_OR_EQUAL Word:">="],
      [[Token alloc] initWithKind:LESS_THAN_OR_EQUAL Word:"<="],
      [[Token alloc] initWithKind:NOT_EQUAL Word:"<>"],
      [[Token alloc] initWithKind:ASTERISK Word:"*"],
      [[Token alloc] initWithKind:COMMA Word:","],
      [[Token alloc] initWithKind:CLOSE_PAREN Word:")"],
      [[Token alloc] initWithKind:DOT Word:"."],
      [[Token alloc] initWithKind:EQUAL Word:"="],
      [[Token alloc] initWithKind:GREATER_THAN Word:">"],
      [[Token alloc] initWithKind:LESS_THAN Word:"<"],
      [[Token alloc] initWithKind:MINUS Word:"-"],
      [[Token alloc] initWithKind:OPEN_PAREN Word:"("],
      [[Token alloc] initWithKind:PLUS Word:"+"],
      [[Token alloc] initWithKind:SLASH Word:"/"]
    ];

    NSMutableArray *tokens =
        [NSMutableArray array]; // SQLを分割したトークンです。

    // 演算子の情報です。
    Operator *operators[] = {
        [[Operator alloc] initWithKind:ASTERISK Order:1],
        [[Operator alloc] initWithKind:SLASH Order:1],
        [[Operator alloc] initWithKind:PLUS Order:2],
        [[Operator alloc] initWithKind:MINUS Order:2],
        [[Operator alloc] initWithKind:EQUAL Order:3],
        [[Operator alloc] initWithKind:GREATER_THAN Order:3],
        [[Operator alloc] initWithKind:GREATER_THAN_OR_EQUAL Order:3],
        [[Operator alloc] initWithKind:LESS_THAN Order:3],
        [[Operator alloc] initWithKind:LESS_THAN_OR_EQUAL Order:3],
        [[Operator alloc] initWithKind:NOT_EQUAL Order:3],
        [[Operator alloc] initWithKind:AND Order:4],
        [[Operator alloc] initWithKind:OR Order:5]};

    const char *charactorBackPoint =
        NULL; // SQLをトークンに分割して読み込む時に戻るポイントを記録しておきます。

    const char *charactorCursol =
        sql; // SQLをトークンに分割して読み込む時に現在読んでいる文字の場所を表します。

    char tableNames[MAX_TABLE_COUNT]
                   [MAX_WORD_LENGTH]; // FROM句で指定しているテーブル名です。
    // tableNamesを初期化します。
    for (size_t i = 0; i < sizeof(tableNames) / sizeof(tableNames[0]); i++) {
      strncpy(tableNames[i], "", MAX_WORD_LENGTH);
    }

    // SQLをトークンに分割て読み込みます。
    while (*charactorCursol) {

      // 空白を読み飛ばします。
      for (search = space; *search && *charactorCursol != *search; ++search) {
      }
      if (*search) {
        charactorCursol++;
        continue;
      }

      // 数値リテラルを読み込みます。

      // 先頭文字が数字であるかどうかを確認します。
      charactorBackPoint = charactorCursol;
      for (search = num; *search && *charactorCursol != *search; ++search) {
      }
      if (*search) {
        Token *literal = [[Token alloc]
            initWithKind:INT_LITERAL
                    Word:""]; // 読み込んだ数値リテラルの情報です。
        int wordLength = 0; // 数値リテラルに現在読み込んでいる文字の数です。

        // 数字が続く間、文字を読み込み続けます。
        do {
          for (search = num; *search && *charactorCursol != *search; ++search) {
          }
          if (*search) {
            if (MAX_WORD_LENGTH - 1 <= wordLength) {
              @throw
                  [[TynySQLException alloc] initWithErrorCode:ERR_MEMORY_OVER];
            }
            literal.word[wordLength++] = *search;
            ++charactorCursol;
          }
        } while (*search);

        // 数字の後にすぐに識別子が続くのは紛らわしいので数値リテラルとは扱いません。
        for (search = alpahUnder; *search && *charactorCursol != *search;
             ++search) {
        }
        if (!*search) {
          literal.word[wordLength] = '\0';
          [tokens addObject:literal];
          continue;
        } else {
          charactorCursol = charactorBackPoint;
        }
      }

      // 文字列リテラルを読み込みます。

      // 文字列リテラルを開始するシングルクォートを判別し、読み込みます。
      // メトリクス測定ツールのccccはシングルクォートの文字リテラル中のエスケープを認識しないため、文字リテラルを使わないことで回避しています。
      if (*charactorCursol == "\'"[0]) {
        ++charactorCursol;
        Token *literal = [[Token alloc]
            initWithKind:STRING_LITERAL
                    Word:"\'"]; // 読み込んだ文字列リテラルの情報です。
        int wordLength =
            1; // 文字列リテラルに現在読み込んでいる文字の数です。初期値の段階で最初のシングルクォートは読み込んでいます。

        // 次のシングルクォートがくるまで文字を読み込み続けます。
        while (*charactorCursol && *charactorCursol != "\'"[0]) {
          if (MAX_WORD_LENGTH - 1 <= wordLength) {
            @throw [[TynySQLException alloc] initWithErrorCode:ERR_MEMORY_OVER];
          }
          literal.word[wordLength++] = *charactorCursol++;
        }
        if (*charactorCursol == "\'"[0]) {
          if (MAX_WORD_LENGTH - 1 <= wordLength) {
            @throw [[TynySQLException alloc] initWithErrorCode:ERR_MEMORY_OVER];
          }
          // 最後のシングルクォートを読み込みます。
          literal.word[wordLength++] = *charactorCursol++;

          // 文字列の終端文字をつけます。
          literal.word[wordLength] = '\0';
          [tokens addObject:literal];
          continue;
        } else {
          @throw
              [[TynySQLException alloc] initWithErrorCode:ERR_TOKEN_CANT_READ];
        }
      }

      // キーワードを読み込みます。
      found = false;
      for (Token *condition in keywordConditions) {
        charactorBackPoint = charactorCursol;
        char *wordCursol =
            condition
                .word; // 確認するキーワードの文字列のうち、現在確認している一文字を指します。

        // キーワードが指定した文字列となっているか確認します。
        while (*wordCursol && toupper(*charactorCursol++) == *wordCursol) {
          ++wordCursol;
        }

        // キーワードに識別子が区切りなしに続いていないかを確認するため、キーワードの終わった一文字あとを調べます。
        for (search = alpahNumUnder; *search && *charactorCursol != *search;
             ++search) {
        };

        if (!*wordCursol && !*search) {

          // 見つかったキーワードを生成します。
          [tokens
              addObject:[[Token alloc] initWithKind:condition.kind Word:""]];
          found = true;
        } else {
          charactorCursol = charactorBackPoint;
        }
      }
      if (found) {
        continue;
      }

      // 記号を読み込みます。
      found = false;
      for (Token *condition in signConditions) {
        charactorBackPoint = charactorCursol;
        char *wordCursol =
            condition
                .word; // 確認する記号の文字列のうち、現在確認している一文字を指します。

        // 記号が指定した文字列となっているか確認します。
        while (*wordCursol && toupper(*charactorCursol++) == *wordCursol) {
          ++wordCursol;
        }
        if (!*wordCursol) {

          // 見つかった記号を生成します。
          [tokens
              addObject:[[Token alloc] initWithKind:condition.kind Word:""]];
          found = true;
        } else {
          charactorCursol = charactorBackPoint;
        }
      }
      if (found) {
        continue;
      }

      // 識別子を読み込みます。

      // 識別子の最初の文字を確認します。
      for (search = alpahUnder; *search && *charactorCursol != *search;
           ++search) {
      };
      if (*search) {
        Token *identifier =
            [[Token alloc] initWithKind:IDENTIFIER
                                   Word:""]; // 読み込んだ識別子の情報です。
        int wordLength = 0; // 識別子に現在読み込んでいる文字の数です。
        do {
          // 二文字目以降は数字も許可して文字の種類を確認します。
          for (search = alpahNumUnder; *search && *charactorCursol != *search;
               ++search) {
          };
          if (*search) {
            if (MAX_WORD_LENGTH - 1 <= wordLength) {
              @throw
                  [[TynySQLException alloc] initWithErrorCode:ERR_MEMORY_OVER];
            }
            identifier.word[wordLength++] = *search;
            charactorCursol++;
          }
        } while (*search);

        // 識別子の文字列の終端文字を設定します。
        identifier.word[wordLength] = '\0';

        // 読み込んだ識別子を登録します。
        [tokens addObject:identifier];
        continue;
      }

      @throw [[TynySQLException alloc] initWithErrorCode:ERR_TOKEN_CANT_READ];
    }

    // トークン列を解析し、構文を読み取ります。

    NSEnumerator *tokenCursol =
        [tokens objectEnumerator]; // 現在見ているトークンを指します。

    Column *selectColumns[MAX_TABLE_COUNT *
                          MAX_COLUMN_COUNT]; // SELECT句に指定された列名です。
    // selectColumnsを初期化します。
    for (size_t i = 0; i < sizeof(selectColumns) / sizeof(selectColumns[0]);
         i++) {
      selectColumns[i] = [[Column alloc] init];
    }
    int selectColumnsNum = 0; // SELECT句から現在読み込まれた列名の数です。

    Column *orderByColumns[MAX_COLUMN_COUNT]; // ORDER句に指定された列名です。
    // orderByColumnsを初期化します。
    for (size_t i = 0; i < sizeof(orderByColumns) / sizeof(orderByColumns[0]);
         i++) {
      orderByColumns[i] = [[Column alloc] init];
    }
    int orderByColumnsNum = 0; // ORDER句から現在読み込まれた列名の数です。

    enum TOKEN_KIND orders[MAX_COLUMN_COUNT] = {
        0}; // 同じインデックスのorderByColumnsに対応している、昇順、降順の指定です。

    ExtensionTreeNode *whereExtensionNodes
        [MAX_EXTENSION_TREE_NODE_COUNT]; // WHEREに指定された木のノードを、木構造とは無関係に格納します。
    // whereExtensionNodesを初期化します。
    for (size_t i = 0;
         i < sizeof(whereExtensionNodes) / sizeof(whereExtensionNodes[0]);
         i++) {
      whereExtensionNodes[i] = [[ExtensionTreeNode alloc] init];
    }
    int whereExtensionNodesNum =
        0; // 現在読み込まれているのwhereExtensionNodesの数です。

    ExtensionTreeNode *whereTopNode = NULL; // 式木の根となるノードです。

    // SQLの構文を解析し、必要な情報を取得します。

    // SELECT句を読み込みます。
    Token *nextToken = [tokenCursol nextObject];
    if (nextToken.kind == SELECT) {
      nextToken = [tokenCursol nextObject];
    } else {
      @throw [[TynySQLException alloc] initWithErrorCode:ERR_SQL_SYNTAX];
    }

    if (nextToken.kind == ASTERISK) {
      nextToken = [tokenCursol nextObject];
    } else {
      bool first =
          true; // SELECT句に最初に指定された列名の読み込みかどうかです。
      while (nextToken.kind == COMMA || first) {
        if (nextToken.kind == COMMA) {
          nextToken = [tokenCursol nextObject];
          ;
        }
        if (nextToken.kind == IDENTIFIER) {
          if (MAX_COLUMN_COUNT <= selectColumnsNum) {

            @throw [[TynySQLException alloc] initWithErrorCode:ERR_MEMORY_OVER];
          }
          // テーブル名が指定されていない場合と仮定して読み込みます。
          strncpy(selectColumns[selectColumnsNum].tableName, "",
                  MAX_WORD_LENGTH);
          strncpy(selectColumns[selectColumnsNum].columnName, nextToken.word,
                  MAX_WORD_LENGTH);
          nextToken = [tokenCursol nextObject];
          ;
          if (nextToken.kind == DOT) {
            nextToken = [tokenCursol nextObject];
            ;
            if (nextToken.kind == IDENTIFIER) {

              // テーブル名が指定されていることがわかったので読み替えます。
              strncpy(selectColumns[selectColumnsNum].tableName,
                      selectColumns[selectColumnsNum].columnName,
                      MAX_WORD_LENGTH);
              strncpy(selectColumns[selectColumnsNum].columnName,
                      nextToken.word, MAX_WORD_LENGTH);
              nextToken = [tokenCursol nextObject];
              ;
            } else {
              @throw
                  [[TynySQLException alloc] initWithErrorCode:ERR_SQL_SYNTAX];
            }
          }
          ++selectColumnsNum;
        } else {
          @throw [[TynySQLException alloc] initWithErrorCode:ERR_SQL_SYNTAX];
        }
        first = false;
      }
    }

    // ORDER句とWHERE句を読み込みます。最大各一回ずつ書くことができます。
    bool readOrder = false; // すでにORDER句が読み込み済みかどうかです。
    bool readWhere = false; // すでにWHERE句が読み込み済みかどうかです。
    while (nextToken.kind == ORDER || nextToken.kind == WHERE) {

      // 二度目のORDER句はエラーです。
      if (readOrder && nextToken.kind == ORDER) {
        @throw [[TynySQLException alloc] initWithErrorCode:ERR_SQL_SYNTAX];
      }

      // 二度目のWHERE句はエラーです。
      if (readWhere && nextToken.kind == WHERE) {
        @throw [[TynySQLException alloc] initWithErrorCode:ERR_SQL_SYNTAX];
      }
      // ORDER句を読み込みます。
      if (nextToken.kind == ORDER) {
        readOrder = true;
        nextToken = [tokenCursol nextObject];
        ;
        if (nextToken.kind == BY) {
          nextToken = [tokenCursol nextObject];
          ;
          bool first = true; // ORDER句の最初の列名の読み込みかどうかです。
          while (nextToken.kind == COMMA || first) {
            if (nextToken.kind == COMMA) {
              nextToken = [tokenCursol nextObject];
              ;
            }
            if (nextToken.kind == IDENTIFIER) {
              if (MAX_COLUMN_COUNT <= orderByColumnsNum) {
                @throw [[TynySQLException alloc]
                    initWithErrorCode:ERR_MEMORY_OVER];
              }
              // テーブル名が指定されていない場合と仮定して読み込みます。
              strncpy(orderByColumns[orderByColumnsNum].tableName, "",
                      MAX_WORD_LENGTH);
              strncpy(orderByColumns[orderByColumnsNum].columnName,
                      nextToken.word, MAX_WORD_LENGTH);
              nextToken = [tokenCursol nextObject];
              ;
              if (nextToken.kind == DOT) {
                nextToken = [tokenCursol nextObject];
                ;
                if (nextToken.kind == IDENTIFIER) {

                  // テーブル名が指定されていることがわかったので読み替えます。
                  strncpy(orderByColumns[orderByColumnsNum].tableName,
                          orderByColumns[orderByColumnsNum].columnName,
                          MAX_WORD_LENGTH);
                  strncpy(orderByColumns[orderByColumnsNum].columnName,
                          nextToken.word, MAX_WORD_LENGTH);
                  nextToken = [tokenCursol nextObject];
                  ;
                } else {
                  @throw [[TynySQLException alloc]
                      initWithErrorCode:ERR_SQL_SYNTAX];
                }
              }

              // 並び替えの昇順、降順を指定します。
              if (nextToken.kind == ASC) {
                orders[orderByColumnsNum] = ASC;
                nextToken = [tokenCursol nextObject];
                ;
              } else if (nextToken.kind == DESC) {
                orders[orderByColumnsNum] = DESC;
                nextToken = [tokenCursol nextObject];
                ;
              } else {
                // 指定がない場合は昇順となります。
                orders[orderByColumnsNum] = ASC;
              }
              ++orderByColumnsNum;
            } else {
              @throw
                  [[TynySQLException alloc] initWithErrorCode:ERR_SQL_SYNTAX];
            }
            first = false;
          }
        } else {
          @throw [[TynySQLException alloc] initWithErrorCode:ERR_SQL_SYNTAX];
        }
      }

      // WHERE句を読み込みます。
      if (nextToken.kind == WHERE) {
        readWhere = true;
        nextToken = [tokenCursol nextObject];
        ;
        ExtensionTreeNode *currentNode = NULL; // 現在読み込んでいるノードです。
        while (true) {
          // オペランドを読み込みます。

          // オペランドのノードを新しく生成します。
          if (MAX_EXTENSION_TREE_NODE_COUNT <= whereExtensionNodesNum) {
            @throw [[TynySQLException alloc] initWithErrorCode:ERR_MEMORY_OVER];
          }
          if (currentNode) {
            // 現在のノードを右の子にずらし、元の位置に新しいノードを挿入します。
            currentNode.right = whereExtensionNodes[whereExtensionNodesNum++];
            currentNode.right.parent = currentNode;
            currentNode = currentNode.right;
          } else {
            // 最初はカレントノードに新しいノードを入れます。
            currentNode = whereExtensionNodes[whereExtensionNodesNum++];
          }

          // カッコ開くを読み込みます。
          while (nextToken.kind == OPEN_PAREN) {
            ++currentNode.parenOpenBeforeClose;
            nextToken = [tokenCursol nextObject];
            ;
          }

          // オペランドに前置される+か-を読み込みます。
          if (nextToken.kind == PLUS || nextToken.kind == MINUS) {
            if (nextToken.kind == MINUS) {
              currentNode.signCoefficient = -1;
            }
            nextToken = [tokenCursol nextObject];
            // +-を前置するのは列名と数値リテラルのみです。
            if (nextToken.kind != IDENTIFIER && nextToken.kind != INT_LITERAL) {
              @throw [[TynySQLException alloc]
                  initWithErrorCode:ERR_WHERE_OPERAND_TYPE];
            }
          }

          // 列名、整数リテラル、文字列リテラルのいずれかをオペランドとして読み込みます。
          if (nextToken.kind == IDENTIFIER) {

            // テーブル名が指定されていない場合と仮定して読み込みます。
            strncpy(currentNode.column.tableName, "", MAX_WORD_LENGTH);
            strncpy(currentNode.column.columnName, nextToken.word,
                    MAX_WORD_LENGTH);
            nextToken = [tokenCursol nextObject];
            ;
            if (nextToken.kind == DOT) {
              nextToken = [tokenCursol nextObject];
              ;
              if (nextToken.kind == IDENTIFIER) {

                // テーブル名が指定されていることがわかったので読み替えます。
                strncpy(currentNode.column.tableName,
                        currentNode.column.columnName, MAX_WORD_LENGTH);
                strncpy(currentNode.column.columnName, nextToken.word,
                        MAX_WORD_LENGTH);
                nextToken = [tokenCursol nextObject];
                ;
              } else {
                @throw
                    [[TynySQLException alloc] initWithErrorCode:ERR_SQL_SYNTAX];
              }
            }
          } else if (nextToken.kind == INT_LITERAL) {
            currentNode.value = &(Data){
                .type = INTEGER, .value = {.integer = atoi(nextToken.word)}};
            nextToken = [tokenCursol nextObject];
            ;
          } else if (nextToken.kind == STRING_LITERAL) {
            currentNode.value =
                &(Data){.type = STRING, .value = {.string = ""}};

            // 前後のシングルクォートを取り去った文字列をデータとして読み込みます。
            strncpy(currentNode.value->value.string, nextToken.word + 1,
                    MAX_WORD_LENGTH < MAX_DATA_LENGTH ? MAX_WORD_LENGTH
                                                      : MAX_DATA_LENGTH);
            currentNode.value->value.string[MAX_DATA_LENGTH - 1] = '\0';
            currentNode.value->value
                .string[strlen(currentNode.value->value.string) - 1] = '\0';
            nextToken = [tokenCursol nextObject];
            ;
          } else {
            @throw [[TynySQLException alloc] initWithErrorCode:ERR_SQL_SYNTAX];
          }

          // オペランドの右のカッコ閉じるを読み込みます。
          while (nextToken.kind == CLOSE_PAREN) {
            ExtensionTreeNode *searchedAncestor =
                currentNode
                    .parent; // カッコ閉じると対応するカッコ開くを両方含む祖先ノードを探すためのカーソルです。
            while (searchedAncestor) {

              // searchedAncestorの左の子に対応するカッコ開くがないかを検索します。
              ExtensionTreeNode *searched =
                  searchedAncestor; // searchedAncestorの内部からカッコ開くを検索するためのカーソルです。
              while (searched && !searched.parenOpenBeforeClose) {
                searched = searched.left;
              }
              if (searched) {
                // 対応付けられていないカッコ開くを一つ削除し、ノードがカッコに囲まれていることを記録します。
                --searched.parenOpenBeforeClose;
                searchedAncestor.inParen = true;
                break;
              } else {
                searchedAncestor = searchedAncestor.parent;
              }
            }
            nextToken = [tokenCursol nextObject];
            ;
          }

          // 演算子(オペレーターを読み込みます。
          Operator *operator=
              [[Operator alloc] init]; // 現在読み込んでいる演算子の情報です。

          // 現在見ている演算子の情報を探します。
          found = false;
          for (int j = 0; j < sizeof(operators) / sizeof(operators[0]); ++j) {
            if (operators[j].kind == nextToken.kind) {
              operator= operators[j];
              found = true;
              break;
            }
          }
          if (found) {
            // 見つかった演算子の情報をもとにノードを入れ替えます。
            ExtensionTreeNode *tmp =
                currentNode; //ノードを入れ替えるために使う変数です。

            ExtensionTreeNode *searched =
                tmp; // 入れ替えるノードを探すためのカーソルです。

            //カッコにくくられていなかった場合に、演算子の優先順位を参考に結合するノードを探します。
            bool first = true; // 演算子の優先順位を検索する最初のループです。
            do {
              if (!first) {
                tmp = tmp.parent;
                searched = tmp;
              }
              // 現在の読み込み場所をくくるカッコが開く場所を探します。
              while (searched && !searched.parenOpenBeforeClose) {
                searched = searched.left;
              }
              first = false;
            } while (!searched && tmp.parent &&
                     (tmp.parent.operator.order <= operator.order ||
                      tmp.parent.inParen));

            // 演算子のノードを新しく生成します。
            if (MAX_EXTENSION_TREE_NODE_COUNT <= whereExtensionNodesNum) {
              @throw
                  [[TynySQLException alloc] initWithErrorCode:ERR_MEMORY_OVER];
            }
            currentNode = whereExtensionNodes[whereExtensionNodesNum++];
            currentNode.operator= operator;

            // 見つかった場所に新しいノードを配置します。これまでその位置にあったノードは左の子となるよう、親ノードと子ノードのポインタをつけかえます。
            currentNode.parent = tmp.parent;
            if (currentNode.parent) {
              currentNode.parent.right = currentNode;
            }
            currentNode.left = tmp;
            tmp.parent = currentNode;

            nextToken = [tokenCursol nextObject];
            ;
          } else {
            // 現在見ている種類が演算子の一覧から見つからなければ、WHERE句は終わります。
            break;
          }
        }

        // 木を根に向かってさかのぼり、根のノードを設定します。
        whereTopNode = currentNode;
        while (whereTopNode.parent) {
          whereTopNode = whereTopNode.parent;
        }
      }
    }

    // FROM句を読み込みます。
    if (nextToken.kind == FROM) {
      nextToken = [tokenCursol nextObject];
      ;
    } else {
      @throw [[TynySQLException alloc] initWithErrorCode:ERR_SQL_SYNTAX];
    }
    bool first = true; // FROM句の最初のテーブル名を読み込み中かどうかです。
    while (nextToken.kind == COMMA || first) {
      if (nextToken.kind == COMMA) {
        nextToken = [tokenCursol nextObject];
        ;
      }
      if (nextToken.kind == IDENTIFIER) {
        if (MAX_TABLE_COUNT <= tableNamesNum) {
          @throw [[TynySQLException alloc] initWithErrorCode:ERR_MEMORY_OVER];
        }
        strncpy(tableNames[tableNamesNum++], nextToken.word, MAX_WORD_LENGTH);
        nextToken = [tokenCursol nextObject];
        ;
      } else {
        @throw [[TynySQLException alloc] initWithErrorCode:ERR_SQL_SYNTAX];
      }
      first = false;
    }

    // 最後のトークンまで読み込みが進んでいなかったらエラーです。
    if (nextToken) {
      @throw [[TynySQLException alloc] initWithErrorCode:ERR_SQL_SYNTAX];
    }
    Column *inputColumns[MAX_TABLE_COUNT]
                        [MAX_COLUMN_COUNT]; // 入力されたCSVの行の情報です。
    // inputColumnsを初期化します。
    for (size_t i = 0; i < sizeof(inputColumns) / sizeof(inputColumns[0]);
         i++) {
      for (size_t j = 0;
           j < sizeof(inputColumns[0]) / sizeof(inputColumns[0][0]); j++) {
        inputColumns[i][j] = [[Column alloc] init];
      }
    }
    int inputColumnNums[MAX_TABLE_COUNT] = {0}; // 各テーブルごとの列の数です。

    for (int i = 0; i < tableNamesNum; ++i) {

      // 入力ファイル名を生成します。
      const char csvExtension[] = ".csv"; // csvの拡張子です。
      char fileName[MAX_WORD_LENGTH + sizeof(csvExtension) - 1] =
          ""; // 拡張子を含む、入力ファイルのファイル名です。
      strncat(fileName, tableNames[i],
              MAX_WORD_LENGTH + sizeof(csvExtension) - 1);
      strncat(fileName, csvExtension,
              MAX_WORD_LENGTH + sizeof(csvExtension) - 1);

      // 入力ファイルを開きます。
      inputTableFiles[i] = fopen(fileName, "r");
      if (!inputTableFiles[i]) {
        @throw [[TynySQLException alloc] initWithErrorCode:ERR_FILE_OPEN];
      }

      // 入力CSVのヘッダ行を読み込みます。
      char inputLine[MAX_FILE_LINE_LENGTH] =
          ""; // ファイルから読み込んだ行文字列です。
      if (fgets(inputLine, MAX_FILE_LINE_LENGTH, inputTableFiles[i])) {
        charactorCursol = inputLine;

        // 読み込んだ行を最後まで読みます。
        while (*charactorCursol && *charactorCursol != '\r' &&
               *charactorCursol != '\n') {
          if (MAX_COLUMN_COUNT <= inputColumnNums[i]) {
            @throw [[TynySQLException alloc] initWithErrorCode:ERR_MEMORY_OVER];
          }
          strncpy(inputColumns[i][inputColumnNums[i]].tableName, tableNames[i],
                  MAX_WORD_LENGTH);
          char *writeCursol =
              inputColumns[i][inputColumnNums[i]++]
                  .columnName; // 列名の書き込みに利用するカーソルです。

          // 列名を一つ読みます。
          while (*charactorCursol && *charactorCursol != ',' &&
                 *charactorCursol != '\r' && *charactorCursol != '\n') {
            *writeCursol++ = *charactorCursol++;
          }
          // 書き込んでいる列名の文字列に終端文字を書き込みます。
          writeCursol[1] = '\0';

          // 入力行のカンマの分を読み進めます。
          ++charactorCursol;
        }
      } else {
        @throw [[TynySQLException alloc] initWithErrorCode:ERR_CSV_SYNTAX];
      }

      // 入力CSVのデータ行を読み込みます。
      int rowNum = 0;
      while (fgets(inputLine, MAX_FILE_LINE_LENGTH, inputTableFiles[i])) {
        if (MAX_ROW_COUNT <= rowNum) {
          @throw [[TynySQLException alloc] initWithErrorCode:ERR_MEMORY_OVER];
        }
        Data **row = inputData[i][rowNum++] =
            malloc(MAX_COLUMN_COUNT *
                   sizeof(Data *)); // 入力されている一行分のデータです。
        if (!row) {
          @throw
              [[TynySQLException alloc] initWithErrorCode:ERR_MEMORY_ALLOCATE];
        }
        // 生成した行を初期化します。
        for (int j = 0; j < MAX_COLUMN_COUNT; ++j) {
          row[j] = NULL;
        }

        charactorCursol = inputLine;
        int columnNum =
            0; // いま何列目を読み込んでいるか。0基底の数字となります。

        // 読み込んだ行を最後まで読みます。
        while (*charactorCursol && *charactorCursol != '\r' &&
               *charactorCursol != '\n') {

          // 読み込んだデータを書き込む行のカラムを生成します。
          if (MAX_COLUMN_COUNT <= columnNum) {
            @throw [[TynySQLException alloc] initWithErrorCode:ERR_MEMORY_OVER];
          }
          row[columnNum] = malloc(sizeof(Data));
          if (!row[columnNum]) {
            @throw [[TynySQLException alloc]
                initWithErrorCode:ERR_MEMORY_ALLOCATE];
          }
          *row[columnNum] = (Data){.type = STRING, .value = {.string = ""}};
          char *writeCursol =
              row[columnNum++]
                  ->value
                  .string; // データ文字列の書き込みに利用するカーソルです。

          // データ文字列を一つ読みます。
          while (*charactorCursol && *charactorCursol != ',' &&
                 *charactorCursol != '\r' && *charactorCursol != '\n') {
            *writeCursol++ = *charactorCursol++;
          }
          // 書き込んでいる列名の文字列に終端文字を書き込みます。
          writeCursol[1] = '\0';

          // 入力行のカンマの分を読み進めます。
          ++charactorCursol;
        }
      }

      // 全てが数値となる列は数値列に変換します。
      for (int j = 0; j < inputColumnNums[i]; ++j) {

        // 全ての行のある列について、データ文字列から符号と数値以外の文字を探します。
        currentRow = inputData[i];
        found = false;
        while (*currentRow) {
          char *currentChar = (*currentRow)[j]->value.string;
          while (*currentChar) {
            bool isNum = false;
            const char *currentNum = signNum;
            while (*currentNum) {
              if (*currentChar == *currentNum) {
                isNum = true;
                break;
              }
              ++currentNum;
            }
            if (!isNum) {
              found = true;
              break;
            }
            ++currentChar;
          }
          if (found) {
            break;
          }
          ++currentRow;
        }

        // 符号と数字以外が見つからない列については、数値列に変換します。
        if (!found) {
          currentRow = inputData[i];
          while (*currentRow) {
            *(*currentRow)[j] = (Data){
                .type = INTEGER,
                .value = {.integer = atoi((*currentRow)[j]->value.string)}};
            ++currentRow;
          }
        }
      }
    }

    Column *allInputColumns
        [MAX_TABLE_COUNT *
         MAX_COLUMN_COUNT]; // 入力に含まれるすべての列の一覧です。
    // allInputColumnsを初期化します。
    for (size_t i = 0; i < sizeof(allInputColumns) / sizeof(allInputColumns[0]);
         i++) {
      allInputColumns[i] = [[Column alloc] init];
    }
    int allInputColumnsNum = 0; // 入力に含まれるすべての列の数です。

    // 入力ファイルに書いてあったすべての列をallInputColumnsに設定します。
    for (int i = 0; i < tableNamesNum; ++i) {
      for (int j = 0; j < inputColumnNums[i]; ++j) {
        strncpy(allInputColumns[allInputColumnsNum].tableName, tableNames[i],
                MAX_WORD_LENGTH);
        strncpy(allInputColumns[allInputColumnsNum++].columnName,
                inputColumns[i][j].columnName, MAX_WORD_LENGTH);
      }
    }

    // SELECT句の列名指定が*だった場合は、入力CSVの列名がすべて選択されます。
    if (!selectColumnsNum) {
      for (int i = 0; i < allInputColumnsNum; ++i) {
        strncpy(selectColumns[selectColumnsNum].tableName,
                allInputColumns[i].tableName, MAX_WORD_LENGTH);
        strncpy(selectColumns[selectColumnsNum++].columnName,
                allInputColumns[i].columnName, MAX_WORD_LENGTH);
      }
    }

    Column *outputColumns[MAX_TABLE_COUNT *
                          MAX_COLUMN_COUNT]; // 出力するすべての行の情報です。
    // outputColumnsを初期化します。
    for (size_t i = 0; i < sizeof(outputColumns) / sizeof(outputColumns[0]);
         i++) {
      outputColumns[i] = [[Column alloc] init];
    }
    int outputColumnNum = 0; // 出力するすべての行の現在の数です。

    // SELECT句で指定された列名が、何個目の入力ファイルの何列目に相当するかを判別します。
    ColumnIndex *selectColumnIndexes
        [MAX_TABLE_COUNT *
         MAX_COLUMN_COUNT];         // SELECT句で指定された列の、入力ファイルとしてのインデックスです。
    int selectColumnIndexesNum = 0; // selectColumnIndexesの現在の数。
    for (int i = 0; i < selectColumnsNum; ++i) {
      found = false;
      for (int j = 0; j < tableNamesNum; ++j) {
        for (int k = 0; k < inputColumnNums[j]; ++k) {
          char *selectTableNameCursol = selectColumns[i].tableName;
          char *inputTableNameCursol = inputColumns[j][k].tableName;
          while (*selectTableNameCursol &&
                 toupper(*selectTableNameCursol) ==
                     toupper(*inputTableNameCursol++)) {
            ++selectTableNameCursol;
          }
          char *selectColumnNameCursol = selectColumns[i].columnName;
          char *inputColumnNameCursol = inputColumns[j][k].columnName;
          while (*selectColumnNameCursol &&
                 toupper(*selectColumnNameCursol) ==
                     toupper(*inputColumnNameCursol++)) {
            ++selectColumnNameCursol;
          }
          if (!*selectColumnNameCursol && !*inputColumnNameCursol &&
              (!*selectColumns[i]
                     .tableName || // テーブル名が設定されている場合のみテーブル名の比較を行います。
               (!*selectTableNameCursol && !*inputTableNameCursol))) {

            // 既に見つかっているのにもう一つ見つかったらエラーです。
            if (found) {
              @throw [[TynySQLException alloc]
                  initWithErrorCode:ERR_BAD_COLUMN_NAME];
            }
            found = true;
            // 見つかった値を持つ列のデータを生成します。
            if (MAX_COLUMN_COUNT <= selectColumnIndexesNum) {
              @throw
                  [[TynySQLException alloc] initWithErrorCode:ERR_MEMORY_OVER];
            }
            selectColumnIndexes[selectColumnIndexesNum++] =
                [[ColumnIndex alloc] initWithTable:j Column:k];
          }
        }
      }
      // 一つも見つからなくてもエラーです。
      if (!found) {
        @throw [[TynySQLException alloc] initWithErrorCode:ERR_BAD_COLUMN_NAME];
      }
    }

    // 出力する列名を設定します。
    for (int i = 0; i < selectColumnsNum; ++i) {
      strncpy(outputColumns[outputColumnNum].tableName,
              inputColumns[selectColumnIndexes[i].table]
                          [selectColumnIndexes[i].column]
                              .tableName,
              MAX_WORD_LENGTH);
      strncpy(outputColumns[outputColumnNum].columnName,
              inputColumns[selectColumnIndexes[i].table]
                          [selectColumnIndexes[i].column]
                              .columnName,
              MAX_WORD_LENGTH);
      ++outputColumnNum;
    }

    if (whereTopNode) {
      // 既存数値の符号を計算します。
      for (int i = 0; i < whereExtensionNodesNum; ++i) {
        if (whereExtensionNodes[i]
                .
                operator.kind == NOT_TOKEN && !*whereExtensionNodes[i]
                .column.columnName && whereExtensionNodes[i]
                .value->type == INTEGER) {
          whereExtensionNodes[i].value->value.integer *=
              whereExtensionNodes[i].signCoefficient;
        }
      }
    }

    int outputRowsNum = 0; // 出力データの現在の行数です。

    Data ***currentRows[MAX_TABLE_COUNT] = {
        NULL}; // 入力された各テーブルの、現在出力している行を指すカーソルです。
    for (int i = 0; i < tableNamesNum; ++i) {
      // 各テーブルの先頭行を設定します。
      currentRows[i] = inputData[i];
    }

    // 出力するデータを設定します。
    while (true) {
      if (MAX_ROW_COUNT <= outputRowsNum) {
        @throw [[TynySQLException alloc] initWithErrorCode:ERR_MEMORY_OVER];
      }
      Data **row = outputData[outputRowsNum] =
          malloc(MAX_COLUMN_COUNT *
                 sizeof(Data *)); // 出力している一行分のデータです。
      if (!row) {
        @throw [[TynySQLException alloc] initWithErrorCode:ERR_MEMORY_ALLOCATE];
      }

      // 生成した行を初期化します。
      for (int i = 0; i < MAX_COLUMN_COUNT; ++i) {
        row[i] = NULL;
      }

      // 行の各列のデータを入力から持ってきて設定します。
      for (int i = 0; i < selectColumnIndexesNum; ++i) {
        row[i] = malloc(sizeof(Data));
        if (!row[i]) {
          @throw
              [[TynySQLException alloc] initWithErrorCode:ERR_MEMORY_ALLOCATE];
        }
        *row[i] = *(*currentRows[selectColumnIndexes[i]
                                     .table])[selectColumnIndexes[i].column];
      }

      Data **allColumnsRow = allColumnOutputData[outputRowsNum++] = malloc(
          MAX_TABLE_COUNT * MAX_COLUMN_COUNT *
          sizeof(
              Data
                  *)); // WHEREやORDERのためにすべての情報を含む行。rowとインデックスを共有します。
      if (!allColumnsRow) {
        @throw [[TynySQLException alloc] initWithErrorCode:ERR_MEMORY_ALLOCATE];
      }
      // 生成した行を初期化します。
      for (int i = 0; i < MAX_TABLE_COUNT * MAX_COLUMN_COUNT; ++i) {
        allColumnsRow[i] = NULL;
      }

      // allColumnsRowの列を設定します。
      int allColumnsNum = 0; // allColumnsRowの現在の列数です。
      for (int i = 0; i < tableNamesNum; ++i) {
        for (int j = 0; j < inputColumnNums[i]; ++j) {
          allColumnsRow[allColumnsNum] = malloc(sizeof(Data));
          if (!allColumnsRow[allColumnsNum]) {
            @throw [[TynySQLException alloc]
                initWithErrorCode:ERR_MEMORY_ALLOCATE];
          }
          *allColumnsRow[allColumnsNum++] = *(*currentRows[i])[j];
        }
      }
      // WHEREの条件となる値を再帰的に計算します。
      if (whereTopNode) {
        ExtensionTreeNode *currentNode =
            whereTopNode; // 現在見ているノードです。
        while (currentNode) {
          // 子ノードの計算が終わってない場合は、まずそちらの計算を行います。
          if (currentNode.left && !currentNode.left.calculated) {
            currentNode = currentNode.left;
            continue;
          } else if (currentNode.right && !currentNode.right.calculated) {
            currentNode = currentNode.right;
            continue;
          }

          // 自ノードの値を計算します。
          switch (currentNode.operator.kind) {
          case NOT_TOKEN:
            // ノードにデータが設定されている場合です。

            // データが列名で指定されている場合、今扱っている行のデータを設定します。
            if (*currentNode.column.columnName) {
              found = false;
              for (int i = 0; i < allInputColumnsNum; ++i) {
                char *whereTableNameCursol = currentNode.column.tableName;
                char *allInputTableNameCursol = allInputColumns[i].tableName;
                while (*whereTableNameCursol &&
                       toupper(*whereTableNameCursol) ==
                           toupper(*allInputTableNameCursol++)) {
                  ++whereTableNameCursol;
                }
                char *whereColumnNameCursol = currentNode.column.columnName;
                char *allInputColumnNameCursol = allInputColumns[i].columnName;
                while (*whereColumnNameCursol &&
                       toupper(*whereColumnNameCursol) ==
                           toupper(*allInputColumnNameCursol++)) {
                  ++whereColumnNameCursol;
                }
                if (!*whereColumnNameCursol && !*allInputColumnNameCursol &&
                    (!*currentNode.column
                           .tableName || // テーブル名が設定されている場合のみテーブル名の比較を行います。
                     (!*whereTableNameCursol && !*allInputTableNameCursol))) {
                  // 既に見つかっているのにもう一つ見つかったらエラーです。
                  if (found) {
                    @throw [[TynySQLException alloc]
                        initWithErrorCode:ERR_BAD_COLUMN_NAME];
                  }
                  found = true;
                  currentNode.value = allColumnsRow[i];
                }
              }
              // 一つも見つからなくてもエラーです。
              if (!found) {
                @throw [[TynySQLException alloc]
                    initWithErrorCode:ERR_BAD_COLUMN_NAME];
              };
              // 符号を考慮して値を計算します。
              if (currentNode.value->type == INTEGER) {
                currentNode.value->value.integer *= currentNode.signCoefficient;
              }
            }
            break;
          case EQUAL:
          case GREATER_THAN:
          case GREATER_THAN_OR_EQUAL:
          case LESS_THAN:
          case LESS_THAN_OR_EQUAL:
          case NOT_EQUAL:
            // 比較演算子の場合です。

            // 比較できるのは文字列型か整数型で、かつ左右の型が同じ場合です。
            if ((currentNode.left.value->type != INTEGER &&
                 currentNode.left.value->type != STRING) ||
                currentNode.left.value->type != currentNode.right.value->type) {
              @throw [[TynySQLException alloc]
                  initWithErrorCode:ERR_WHERE_OPERAND_TYPE];
            }
            currentNode.value->type = BOOLEAN;

            // 比較結果を型と演算子によって計算方法を変えて、計算します。
            switch (currentNode.left.value->type) {
            case INTEGER:
              switch (currentNode.operator.kind) {
              case EQUAL:
                currentNode.value->value.boolean =
                    currentNode.left.value->value.integer ==
                    currentNode.right.value->value.integer;
                break;
              case GREATER_THAN:
                currentNode.value->value.boolean =
                    currentNode.left.value->value.integer >
                    currentNode.right.value->value.integer;
                break;
              case GREATER_THAN_OR_EQUAL:
                currentNode.value->value.boolean =
                    currentNode.left.value->value.integer >=
                    currentNode.right.value->value.integer;
                break;
              case LESS_THAN:
                currentNode.value->value.boolean =
                    currentNode.left.value->value.integer <
                    currentNode.right.value->value.integer;
                break;
              case LESS_THAN_OR_EQUAL:
                currentNode.value->value.boolean =
                    currentNode.left.value->value.integer <=
                    currentNode.right.value->value.integer;
                break;
              case NOT_EQUAL:
                currentNode.value->value.boolean =
                    currentNode.left.value->value.integer !=
                    currentNode.right.value->value.integer;
                break;
              default:
                @throw
                    [[TynySQLException alloc] initWithErrorCode:ERR_SQL_SYNTAX];
              }
              break;
            case STRING:
              switch (currentNode.operator.kind) {
              case EQUAL:
                currentNode.value->value.boolean =
                    strcmp(currentNode.left.value->value.string,
                           currentNode.right.value->value.string) == 0;
                break;
              case GREATER_THAN:
                currentNode.value->value.boolean =
                    strcmp(currentNode.left.value->value.string,
                           currentNode.right.value->value.string) > 0;
                break;
              case GREATER_THAN_OR_EQUAL:
                currentNode.value->value.boolean =
                    strcmp(currentNode.left.value->value.string,
                           currentNode.right.value->value.string) >= 0;
                break;
              case LESS_THAN:
                currentNode.value->value.boolean =
                    strcmp(currentNode.left.value->value.string,
                           currentNode.right.value->value.string) < 0;
                break;
              case LESS_THAN_OR_EQUAL:
                currentNode.value->value.boolean =
                    strcmp(currentNode.left.value->value.string,
                           currentNode.right.value->value.string) <= 0;
                break;
              case NOT_EQUAL:
                currentNode.value->value.boolean =
                    strcmp(currentNode.left.value->value.string,
                           currentNode.right.value->value.string) != 0;
                break;
              default:
                @throw
                    [[TynySQLException alloc] initWithErrorCode:ERR_SQL_SYNTAX];
              }
              break;
            default:
              @throw
                  [[TynySQLException alloc] initWithErrorCode:ERR_SQL_SYNTAX];
            }
            break;
          case PLUS:
          case MINUS:
          case ASTERISK:
          case SLASH:
            // 四則演算の場合です。

            // 演算できるのは整数型同士の場合のみです。
            if (currentNode.left.value->type != INTEGER ||
                currentNode.right.value->type != INTEGER) {
              @throw [[TynySQLException alloc]
                  initWithErrorCode:ERR_WHERE_OPERAND_TYPE];
            }
            currentNode.value->type = INTEGER;

            // 比較結果を演算子によって計算方法を変えて、計算します。
            switch (currentNode.operator.kind) {
            case PLUS:
              currentNode.value->value.integer =
                  currentNode.left.value->value.integer +
                  currentNode.right.value->value.integer;
              break;
            case MINUS:
              currentNode.value->value.integer =
                  currentNode.left.value->value.integer -
                  currentNode.right.value->value.integer;
              break;
            case ASTERISK:
              currentNode.value->value.integer =
                  currentNode.left.value->value.integer *
                  currentNode.right.value->value.integer;
              break;
            case SLASH:
              currentNode.value->value.integer =
                  currentNode.left.value->value.integer /
                  currentNode.right.value->value.integer;
              break;
            default:
              @throw
                  [[TynySQLException alloc] initWithErrorCode:ERR_SQL_SYNTAX];
            }
            break;
          case AND:
          case OR:
            // 論理演算の場合です。

            // 演算できるのは真偽値型同士の場合のみです。
            if (currentNode.left.value->type != BOOLEAN ||
                currentNode.right.value->type != BOOLEAN) {
              @throw [[TynySQLException alloc]
                  initWithErrorCode:ERR_WHERE_OPERAND_TYPE];
            }
            currentNode.value->type = BOOLEAN;

            // 比較結果を演算子によって計算方法を変えて、計算します。
            switch (currentNode.operator.kind) {
            case AND:
              currentNode.value->value.boolean =
                  currentNode.left.value->value.boolean &&
                  currentNode.right.value->value.boolean;
              break;
            case OR:
              currentNode.value->value.boolean =
                  currentNode.left.value->value.boolean ||
                  currentNode.right.value->value.boolean;
              break;
            default:
              @throw
                  [[TynySQLException alloc] initWithErrorCode:ERR_SQL_SYNTAX];
            }
            break;
          default:
            @throw [[TynySQLException alloc] initWithErrorCode:ERR_SQL_SYNTAX];
          }
          currentNode.calculated = true;

          // 自身の計算が終わった後は親の計算に戻ります。
          currentNode = currentNode.parent;
        }

        // 条件に合わない行は出力から削除します。
        if (!whereTopNode.value->value.boolean) {
          free(row);
          free(allColumnsRow);
          allColumnOutputData[--outputRowsNum] = NULL;
          outputData[outputRowsNum] = NULL;
        }
        // WHERE条件の計算結果をリセットします。
        for (int i = 0; i < whereExtensionNodesNum; ++i) {
          whereExtensionNodes[i].calculated = false;
        }
      }

      // 各テーブルの行のすべての組み合わせを出力します。

      // 最後のテーブルのカレント行をインクリメントします。
      ++currentRows[tableNamesNum - 1];

      // 最後のテーブルが最終行になっていた場合は先頭に戻し、順に前のテーブルのカレント行をインクリメントします。
      for (int i = tableNamesNum - 1; !*currentRows[i] && 0 < i; --i) {
        ++currentRows[i - 1];
        currentRows[i] = inputData[i];
      }

      // 最初のテーブルが最後の行を超えたなら出力行の生成は終わりです。
      if (!*currentRows[0]) {
        break;
      }
    }

    // ORDER句による並び替えの処理を行います。
    if (orderByColumnsNum) {
      // ORDER句で指定されている列が、全ての入力行の中のどの行なのかを計算します。
      int orderByColumnIndexes
          [MAX_COLUMN_COUNT];          // ORDER句で指定された列の、すべての行の中でのインデックスです。
      int orderByColumnIndexesNum = 0; // 現在のorderByColumnIndexesの数です。
      for (int i = 0; i < orderByColumnsNum; ++i) {
        found = false;
        for (int j = 0; j < allInputColumnsNum; ++j) {
          char *orderByTableNameCursol = orderByColumns[i].tableName;
          char *allInputTableNameCursol = allInputColumns[j].tableName;
          while (*orderByTableNameCursol &&
                 toupper(*orderByTableNameCursol) ==
                     toupper(*allInputTableNameCursol)) {
            ++orderByTableNameCursol;
            ++allInputTableNameCursol;
          }
          char *orderByColumnNameCursol = orderByColumns[i].columnName;
          char *allInputColumnNameCursol = allInputColumns[j].columnName;
          while (*orderByColumnNameCursol &&
                 toupper(*orderByColumnNameCursol) ==
                     toupper(*allInputColumnNameCursol)) {
            ++orderByColumnNameCursol;
            ++allInputColumnNameCursol;
          }
          if (!*orderByColumnNameCursol && !*allInputColumnNameCursol &&
              (!*orderByColumns[i]
                     .tableName || // テーブル名が設定されている場合のみテーブル名の比較を行います。
               (!*orderByTableNameCursol && !*allInputTableNameCursol))) {
            // 既に見つかっているのにもう一つ見つかったらエラーです。
            if (found) {
              @throw [[TynySQLException alloc]
                  initWithErrorCode:ERR_BAD_COLUMN_NAME];
            }
            found = true;
            if (MAX_COLUMN_COUNT <= orderByColumnIndexesNum) {
              @throw
                  [[TynySQLException alloc] initWithErrorCode:ERR_MEMORY_OVER];
            }
            orderByColumnIndexes[orderByColumnIndexesNum++] = j;
          }
        }
        // 一つも見つからなくてもエラーです。
        if (!found) {
          @throw
              [[TynySQLException alloc] initWithErrorCode:ERR_BAD_COLUMN_NAME];
        }
      }

      // outputDataとallColumnOutputDataのソートを一緒に行います。簡便のため凝ったソートは使わず、選択ソートを利用します。
      for (int i = 0; i < outputRowsNum; ++i) {
        int minIndex = i; // 現在までで最小の行のインデックスです。
        for (int j = i + 1; j < outputRowsNum; ++j) {
          bool jLessThanMin =
              false; // インデックスがjの値が、minIndexの値より小さいかどうかです。
          for (int k = 0; k < orderByColumnIndexesNum; ++k) {
            Data *mData = allColumnOutputData
                [minIndex][orderByColumnIndexes
                               [k]]; // インデックスがminIndexのデータです。
            Data *jData = allColumnOutputData
                [j][orderByColumnIndexes[k]]; // インデックスがjのデータです。
            int cmp =
                0; // 比較結果です。等しければ0、インデックスjの行が大きければプラス、インデックスminIndexの行が大きければマイナスとなります。
            switch (mData->type) {
            case INTEGER:
              cmp = jData->value.integer - mData->value.integer;
              break;
            case STRING:
              cmp = strcmp(jData->value.string, mData->value.string);
              break;
            default:
              @throw
                  [[TynySQLException alloc] initWithErrorCode:ERR_SQL_SYNTAX];
            }

            // 降順ならcmpの大小を入れ替えます。
            if (orders[k] == DESC) {
              cmp *= -1;
            }
            if (cmp < 0) {
              jLessThanMin = true;
              break;
            } else if (0 < cmp) {
              break;
            }
          }
          if (jLessThanMin) {
            minIndex = j;
          }
        }
        Data **tmp = outputData[minIndex];
        outputData[minIndex] = outputData[i];
        outputData[i] = tmp;

        tmp = allColumnOutputData[minIndex];
        allColumnOutputData[minIndex] = allColumnOutputData[i];
        allColumnOutputData[i] = tmp;
      }
    }

    // 出力ファイルを開きます。
    outputFile = fopen(outputFileName, "w");
    if (outputFile == NULL) {
      @throw [[TynySQLException alloc] initWithErrorCode:ERR_FILE_OPEN];
    }

    // 出力ファイルに列名を出力します。
    for (int i = 0; i < selectColumnsNum; ++i) {
      result = fputs(outputColumns[i].columnName, outputFile);
      if (result == EOF) {
        @throw [[TynySQLException alloc] initWithErrorCode:ERR_FILE_WRITE];
      }
      if (i < selectColumnsNum - 1) {
        result = fputs(",", outputFile);
        if (result == EOF) {
          @throw [[TynySQLException alloc] initWithErrorCode:ERR_FILE_WRITE];
        }
      } else {
        result = fputs("\n", outputFile);
        if (result == EOF) {
          @throw [[TynySQLException alloc] initWithErrorCode:ERR_FILE_WRITE];
        }
      }
    }

    // 出力ファイルにデータを出力します。
    currentRow = outputData;
    while (*currentRow) {
      Data **column = *currentRow;
      for (int i = 0; i < selectColumnsNum; ++i) {
        char outputString[MAX_DATA_LENGTH] = "";
        switch ((*column)->type) {
        case INTEGER:
          sprintf(outputString, "%d", (*column)->value.integer);
          break;
        case STRING:
          strcpy(outputString, (*column)->value.string);
          break;
        default:
          @throw [[TynySQLException alloc] initWithErrorCode:ERR_SQL_SYNTAX];
        }
        result = fputs(outputString, outputFile);
        if (result == EOF) {
          @throw [[TynySQLException alloc] initWithErrorCode:ERR_FILE_WRITE];
        }
        if (i < selectColumnsNum - 1) {
          result = fputs(",", outputFile);
          if (result == EOF) {
            @throw [[TynySQLException alloc] initWithErrorCode:ERR_FILE_WRITE];
          }
        } else {
          result = fputs("\n", outputFile);
          if (result == EOF) {
            @throw [[TynySQLException alloc] initWithErrorCode:ERR_FILE_WRITE];
          }
        }
        ++column;
      }
      ++currentRow;
    }

    // 正常時の後処理です。

    // ファイルリソースを解放します。
    for (int i = 0; i < MAX_TABLE_COUNT; ++i) {
      if (inputTableFiles[i]) {
        fclose(inputTableFiles[i]);
        if (result == EOF) {
          @throw [[TynySQLException alloc] initWithErrorCode:ERR_FILE_CLOSE];
        }
      }
    }
    if (outputFile) {
      fclose(outputFile);
      if (result == EOF) {
        @throw [[TynySQLException alloc] initWithErrorCode:ERR_FILE_CLOSE];
      }
    }

    // メモリリソースを解放します。
    for (int i = 0; i < tableNamesNum; ++i) {
      currentRow = inputData[i];
      while (*currentRow) {
        Data **dataCursol = *currentRow;
        while (*dataCursol) {
          free(*dataCursol++);
        }
        free(*currentRow);
        currentRow++;
      }
    }
    currentRow = outputData;
    while (*currentRow) {
      Data **dataCursol = *currentRow;
      while (*dataCursol) {
        free(*dataCursol++);
      }
      free(*currentRow);
      currentRow++;
    }
    currentRow = allColumnOutputData;
    while (*currentRow) {
      Data **dataCursol = *currentRow;
      while (*dataCursol) {
        free(*dataCursol++);
      }
      free(*currentRow);
      currentRow++;
    }

    return OK;
  } @catch (TynySQLException *ex) {
    // エラー時の処理です。

    // ファイルリソースを解放します。
    for (int i = 0; i < MAX_TABLE_COUNT; ++i) {
      if (inputTableFiles[i]) {
        fclose(inputTableFiles[i]);
      }
    }
    if (outputFile) {
      fclose(outputFile);
    }

    // メモリリソースを解放します。
    for (int i = 0; i < tableNamesNum; ++i) {
      currentRow = inputData[i];
      while (*currentRow) {
        Data **dataCursol = *currentRow;
        while (*dataCursol) {
          free(*dataCursol++);
        }
        free(*currentRow);
        currentRow++;
      }
    }
    currentRow = outputData;
    while (*currentRow) {
      Data **dataCursol = *currentRow;
      while (*dataCursol) {
        free(*dataCursol++);
      }
      free(*currentRow);
      currentRow++;
    }
    currentRow = allColumnOutputData;
    while (*currentRow) {
      Data **dataCursol = *currentRow;
      while (*dataCursol) {
        free(*dataCursol++);
      }
      free(*currentRow);
      currentRow++;
    }
    return ex.errorCode;
  }
ERROR:
  // エラー時の処理です。

  // ファイルリソースを解放します。
  for (int i = 0; i < MAX_TABLE_COUNT; ++i) {
    if (inputTableFiles[i]) {
      fclose(inputTableFiles[i]);
    }
  }
  if (outputFile) {
    fclose(outputFile);
  }

  // メモリリソースを解放します。
  for (int i = 0; i < tableNamesNum; ++i) {
    currentRow = inputData[i];
    while (*currentRow) {
      Data **dataCursol = *currentRow;
      while (*dataCursol) {
        free(*dataCursol++);
      }
      free(*currentRow);
      currentRow++;
    }
  }
  currentRow = outputData;
  while (*currentRow) {
    Data **dataCursol = *currentRow;
    while (*dataCursol) {
      free(*dataCursol++);
    }
    free(*currentRow);
    currentRow++;
  }
  currentRow = allColumnOutputData;
  while (*currentRow) {
    Data **dataCursol = *currentRow;
    while (*dataCursol) {
      free(*dataCursol++);
    }
    free(*currentRow);
    currentRow++;
  }
  return error;
}
