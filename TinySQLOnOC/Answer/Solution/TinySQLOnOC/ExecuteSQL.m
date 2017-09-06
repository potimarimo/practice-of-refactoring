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
@interface Data : NSObject
@property enum DataType type; //!< データの型です。

//! 実際のデータを格納するオブジェクトです。
@property NSObject *value;
- (NSString *)stringValue;
- (NSInteger)integerValue;
- (BOOL)boolValue;
- (Data *)init;
- (Data *)initWithString:(NSString *)string;
- (Data *)initWithInteger:(long)integer;
- (Data *)initWithBool:(BOOL)boolean;
@end

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
- (Token *)initWithKind:(TokenKind)kind word:(NSString *)word;

@end

@interface TokenEnumerator : NSEnumerator
@property(readonly) NSString *document;
@property NSInteger
    cursol; // SQLをトークンに分割して読み込む時に現在読んでいる文字の場所を表します。
- (Token *)nextObject;
@end

@interface Tokenizer : NSObject
- (Tokenizer *)initWithRules:(NSArray *)rules;
- (TokenEnumerator *)enumeratorWithReadString:(NSString *)read;
@end

@interface RegulerExpressionTokenizeRule : NSObject
@property NSRegularExpression *pattern;
@property TokenKind kind;
- (RegulerExpressionTokenizeRule *)initWithPattern:(NSString *)pattern
                                              kind:(TokenKind)kind;
- (RegulerExpressionTokenizeRule *)initWithPattern:(NSString *)pattern
                                              kind:(TokenKind)kind
                                           options:(NSRegularExpressionOptions)
                                                       options;
- (Token *)parseDocument:(NSString *)string cursol:(NSInteger *)cursol;
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
@interface ExtensionTreeNode : NSObject
- (ExtensionTreeNode *)init;
@property __weak ExtensionTreeNode
    *parent;                       //!< 親となるノードです。根の式木の場合はNULLとなります。
@property ExtensionTreeNode *left; //!<
//!左の子となるノードです。自身が末端の葉となる式木の場合はNULLとなります。
@property Operator *operator; //!<
//!中置される演算子です。自身が末端のとなる式木の場合の種類はNOT_TOKENとなります。
@property ExtensionTreeNode *right; //!<
//!右の子となるノードです。自身が末端の葉となる式木の場合はNULLとなります。
@property BOOL inParen; //!< 自身がかっこにくるまれているかどうかです。
@property int parenOpenBeforeClose; //!<
//!木の構築中に0以外となり、自身の左にあり、まだ閉じてないカッコの開始の数となります。
@property int signCoefficient; //!<
//!自身が葉にあり、マイナス単項演算子がついている場合は-1、それ以外は1となります。
@property Column *column; //!<
//!列場指定されている場合に、その列を表します。列指定ではない場合はcolumnNameが空文字列となります。
@property BOOL calculated; //!< 式の値を計算中に、計算済みかどうかです。
@property Data *value; //!< 指定された、もしくは計算された値です。

@end

//! 行の情報を入力のテーブルインデックス、列インデックスの形で持ちます。
@interface ColumnIndex : NSObject
- (ColumnIndex *)initWithTable:(int)table column:(int)column;
@property int table;  //!< 列が入力の何テーブル目の列かです。
@property int column; //!< 列が入力のテーブルの何列目かです。
@end

@interface TynySQLException : NSException
- (TynySQLException *)initWithErrorCode:(enum ResultValue)code;
@property enum ResultValue errorCode;
@end

// 以上ヘッダに相当する部分。

@implementation Data
- (NSString *)stringValue {
  if (self.type != String)
    return @"";
  return (NSString *)self.value;
}
- (NSInteger)integerValue {
  if (self.type != Integer)
    return 0;
  return ((NSNumber *)self.value).integerValue;
}
- (BOOL)boolValue {
  if (self.type != Bool)
    return NO;
  return ((NSNumber *)self.value).boolValue;
}
- (instancetype)init {
  _type = String;
  _value = @"";
  return self;
}
- (Data *)initWithString:(NSString *)string {
  _type = String;
  _value = string;
  return self;
}
- (Data *)initWithInteger:(long)integer {
  _type = Integer;
  _value = @(integer);
  return self;
}
- (Data *)initWithBool:(BOOL)boolean {
  _type = Bool;
  _value = @(boolean);
  return self;
}

@end

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
- (Token *)initWithKind:(enum TokenKind)kind word:(NSString *)word {
  _kind = kind;
  _word = word;
  return self;
}

@end

