//! @file
#include "ExecuteSQL.h"
#import <Foundation/Foundation.h>

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
typedef NS_ENUM(NSUInteger, ResultValue) {
  ResultOk = 0,      //!< 問題なく終了しました。
  FileOpenError = 1, //!< ファイルを開くことに失敗しました。
  FileWriteError = 2, //!< ファイルに書き込みを行うことに失敗しました。
  FileCloseError = 3, //!< ファイルを閉じることに失敗しました。
  TokenCantReadError = 4, //!< トークン解析に失敗しました。
  SqlSyntaxError = 5,     //!< SQLの構文解析が失敗しました。
  BadColumnNameError = 6, //!< テーブル指定を含む列名が適切ではありません。
  WhereOperandTypeError = 7, //!< 演算の左右の型が適切ではありません。
  CsvSyntaxError = 8,      //!< CSVの構文解析が失敗しました。
  MemoryAllocateError = 9, //!< メモリの取得に失敗しました。
  MemoryOverError = 10 //!< 用意したメモリ領域の上限を超えました。
};

//! 入力や出力、経過の計算に利用するデータのデータ型の種類を表します。
typedef NS_ENUM(NSUInteger, DataType) {
  String,  //!< 文字列型です。
  Integer, //!< 整数型です。
  Bool     //!< 真偽値型です。
};

//! トークンの種類を表します。
typedef NS_ENUM(NSUInteger, TokenKind) {
  NoToken,                 //!< トークンを表しません。
  AscToken,                //!< ASCキーワードです。
  AndToken,                //!< ANDキーワードです。
  ByToken,                 //!< BYキーワードです。
  DescToken,               //!< DESCキーワードです。
  FromToken,               //!< FROMキーワードです。
  OrToken,                 //!< ORキーワードです。
  OrderToken,              //!< ORDERキーワードです。
  SelectToken,             //!< SELECTキーワードです。
  WhereToken,              //!< WHEREキーワードです。
  AsteriskToken,           //!< ＊ 記号です。
  CommaToken,              //!< ， 記号です。
  CloseParenToken,         //!< ） 記号です。
  DotToken,                //!< ． 記号です。
  EqualToken,              //!< ＝ 記号です。
  GreaterThanToken,        //!< ＞ 記号です。
  GreaterThanOrEqualToken, //!< ＞＝ 記号です。
  LessThanToken,           //!< ＜ 記号です。
  LessThanOrEqualToken,    //!< ＜＝ 記号です。
  MinusToken,              //!< － 記号です。
  NotEqualToken,           //!< ＜＞ 記号です。
  OpenParenToken,          //!< （ 記号です。
  PlusToken,               //!< ＋ 記号です。
  SlashToken,              //!< ／ 記号です。
  IdentifierToken,         //!< 識別子です。
  IntLiteralToken,         //!< 整数リテラルです。
  StringLiteralToken       //!< 文字列リテラルです。
};

//! 一つの値を持つデータです。
typedef struct {
  enum DataType type; //!< データの型です。

  //! 実際のデータを格納する共用体です。
  union {
    char string[MAX_DATA_LENGTH]; //!< データが文字列型の場合の値です。
    long integer; //!< データが整数型の場合の値です。
    BOOL boolean; //!< データが真偽値型の場合の値です。
  } value;
} Data;

//! WHERE句に指定する演算子の情報を表します。
@interface Operator : NSObject
@property enum TokenKind
    kind;            //!< 演算子の種類を、演算子を記述するトークンの種類で表します。
@property int order; //!< 演算子の優先順位です。
- (Operator *)init;
- (Operator *)initWithKind:(enum TokenKind)kind Order:(int)order;
@end

//! トークンを表します。
@interface Token : NSObject
@property enum TokenKind kind; //!< トークンの種類です。
@property NSString *word;      //!<
//!記録されているトークンの文字列です。記録の必要がなければ空白です。
- (Token *)init;
- (Token *)initWithKind:(TokenKind)kind Word:(NSString *)word;

@end

//! 指定された列の情報です。どのテーブルに所属するかの情報も含みます。
@interface Column : NSObject
@property NSString *tableName; //!<
//!列が所属するテーブル名です。指定されていない場合は空文字列となります。
@property NSString *columnName; //!< 指定された列の列名です。
- (Column *)init;
- (Column *)initWithTableName:(NSString *)tableName
                   ColumnName:(NSString *)columnName;
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
@property BOOL inParen; //!< 自身がかっこにくるまれているかどうかです。
@property int parenOpenBeforeClose; //!<
                                    //!木の構築中に0以外となり、自身の左にあり、まだ閉じてないカッコの開始の数となります。
@property int signCoefficient;      //!<
                                    //!自身が葉にあり、マイナス単項演算子がついている場合は-1、それ以外は1となります。
@property Column *column;           //!<
                                    //!列場指定されている場合に、その列を表します。列指定ではない場合はcolumnNameが空文字列となります。
@property BOOL calculated; //!< 式の値を計算中に、計算済みかどうかです。
@property(nonatomic) Data *value; //!< 指定された、もしくは計算された値です。

@end

//! 行の情報を入力のテーブルインデックス、列インデックスの形で持ちます。
@interface ColumnIndex : NSObject
- (ColumnIndex *)initWithTable:(int)table Column:(int)column;
@property int table;  //!< 列が入力の何テーブル目の列かです。
@property int column; //!< 列が入力のテーブルの何列目かです。
@end

@interface TynySQLException : NSException
- (TynySQLException *)initWithErrorCode:(enum ResultValue)code;
@property enum ResultValue errorCode;
@end

// 以上ヘッダに相当する部分。

@implementation Operator
- (Operator *)init {
  _kind = NoToken;
  _order = 0;
  return self;
}
- (Operator *)initWithKind:(enum TokenKind)kind Order:(int)order {
  _kind = kind;
  _order = order;
  return self;
}
@end
@implementation Token

- (Token *)init {
  _kind = NoToken;
  _word = NSString.new;
  return self;
}
- (Token *)initWithKind:(enum TokenKind)kind Word:(NSString *)word {
  _kind = kind;
  _word = word;
  return self;
}

@end

@implementation Column

- (Column *)init {
  _tableName = @"";
  _columnName = @"";
  return self;
}
- (Column *)initWithTableName:(NSString *)tableName
                   ColumnName:(NSString *)columnName {
  _tableName = tableName;
  _columnName = columnName;
  return self;
}

@end

@implementation ExtensionTreeNode