@implementation TokenEnumerator {
@public
  Token * (^_generater)();
  NSString * (^_string)();
  NSInteger _cursol;
}
- (Token *)nextObject {
  return _generater();
}
- (NSString *)document {
  return _string();
}
@end
@implementation Tokenizer {
  NSArray *_rules;
}
- (Tokenizer *)initWithRules:(NSArray *)rules {
  _rules = rules;
  return self;
}
- (TokenEnumerator *)enumeratorWithReadString:(NSString *)read {
  TokenEnumerator *ret = TokenEnumerator.new;
  ret->_string = ^() {
    return read;
  };
  NSInteger *pCursol = &ret->_cursol;
  ret->_generater = ^() {
    for (RegulerExpressionTokenizeRule *rule in _rules) {
      Token *token = [rule parseDocument:read cursol:pCursol];
      if (token) {

        return token;
      }
    }
    return (Token *)nil;
  };
  return ret;
}
@end

@implementation RegulerExpressionTokenizeRule
- (RegulerExpressionTokenizeRule *)initWithPattern:(NSString *)pattern
                                              kind:(TokenKind)kind {
  _kind = kind;
  _pattern = [NSRegularExpression
      regularExpressionWithPattern:[@"^" stringByAppendingString:pattern]
                           options:0
                             error:NULL];
  return self;
}
- (RegulerExpressionTokenizeRule *)initWithPattern:(NSString *)pattern
                                              kind:(TokenKind)kind
                                           options:(NSRegularExpressionOptions)
                                                       options {
  _kind = kind;
  _pattern = [NSRegularExpression
      regularExpressionWithPattern:[@"^" stringByAppendingString:pattern]
                           options:options
                             error:NULL];
  return self;
}
- (Token *)parseDocument:(NSString *)document cursol:(NSInteger *)cursol {
  NSTextCheckingResult *result = [self.pattern
      firstMatchInString:document
                 options:0
                   range:NSMakeRange(*cursol, document.length - *cursol)];

  if (result != nil) {
    *cursol += result.range.length;
    return
        [Token.alloc initWithKind:self.kind
                             word:[document substringWithRange:result.range]];
  } else {
    return nil;
  }
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
  _value = Data.new;
  return self;
}
@end

@implementation ColumnIndex
- (ColumnIndex *)initWithTable:(int)table column:(int)column {
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

char getChar(NSString *string, long cursol) {
  if ([string length] <= cursol) {
    return 0;
  }
  NSString *charactor = [string substringWithRange:NSMakeRange(cursol, 1)];

  char buf[] = " ";
  char *ch = buf;
  [charactor getCString:ch maxLength:2 encoding:NSUTF8StringEncoding];
  return *ch;
}

NSString *getOneCharactor(NSString *string, long cursol) {
  if ([string length] <= cursol) {
    return nil;
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
  NSMutableArray *inputData = NSMutableArray.new;  // 入力データです。
  NSMutableArray *outputData = NSMutableArray.new; // 出力データです。
  NSMutableArray *allColumnOutputData =
      NSMutableArray
          .new; // 出力するデータに対応するインデックスを持ち、すべての入力データを保管します。
  NSMutableArray *tableNames =
      NSMutableArray.new; // FROM句で指定しているテーブル名です。
  @try {

    BOOL found = NO;                      // 検索時に見つかったかどうかの結果を一時的に保存します。
    const char *signNum = "+-0123456789"; // 全ての符号と数字です。

    // SQLからトークンを読み込みます。

    // keywordConditionsとsignConditionsは先頭から順に検索されるので、前方一致となる二つの項目は順番に気をつけて登録しなくてはいけません。

    NSMutableArray *tokens = NSMutableArray.new; // SQLを分割したトークンです。

    // 演算子の情報です。
    NSArray *operators = @[
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
      [Operator.alloc initWithKind:OrToken Order:5]
    ];

    Tokenizer *tokenizer = [Tokenizer.alloc initWithRules:@[
      [RegulerExpressionTokenizeRule.alloc initWithPattern:@"\\s+"
                                                      kind:NoToken],
      [RegulerExpressionTokenizeRule.alloc
          initWithPattern:@"\\d+(?![\\p{Ll}\\p{Lu}\\d_])"
                     kind:IntLiteralToken],
      [RegulerExpressionTokenizeRule.alloc initWithPattern:@"\'.*\'"
                                                      kind:StringLiteralToken],
      [RegulerExpressionTokenizeRule.alloc
          initWithPattern:@"AND(?![\\p{Ll}\\p{Lu}\\d_])"
                     kind:AndToken
                  options:NSRegularExpressionCaseInsensitive],
      [RegulerExpressionTokenizeRule.alloc
          initWithPattern:@"ASC(?![\\p{Ll}\\p{Lu}\\d_])"
                     kind:AscToken
                  options:NSRegularExpressionCaseInsensitive],
      [RegulerExpressionTokenizeRule.alloc
          initWithPattern:@"BY(?![\\p{Ll}\\p{Lu}\\d_])"
                     kind:ByToken
                  options:NSRegularExpressionCaseInsensitive],
      [RegulerExpressionTokenizeRule.alloc
          initWithPattern:@"DESC(?![\\p{Ll}\\p{Lu}\\d_])"
                     kind:DescToken
                  options:NSRegularExpressionCaseInsensitive],
      [RegulerExpressionTokenizeRule.alloc
          initWithPattern:@"FROM(?![\\p{Ll}\\p{Lu}\\d_])"
                     kind:FromToken
                  options:NSRegularExpressionCaseInsensitive],
      [RegulerExpressionTokenizeRule.alloc
          initWithPattern:@"ORDER(?![\\p{Ll}\\p{Lu}\\d_])"
                     kind:OrderToken
                  options:NSRegularExpressionCaseInsensitive],
      [RegulerExpressionTokenizeRule.alloc
          initWithPattern:@"OR(?![\\p{Ll}\\p{Lu}\\d_])"
                     kind:OrToken
                  options:NSRegularExpressionCaseInsensitive],
      [RegulerExpressionTokenizeRule.alloc
          initWithPattern:@"SELECT(?![\\p{Ll}\\p{Lu}\\d_])"
                     kind:SelectToken
                  options:NSRegularExpressionCaseInsensitive],
      [RegulerExpressionTokenizeRule.alloc
          initWithPattern:@"WHERE(?![\\p{Ll}\\p{Lu}\\d_])"
                     kind:WhereToken
                  options:NSRegularExpressionCaseInsensitive],
      [RegulerExpressionTokenizeRule.alloc
          initWithPattern:@">="
                     kind:GreaterThanOrEqualToken],
      [RegulerExpressionTokenizeRule.alloc
          initWithPattern:@"<="
                     kind:LessThanOrEqualToken],
      [RegulerExpressionTokenizeRule.alloc initWithPattern:@"<>"
                                                      kind:NotEqualToken],
      [RegulerExpressionTokenizeRule.alloc initWithPattern:@"\\*"
                                                      kind:AsteriskToken],
      [RegulerExpressionTokenizeRule.alloc initWithPattern:@","
                                                      kind:CommaToken],
      [RegulerExpressionTokenizeRule.alloc initWithPattern:@"\\)"
                                                      kind:CloseParenToken],
      [RegulerExpressionTokenizeRule.alloc initWithPattern:@"\\."
                                                      kind:DotToken],
      [RegulerExpressionTokenizeRule.alloc initWithPattern:@"="
                                                      kind:EqualToken],
      [RegulerExpressionTokenizeRule.alloc initWithPattern:@">"
                                                      kind:GreaterThanToken],
      [RegulerExpressionTokenizeRule.alloc initWithPattern:@"<"
                                                      kind:LessThanToken],
      [RegulerExpressionTokenizeRule.alloc initWithPattern:@"\\-"
                                                      kind:MinusToken],
      [RegulerExpressionTokenizeRule.alloc initWithPattern:@"\\("
                                                      kind:OpenParenToken],
      [RegulerExpressionTokenizeRule.alloc initWithPattern:@"\\+"
                                                      kind:PlusToken],
      [RegulerExpressionTokenizeRule.alloc initWithPattern:@"/"
                                                      kind:SlashToken],
      [RegulerExpressionTokenizeRule.alloc
          initWithPattern:@"[\\p{Ll}\\p{Lu}_][\\p{Ll}\\p{Lu}\\d_]*"
                     kind:IdentifierToken],
    ]];

    TokenEnumerator *tokenEnumerator = [tokenizer
        enumeratorWithReadString:[NSString
                                     stringWithCString:sql
                                              encoding:NSUTF8StringEncoding]];

    // SQLをトークンに分割て読み込みます。
    for (Token *token = tokenEnumerator.nextObject; token;
         token = tokenEnumerator.nextObject) {
      if (token.kind != NoToken) {
        [tokens addObject:token];
      }
    }
    if (tokenEnumerator.cursol < tokenEnumerator.document.length) {
      @throw [TynySQLException.alloc initWithErrorCode:TokenCantReadError];
    }

    // トークン列を解析し、構文を読み取ります。

    NSEnumerator *tokenCursol =
        [tokens objectEnumerator]; // 現在見ているトークンを指します。

    NSMutableArray *selectColumns =
        NSMutableArray.new; // SELECT句に指定された列名です。

    NSMutableArray *orderByColumns =
        NSMutableArray.new; // ORDER句に指定された列名です。

    NSMutableArray *orders =
        NSMutableArray
            .new; // 同じインデックスのorderByColumnsに対応している、昇順、降順の指定です。

    ExtensionTreeNode *whereTopNode = nil; // 式木の根となるノードです。

    // SQLの構文を解析し、必要な情報を取得します。

    // SELECT句を読み込みます。
    Token *nextToken = tokenCursol.nextObject;
    if (nextToken.kind == SelectToken) {
      nextToken = tokenCursol.nextObject;
    } else {
      @throw [TynySQLException.alloc initWithErrorCode:SqlSyntaxError];
    }

    if (nextToken.kind == AsteriskToken) {
      nextToken = tokenCursol.nextObject;
    } else {
      BOOL first =
          YES; // SELECT句に最初に指定された列名の読み込みかどうかです。
      while (nextToken.kind == CommaToken || first) {
        if (nextToken.kind == CommaToken) {
          nextToken = tokenCursol.nextObject;
          ;
        }
        if (nextToken.kind == IdentifierToken) {
          // テーブル名が指定されていない場合と仮定して読み込みます。
          Column *column =
              [[Column alloc] initWithTableName:@"" ColumnName:nextToken.word];
          [selectColumns addObject:column];

          nextToken = tokenCursol.nextObject;
          if (nextToken.kind == DotToken) {
            nextToken = tokenCursol.nextObject;
            if (nextToken.kind == IdentifierToken) {

              // テーブル名が指定されていることがわかったので読み替えます。
              column.tableName = column.columnName;
              column.columnName = nextToken.word;
              nextToken = tokenCursol.nextObject;
              ;
            } else {
              @throw [TynySQLException.alloc initWithErrorCode:SqlSyntaxError];
            }
          }
        } else {
          @throw [TynySQLException.alloc initWithErrorCode:SqlSyntaxError];
        }
        first = NO;
      }
    }
    NSMutableArray *allNodes = NSMutableArray.new;
    // ORDER句とWHERE句を読み込みます。最大各一回ずつ書くことができます。
    BOOL readOrder = NO; // すでにORDER句が読み込み済みかどうかです。
    BOOL readWhere = NO; // すでにWHERE句が読み込み済みかどうかです。
    while (nextToken.kind == OrderToken || nextToken.kind == WhereToken) {

      // 二度目のORDER句はエラーです。
      if (readOrder && nextToken.kind == OrderToken) {
        @throw [TynySQLException.alloc initWithErrorCode:SqlSyntaxError];
      }

      // 二度目のWHERE句はエラーです。
      if (readWhere && nextToken.kind == WhereToken) {
        @throw [TynySQLException.alloc initWithErrorCode:SqlSyntaxError];
      }
      // ORDER句を読み込みます。
      if (nextToken.kind == OrderToken) {
        readOrder = YES;
        nextToken = tokenCursol.nextObject;

        if (nextToken.kind == ByToken) {
          nextToken = tokenCursol.nextObject;

          BOOL first = YES; // ORDER句の最初の列名の読み込みかどうかです。
          while (nextToken.kind == CommaToken || first) {
            if (nextToken.kind == CommaToken) {
              nextToken = tokenCursol.nextObject;
            }
            if (nextToken.kind == IdentifierToken) {

              // テーブル名が指定されていない場合と仮定して読み込みます。
              Column *column = [Column.alloc initWithTableName:@""
                                                    ColumnName:nextToken.word];
              [orderByColumns addObject:column];
              nextToken = tokenCursol.nextObject;
              ;
              if (nextToken.kind == DotToken) {
                nextToken = tokenCursol.nextObject;
                ;
                if (nextToken.kind == IdentifierToken) {

                  // テーブル名が指定されていることがわかったので読み替えます。
                  column.tableName = column.columnName;
                  column.columnName = nextToken.word;

                  nextToken = tokenCursol.nextObject;
                  ;
                } else {
                  @throw
                      [TynySQLException.alloc initWithErrorCode:SqlSyntaxError];
                }
              }

              // 並び替えの昇順、降順を指定します。
              if (nextToken.kind == AscToken) {

                [orders addObject:@(AscToken)];
                nextToken = tokenCursol.nextObject;

              } else if (nextToken.kind == DescToken) {
                [orders addObject:@(DescToken)];
                nextToken = tokenCursol.nextObject;
                ;
              } else {
                // 指定がない場合は昇順となります。
                [orders addObject:@(AscToken)];
              }
            } else {
              @throw [TynySQLException.alloc initWithErrorCode:SqlSyntaxError];
            }
            first = NO;
          }
        } else {
          @throw [TynySQLException.alloc initWithErrorCode:SqlSyntaxError];
        }
      }

      // WHERE句を読み込みます。

      if (nextToken.kind == WhereToken) {
        readWhere = YES;
        nextToken = tokenCursol.nextObject;

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
            nextToken = tokenCursol.nextObject;
            ;
          }

          // オペランドに前置される+か-を読み込みます。
          if (nextToken.kind == PlusToken || nextToken.kind == MinusToken) {
            if (nextToken.kind == MinusToken) {
              currentNode.signCoefficient = -1;
            }
            nextToken = tokenCursol.nextObject;
            // +-を前置するのは列名と数値リテラルのみです。
            if (nextToken.kind != IdentifierToken &&
                nextToken.kind != IntLiteralToken) {
              @throw [TynySQLException.alloc
                  initWithErrorCode:WhereOperandTypeError];
            }
          }

          // 列名、整数リテラル、文字列リテラルのいずれかをオペランドとして読み込みます。
          if (nextToken.kind == IdentifierToken) {

            // テーブル名が指定されていない場合と仮定して読み込みます。
            currentNode.column.tableName = @"";
            currentNode.column.columnName = nextToken.word;

            nextToken = tokenCursol.nextObject;
            ;
            if (nextToken.kind == DotToken) {
              nextToken = tokenCursol.nextObject;

              if (nextToken.kind == IdentifierToken) {

                // テーブル名が指定されていることがわかったので読み替えます。
                currentNode.column.tableName = currentNode.column.columnName;

                currentNode.column.columnName = nextToken.word;
                nextToken = tokenCursol.nextObject;
              } else {
                @throw
                    [TynySQLException.alloc initWithErrorCode:SqlSyntaxError];
              }
            }
          } else if (nextToken.kind == IntLiteralToken) {
            currentNode.value =
                [Data.alloc initWithInteger:[nextToken.word integerValue]];
            nextToken = tokenCursol.nextObject;

          } else if (nextToken.kind == StringLiteralToken) {
            currentNode.value = Data.new;

            // 前後のシングルクォートを取り去った文字列をデータとして読み込みます。
            currentNode.value.value = [nextToken.word
                substringWithRange:NSMakeRange(1, [nextToken.word length] - 2)];
            nextToken = tokenCursol.nextObject;
          } else {
            @throw [TynySQLException.alloc initWithErrorCode:SqlSyntaxError];
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
            nextToken = tokenCursol.nextObject;
            ;
          }

          // 演算子(オペレーターを読み込みます。
          Operator *operator=
              Operator.new; // 現在読み込んでいる演算子の情報です。

          // 現在見ている演算子の情報を探します。
          found = NO;
          for (Operator *oneOperator in operators) {
            if (oneOperator.kind == nextToken.kind) {
              operator= oneOperator;
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

            nextToken = tokenCursol.nextObject;
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
      nextToken = tokenCursol.nextObject;
    } else {
      @throw [TynySQLException.alloc initWithErrorCode:SqlSyntaxError];
    }
    BOOL first = YES; // FROM句の最初のテーブル名を読み込み中かどうかです。
    while (nextToken.kind == CommaToken || first) {
      if (nextToken.kind == CommaToken) {
        nextToken = tokenCursol.nextObject;
      }
      if (nextToken.kind == IdentifierToken) {
        [tableNames addObject:nextToken.word];
        nextToken = tokenCursol.nextObject;
      } else {
        @throw [TynySQLException.alloc initWithErrorCode:SqlSyntaxError];
      }
      first = NO;
    }

    // 最後のトークンまで読み込みが進んでいなかったらエラーです。
    if (nextToken) {
      @throw [TynySQLException.alloc initWithErrorCode:SqlSyntaxError];
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

    int tableNamesNum = 0;
    for (NSString *tableName in tableNames) {
      NSMutableArray *table = NSMutableArray.new;
      [inputData addObject:table];

      // 入力ファイルを開きます。
      NSFileHandle *inputFile = [NSFileHandle
          fileHandleForReadingAtPath:[tableName
                                         stringByAppendingString:@".csv"]];
      if (!inputFile) {
        @throw [TynySQLException.alloc initWithErrorCode:FileOpenError];
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
        while (getOneCharactor(inputLine, charactorCursol) &&
               ![getOneCharactor(inputLine, charactorCursol)
                   isEqualToString:@"\r"] &&
               ![getOneCharactor(inputLine, charactorCursol)
                   isEqualToString:@"\n"]) {
          inputColumns[tableNamesNum][inputColumnNums[tableNamesNum]]
              .tableName = tableName;

          NSMutableString *wrote = NSMutableString.new;

          // 列名を一つ読みます。
          while (getOneCharactor(inputLine, charactorCursol) &&
                 ![getOneCharactor(inputLine, charactorCursol)
                     isEqualToString:@","] &&
                 ![getOneCharactor(inputLine, charactorCursol)
                     isEqualToString:@"\r"] &&
                 ![getOneCharactor(inputLine, charactorCursol)
                     isEqualToString:@"\n"]) {
            [wrote appendString:getOneCharactor(inputLine, charactorCursol++)];
          }

          inputColumns[tableNamesNum][inputColumnNums[tableNamesNum]++]
              .columnName = wrote;

          // 入力行のカンマの分を読み進めます。
          ++charactorCursol;
        }
      } else {
        @throw [TynySQLException.alloc initWithErrorCode:CsvSyntaxError];
      }

      // 入力CSVのデータ行を読み込みます。
      int rowNum = 0;
      while (rowNum < [allLines count] - 1) {
        if (MAX_ROW_COUNT <= rowNum) {
          @throw [TynySQLException.alloc initWithErrorCode:MemoryOverError];
        }
        NSMutableArray *row =
            NSMutableArray.new; // 入力されている一行分のデータです。
        [inputData[inputData.count - 1] addObject:row];
        inputLine = allLines[rowNum++ + 1];
        int charactorCursol = 0;

        // 読み込んだ行を最後まで読みます。
        while (getOneCharactor(inputLine, charactorCursol) &&
               ![getOneCharactor(inputLine, charactorCursol)
                   isEqualToString:@"\r"] &&
               ![getOneCharactor(inputLine, charactorCursol)
                   isEqualToString:@"\n"]) {

          // 読み込んだデータを書き込む行のカラムを生成します。;
          [row addObject:Data.new];
          NSMutableString *wrote = NSMutableString.new;

          // データ文字列を一つ読みます。
          while (getOneCharactor(inputLine, charactorCursol) &&
                 ![getOneCharactor(inputLine, charactorCursol)
                     isEqualToString:@","] &&
                 ![getOneCharactor(inputLine, charactorCursol)
                     isEqualToString:@"\r"] &&
                 ![getOneCharactor(inputLine, charactorCursol)
                     isEqualToString:@"\n"]) {
            [wrote appendString:getOneCharactor(inputLine, charactorCursol++)];
          }
          // 書き込んでいる列名の文字列に終端文字を書き込みます。
          row[row.count - 1] = [Data.alloc initWithString:wrote];

          // 入力行のカンマの分を読み進めます。
          ++charactorCursol;
        }
      }

      // 全てが数値となる列は数値列に変換します。
      for (int j = 0; j < inputColumnNums[tableNamesNum]; ++j) {

        // 全ての行のある列について、データ文字列から符号と数値以外の文字を探します。
        found = NO;
        for (NSArray *tableRow in inputData[inputData.count - 1]) {
          char word[MAX_WORD_LENGTH] = "";
          char *currentChar = word;
          [((Data *)tableRow[j]).stringValue getCString:word
                                              maxLength:MAX_WORD_LENGTH
                                               encoding:NSUTF8StringEncoding];
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
        }

        // 符号と数字以外が見つからない列については、数値列に変換します。
        if (!found) {
          for (NSMutableArray *tableRow in inputData[inputData.count - 1]) {
            tableRow[j] =
                [Data.alloc initWithInteger:[((Data *)tableRow[j])
                                                    .stringValue integerValue]];
          }
        }
      }
      tableNamesNum++;
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
      for (int j = 0; j < tableNamesNum; ++j) {
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
              @throw
                  [TynySQLException.alloc initWithErrorCode:BadColumnNameError];
            }
            found = YES;
            // 見つかった値を持つ列のデータを生成します。
            [selectColumnIndexes
                addObject:[[ColumnIndex alloc] initWithTable:j column:k]];
          }
        }
      }
      // 一つも見つからなくてもエラーです。
      if (!found) {
        @throw [TynySQLException.alloc initWithErrorCode:BadColumnNameError];
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
            node.value.type == Integer) {
          node.value = [Data.alloc
              initWithInteger:node.value.integerValue * node.signCoefficient];
        }
      }
    }

    NSMutableArray *currentRowNums =
        NSMutableArray
            .new; // 入力された各テーブルの、現在出力している行を指すカーソルです。
    for (int i = 0; i < tableNamesNum; ++i) {
      // 各テーブルの先頭行を設定します。
      [currentRowNums addObject:@0];
    }

    // 出力するデータを設定します。
    while (YES) {
      NSMutableArray *row =
          NSMutableArray.new; // 出力している一行分のデータです。
      [outputData addObject:row];

      // 行の各列のデータを入力から持ってきて設定します。
      for (ColumnIndex *index in selectColumnIndexes) {
        [row addObject:((NSArray *)
                            inputData[index.table]
                                     [((NSNumber *)currentRowNums[index.table])
                                          .integerValue])[index.column]];
      }

      NSMutableArray *allColumnsRow = NSMutableArray.new;
      [allColumnOutputData
          addObject:
              allColumnsRow]; // WHEREやORDERのためにすべての情報を含む行。rowとインデックスを共有します。

      // allColumnsRowの列を設定します。
      for (int i = 0; i < tableNamesNum; ++i) {
        for (int j = 0; j < inputColumnNums[i]; ++j) {

          [allColumnsRow
              addObject:((NSArray *)inputData[i][((NSNumber *)currentRowNums[i])
                                                     .integerValue])[j]];
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
                    @throw [TynySQLException.alloc
                        initWithErrorCode:BadColumnNameError];
                  }
                  found = YES;
                  currentNode.value = allColumnsRow[i];
                }
              }
              // 一つも見つからなくてもエラーです。
              if (!found) {
                @throw [TynySQLException.alloc
                    initWithErrorCode:BadColumnNameError];
              };
              // 符号を考慮して値を計算します。
              if (currentNode.value.type == Integer) {
                currentNode.value =
                    [Data.alloc initWithInteger:currentNode.value.integerValue *
                                                currentNode.signCoefficient];
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
            if ((currentNode.left.value.type != Integer &&
                 currentNode.left.value.type != String) ||
                currentNode.left.value.type != currentNode.right.value.type) {
              @throw [TynySQLException.alloc
                  initWithErrorCode:WhereOperandTypeError];
            }
            // 比較結果を型と演算子によって計算方法を変えて、計算します。
            switch (currentNode.left.value.type) {
            case Integer:
              switch (currentNode.operator.kind) {
              case EqualToken:
                currentNode.value = [Data.alloc
                    initWithBool:currentNode.left.value.integerValue ==
                                 currentNode.right.value.integerValue];
                break;
              case GreaterThanToken:
                currentNode.value = [Data.alloc
                    initWithBool:currentNode.left.value.integerValue >
                                 currentNode.right.value.integerValue];
                break;
              case GreaterThanOrEqualToken:
                currentNode.value = [Data.alloc
                    initWithBool:currentNode.left.value.integerValue >=
                                 currentNode.right.value.integerValue];
                break;
              case LessThanToken:
                currentNode.value = [Data.alloc
                    initWithBool:currentNode.left.value.integerValue <
                                 currentNode.right.value.integerValue];
                break;
              case LessThanOrEqualToken:
                currentNode.value = [Data.alloc
                    initWithBool:currentNode.left.value.integerValue <=
                                 currentNode.right.value.integerValue];
                break;
              case NotEqualToken:
                currentNode.value = [Data.alloc
                    initWithBool:currentNode.left.value.integerValue !=
                                 currentNode.right.value.integerValue];
                break;
              default:
                @throw
                    [TynySQLException.alloc initWithErrorCode:SqlSyntaxError];
              }
              break;
            case String:
              switch (currentNode.operator.kind) {
              case EqualToken:
                currentNode.value =
                    [Data.alloc initWithBool:[currentNode.left.value.stringValue
                                                 compare:currentNode.right.value
                                                             .stringValue] ==
                                             NSOrderedSame];
                break;
              case GreaterThanToken:

                currentNode.value =
                    [Data.alloc initWithBool:[currentNode.left.value.stringValue
                                                 compare:currentNode.right.value
                                                             .stringValue] ==
                                             NSOrderedDescending];
                break;
              case GreaterThanOrEqualToken:

                currentNode.value =
                    [Data.alloc initWithBool:[currentNode.left.value.stringValue
                                                 compare:currentNode.right.value
                                                             .stringValue] !=
                                             NSOrderedAscending];
                break;
              case LessThanToken:

                currentNode.value =
                    [Data.alloc initWithBool:[currentNode.left.value.stringValue
                                                 compare:currentNode.right.value
                                                             .stringValue] ==
                                             NSOrderedAscending];
                break;
              case LessThanOrEqualToken:
                currentNode.value =
                    [Data.alloc initWithBool:[currentNode.left.value.stringValue
                                                 compare:currentNode.right.value
                                                             .stringValue] !=
                                             NSOrderedDescending];
                break;
              case NotEqualToken:

                currentNode.value =
                    [Data.alloc initWithBool:[currentNode.left.value.stringValue
                                                 compare:currentNode.right.value
                                                             .stringValue] !=
                                             NSOrderedSame];
                ;
                break;
              default:
                @throw
                    [TynySQLException.alloc initWithErrorCode:SqlSyntaxError];
              }
              break;
            default:
              @throw [TynySQLException.alloc initWithErrorCode:SqlSyntaxError];
            }
            break;
          case PlusToken:
          case MinusToken:
          case AsteriskToken:
          case SlashToken:
            // 四則演算の場合です。

            // 演算できるのは整数型同士の場合のみです。
            if (currentNode.left.value.type != Integer ||
                currentNode.right.value.type != Integer) {
              @throw [TynySQLException.alloc
                  initWithErrorCode:WhereOperandTypeError];
            }
            currentNode.value.type = Integer;

            // 比較結果を演算子によって計算方法を変えて、計算します。
            switch (currentNode.operator.kind) {
            case PlusToken:
              currentNode.value = [Data.alloc
                  initWithInteger:currentNode.left.value.integerValue +
                                  currentNode.right.value.integerValue];
              break;
            case MinusToken:
              currentNode.value = [Data.alloc
                  initWithInteger:currentNode.left.value.integerValue -
                                  currentNode.right.value.integerValue];
              break;
            case AsteriskToken:
              currentNode.value = [Data.alloc
                  initWithInteger:currentNode.left.value.integerValue *
                                  currentNode.right.value.integerValue];
              break;
            case SlashToken:
              currentNode.value = [Data.alloc
                  initWithInteger:currentNode.left.value.integerValue /
                                  currentNode.right.value.integerValue];
              break;
            default:
              @throw [TynySQLException.alloc initWithErrorCode:SqlSyntaxError];
            }
            break;
          case AndToken:
          case OrToken:
            // 論理演算の場合です。

            // 演算できるのは真偽値型同士の場合のみです。
            if (currentNode.left.value.type != Bool ||
                currentNode.right.value.type != Bool) {
              @throw [TynySQLException.alloc
                  initWithErrorCode:WhereOperandTypeError];
            }

            // 比較結果を演算子によって計算方法を変えて、計算します。
            switch (currentNode.operator.kind) {
            case AndToken:
              currentNode.value =
                  [Data.alloc initWithBool:currentNode.left.value.boolValue &&
                                           currentNode.right.value.boolValue];
              break;
            case OrToken:

              currentNode.value =
                  [Data.alloc initWithBool:currentNode.left.value.boolValue ||
                                           currentNode.right.value.boolValue];
              break;
            default:
              @throw [TynySQLException.alloc initWithErrorCode:SqlSyntaxError];
            }
            break;
          default:
            @throw [TynySQLException.alloc initWithErrorCode:SqlSyntaxError];
          }
          currentNode.calculated = YES;

          // 自身の計算が終わった後は親の計算に戻ります。
          currentNode = currentNode.parent;
        }

        // 条件に合わない行は出力から削除します。
        if (!whereTopNode.value.boolValue) {
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
      currentRowNums[[tableNames count] - 1] =
          @(((NSNumber *)currentRowNums[[tableNames count] - 1]).integerValue +
            1);

      // 最後のテーブルが最終行になっていた場合は先頭に戻し、順に前のテーブルのカレント行をインクリメントします。
      for (unsigned long i = [tableNames count] - 1;
           ((NSArray *)inputData[i]).count <=
               ((NSNumber *)currentRowNums[i]).integerValue &&
           0 < i;
           --i) {
        currentRowNums[i - 1] =
            @(((NSNumber *)currentRowNums[i - 1]).integerValue + 1);
        currentRowNums[i] = @0;
      }

      // 最初のテーブルが最後の行を超えたなら出力行の生成は終わりです。
      if (((NSArray *)inputData[0]).count <=
          ((NSNumber *)currentRowNums[0]).integerValue) {
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
              @throw
                  [TynySQLException.alloc initWithErrorCode:BadColumnNameError];
            }
            found = YES;
            if (MAX_COLUMN_COUNT <= orderByColumnIndexesNum) {
              @throw [TynySQLException.alloc initWithErrorCode:MemoryOverError];
            }
            orderByColumnIndexes[orderByColumnIndexesNum++] = j;
          }
        }
        // 一つも見つからなくてもエラーです。
        if (!found) {
          @throw [TynySQLException.alloc initWithErrorCode:BadColumnNameError];
        }
      }

      // outputDataとallColumnOutputDataのソートを一緒に行います。簡便のため凝ったソートは使わず、選択ソートを利用します。
      for (int i = 0; i < [outputData count]; ++i) {
        int minIndex = i; // 現在までで最小の行のインデックスです。
        for (int j = i + 1; j < [outputData count]; ++j) {
          BOOL jLessThanMin =
              NO; // インデックスがjの値が、minIndexの値より小さいかどうかです。
          for (int k = 0; k < orderByColumnIndexesNum; ++k) {
            Data *mData = allColumnOutputData
                [minIndex][orderByColumnIndexes
                               [k]]; // インデックスがminIndexのデータです。
            Data *jData = allColumnOutputData
                [j][orderByColumnIndexes[k]]; // インデックスがjのデータです。
            long cmp =
                0; // 比較結果です。等しければ0、インデックスjの行が大きければプラス、インデックスminIndexの行が大きければマイナスとなります。
            switch (mData.type) {
            case Integer:
              cmp = jData.integerValue - mData.integerValue;
              break;
            case String:
              cmp = [jData.stringValue compare:mData.stringValue];
              break;
            default:
              @throw [TynySQLException.alloc initWithErrorCode:SqlSyntaxError];
            }

            // 降順ならcmpの大小を入れ替えます。
            if (((NSNumber *)orders[k]).integerValue == DescToken) {
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
      @throw [TynySQLException.alloc initWithErrorCode:FileOpenError];
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
        Data *column = currentRow[i];
        NSString *outputString = nil;
        switch (column.type) {
        case Integer:
          outputString =
              [NSString stringWithFormat:@"%ld", column.integerValue];
          break;
        case String:
          outputString = [NSString stringWithFormat:@"%@", column.stringValue];
          break;
        default:
          @throw [TynySQLException.alloc initWithErrorCode:SqlSyntaxError];
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
  }
}