- (ExtensionTreeNode *)init {
  _parent = nil;
  _left = nil;
  _operator = Operator.new;
  _right = nil;
  _inParen = NO;
  _parenOpenBeforeClose = 0;
  _signCoefficient = 1;
  _column = Column.new;
  _calculated = NO;
  __value = (Data){.type = String, .value = {.string = ""}};
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
- (TynySQLException *)initWithErrorCode:(enum ResultValue)code {
  _errorCode = code;
  return self;
}

@end

char getChar(NSString *string, int cursol) {
  if ([string length] <= cursol) {
    return 0;
  }
  NSString *charactor = [string substringWithRange:NSMakeRange(cursol, 1)];

  char buf[] = " ";
  char *ch = buf;
  [charactor getCString:ch maxLength:2 encoding:NSUTF8StringEncoding];
  return *ch;
}

NSString *getOneCharactor(NSString *string, int cursol) {
  if ([string length] <= cursol) {
    return @"¥0";
  }
  return [string substringWithRange:NSMakeRange(cursol, 1)];
}

//! カレントディレクトリにあるCSVに対し、簡易的なSQLを実行し、結果をファイルに出力します。
//! @param [in] sql 実行するSQLです。
//! @param[in] outputFileName
//! SQLの実行結果をCSVとして出力するファイル名です。拡張子を含みます。
//! @return 実行した結果の状態です。
//! @retval ResultOk=0                      問題なく終了しました。
//! @retval FileOpenError=1           ファイルを開くことに失敗しました。
//! @retval FileWriteError=2 ファイルに書き込みを行うことに失敗しました。
//! @retval FileCloseError=3          ファイルを閉じることに失敗しました。
//! @retval TokenCantReadError=4     トークン解析に失敗しました。
//! @retval SqlSyntaxError=5          SQLの構文解析が失敗しました。
//! @retval BadColumnNameError=6 テーブル指定を含む列名が適切ではありません。
//! @retval WhereOperandTypeError=7  演算の左右の型が適切ではありません。
//! @retval CsvSyntaxError=8          CSVの構文解析が失敗しました。
//! @retval MemoryAllocateError=9     メモリの取得に失敗しました。
//! @retval MemoryOverError=10        用意したメモリ領域の上限を超えました。
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
  NSMutableArray *inputTableFiles =
      NSMutableArray.new; // 読み込む入力ファイルの全てのファイルポインタです。
  NSFileHandle *outputFile = nil; // 書き込むファイルのファイルポインタです。
  Data ***currentRow = NULL; // データ検索時に現在見ている行を表します。
  Data **inputData[MAX_TABLE_COUNT][MAX_ROW_COUNT]; // 入力データです。
  NSMutableArray *outputData = NSMutableArray.new;  // 出力データです。
  NSMutableArray *allColumnOutputData =
      NSMutableArray
          .new; // 出力するデータに対応するインデックスを持ち、すべての入力データを保管します。
  NSMutableArray *tableNames =
      NSMutableArray.new; // FROM句で指定しているテーブル名です。
  @try {

    BOOL found = NO; // 検索時に見つかったかどうかの結果を一時的に保存します。

    NSString *alpahUnder =
        @"_abcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPQRSTUVWXY"
        @"Z"; // 全てのアルファベットの大文字小文字とアンダーバーです。
    NSString *alpahNumUnder =
        @"_abcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPQRSTUVWXYZ0123"
        @"456789";                        // 全ての数字とアルファベットの大文字小文字とアンダーバーです。
    const char *signNum = "+-0123456789"; // 全ての符号と数字です。
    NSString *num = @"0123456789";        // 全ての数字です。
    NSString *space = @" \t\r\n";         // 全ての空白文字です。

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
      [Token.alloc initWithKind:AndToken Word:@"AND"],
      [Token.alloc initWithKind:AscToken Word:@"ASC"],
      [Token.alloc initWithKind:ByToken Word:@"BY"],
      [Token.alloc initWithKind:DescToken Word:@"DESC"],
      [Token.alloc initWithKind:FromToken Word:@"FROM"],
      [Token.alloc initWithKind:OrderToken Word:@"ORDER"],
      [Token.alloc initWithKind:OrToken Word:@"OR"],
      [Token.alloc initWithKind:SelectToken Word:@"SELECT"],
      [Token.alloc initWithKind:WhereToken Word:@"WHERE"]
    ];

    // 記号をトークンとして認識するための記号一覧情報です。
    NSArray *signConditions = @[
      [Token.alloc initWithKind:GreaterThanOrEqualToken Word:@">="],
      [Token.alloc initWithKind:LessThanOrEqualToken Word:@"<="],
      [Token.alloc initWithKind:NotEqualToken Word:@"<>"],
      [Token.alloc initWithKind:AsteriskToken Word:@"*"],
      [Token.alloc initWithKind:CommaToken Word:@","],
      [Token.alloc initWithKind:CloseParenToken Word:@")"],
      [Token.alloc initWithKind:DotToken Word:@"."],
      [Token.alloc initWithKind:EqualToken Word:@"="],
      [Token.alloc initWithKind:GreaterThanToken Word:@">"],
      [Token.alloc initWithKind:LessThanToken Word:@"<"],
      [Token.alloc initWithKind:MinusToken Word:@"-"],
      [Token.alloc initWithKind:OpenParenToken Word:@"("],
      [Token.alloc initWithKind:PlusToken Word:@"+"],
      [Token.alloc initWithKind:SlashToken Word:@"/"]
    ];

    NSMutableArray *tokens = NSMutableArray.new; // SQLを分割したトークンです。

    // 演算子の情報です。
    Operator *operators[] = {
        [Operator.alloc initWithKind:AsteriskToken Order:1],
        [Operator.alloc initWithKind:SlashToken Order:1],
        [Operator.alloc initWithKind:PlusToken Order:2],
        [Operator.alloc initWithKind:MinusToken Order:2],
        [Operator.alloc initWithKind:EqualToken Order:3],
        [Operator.alloc initWithKind:GreaterThanToken Order:3],
        [Operator.alloc initWithKind:GreaterThanOrEqualToken Order:3],
        [Operator.alloc initWithKind:LessThanToken Order:3],
        [Operator.alloc initWithKind:LessThanOrEqualToken Order:3],
        [Operator.alloc initWithKind:NotEqualToken Order:3],
        [Operator.alloc initWithKind:AndToken Order:4],
        [Operator.alloc initWithKind:OrToken Order:5]};

    NSString *sqlString =
        [NSString stringWithCString:sql encoding:NSUTF8StringEncoding];
    int charactorBackPoint =
        0; // SQLをトークンに分割して読み込む時に戻るポイントを記録しておきます。

    int charactorCursol =
        0; // SQLをトークンに分割して読み込む時に現在読んでいる文字の場所を表します。

    // SQLをトークンに分割て読み込みます。
    while (getChar(sqlString, charactorCursol)) {
      // 空白を読み飛ばします。
      if ([space containsString:getOneCharactor(sqlString, charactorCursol)]) {
        charactorCursol++;
        continue;
      }

      // 数値リテラルを読み込みます。

      // 先頭文字が数字であるかどうかを確認します。
      charactorBackPoint = charactorCursol;
      if ([num containsString:getOneCharactor(sqlString, charactorCursol)]) {

        NSMutableString *word = [NSMutableString string];

        // 数字が続く間、文字を読み込み続けます。
        do {
          if ([num
                  containsString:getOneCharactor(sqlString, charactorCursol)]) {
            [word appendString:getOneCharactor(sqlString, charactorCursol)];
            ++charactorCursol;
          }
        } while (
            [num containsString:getOneCharactor(sqlString, charactorCursol)]);

        // 数字の後にすぐに識別子が続くのは紛らわしいので数値リテラルとは扱いません。
        if (![alpahUnder
                containsString:getOneCharactor(sqlString, charactorCursol)]) {
          [tokens
              addObject:[[Token alloc] initWithKind:IntLiteralToken Word:word]];
          continue;
        } else {
          charactorCursol = charactorBackPoint;
        }
      }

      // 文字列リテラルを読み込みます。

      // 文字列リテラルを開始するシングルクォートを判別し、読み込みます。
      // メトリクス測定ツールのccccはシングルクォートの文字リテラル中のエスケープを認識しないため、文字リテラルを使わないことで回避しています。
      if (getChar(sqlString, charactorCursol) == "\'"[0]) {
        ++charactorCursol;

        // 読み込んだ文字列リテラルの情報です。初期値の段階で最初のシングルクォートは読み込んでいます。

        NSMutableString *word = [NSMutableString stringWithString:@"\'"];

        // 次のシングルクォートがくるまで文字を読み込み続けます。
        while (getChar(sqlString, charactorCursol) &&
               getChar(sqlString, charactorCursol) != "\'"[0]) {

          [word appendString:getOneCharactor(sqlString, charactorCursol++)];
        }
        if (getChar(sqlString, charactorCursol) == "\'"[0]) {
          // 最後のシングルクォートを読み込みます。
          [word appendString:getOneCharactor(sqlString, charactorCursol++)];

          // 文字列の終端文字をつけます。
          [tokens addObject:[[Token alloc] initWithKind:StringLiteralToken
                                                   Word:word]];
          continue;
        } else {
          @throw
              [[TynySQLException alloc] initWithErrorCode:TokenCantReadError];
        }
      }

      // キーワードを読み込みます。
      found = NO;
      for (Token *condition in keywordConditions) {
        charactorBackPoint = charactorCursol;
        char word[MAX_WORD_LENGTH];
        char *wordCursol = word;

        [condition.word
            getCString:wordCursol
             maxLength:MAX_WORD_LENGTH
              encoding:
                  NSUTF8StringEncoding]; // 確認するキーワードの文字列のうち、現在確認している一文字を指します。

        // キーワードが指定した文字列となっているか確認します。
        while (*wordCursol &&
               toupper(getChar(sqlString, charactorCursol++)) == *wordCursol) {
          ++wordCursol;
        }

        // キーワードに識別子が区切りなしに続いていないかを確認するため、キーワードの終わった一文字あとを調べます。
        if (!*wordCursol &&
            ![alpahNumUnder
                containsString:getOneCharactor(sqlString, charactorCursol)]) {

          // 見つかったキーワードを生成します。
          [tokens
              addObject:[[Token alloc] initWithKind:condition.kind Word:@""]];
          found = YES;
        } else {
          charactorCursol = charactorBackPoint;
        }
      }
      if (found) {
        continue;
      }

      // 記号を読み込みます。
      found = NO;
      for (Token *condition in signConditions) {
        charactorBackPoint = charactorCursol;
        char word[MAX_WORD_LENGTH];
        char *wordCursol =
            word; // 確認する記号の文字列のうち、現在確認している一文字を指します。
        [condition.word getCString:wordCursol
                         maxLength:MAX_WORD_LENGTH
                          encoding:NSUTF8StringEncoding];

        // 記号が指定した文字列となっているか確認します。
        while (*wordCursol &&
               toupper(getChar(sqlString, charactorCursol++)) == *wordCursol) {
          ++wordCursol;
        }
        if (!*wordCursol) {

          // 見つかった記号を生成します。
          [tokens
              addObject:[[Token alloc] initWithKind:condition.kind Word:@""]];
          found = YES;
        } else {
          charactorCursol = charactorBackPoint;
        }
      }
      if (found) {
        continue;
      }

      // 識別子を読み込みます。

      // 識別子の最初の文字を確認します。
      if ([alpahUnder
              containsString:getOneCharactor(sqlString, charactorCursol)]) {
        NSMutableString *word = [NSMutableString string];
        do {
          // 二文字目以降は数字も許可して文字の種類を確認します。
          if ([alpahNumUnder
                  containsString:getOneCharactor(sqlString, charactorCursol)]) {
            [word appendString:getOneCharactor(sqlString, charactorCursol)];

            charactorCursol++;
          }
        } while ([alpahNumUnder
            containsString:getOneCharactor(sqlString, charactorCursol)]);

        // 読み込んだ識別子を登録します。
        [tokens
            addObject:[[Token alloc] initWithKind:IdentifierToken Word:word]];
        continue;
      }

      @throw [[TynySQLException alloc] initWithErrorCode:TokenCantReadError];
    }

    // トークン列を解析し、構文を読み取ります。

    NSEnumerator *tokenCursol =
        [tokens objectEnumerator]; // 現在見ているトークンを指します。

    NSMutableArray *selectColumns =
        [NSMutableArray array]; // SELECT句に指定された列名です。

    NSMutableArray *orderByColumns =
        NSMutableArray.new; // ORDER句に指定された列名です。

    enum TokenKind orders[MAX_COLUMN_COUNT] = {
        0}; // 同じインデックスのorderByColumnsに対応している、昇順、降順の指定です。

    ExtensionTreeNode *whereTopNode = NULL; // 式木の根となるノードです。

    // SQLの構文を解析し、必要な情報を取得します。

    // SELECT句を読み込みます。
    Token *nextToken = [tokenCursol nextObject];
    if (nextToken.kind == SelectToken) {
      nextToken = [tokenCursol nextObject];
    } else {
      @throw [[TynySQLException alloc] initWithErrorCode:SqlSyntaxError];
    }

    if (nextToken.kind == AsteriskToken) {
      nextToken = [tokenCursol nextObject];
    } else {
      BOOL first =
          YES; // SELECT句に最初に指定された列名の読み込みかどうかです。
      while (nextToken.kind == CommaToken || first) {
        if (nextToken.kind == CommaToken) {
          nextToken = [tokenCursol nextObject];
          ;
        }
        if (nextToken.kind == IdentifierToken) {
          // テーブル名が指定されていない場合と仮定して読み込みます。
          Column *column =
              [[Column alloc] initWithTableName:@"" ColumnName:nextToken.word];
          [selectColumns addObject:column];

          nextToken = [tokenCursol nextObject];
          ;
          if (nextToken.kind == DotToken) {
            nextToken = [tokenCursol nextObject];
            ;
            if (nextToken.kind == IdentifierToken) {

              // テーブル名が指定されていることがわかったので読み替えます。
              column.tableName = column.columnName;
              column.columnName = nextToken.word;
              nextToken = [tokenCursol nextObject];
              ;
            } else {
              @throw
                  [[TynySQLException alloc] initWithErrorCode:SqlSyntaxError];
            }
          }
        } else {
          @throw [[TynySQLException alloc] initWithErrorCode:SqlSyntaxError];
        }
        first = NO;
      }
    }
    NSMutableArray *allNodes = [NSMutableArray array];
    // ORDER句とWHERE句を読み込みます。最大各一回ずつ書くことができます。
    BOOL readOrder = NO; // すでにORDER句が読み込み済みかどうかです。
    BOOL readWhere = NO; // すでにWHERE句が読み込み済みかどうかです。
    while (nextToken.kind == OrderToken || nextToken.kind == WhereToken) {

      // 二度目のORDER句はエラーです。
      if (readOrder && nextToken.kind == OrderToken) {
        @throw [[TynySQLException alloc] initWithErrorCode:SqlSyntaxError];
      }

      // 二度目のWHERE句はエラーです。
      if (readWhere && nextToken.kind == WhereToken) {
        @throw [[TynySQLException alloc] initWithErrorCode:SqlSyntaxError];
      }
      // ORDER句を読み込みます。
      if (nextToken.kind == OrderToken) {
        readOrder = YES;
        nextToken = [tokenCursol nextObject];
        ;
        if (nextToken.kind == ByToken) {
          nextToken = [tokenCursol nextObject];
          ;
          BOOL first = YES; // ORDER句の最初の列名の読み込みかどうかです。
          while (nextToken.kind == CommaToken || first) {
            if (nextToken.kind == CommaToken) {
              nextToken = [tokenCursol nextObject];
              ;
            }
            if (nextToken.kind == IdentifierToken) {

              // テーブル名が指定されていない場合と仮定して読み込みます。
              Column *column = [Column.alloc initWithTableName:@""
                                                    ColumnName:nextToken.word];
              [orderByColumns addObject:column];
              nextToken = [tokenCursol nextObject];
              ;
              if (nextToken.kind == DotToken) {
                nextToken = [tokenCursol nextObject];
                ;
                if (nextToken.kind == IdentifierToken) {

                  // テーブル名が指定されていることがわかったので読み替えます。
                  column.tableName = column.columnName;
                  column.columnName = nextToken.word;

                  nextToken = [tokenCursol nextObject];
                  ;
                } else {
                  @throw [[TynySQLException alloc]
                      initWithErrorCode:SqlSyntaxError];
                }
              }

              // 並び替えの昇順、降順を指定します。
              if (nextToken.kind == AscToken) {

                orders[orderByColumns.count - 1] = AscToken;
                nextToken = [tokenCursol nextObject];

              } else if (nextToken.kind == DescToken) {
                orders[orderByColumns.count - 1] = DescToken;
                nextToken = [tokenCursol nextObject];
                ;
              } else {
                // 指定がない場合は昇順となります。
                orders[orderByColumns.count - 1] = AscToken;
              }
            } else {
              @throw
                  [[TynySQLException alloc] initWithErrorCode:SqlSyntaxError];
            }
            first = NO;
          }
        } else {
          @throw [[TynySQLException alloc] initWithErrorCode:SqlSyntaxError];
        }
      }

      // WHERE句を読み込みます。

      if (nextToken.kind == WhereToken) {
        readWhere = YES;
        nextToken = [tokenCursol nextObject];

        ExtensionTreeNode *currentNode = NULL; // 現在読み込んでいるノードです。
        while (YES) {
          // オペランドを読み込みます。

          // オペランドのノードを新しく生成します。
          if (currentNode) {
            // 現在のノードを右の子にずらし、元の位置に新しいノードを挿入します。
            currentNode.right = [[ExtensionTreeNode alloc] init];
            [allNodes addObject:currentNode.right];
            currentNode.right.parent = currentNode;
            currentNode = currentNode.right;
          } else {
            // 最初はカレントノードに新しいノードを入れます。
            currentNode = [[ExtensionTreeNode alloc] init];
            [allNodes addObject:currentNode];
          }

          // カッコ開くを読み込みます。
          while (nextToken.kind == OpenParenToken) {
            ++currentNode.parenOpenBeforeClose;
            nextToken = [tokenCursol nextObject];
            ;
          }

          // オペランドに前置される+か-を読み込みます。
          if (nextToken.kind == PlusToken || nextToken.kind == MinusToken) {
            if (nextToken.kind == MinusToken) {
              currentNode.signCoefficient = -1;
            }
            nextToken = [tokenCursol nextObject];
            // +-を前置するのは列名と数値リテラルのみです。
            if (nextToken.kind != IdentifierToken &&
                nextToken.kind != IntLiteralToken) {
              @throw [[TynySQLException alloc]
                  initWithErrorCode:WhereOperandTypeError];
            }
          }

          // 列名、整数リテラル、文字列リテラルのいずれかをオペランドとして読み込みます。
          if (nextToken.kind == IdentifierToken) {

            // テーブル名が指定されていない場合と仮定して読み込みます。
            currentNode.column.tableName = @"";
            currentNode.column.columnName = nextToken.word;

            nextToken = [tokenCursol nextObject];
            ;
            if (nextToken.kind == DotToken) {
              nextToken = [tokenCursol nextObject];
              ;
              if (nextToken.kind == IdentifierToken) {

                // テーブル名が指定されていることがわかったので読み替えます。
                currentNode.column.tableName = currentNode.column.columnName;

                currentNode.column.columnName = nextToken.word;
                nextToken = [tokenCursol nextObject];
                ;
              } else {
                @throw
                    [[TynySQLException alloc] initWithErrorCode:SqlSyntaxError];
              }
            }
          } else if (nextToken.kind == IntLiteralToken) {
            currentNode.value =
                &(Data){.type = Integer,
                        .value = {.integer = [nextToken.word integerValue]}};
            nextToken = [tokenCursol nextObject];
            ;
          } else if (nextToken.kind == StringLiteralToken) {
            currentNode.value =
                &(Data){.type = String, .value = {.string = ""}};

            // 前後のシングルクォートを取り去った文字列をデータとして読み込みます。
            [[nextToken.word
                substringWithRange:NSMakeRange(1, [nextToken.word length] - 2)]
                getCString:currentNode.value->value.string
                 maxLength:MAX_WORD_LENGTH < MAX_DATA_LENGTH ? MAX_WORD_LENGTH
                                                             : MAX_DATA_LENGTH
                  encoding:NSUTF8StringEncoding];
            nextToken = [tokenCursol nextObject];
          } else {
            @throw [[TynySQLException alloc] initWithErrorCode:SqlSyntaxError];
          }

          // オペランドの右のカッコ閉じるを読み込みます。
          while (nextToken.kind == CloseParenToken) {
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
                searchedAncestor.inParen = YES;
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
          found = NO;
          for (int j = 0; j < sizeof(operators) / sizeof(operators[0]); ++j) {
            if (operators[j].kind == nextToken.kind) {
              operator= operators[j];
              found = YES;
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
            BOOL first = YES; // 演算子の優先順位を検索する最初のループです。
            do {
              if (!first) {
                tmp = tmp.parent;
                searched = tmp;
              }
              // 現在の読み込み場所をくくるカッコが開く場所を探します。
              while (searched && !searched.parenOpenBeforeClose) {
                searched = searched.left;
              }
              first = NO;
            } while (!searched && tmp.parent &&
                     (tmp.parent.operator.order <= operator.order ||
                      tmp.parent.inParen));

            // 演算子のノードを新しく生成します。
            currentNode = [[ExtensionTreeNode alloc] init];
            [allNodes addObject:currentNode];
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
    if (nextToken.kind == FromToken) {
      nextToken = [tokenCursol nextObject];
      ;
    } else {
      @throw [[TynySQLException alloc] initWithErrorCode:SqlSyntaxError];
    }
    BOOL first = YES; // FROM句の最初のテーブル名を読み込み中かどうかです。
    while (nextToken.kind == CommaToken || first) {
      if (nextToken.kind == CommaToken) {
        nextToken = [tokenCursol nextObject];
        ;
      }
      if (nextToken.kind == IdentifierToken) {
        [tableNames addObject:nextToken.word];
        nextToken = [tokenCursol nextObject];
      } else {
        @throw [[TynySQLException alloc] initWithErrorCode:SqlSyntaxError];
      }
      first = NO;
    }

    // 最後のトークンまで読み込みが進んでいなかったらエラーです。
    if (nextToken) {
      @throw [[TynySQLException alloc] initWithErrorCode:SqlSyntaxError];
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

    int tableNamesCount = 0;
    for (NSString *tableName in tableNames) {

      // 入力ファイル名を生成します。
      const char csvExtension[] = ".csv"; // csvの拡張子です。
      char fileName[MAX_WORD_LENGTH + sizeof(csvExtension) -
                    1]; // 拡張子を含む、入力ファイルのファイル名です。
      [tableName getCString:fileName
                  maxLength:MAX_WORD_LENGTH + sizeof(csvExtension) - 1
                   encoding:NSUTF8StringEncoding];
      strncat(fileName, csvExtension,
              MAX_WORD_LENGTH + sizeof(csvExtension) - 1);

      // 入力ファイルを開きます。
      NSFileHandle *inputFile =
          [NSFileHandle fileHandleForReadingAtPath:
                            [NSString stringWithCString:fileName
                                               encoding:NSUTF8StringEncoding]];
      if (!inputFile) {
        @throw [[TynySQLException alloc] initWithErrorCode:FileOpenError];
      }
      [inputTableFiles addObject:inputFile];

      // 入力CSVのヘッダ行を読み込みます。
      NSString *inputLine; // ファイルから読み込んだ行文字列です。
      NSData *allFile = nil;
      allFile = [inputFile readDataToEndOfFile];
      NSArray *allLines =
          [[[NSString alloc] initWithData:allFile encoding:NSUTF8StringEncoding]
              componentsSeparatedByString:@"\n"];
      while ([[allLines lastObject] isEqualToString:@""]) {
        allLines =
            [allLines subarrayWithRange:NSMakeRange(0, [allLines count] - 1)];
      }
      inputLine = allLines[0];
      if (allFile) {
        int charactorCursol = 0;

        // 読み込んだ行を最後まで読みます。
        while (getChar(inputLine, charactorCursol) &&
               getChar(inputLine, charactorCursol) != '\r' &&
               getChar(inputLine, charactorCursol) != '\n') {
          if (MAX_COLUMN_COUNT <= inputColumnNums[tableNamesCount]) {
            @throw [[TynySQLException alloc] initWithErrorCode:MemoryOverError];
          }
          inputColumns[tableNamesCount][inputColumnNums[tableNamesCount]]
              .tableName = tableName;

          char wrote[MAX_WORD_LENGTH] = "";
          char *writeCursol = wrote; // 列名の書き込みに利用するカーソルです。

          // 列名を一つ読みます。
          while (getChar(inputLine, charactorCursol) &&
                 getChar(inputLine, charactorCursol) != ',' &&
                 getChar(inputLine, charactorCursol) != '\r' &&
                 getChar(inputLine, charactorCursol) != '\n') {
            *writeCursol++ = getChar(inputLine, charactorCursol++);
          }
          // 書き込んでいる列名の文字列に終端文字を書き込みます。
          writeCursol[1] = '\0';
          inputColumns[tableNamesCount][inputColumnNums[tableNamesCount]++]
              .columnName =
              [NSString stringWithCString:wrote encoding:NSUTF8StringEncoding];

          // 入力行のカンマの分を読み進めます。
          ++charactorCursol;
        }
      } else {
        @throw [[TynySQLException alloc] initWithErrorCode:CsvSyntaxError];
      }

      // 入力CSVのデータ行を読み込みます。
      int rowNum = 0;
      while (rowNum < [allLines count] - 1) {
        if (MAX_ROW_COUNT <= rowNum) {
          @throw [[TynySQLException alloc] initWithErrorCode:MemoryOverError];
        }
        Data **row = inputData[tableNamesCount][rowNum] =
            malloc(MAX_COLUMN_COUNT *
                   sizeof(Data *)); // 入力されている一行分のデータです。
        if (!row) {
          @throw
              [[TynySQLException alloc] initWithErrorCode:MemoryAllocateError];
        }
        // 生成した行を初期化します。
        for (int j = 0; j < MAX_COLUMN_COUNT; ++j) {
          row[j] = NULL;
        }
        inputLine = allLines[rowNum++ + 1];
        int charactorCursol = 0;
        int columnNum =
            0; // いま何列目を読み込んでいるか。0基底の数字となります。

        // 読み込んだ行を最後まで読みます。
        while (getChar(inputLine, charactorCursol) &&
               getChar(inputLine, charactorCursol) != '\r' &&
               getChar(inputLine, charactorCursol) != '\n') {

          // 読み込んだデータを書き込む行のカラムを生成します。
          if (MAX_COLUMN_COUNT <= columnNum) {
            @throw [[TynySQLException alloc] initWithErrorCode:MemoryOverError];
          }
          row[columnNum] = malloc(sizeof(Data));
          if (!row[columnNum]) {
            @throw [[TynySQLException alloc]
                initWithErrorCode:MemoryAllocateError];
          }
          *row[columnNum] = (Data){.type = String, .value = {.string = ""}};
          char *writeCursol =
              row[columnNum++]
                  ->value
                  .string; // データ文字列の書き込みに利用するカーソルです。

          // データ文字列を一つ読みます。
          while (getChar(inputLine, charactorCursol) &&
                 getChar(inputLine, charactorCursol) != ',' &&
                 getChar(inputLine, charactorCursol) != '\r' &&
                 getChar(inputLine, charactorCursol) != '\n') {
            *writeCursol++ = getChar(inputLine, charactorCursol++);
          }
          // 書き込んでいる列名の文字列に終端文字を書き込みます。
          writeCursol[1] = '\0';

          // 入力行のカンマの分を読み進めます。
          ++charactorCursol;
        }
      }

      // 全てが数値となる列は数値列に変換します。
      for (int j = 0; j < inputColumnNums[tableNamesCount]; ++j) {

        // 全ての行のある列について、データ文字列から符号と数値以外の文字を探します。
        currentRow = inputData[tableNamesCount];
        found = NO;
        while (*currentRow) {
          char *currentChar = (*currentRow)[j]->value.string;
          while (*currentChar) {
            BOOL isNum = NO;
            const char *currentNum = signNum;
            while (*currentNum) {
              if (*currentChar == *currentNum) {
                isNum = YES;
                break;
              }
              ++currentNum;
            }
            if (!isNum) {
              found = YES;
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
          currentRow = inputData[tableNamesCount];
          while (*currentRow) {
            *(*currentRow)[j] = (Data){
                .type = Integer,
                .value = {.integer = atoi((*currentRow)[j]->value.string)}};
            ++currentRow;
          }
        }
      }
      tableNamesCount++;
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
    for (int i = 0; i < [tableNames count]; ++i) {
      for (int j = 0; j < inputColumnNums[i]; ++j) {
        allInputColumns[allInputColumnsNum].tableName = tableNames[i];
        allInputColumns[allInputColumnsNum++].columnName =
            inputColumns[i][j].columnName;
      }
    }

    // SELECT句の列名指定が*だった場合は、入力CSVの列名がすべて選択されます。
    if ([selectColumns count] == 0) {
      for (int i = 0; i < allInputColumnsNum; ++i) {
        [selectColumns
            addObject:[[Column alloc]
                          initWithTableName:allInputColumns[i].tableName
                                 ColumnName:allInputColumns[i].columnName]];
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
    NSMutableArray *selectColumnIndexes = [NSMutableArray
        array]; // SELECT句で指定された列の、入力ファイルとしてのインデックスです。
    for (Column *selectedColumn in selectColumns) {
      found = NO;
      for (int j = 0; j < [tableNames count]; ++j) {
        for (int k = 0; k < inputColumnNums[j]; ++k) {
          if ([selectedColumn.columnName
                  caseInsensitiveCompare:inputColumns[j][k].columnName] ==
                  NSOrderedSame &&
              ([selectedColumn.tableName
                   isEqualToString:
                       @""] || // テーブル名が設定されている場合のみテーブル名の比較を行います。
               ([selectedColumn.tableName
                    caseInsensitiveCompare:inputColumns[j][k].tableName] ==
                NSOrderedSame))) {

            // 既に見つかっているのにもう一つ見つかったらエラーです。
            if (found) {
              @throw [[TynySQLException alloc]
                  initWithErrorCode:BadColumnNameError];
            }
            found = YES;
            // 見つかった値を持つ列のデータを生成します。
            [selectColumnIndexes
                addObject:[[ColumnIndex alloc] initWithTable:j Column:k]];
          }
        }
      }
      // 一つも見つからなくてもエラーです。
      if (!found) {
        @throw [[TynySQLException alloc] initWithErrorCode:BadColumnNameError];
      }
    }

    // 出力する列名を設定します。
    for (ColumnIndex *index in selectColumnIndexes) {
      outputColumns[outputColumnNum].tableName =
          inputColumns[index.table][index.column].tableName;
      outputColumns[outputColumnNum].columnName =
          inputColumns[index.table][index.column].columnName;
      ++outputColumnNum;
    }

    if (whereTopNode) {
      // 既存数値の符号を計算します。
      for (ExtensionTreeNode *node in allNodes) {
        if (node.operator.kind == NoToken &&
            [node.column.columnName isEqualToString:@""] &&
            node.value->type == Integer) {
          node.value->value.integer *= node.signCoefficient;
        }
      }
    }

    Data ***currentRows[MAX_TABLE_COUNT] = {
        NULL}; // 入力された各テーブルの、現在出力している行を指すカーソルです。
    for (int i = 0; i < [tableNames count]; ++i) {
      // 各テーブルの先頭行を設定します。
      currentRows[i] = inputData[i];
    }

    // 出力するデータを設定します。
    while (YES) {
      NSMutableArray *row =
          NSMutableArray.new; // 出力している一行分のデータです。
      [outputData addObject:row];

      // 行の各列のデータを入力から持ってきて設定します。
      for (ColumnIndex *index in selectColumnIndexes) {
        Data *data = malloc(sizeof(Data));
        *data = *(*currentRows[index.table])[index.column];
        [row addObject:[NSValue valueWithPointer:data]];
      }

      NSMutableArray *allColumnsRow = NSMutableArray.new;
      [allColumnOutputData
          addObject:
              allColumnsRow]; // WHEREやORDERのためにすべての情報を含む行。rowとインデックスを共有します。

      // allColumnsRowの列を設定します。
      for (int i = 0; i < [tableNames count]; ++i) {
        for (int j = 0; j < inputColumnNums[i]; ++j) {
          Data *data = malloc(sizeof(Data));
          *data = *(*currentRows[i])[j];
          NSValue *value = [NSValue valueWithPointer:data];

          if (!value) {
            @throw [[TynySQLException alloc]
                initWithErrorCode:MemoryAllocateError];
          }

          [allColumnsRow addObject:value];
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
          case NoToken:
            // ノードにデータが設定されている場合です。

            // データが列名で指定されている場合、今扱っている行のデータを設定します。
            if (![currentNode.column.columnName isEqualToString:@""]) {
              found = NO;
              for (int i = 0; i < allInputColumnsNum; ++i) {

                if ([currentNode.column.columnName
                        caseInsensitiveCompare:allInputColumns[i].columnName] ==
                        NSOrderedSame &&
                    ([currentNode.column.tableName
                         isEqualToString:
                             @""] || // テーブル名が設定されている場合のみテーブル名の比較を行います。
                     ([currentNode.column.tableName
                          caseInsensitiveCompare:allInputColumns[i]
                                                     .tableName] ==
                      NSOrderedSame))) {
                  // 既に見つかっているのにもう一つ見つかったらエラーです。
                  if (found) {
                    @throw [[TynySQLException alloc]
                        initWithErrorCode:BadColumnNameError];
                  }
                  found = YES;
                  currentNode.value =
                      ((NSValue *)allColumnsRow[i]).pointerValue;
                }
              }
              // 一つも見つからなくてもエラーです。
              if (!found) {
                @throw [[TynySQLException alloc]
                    initWithErrorCode:BadColumnNameError];
              };
              // 符号を考慮して値を計算します。
              if (currentNode.value->type == Integer) {
                currentNode.value->value.integer *= currentNode.signCoefficient;
              }
            }
            break;
          case EqualToken:
          case GreaterThanToken:
          case GreaterThanOrEqualToken:
          case LessThanToken:
          case LessThanOrEqualToken:
          case NotEqualToken:
            // 比較演算子の場合です。

            // 比較できるのは文字列型か整数型で、かつ左右の型が同じ場合です。
            if ((currentNode.left.value->type != Integer &&
                 currentNode.left.value->type != String) ||
                currentNode.left.value->type != currentNode.right.value->type) {
              @throw [[TynySQLException alloc]
                  initWithErrorCode:WhereOperandTypeError];
            }
            currentNode.value->type = Bool;

            // 比較結果を型と演算子によって計算方法を変えて、計算します。
            switch (currentNode.left.value->type) {
            case Integer:
              switch (currentNode.operator.kind) {
              case EqualToken:
                currentNode.value->value.boolean =
                    currentNode.left.value->value.integer ==
                    currentNode.right.value->value.integer;
                break;
              case GreaterThanToken:
                currentNode.value->value.boolean =
                    currentNode.left.value->value.integer >
                    currentNode.right.value->value.integer;
                break;
              case GreaterThanOrEqualToken:
                currentNode.value->value.boolean =
                    currentNode.left.value->value.integer >=
                    currentNode.right.value->value.integer;
                break;
              case LessThanToken:
                currentNode.value->value.boolean =
                    currentNode.left.value->value.integer <
                    currentNode.right.value->value.integer;
                break;
              case LessThanOrEqualToken:
                currentNode.value->value.boolean =
                    currentNode.left.value->value.integer <=
                    currentNode.right.value->value.integer;
                break;
              case NotEqualToken:
                currentNode.value->value.boolean =
                    currentNode.left.value->value.integer !=
                    currentNode.right.value->value.integer;
                break;
              default:
                @throw
                    [[TynySQLException alloc] initWithErrorCode:SqlSyntaxError];
              }
              break;
            case String:
              switch (currentNode.operator.kind) {
              case EqualToken:
                currentNode.value->value.boolean =
                    strcmp(currentNode.left.value->value.string,
                           currentNode.right.value->value.string) == 0;
                break;
              case GreaterThanToken:
                currentNode.value->value.boolean =
                    strcmp(currentNode.left.value->value.string,
                           currentNode.right.value->value.string) > 0;
                break;
              case GreaterThanOrEqualToken:
                currentNode.value->value.boolean =
                    strcmp(currentNode.left.value->value.string,
                           currentNode.right.value->value.string) >= 0;
                break;
              case LessThanToken:
                currentNode.value->value.boolean =
                    strcmp(currentNode.left.value->value.string,
                           currentNode.right.value->value.string) < 0;
                break;
              case LessThanOrEqualToken:
                currentNode.value->value.boolean =
                    strcmp(currentNode.left.value->value.string,
                           currentNode.right.value->value.string) <= 0;
                break;
              case NotEqualToken:
                currentNode.value->value.boolean =
                    strcmp(currentNode.left.value->value.string,
                           currentNode.right.value->value.string) != 0;
                break;
              default:
                @throw
                    [[TynySQLException alloc] initWithErrorCode:SqlSyntaxError];
              }
              break;
            default:
              @throw
                  [[TynySQLException alloc] initWithErrorCode:SqlSyntaxError];
            }
            break;
          case PlusToken:
          case MinusToken:
          case AsteriskToken:
          case SlashToken:
            // 四則演算の場合です。

            // 演算できるのは整数型同士の場合のみです。
            if (currentNode.left.value->type != Integer ||
                currentNode.right.value->type != Integer) {
              @throw [[TynySQLException alloc]
                  initWithErrorCode:WhereOperandTypeError];
            }
            currentNode.value->type = Integer;

            // 比較結果を演算子によって計算方法を変えて、計算します。
            switch (currentNode.operator.kind) {
            case PlusToken:
              currentNode.value->value.integer =
                  currentNode.left.value->value.integer +
                  currentNode.right.value->value.integer;
              break;
            case MinusToken:
              currentNode.value->value.integer =
                  currentNode.left.value->value.integer -
                  currentNode.right.value->value.integer;
              break;
            case AsteriskToken:
              currentNode.value->value.integer =
                  currentNode.left.value->value.integer *
                  currentNode.right.value->value.integer;
              break;
            case SlashToken:
              currentNode.value->value.integer =
                  currentNode.left.value->value.integer /
                  currentNode.right.value->value.integer;
              break;
            default:
              @throw
                  [[TynySQLException alloc] initWithErrorCode:SqlSyntaxError];
            }
            break;
          case AndToken:
          case OrToken:
            // 論理演算の場合です。

            // 演算できるのは真偽値型同士の場合のみです。
            if (currentNode.left.value->type != Bool ||
                currentNode.right.value->type != Bool) {
              @throw [[TynySQLException alloc]
                  initWithErrorCode:WhereOperandTypeError];
            }
            currentNode.value->type = Bool;

            // 比較結果を演算子によって計算方法を変えて、計算します。
            switch (currentNode.operator.kind) {
            case AndToken:
              currentNode.value->value.boolean =
                  currentNode.left.value->value.boolean &&
                  currentNode.right.value->value.boolean;
              break;
            case OrToken:
              currentNode.value->value.boolean =
                  currentNode.left.value->value.boolean ||
                  currentNode.right.value->value.boolean;
              break;
            default:
              @throw
                  [[TynySQLException alloc] initWithErrorCode:SqlSyntaxError];
            }
            break;
          default:
            @throw [[TynySQLException alloc] initWithErrorCode:SqlSyntaxError];
          }
          currentNode.calculated = YES;

          // 自身の計算が終わった後は親の計算に戻ります。
          currentNode = currentNode.parent;
        }

        // 条件に合わない行は出力から削除します。
        if (!whereTopNode.value->value.boolean) {
          [allColumnOutputData removeLastObject];
          [outputData removeLastObject];
        }
        // WHERE条件の計算結果をリセットします。
        for (ExtensionTreeNode *node in allNodes) {
          node.calculated = NO;
        }
      }

      // 各テーブルの行のすべての組み合わせを出力します。

      // 最後のテーブルのカレント行をインクリメントします。
      ++currentRows[[tableNames count] - 1];

      // 最後のテーブルが最終行になっていた場合は先頭に戻し、順に前のテーブルのカレント行をインクリメントします。
      for (unsigned long i = [tableNames count] - 1; !*currentRows[i] && 0 < i;
           --i) {
        ++currentRows[i - 1];
        currentRows[i] = inputData[i];
      }

      // 最初のテーブルが最後の行を超えたなら出力行の生成は終わりです。
      if (!*currentRows[0]) {
        break;
      }
    }

    // ORDER句による並び替えの処理を行います。
    if (orderByColumns.count) {
      // ORDER句で指定されている列が、全ての入力行の中のどの行なのかを計算します。
      int orderByColumnIndexes
          [MAX_COLUMN_COUNT];          // ORDER句で指定された列の、すべての行の中でのインデックスです。
      int orderByColumnIndexesNum = 0; // 現在のorderByColumnIndexesの数です。
      for (Column *column in orderByColumns) {
        found = NO;
        for (int j = 0; j < allInputColumnsNum; ++j) {
          if ([column.columnName
                  caseInsensitiveCompare:allInputColumns[j].columnName] ==
                  NSOrderedSame &&
              ([column.tableName
                   isEqualToString:
                       @""] || // テーブル名が設定されている場合のみテーブル名の比較を行います。
               ([column.tableName
                    caseInsensitiveCompare:allInputColumns[j].tableName] ==
                NSOrderedSame))) {

            // 既に見つかっているのにもう一つ見つかったらエラーです。
            if (found) {
              @throw [[TynySQLException alloc]
                  initWithErrorCode:BadColumnNameError];
            }
            found = YES;
            if (MAX_COLUMN_COUNT <= orderByColumnIndexesNum) {
              @throw
                  [[TynySQLException alloc] initWithErrorCode:MemoryOverError];
            }
            orderByColumnIndexes[orderByColumnIndexesNum++] = j;
          }
        }
        // 一つも見つからなくてもエラーです。
        if (!found) {
          @throw
              [[TynySQLException alloc] initWithErrorCode:BadColumnNameError];
        }
      }

      // outputDataとallColumnOutputDataのソートを一緒に行います。簡便のため凝ったソートは使わず、選択ソートを利用します。
      for (int i = 0; i < [outputData count]; ++i) {
        int minIndex = i; // 現在までで最小の行のインデックスです。
        for (int j = i + 1; j < [outputData count]; ++j) {
          BOOL jLessThanMin =
              NO; // インデックスがjの値が、minIndexの値より小さいかどうかです。
          for (int k = 0; k < orderByColumnIndexesNum; ++k) {
            NSValue *mData = allColumnOutputData
                [minIndex][orderByColumnIndexes
                               [k]]; // インデックスがminIndexのデータです。
            NSValue *jData = allColumnOutputData
                [j][orderByColumnIndexes[k]]; // インデックスがjのデータです。
            long cmp =
                0; // 比較結果です。等しければ0、インデックスjの行が大きければプラス、インデックスminIndexの行が大きければマイナスとなります。
            switch (((Data *)mData.pointerValue)->type) {
            case Integer:
              cmp = ((Data *)jData.pointerValue)->value.integer -
                    ((Data *)mData.pointerValue)->value.integer;
              break;
            case String:
              cmp = strcmp(((Data *)jData.pointerValue)->value.string,
                           ((Data *)mData.pointerValue)->value.string);
              break;
            default:
              @throw
                  [[TynySQLException alloc] initWithErrorCode:SqlSyntaxError];
            }

            // 降順ならcmpの大小を入れ替えます。
            if (orders[k] == DescToken) {
              cmp *= -1;
            }
            if (cmp < 0) {
              jLessThanMin = YES;
              break;
            } else if (0 < cmp) {
              break;
            }
          }
          if (jLessThanMin) {
            minIndex = j;
          }
        }
        NSArray *tmp = outputData[minIndex];
        outputData[minIndex] = outputData[i];
        outputData[i] = tmp;

        tmp = allColumnOutputData[minIndex];
        allColumnOutputData[minIndex] = allColumnOutputData[i];
        allColumnOutputData[i] = tmp;
      }
    }

    // 出力ファイルを開きます。
    NSString *outputPath = [NSString stringWithCString:outputFileName
                                              encoding:NSUTF8StringEncoding];
    [[NSFileManager defaultManager] createFileAtPath:outputPath
                                            contents:[NSData data]
                                          attributes:nil];
    outputFile = [NSFileHandle fileHandleForWritingAtPath:outputPath];
    if (outputFile == NULL) {
      @throw [[TynySQLException alloc] initWithErrorCode:FileOpenError];
    }

    // 出力ファイルに列名を出力します。
    for (int i = 0; i < [selectColumns count]; ++i) {
      [outputFile writeData:[outputColumns[i].columnName
                                dataUsingEncoding:NSUTF8StringEncoding]];
      if (i < [selectColumns count] - 1) {
        [outputFile writeData:[@"," dataUsingEncoding:NSUTF8StringEncoding]];
      } else {
        [outputFile writeData:[@"\n" dataUsingEncoding:NSUTF8StringEncoding]];
      }
    }

    // 出力ファイルにデータを出力します。
    for (NSArray *currentRow in outputData) {
      for (int i = 0; i < [selectColumns count]; ++i) {
        NSValue *column = currentRow[i];
        NSString *outputString = nil;
        switch (((Data *)column.pointerValue)->type) {
        case Integer:
          outputString = [NSString
              stringWithFormat:@"%ld",
                               ((Data *)column.pointerValue)->value.integer];
          break;
        case String:
          outputString = [NSString
              stringWithFormat:@"%s",
                               ((Data *)column.pointerValue)->value.string];
          break;
        default:
          @throw [[TynySQLException alloc] initWithErrorCode:SqlSyntaxError];
        }
        [outputFile
            writeData:[outputString dataUsingEncoding:NSUTF8StringEncoding]];
        if (i < [selectColumns count] - 1) {
          [outputFile writeData:[@"," dataUsingEncoding:NSUTF8StringEncoding]];
        } else {
          [outputFile writeData:[@"\n" dataUsingEncoding:NSUTF8StringEncoding]];
        }
      }
    }

    return ResultOk;
  } @catch (TynySQLException *ex) {
    return ex.errorCode;
  } @finally {
    // ファイルリソースを解放します。
    for (NSFileHandle *inputTableFile in inputTableFiles) {
      if (inputTableFile) {
        [inputTableFile synchronizeFile];
        [inputTableFile closeFile];
      }
    }
    if (outputFile) {
      [outputFile synchronizeFile];
      [outputFile closeFile];
    }
    // メモリリソースを解放します。
    for (int i = 0; i < [tableNames count]; ++i) {
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
    for (NSArray *currentRow in outputData) {
      for (NSValue *dataCursol in currentRow) {
        free(dataCursol.pointerValue);
      }
    }
    for (NSArray *currentRow in allColumnOutputData) {
      for (NSValue *dataCursol in currentRow) {
        free(dataCursol.pointerValue);
      }
    }
  }
}
