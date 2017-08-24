#import "ExecuteSQL.h"
#import <Foundation/Foundation.h>
#import <XCTest/XCTest.h>

@interface Tests : XCTestCase
@end

@implementation Tests
char *OUTPUT_PATH = "output.csv";

enum REAULT_VALUE {
  OK = 0,            //!< 問題なく終了しました。
  ERR_FILE_OPEN = 1, //!< ファイルを開くことに失敗しました。
  ERR_FILE_WRITE = 2, //!< ファイルに書き込みを行うことに失敗しました。
  ERR_FILE_CLE = 3, //!< ファイルを閉じることに失敗しました。
  ERR_TOKEN_CANT_READ = 4, //!< トークン解析に失敗しました。
  ERR_SQL_SYNTAX = 5,      //!< SQLの構文解析が失敗しました。
  ERR_BAD_COLUMN_NAME = 6, //!< テーブル指定を含む列名が適切ではありません。
  ERR_WHERE_OPERAND_TYPE = 7, //!< 演算の左右の型が適切ではありません。
  ERR_CSV_SYNTAX = 8,      //!< CSVの構文解析が失敗しました。
  ERR_MEMORY_ALLOCATE = 9, //!< メモリの取得に失敗しました。
  ERR_MEMORY_OVER = 10 //!< 用意したメモリ領域の上限を超えました。
};

- (NSString *)readOutput {
  NSError *error = nil;
  return [NSString
      stringWithContentsOfFile:[NSString stringWithCString:OUTPUT_PATH
                                                  encoding:NSUTF8StringEncoding]
                      encoding:NSUTF8StringEncoding
                         error:&error];
}

- (void)setUp {

  [super setUp];

  NSArray *files = [NSArray
      arrayWithObjects:@"output.csv", @"TABLE1.csv", @"TABLE2.csv",
                       @"TABLE3.csv", @"UNORDERED.csv", @"UNORDERED2.csv",
                       @"PARENTS.csv", @"CHILDREN.csv", @"MINUS.csv", nil];
  NSError *err = nil;
  NSFileManager *manager = [NSFileManager defaultManager];
  for (NSString *file in files) {
    if ([manager fileExistsAtPath:file]) {
      while (![manager removeItemAtPath:file error:&err])
        ;
    }
  }

  NSMutableString *csv = [NSMutableString string];
  [csv appendString:@"Integer,String\n"];
  [csv appendString:@"1,A\n"];
  [csv appendString:@"2,B\n"];
  [csv appendString:@"3,C\n"];
  [[csv dataUsingEncoding:NSShiftJISStringEncoding] writeToFile:@"TABLE1.csv"
                                                     atomically:YES];

  csv = [NSMutableString string];

  [csv appendString:@"Integer,String\n"];
  [csv appendString:@"4,D\n"];
  [csv appendString:@"5,E\n"];
  [csv appendString:@"6,F\n"];
  [[csv dataUsingEncoding:NSShiftJISStringEncoding] writeToFile:@"TABLE2.csv"
                                                     atomically:YES];

  csv = [NSMutableString string];

  [csv appendString:@"Integer,String\n"];
  [csv appendString:@"7,G\n"];
  [csv appendString:@"8,H\n"];
  [[csv dataUsingEncoding:NSShiftJISStringEncoding] writeToFile:@"TABLE3.csv"
                                                     atomically:YES];

  csv = [NSMutableString string];

  [csv appendString:@"Integer,String\n"];
  [csv appendString:@"21,BA\n"];
  [csv appendString:@"2,B\n"];
  [csv appendString:@"12,AB\n"];
  [csv appendString:@"11,AA\n"];
  [csv appendString:@"22,BB\n"];
  [csv appendString:@"1,A\n"];
  [[csv dataUsingEncoding:NSShiftJISStringEncoding] writeToFile:@"UNORDERED.csv"
                                                     atomically:YES];

  csv = [NSMutableString string];

  [csv appendString:@"Integer1,Integer2,String1,String2\n"];
  [csv appendString:@"1,2,A,B\n"];
  [csv appendString:@"2,2,B,B\n"];
  [csv appendString:@"2,1,B,A\n"];
  [csv appendString:@"1,1,A,A\n"];
  [[csv dataUsingEncoding:NSShiftJISStringEncoding]
      writeToFile:@"UNORDERED2.csv"
       atomically:YES];

  csv = [NSMutableString string];

  [csv appendString:@"Id,Name\n"];
  [csv appendString:@"1,Parent1\n"];
  [csv appendString:@"2,Parent2\n"];
  [csv appendString:@"3,Parent3\n"];
  [[csv dataUsingEncoding:NSShiftJISStringEncoding] writeToFile:@"PARENTS.csv"
                                                     atomically:YES];

  csv = [NSMutableString string];

  [csv appendString:@"Id,Name,ParentId\n"];
  [csv appendString:@"1,Child1,1\n"];
  [csv appendString:@"2,Child2,1\n"];
  [csv appendString:@"3,Child3,2\n"];
  [csv appendString:@"4,Child4,2\n"];
  [csv appendString:@"5,Child5,3\n"];
  [csv appendString:@"6,Child6,3\n"];
  [csv appendString:@"7,Child7,3\n"];
  [[csv dataUsingEncoding:NSShiftJISStringEncoding] writeToFile:@"CHILDREN.csv"
                                                     atomically:YES];

  csv = [NSMutableString string];

  [csv appendString:@"Integer\n"];
  [csv appendString:@"-1\n"];
  [csv appendString:@"-2\n"];
  [csv appendString:@"-3\n"];
  [csv appendString:@"-4\n"];
  [csv appendString:@"-5\n"];
  [csv appendString:@"-6\n"];
  [[csv dataUsingEncoding:NSShiftJISStringEncoding] writeToFile:@"MINUS.csv"
                                                     atomically:YES];
}

- (void)testtearDown {
  [super tearDown];
}

- (void)testExecuteSQLは単純なSQLを実行できます {
  char *sql = "SELECT * "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "1,A"
                           "\n"
                           "2,B"
                           "\n"
                           "3,C"
                           "\n";
  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}
- (void)testExecuteSQLは識別子名に数字を利用できます {
  char *sql = "SELECT * "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
}

- (void)testExecuteSQLは識別子名に数字で始まる単語は利用できません {
  char *sql = "SELECT * "
              "FROM 1TABLE";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_TOKEN_CANT_READ, result);
}

- (void)testExecuteSQLは識別子名の2文字目に数字を利用できます {
  char *sql = "SELECT * "
              "FROM T1ABLE";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_FILE_OPEN, result);
}

- (void)testExecuteSQLは識別子名の先頭にアンダーバーを利用できます {
  char *sql = "SELECT * "
              "FROM _TABLE";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_FILE_OPEN, result);
}

- (void)testExecuteSQLは識別子名の二文字目以降にアンダーバーを利用できます {
  char *sql = "SELECT * "
              "FROM T_ABLE";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_FILE_OPEN, result);
}

- (void)testExecuteSQLは複数個続く区切り文字を利用できます {
  char *sql = "SELECT  *  "
              "FROM  TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "1,A"
                           "\n"
                           "2,B"
                           "\n"
                           "3,C"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLは区切り文字としてスペースを認識します {
  char *sql = "SELECT * "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "1,A"
                           "\n"
                           "2,B"
                           "\n"
                           "3,C"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLは区切り文字としてタブを認識します {
  char *sql = "SELECT\t*\t"
              "FROM\tTABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "1,A"
                           "\n"
                           "2,B"
                           "\n"
                           "3,C"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLは区切り文字として改行を認識します {
  char *sql = "SELECT\n*\n"
              "FROM\rTABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "1,A"
                           "\n"
                           "2,B"
                           "\n"
                           "3,C"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)
    testExecuteSQLは認識できないトークンを含む語を指定したときERR_TOKEN_CANT_READエラーとなります {
  char *sql = "?";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_TOKEN_CANT_READ, result);
}

- (void)
    testExecuteSQLは指定したテーブル名を取得し対応するファイルを参照できます {
  char *sql = "SELECT * "
              "FROM TABLE2";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "4,D"
                           "\n"
                           "5,E"
                           "\n"
                           "6,F"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLは二つののテーブルを読み込み全ての組み合わせを出力します {
  char *sql = "SELECT * "
              "FROM TABLE1, TABLE2";

  NSString *expectedCsv = @"Integer,String,Integer,String"
                           "\n"
                           "1,A,4,D"
                           "\n"
                           "1,A,5,E"
                           "\n"
                           "1,A,6,F"
                           "\n"
                           "2,B,4,D"
                           "\n"
                           "2,B,5,E"
                           "\n"
                           "2,B,6,F"
                           "\n"
                           "3,C,4,D"
                           "\n"
                           "3,C,5,E"
                           "\n"
                           "3,C,6,F"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)
    testExecuteSQLは三つ以上のテーブルを読み込み全ての組み合わせを出力します {
  char *sql = "SELECT * "
              "FROM TABLE1, TABLE2, TABLE3";

  NSString *expectedCsv = @"Integer,String,Integer,String,Integer,String"
                           "\n"
                           "1,A,4,D,7,G"
                           "\n"
                           "1,A,4,D,8,H"
                           "\n"
                           "1,A,5,E,7,G"
                           "\n"
                           "1,A,5,E,8,H"
                           "\n"
                           "1,A,6,F,7,G"
                           "\n"
                           "1,A,6,F,8,H"
                           "\n"
                           "2,B,4,D,7,G"
                           "\n"
                           "2,B,4,D,8,H"
                           "\n"
                           "2,B,5,E,7,G"
                           "\n"
                           "2,B,5,E,8,H"
                           "\n"
                           "2,B,6,F,7,G"
                           "\n"
                           "2,B,6,F,8,H"
                           "\n"
                           "3,C,4,D,7,G"
                           "\n"
                           "3,C,4,D,8,H"
                           "\n"
                           "3,C,5,E,7,G"
                           "\n"
                           "3,C,5,E,8,H"
                           "\n"
                           "3,C,6,F,7,G"
                           "\n"
                           "3,C,6,F,8,H"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)
    testExecuteSQLはSQLECT句にテーブルと一緒に指定した列名を指定しSQLを実行できます {
  char *sql = "SELECT String "
              "FROM TABLE1";

  NSString *expectedCsv = @"String"
                           "\n"
                           "A"
                           "\n"
                           "B"
                           "\n"
                           "C"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)
    testExecuteSQLはSQLECT句に複数のテーブルと一緒に指定した列名を指定しSQLを実行できます {
  char *sql = "SELECT String,Integer "
              "FROM TABLE1";

  NSString *expectedCsv = @"String,Integer"
                           "\n"
                           "A,1"
                           "\n"
                           "B,2"
                           "\n"
                           "C,3"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)
    testExecuteSQLはSQLECT句に三つ以上のテーブルと一緒に指定した列名を指定しSQLを実行できます {
  char *sql = "SELECT String,Integer,String,Integer "
              "FROM TABLE1";

  NSString *expectedCsv = @"String,Integer,String,Integer"
                           "\n"
                           "A,1,A,1"
                           "\n"
                           "B,2,B,2"
                           "\n"
                           "C,3,C,3"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはSELECTの指定にテーブル名も指定できます {
  char *sql = "SELECT TABLE1.Integer "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer"
                           "\n"
                           "1"
                           "\n"
                           "2"
                           "\n"
                           "3"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)
    testExecuteSQLは複数ののテーブルを読み込みテーブル名で区別してテーブルと一緒に指定した列名を指定することができます {
  char *sql = "SELECT Table1.Integer "
              "FROM TABLE1, TABLE2";

  NSString *expectedCsv = @"Integer"
                           "\n"
                           "1"
                           "\n"
                           "1"
                           "\n"
                           "1"
                           "\n"
                           "2"
                           "\n"
                           "2"
                           "\n"
                           "2"
                           "\n"
                           "3"
                           "\n"
                           "3"
                           "\n"
                           "3"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)
    testExecuteSQLはSELECT句でテーブル名を二つ目以降のテーブルと一緒に指定した列名に指定することができます {
  char *sql = "SELECT Table1.Integer, Table2.String "
              "FROM TABLE1, TABLE2";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "1,D"
                           "\n"
                           "1,E"
                           "\n"
                           "1,F"
                           "\n"
                           "2,D"
                           "\n"
                           "2,E"
                           "\n"
                           "2,F"
                           "\n"
                           "3,D"
                           "\n"
                           "3,E"
                           "\n"
                           "3,F"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)
    testExecuteSQLはSELECTのテーブルと一緒に指定した列名の指定があいまいな場合にERR_BAD_COLUMN_NAMEエラーとなります {
  char *sql = "SELECT Integer "
              "FROM TABLE1, TABLE2";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_BAD_COLUMN_NAME, result);
}

- (void)
    testExecuteSQLはSELECTで指定したテーブルと一緒に指定した列名の指定の一文字目の違いを見分けます {
  char *sql = "SELECT Ttring "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_BAD_COLUMN_NAME, result);
}

- (void)
    testExecuteSQLはSELECTで指定したテーブルと一緒に指定した列名の指定の二文字目の違いを見分けます {
  char *sql = "SELECT Suring "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_BAD_COLUMN_NAME, result);
}

- (void)
    testExecuteSQLはSELECTで指定したテーブルと一緒に指定した列名の指定の最終文字の違いを見分けます {
  char *sql = "SELECT Surinh "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_BAD_COLUMN_NAME, result);
}

- (void)
    testExecuteSQLはSELECTで指定したテーブルと一緒に指定した列名の指定が一文字多いという違いを見分けます {
  char *sql = "SELECT Suringg "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_BAD_COLUMN_NAME, result);
}

- (void)
    testExecuteSQLはSELECTで指定したテーブルと一緒に指定した列名の指定の一文字少ないという違いを見分けます {
  char *sql = "SELECT Surin "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_BAD_COLUMN_NAME, result);
}

- (void)
    testExecuteSQLはSELECTで指定したテーブル名の指定の一文字目の違いを見分けます {
  char *sql = "SELECT UABLE1.Integer "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_BAD_COLUMN_NAME, result);
}

- (void)
    testExecuteSQLはSELECTで指定したテーブル名の指定の二文字目の違いを見分けます {
  char *sql = "SELECT TBBLE1.Integer "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_BAD_COLUMN_NAME, result);
}

- (void)
    testExecuteSQLはSELECTで指定したテーブル名の指定の最終文字の違いを見分けます {
  char *sql = "SELECT TABLE2.Integer "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_BAD_COLUMN_NAME, result);
}

- (void)
    testExecuteSQLはSELECTで指定したテーブル名の指定が一文字多いという違いを見分けます {
  char *sql = "SELECT TABLE1a.Integer "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_BAD_COLUMN_NAME, result);
}

- (void)
    testExecuteSQLはSELECTで指定したテーブル名の指定の一文字少ないという違いを見分けます {
  char *sql = "SELECT TABLE.Integer "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_BAD_COLUMN_NAME, result);
}

- (void)testExecuteSQLはORDER句で文字列を辞書順で並べ替えます {
  char *sql = "SELECT * "
              "ORDER BY String "
              "FROM UNORDERED";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "1,A"
                           "\n"
                           "11,AA"
                           "\n"
                           "12,AB"
                           "\n"
                           "2,B"
                           "\n"
                           "21,BA"
                           "\n"
                           "22,BB"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)
    testExecuteSQLはORDER句にSELECTで指定されなかった列を指定することができます {
  char *sql = "SELECT String "
              "ORDER BY Integer "
              "FROM UNORDERED";

  NSString *expectedCsv = @"String"
                           "\n"
                           "A"
                           "\n"
                           "B"
                           "\n"
                           "AA"
                           "\n"
                           "AB"
                           "\n"
                           "BA"
                           "\n"
                           "BB"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)
    testExecuteSQLはORDER句にSELECTで指定されなかった入力の最後の列を指定することができます {
  char *sql = "SELECT Integer "
              "ORDER BY String "
              "FROM UNORDERED";

  NSString *expectedCsv = @"Integer"
                           "\n"
                           "1"
                           "\n"
                           "11"
                           "\n"
                           "12"
                           "\n"
                           "2"
                           "\n"
                           "21"
                           "\n"
                           "22"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはORDER句で数字列を大小順で並べ替えます {
  char *sql = "SELECT * "
              "ORDER BY Integer "
              "FROM UNORDERED";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "1,A"
                           "\n"
                           "2,B"
                           "\n"
                           "11,AA"
                           "\n"
                           "12,AB"
                           "\n"
                           "21,BA"
                           "\n"
                           "22,BB"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはORDER句でマイナスの数値を扱えます {
  char *sql = "SELECT * "
              "ORDER BY Integer "
              "FROM MINUS";

  NSString *expectedCsv = @"Integer"
                           "\n"
                           "-6"
                           "\n"
                           "-5"
                           "\n"
                           "-4"
                           "\n"
                           "-3"
                           "\n"
                           "-2"
                           "\n"
                           "-1"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはORDER句で複数の文字列を条件にして並べ替えます {
  char *sql = "SELECT String1, String2 "
              "ORDER BY String1, String2 "
              "FROM UNORDERED2";

  NSString *expectedCsv = @"String1,String2"
                           "\n"
                           "A,A"
                           "\n"
                           "A,B"
                           "\n"
                           "B,A"
                           "\n"
                           "B,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはORDER句で複数の数値列を条件にして並べ替えます {
  char *sql = "SELECT Integer1, Integer2 "
              "ORDER BY Integer1, Integer2 "
              "FROM UNORDERED2";

  NSString *expectedCsv = @"Integer1,Integer2"
                           "\n"
                           "1,1"
                           "\n"
                           "1,2"
                           "\n"
                           "2,1"
                           "\n"
                           "2,2"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)
    testExecuteSQLはORDER句で複数の条件を指定した場合に先に指定した条件を優先して並べ替えます {
  char *sql = "SELECT String1, String2 "
              "ORDER BY String2, String1 "
              "FROM UNORDERED2";

  NSString *expectedCsv = @"String1,String2"
                           "\n"
                           "A,A"
                           "\n"
                           "B,A"
                           "\n"
                           "A,B"
                           "\n"
                           "B,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはORDER句で昇順を指定できます {
  char *sql = "SELECT String1, String2 "
              "ORDER BY String1 ASC, String2 "
              "FROM UNORDERED2";

  NSString *expectedCsv = @"String1,String2"
                           "\n"
                           "A,A"
                           "\n"
                           "A,B"
                           "\n"
                           "B,A"
                           "\n"
                           "B,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはORDER句で降順を指定できます {
  char *sql = "SELECT String1, String2 "
              "ORDER BY String1 DESC, String2 "
              "FROM UNORDERED2";

  NSString *expectedCsv = @"String1,String2"
                           "\n"
                           "B,A"
                           "\n"
                           "B,B"
                           "\n"
                           "A,A"
                           "\n"
                           "A,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはORDER句で二つ目以降の項目に昇順を指定できます {
  char *sql = "SELECT String1, String2 "
              "ORDER BY String1 , String2 ASC "
              "FROM UNORDERED2";

  NSString *expectedCsv = @"String1,String2"
                           "\n"
                           "A,A"
                           "\n"
                           "A,B"
                           "\n"
                           "B,A"
                           "\n"
                           "B,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはORDER句で二つ目以降の項目に降順を指定できます {
  char *sql = "SELECT String1, String2 "
              "ORDER BY String1, String2 DESC "
              "FROM UNORDERED2";

  NSString *expectedCsv = @"String1,String2"
                           "\n"
                           "A,B"
                           "\n"
                           "A,A"
                           "\n"
                           "B,B"
                           "\n"
                           "B,A"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)
    testExecuteSQLはORDER句にテーブル名付のテーブルと一緒に指定した列名を指定することができます {
  char *sql = "SELECT String "
              "ORDER BY UNORDERED.String "
              "FROM UNORDERED";

  NSString *expectedCsv = @"String"
                           "\n"
                           "A"
                           "\n"
                           "AA"
                           "\n"
                           "AB"
                           "\n"
                           "B"
                           "\n"
                           "BA"
                           "\n"
                           "BB"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)
    testExecuteSQLはORDER句にテーブル名付のテーブルと一緒に指定した列名を指定しテーブルを選択することができます {
  char *sql = "SELECT *"
              "ORDER BY Table2.String "
              "FROM TABLE1, TABLE2";

  NSString *expectedCsv = @"Integer,String,Integer,String"
                           "\n"
                           "1,A,4,D"
                           "\n"
                           "2,B,4,D"
                           "\n"
                           "3,C,4,D"
                           "\n"
                           "1,A,5,E"
                           "\n"
                           "2,B,5,E"
                           "\n"
                           "3,C,5,E"
                           "\n"
                           "1,A,6,F"
                           "\n"
                           "2,B,6,F"
                           "\n"
                           "3,C,6,F"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)
    testExecuteSQLはORDERBY指定したテーブルと一緒に指定した列名の指定の一文字目の違いを見分けます {
  char *sql = "SELECT * "
              "ORDER BY Ttring "
              "FROM UNORDERED";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_BAD_COLUMN_NAME, result);
}

- (void)
    testExecuteSQLはORDERBY指定したテーブルと一緒に指定した列名の指定の二文字目の違いを見分けます {
  char *sql = "SELECT * "
              "ORDER BY Suring "
              "FROM UNORDERED";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_BAD_COLUMN_NAME, result);
}

- (void)
    testExecuteSQLはORDERBY指定したテーブルと一緒に指定した列名の指定の最終文字の違いを見分けます {
  char *sql = "SELECT * "
              "ORDER BY Strinh "
              "FROM UNORDERED";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_BAD_COLUMN_NAME, result);
}

- (void)
    testExecuteSQLはORDERBY指定したテーブルと一緒に指定した列名の指定が一文字多いという違いを見分けます {
  char *sql = "SELECT * "
              "ORDER BY Stringg "
              "FROM UNORDERED";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_BAD_COLUMN_NAME, result);
}

- (void)
    testExecuteSQLはORDERBY指定したテーブルと一緒に指定した列名の指定の一文字少ないという違いを見分けます {
  char *sql = "SELECT * "
              "ORDER BY Strin "
              "FROM UNORDERED";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_BAD_COLUMN_NAME, result);
}

- (void)
    testExecuteSQLはORDERBY指定したテーブル名の指定の一文字目の違いを見分けます {
  char *sql = "SELECT * "
              "ORDER BY VNORDERED.String "
              "FROM UNORDERED";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_BAD_COLUMN_NAME, result);
}

- (void)
    testExecuteSQLはORDERBY指定したテーブル名の指定の二文字目の違いを見分けます {
  char *sql = "SELECT * "
              "ORDER BY UMORDERED.String "
              "FROM UNORDERED";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_BAD_COLUMN_NAME, result);
}

- (void)
    testExecuteSQLはORDERBY指定したテーブル名の指定の最終文字の違いを見分けます {
  char *sql = "SELECT * "
              "ORDER BY UNORDEREE.String "
              "FROM UNORDERED";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_BAD_COLUMN_NAME, result);
}

- (void)
    testExecuteSQLはORDERBY指定したテーブル名の指定が一文字多いという違いを見分けます {
  char *sql = "SELECT * "
              "ORDER BY UNORDEREDD.String "
              "FROM UNORDERED";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_BAD_COLUMN_NAME, result);
}

- (void)
    testExecuteSQLはORDERBY指定したテーブル名の指定の一文字少ないという違いを見分けます {
  char *sql = "SELECT * "
              "ORDER BY UNORDERE.String "
              "FROM UNORDERED";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_BAD_COLUMN_NAME, result);
}

- (void)
    testExecuteSQLはORDERBYで曖昧なテーブルと一緒に指定した列名を指定した場合にERR_BAD_COLUMN_NAMEエラーとなります {
  char *sql = "SELECT * "
              "ORDER BY String "
              "FROM TABLE1, TABLE2";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_BAD_COLUMN_NAME, result);
}

- (void)
    testExecuteSQLはWHERE句で数値列に対する条件として文字列は指定できません {
  char *sql = "SELECT * "
              "WHERE Integer = \'2\' "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_WHERE_OPERAND_TYPE, result);
}

- (void)testExecuteSQLはWHERE句で数値として等しい条件の指定ができます {
  char *sql = "SELECT * "
              "WHERE Integer = 2 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "2,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはWHERE句で数値として等しくない条件の指定ができます {
  char *sql = "SELECT * "
              "WHERE Integer <> 2 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "1,A"
                           "\n"
                           "3,C"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはWHERE句で数値として大きい条件の指定ができます {
  char *sql = "SELECT * "
              "WHERE Integer > 2 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "3,C"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはWHERE句で数値として小さい条件の指定ができます {
  char *sql = "SELECT * "
              "WHERE Integer < 2 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "1,A"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはWHERE句で数値として以上の条件の指定ができます {
  char *sql = "SELECT * "
              "WHERE Integer >= 2 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "2,B"
                           "\n"
                           "3,C"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはWHERE句で数値として以下の条件の指定ができます {
  char *sql = "SELECT * "
              "WHERE Integer <= 2 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "1,A"
                           "\n"
                           "2,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはWHERE句でマイナスの数値が扱えます {
  char *sql = "SELECT * "
              "WHERE Integer < -3 "
              "FROM MINUS";

  NSString *expectedCsv = @"Integer"
                           "\n"
                           "-4"
                           "\n"
                           "-5"
                           "\n"
                           "-6"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはWHERE句でプラスを明示したの数値が扱えます {
  char *sql = "SELECT * "
              "WHERE Integer <= +2 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "1,A"
                           "\n"
                           "2,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはWHERE句でマイナスを指定したの列名が扱えます {
  char *sql = "SELECT * "
              "WHERE -Integer > 3 "
              "FROM MINUS";

  NSString *expectedCsv = @"Integer"
                           "\n"
                           "-4"
                           "\n"
                           "-5"
                           "\n"
                           "-6"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはWHERE句でプラスを明示した列名が扱えます {
  char *sql = "SELECT * "
              "WHERE +Integer <= 2 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "1,A"
                           "\n"
                           "2,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはWHERE句で文字列にマイナスの指定はできません {
  char *sql = "SELECT * "
              "WHERE String = -\'B\' "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_WHERE_OPERAND_TYPE, result);
}

- (void)testExecuteSQLはWHERE句で文字列にプラスの指定はできません {
  char *sql = "SELECT * "
              "WHERE String = +\'B\' "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_WHERE_OPERAND_TYPE, result);
}

- (void)
    testExecuteSQLはWHERE句でSELECT句で指定していない列の条件の指定ができます {
  char *sql = "SELECT String "
              "WHERE Integer = 2 "
              "FROM TABLE1";

  NSString *expectedCsv = @"String"
                           "\n"
                           "B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)
    testExecuteSQLはWHERE句でSELECT句で指定していない入力の最後の列がテーブル名を指定せずに条件の指定ができます {
  char *sql = "SELECT Integer "
              "WHERE String = \'B\' "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer"
                           "\n"
                           "2"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)
    testExecuteSQLはWHERE句でSELECT句で指定していない入力の最後の列がテーブル名を指定して条件の指定ができます {
  char *sql = "SELECT Integer "
              "WHERE TABLE1.String = \'B\' "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer"
                           "\n"
                           "2"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)
    testExecuteSQLはWHERE句で文字列と数値の等しい条件の比較をした場合にERR_WHERE_OPERAND_TYPEエラーとなります {
  char *sql = "SELECT * "
              "WHERE String = 2 "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_WHERE_OPERAND_TYPE, result);
}

- (void)
    testExecuteSQLはWHERE句で文字列と数値の等しくない条件の比較をした場合にERR_WHERE_OPERAND_TYPEエラーとなります {
  char *sql = "SELECT * "
              "WHERE String <> 2 "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_WHERE_OPERAND_TYPE, result);
}

- (void)
    testExecuteSQLはWHERE句で文字列と数値の小さい条件の比較をした場合にERR_WHERE_OPERAND_TYPEエラーとなります {
  char *sql = "SELECT * "
              "WHERE String < 2 "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_WHERE_OPERAND_TYPE, result);
}

- (void)
    testExecuteSQLはWHERE句で文字列と数値の以下条件の比較をした場合にERR_WHERE_OPERAND_TYPEエラーとなります {
  char *sql = "SELECT * "
              "WHERE String <= 2 "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_WHERE_OPERAND_TYPE, result);
}

- (void)
    testExecuteSQLはWHERE句で文字列と数値の大きい条件の比較をした場合にERR_WHERE_OPERAND_TYPEエラーとなります {
  char *sql = "SELECT * "
              "WHERE String > 2 "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_WHERE_OPERAND_TYPE, result);
}

- (void)
    testExecuteSQLはWHERE句で文字列と数値の以上条件の比較をした場合にERR_WHERE_OPERAND_TYPEエラーとなります {
  char *sql = "SELECT * "
              "WHERE String >= 2 "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_WHERE_OPERAND_TYPE, result);
}

- (void)
    testExecuteSQLはWHERE句で数値と文字列の等しい条件の比較をした場合にERR_WHERE_OPERAND_TYPEエラーとなります {
  char *sql = "SELECT * "
              "WHERE Integer = \'B\' "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_WHERE_OPERAND_TYPE, result);
}

- (void)
    testExecuteSQLはWHERE句で数値と文字列の等しくない条件の比較をした場合にERR_WHERE_OPERAND_TYPEエラーとなります {
  char *sql = "SELECT * "
              "WHERE Integer <> \'B\' "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_WHERE_OPERAND_TYPE, result);
}

- (void)
    testExecuteSQLはWHERE句で数値と文字列の小さい条件の比較をした場合にERR_WHERE_OPERAND_TYPEエラーとなります {
  char *sql = "SELECT * "
              "WHERE Integer < \'B\' "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_WHERE_OPERAND_TYPE, result);
}

- (void)
    testExecuteSQLはWHERE句で数値と文字列の以下条件の比較をした場合にERR_WHERE_OPERAND_TYPEエラーとなります {
  char *sql = "SELECT * "
              "WHERE Integer <= \'B\' "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_WHERE_OPERAND_TYPE, result);
}

- (void)
    testExecuteSQLはWHERE句で数値と文字列の大きい条件の比較をした場合にERR_WHERE_OPERAND_TYPEエラーとなります {
  char *sql = "SELECT * "
              "WHERE Integer > \'B\' "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_WHERE_OPERAND_TYPE, result);
}

- (void)
    testExecuteSQLはWHERE句で数値と文字列の以上条件の比較をした場合にERR_WHERE_OPERAND_TYPEエラーとなります {
  char *sql = "SELECT * "
              "WHERE Integer >= \'B\' "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_WHERE_OPERAND_TYPE, result);
}

- (void)testExecuteSQLはWHERE句で文字列として等しい条件の指定ができます {
  char *sql = "SELECT * "
              "WHERE String = \'B\' "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "2,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはWHERE句で文字列として等しくない条件の指定ができます {
  char *sql = "SELECT * "
              "WHERE String <> \'B\' "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "1,A"
                           "\n"
                           "3,C"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはWHERE句で文字列として大きい条件の指定ができます {
  char *sql = "SELECT * "
              "WHERE String > \'B\' "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "3,C"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはWHERE句で文字列として小さい条件の指定ができます {
  char *sql = "SELECT * "
              "WHERE String < \'B\' "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "1,A"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはWHERE句で文字列として以上の条件の指定ができます {
  char *sql = "SELECT * "
              "WHERE String >= \'B\' "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "2,B"
                           "\n"
                           "3,C"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはWHERE句で文字列として以下の条件の指定ができます {
  char *sql = "SELECT * "
              "WHERE String <= \'B\' "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "1,A"
                           "\n"
                           "2,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)
    testExecuteSQLはWHERE指定したテーブルと一緒に指定した列名の指定の一文字目の違いを見分けます {
  char *sql = "SELECT * "
              "WHERE Ttring = \'A\' "
              "FROM UNORDERED";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_BAD_COLUMN_NAME, result);
}

- (void)
    testExecuteSQLはWHERE指定したテーブルと一緒に指定した列名の指定の二文字目の違いを見分けます {
  char *sql = "SELECT * "
              "WHERE Suring = \'A\' "
              "FROM UNORDERED";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_BAD_COLUMN_NAME, result);
}

- (void)
    testExecuteSQLはWHERE指定したテーブルと一緒に指定した列名の指定の最終文字の違いを見分けます {
  char *sql = "SELECT * "
              "WHERE Strinh  = \'A\'"
              "FROM UNORDERED";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_BAD_COLUMN_NAME, result);
}

- (void)
    testExecuteSQLはWHERE指定したテーブルと一緒に指定した列名の指定が一文字多いという違いを見分けます {
  char *sql = "SELECT * "
              "WHERE Stringg  = \'A\'"
              "FROM UNORDERED";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_BAD_COLUMN_NAME, result);
}

- (void)
    testExecuteSQLはWHERE指定したテーブルと一緒に指定した列名の指定の一文字少ないという違いを見分けます {
  char *sql = "SELECT * "
              "WHERE Strin  = \'A\'"
              "FROM UNORDERED";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_BAD_COLUMN_NAME, result);
}

- (void)
    testExecuteSQLはWHERE句で比較のテーブルと一緒に指定した列名を右辺に持ってくることができます {
  char *sql = "SELECT * "
              "WHERE 2 = Integer "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "2,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはWHERE句で加算演算子が使えます {
  char *sql = "SELECT * "
              "WHERE Integer = 1 + 2 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "3,C"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)
    testExecuteSQLはWHERE句の加算演算子の左辺が数値でない場合はERR_WHERE_OPERAND_TYPEエラーとなります {
  char *sql = "SELECT * "
              "WHERE Integer = \'A\' + 2 "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_WHERE_OPERAND_TYPE, result);
}

- (void)
    testExecuteSQLはWHERE句の加算演算子の右辺が数値でない場合はERR_WHERE_OPERAND_TYPEエラーとなります {
  char *sql = "SELECT * "
              "WHERE Integer = 1 + \'B\' "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_WHERE_OPERAND_TYPE, result);
}

- (void)testExecuteSQLはWHERE句で減算演算子が使えます {
  char *sql = "SELECT * "
              "WHERE Integer = 3 - 1 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "2,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)
    testExecuteSQLはWHERE句の減算演算子の左辺が数値でない場合はERR_WHERE_OPERAND_TYPEエラーとなります {
  char *sql = "SELECT * "
              "WHERE Integer = \'A\' - 2 "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_WHERE_OPERAND_TYPE, result);
}

- (void)
    testExecuteSQLはWHERE句の減算演算子の右辺が数値でない場合はERR_WHERE_OPERAND_TYPEエラーとなります {
  char *sql = "SELECT * "
              "WHERE Integer = 1 - \'B\' "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_WHERE_OPERAND_TYPE, result);
}

- (void)testExecuteSQLはWHERE句で乗算演算子が使えます {
  char *sql = "SELECT * "
              "WHERE Integer = 1 * 2 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "2,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)
    testExecuteSQLはWHERE句の乗算演算子の左辺が数値でない場合はERR_WHERE_OPERAND_TYPEエラーとなります {
  char *sql = "SELECT * "
              "WHERE Integer = \'A\' * 2 "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_WHERE_OPERAND_TYPE, result);
}

- (void)
    testExecuteSQLはWHERE句の乗算演算子の右辺が数値でない場合はERR_WHERE_OPERAND_TYPEエラーとなります {
  char *sql = "SELECT * "
              "WHERE Integer = 1 * \'B\' "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_WHERE_OPERAND_TYPE, result);
}

- (void)testExecuteSQLはWHERE句で除算演算子が使えます {
  char *sql = "SELECT * "
              "WHERE Integer = 5 / 2 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "2,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)
    testExecuteSQLはWHERE句の除算演算子の左辺が数値でない場合はERR_WHERE_OPERAND_TYPEエラーとなります {
  char *sql = "SELECT * "
              "WHERE Integer = \'A\' / 2 "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_WHERE_OPERAND_TYPE, result);
}

- (void)
    testExecuteSQLはWHERE句の除算演算子の右辺が数値でない場合はERR_WHERE_OPERAND_TYPEエラーとなります {
  char *sql = "SELECT * "
              "WHERE Integer = 1 / \'B\' "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_WHERE_OPERAND_TYPE, result);
}

- (void)testExecuteSQLはWHERE句でAND演算子が使えます {
  char *sql = "SELECT * "
              "WHERE 1 < Integer AND Integer < 3 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "2,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)
    testExecuteSQLはWHERE句のAND演算子の左辺が真偽値でない場合はERR_WHERE_OPERAND_TYPEエラーとなります {
  char *sql = "SELECT * "
              "WHERE 2 AND Integer = 2 "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_WHERE_OPERAND_TYPE, result);
}

- (void)
    testExecuteSQLはWHERE句のAND演算子の右辺が真偽値でない場合はERR_WHERE_OPERAND_TYPEエラーとなります {
  char *sql = "SELECT * "
              "WHERE Integer = 1 AND 2 "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_WHERE_OPERAND_TYPE, result);
}

- (void)testExecuteSQLはWHERE句でOR演算子が使えます {
  char *sql = "SELECT * "
              "WHERE Integer < 2 OR 2 < Integer "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "1,A"
                           "\n"
                           "3,C"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)
    testExecuteSQLはWHERE句のOR演算子の左辺が真偽値でない場合はERR_WHERE_OPERAND_TYPEエラーとなります {
  char *sql = "SELECT * "
              "WHERE 2 OR Integer = 2 "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_WHERE_OPERAND_TYPE, result);
}

- (void)
    testExecuteSQLはWHERE句のOR演算子の右辺が真偽値でない場合はERR_WHERE_OPERAND_TYPEエラーとなります {
  char *sql = "SELECT * "
              "WHERE Integer = 1 OR 2 "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_WHERE_OPERAND_TYPE, result);
}

- (void)testExecuteSQLはWHERE句で演算子の優先順位が考慮されます {
  char *sql = "SELECT * "
              "WHERE Integer = 2 * 1 + 1 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "3,C"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)
    testExecuteSQLはWHERE句で加算演算子は減算演算子より強くはない優先順位です {
  char *sql = "SELECT * "
              "WHERE Integer = 2 - 1 + 1 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "2,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはWHERE句で乗算演算子は減算演算子より強い優先順位です {
  char *sql = "SELECT * "
              "WHERE Integer = 8 - 3 * 2 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "2,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはWHERE句で乗算演算子は加算演算子より強い優先順位です {
  char *sql = "SELECT * "
              "WHERE Integer = 1 + 1 * 2 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "3,C"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはWHERE句で乗算演算子は除算演算子と同じ優先順位です {
  char *sql = "SELECT * "
              "WHERE Integer = 2 * 5 / 3 * 2 - 4 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "2,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはWHERE句で等しい演算子は加算演算子より弱い優先順位です {
  char *sql = "SELECT * "
              "WHERE Integer = 1 + 1 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "2,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)
    testExecuteSQLはWHERE句で等しくない演算子は加算演算子より弱い優先順位です {
  char *sql = "SELECT * "
              "WHERE Integer <> 1 + 1 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "1,A"
                           "\n"
                           "3,C"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはWHERE句で大きい演算子は加算演算子より弱い優先順位です {
  char *sql = "SELECT * "
              "WHERE Integer > 1 + 1 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "3,C"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはWHERE句で小さい演算子は加算演算子より弱い優先順位です {
  char *sql = "SELECT * "
              "WHERE Integer < 1 + 1 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "1,A"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはWHERE句で以上演算子は加算演算子より弱い優先順位です {
  char *sql = "SELECT * "
              "WHERE Integer >= 1 + 1 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "2,B"
                           "\n"
                           "3,C"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはWHERE句で以下演算子は加算演算子より弱い優先順位です {
  char *sql = "SELECT * "
              "WHERE Integer <= 1 + 1 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "1,A"
                           "\n"
                           "2,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはWHERE句でAND演算子は比較演算子より弱い優先順位です {
  char *sql = "SELECT * "
              "WHERE 1 < Integer AND Integer < 3 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "2,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはWHERE句でOR演算子はAND演算子より弱い優先順位です {
  char *sql = "SELECT * "
              "WHERE Integer = 1 OR Integer <= 2 AND 2 <= Integer "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "1,A"
                           "\n"
                           "2,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはWHERE句でカッコによる優先順位の指定ができます {
  char *sql = "SELECT * "
              "WHERE Integer = (1 + 2) * 3 - 7 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "2,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはWHERE句でカッコにより左結合を制御することができます {
  char *sql = "SELECT * "
              "WHERE Integer = 1 - (2 - 3) "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "2,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)
    testExecuteSQLはWHERE句でネストしたカッコによる優先順位の指定ができます {
  char *sql = "SELECT * "
              "WHERE Integer = (2 * (2 + 1) + 2) / 3 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "2,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはWHERE句でカッコ内部の演算子の優先順位の指定ができます {
  char *sql = "SELECT * "
              "WHERE Integer = (3 * 2 - 2 * 2) "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "2,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはWHERE句でカッコ開くを連続で記述することができます {
  char *sql = "SELECT * "
              "WHERE Integer = ((3 - 2) * 2) "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "2,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはWHERE句でカッコ閉じるを連続で記述することができます {
  char *sql = "SELECT * "
              "WHERE Integer = (2 * (3 - 2))"
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "2,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはWHERE句でテーブル名の指定ができます {
  char *sql = "SELECT * "
              "WHERE TABLE1.Integer = 2 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "2,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)
    testExecuteSQLはWHERE句にテーブル名付のテーブルと一緒に指定した列名を指定しテーブルを選択することができます {
  char *sql = "SELECT *"
              "WHERE Table2.Integer = 5 "
              "FROM TABLE1, TABLE2";

  NSString *expectedCsv = @"Integer,String,Integer,String"
                           "\n"
                           "1,A,5,E"
                           "\n"
                           "2,B,5,E"
                           "\n"
                           "3,C,5,E"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはWHERE句を利用して結合を行うことができます {
  char *sql = "SELECT PARENTS.Name, CHILDREN.Name "
              "WHERE PARENTS.Id = CHILDREN.ParentId "
              "FROM PARENTS, CHILDREN";

  NSString *expectedCsv = @"Name,Name"
                           "\n"
                           "Parent1,Child1"
                           "\n"
                           "Parent1,Child2"
                           "\n"
                           "Parent2,Child3"
                           "\n"
                           "Parent2,Child4"
                           "\n"
                           "Parent3,Child5"
                           "\n"
                           "Parent3,Child6"
                           "\n"
                           "Parent3,Child7"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)
    testExecuteSQLはWHERE句のテーブルと一緒に指定した列名の指定があいまいな場合にERR_BAD_COLUMN_NAMEエラーとなります {
  char *sql = "SELECT * "
              "WHERE Integer = 2 "
              "FROM TABLE1, TABLE2";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_BAD_COLUMN_NAME, result);
}

- (void)
    testExecuteSQLはWHERE句で指定したテーブルと一緒に指定した列名の指定の一文字目の違いを見分けます {
  char *sql = "SELECT * "
              "WHERE TABLE1.Jnteger = 2 "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_BAD_COLUMN_NAME, result);
}

- (void)
    testExecuteSQLはWHERE句で指定したテーブルと一緒に指定した列名の指定の二文字目の違いを見分けます {
  char *sql = "SELECT * "
              "WHERE TABLE1.Ioteger = 2 "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_BAD_COLUMN_NAME, result);
}

- (void)
    testExecuteSQLはWHERE句で指定したテーブルと一緒に指定した列名の指定の最終文字の違いを見分けます {
  char *sql = "SELECT * "
              "WHERE TABLE1.Integes = 2 "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_BAD_COLUMN_NAME, result);
}

- (void)
    testExecuteSQLはWHERE句で指定したテーブルと一緒に指定した列名の指定が一文字多いという違いを見分けます {
  char *sql = "SELECT * "
              "WHERE TABLE1.Integerr = 2 "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_BAD_COLUMN_NAME, result);
}

- (void)
    testExecuteSQLはWHERE句で指定したテーブルと一緒に指定した列名の指定の一文字少ないという違いを見分けます {
  char *sql = "SELECT * "
              "WHERE TABLE.Intege = 2 "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_BAD_COLUMN_NAME, result);
}

- (void)
    testExecuteSQLはWHERE句で指定したテーブル名の指定の一文字目の違いを見分けます {
  char *sql = "SELECT * "
              "WHERE UABLE1.Integer = 2 "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_BAD_COLUMN_NAME, result);
}

- (void)
    testExecuteSQLはWHERE句で指定したテーブル名の指定の二文字目の違いを見分けます {
  char *sql = "SELECT * "
              "WHERE TBBLE1.Integer = 2 "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_BAD_COLUMN_NAME, result);
}

- (void)
    testExecuteSQLはWHERE句で指定したテーブル名の指定の最終文字の違いを見分けます {
  char *sql = "SELECT * "
              "WHERE TABLE2.Integer = 2 "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_BAD_COLUMN_NAME, result);
}

- (void)
    testExecuteSQLはWHERE句で指定したテーブル名の指定が一文字多いという違いを見分けます {
  char *sql = "SELECT * "
              "WHERE TABLE1a.Integer = 2 "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_BAD_COLUMN_NAME, result);
}

- (void)
    testExecuteSQLはWHERE句で指定したテーブル名の指定の一文字少ないという違いを見分けます {
  char *sql = "SELECT * "
              "WHERE TABLE.Integer = 2 "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_BAD_COLUMN_NAME, result);
}

- (void)testExecuteSQLはWHERE句の後にORDER句を記述することができます {
  char *sql = "SELECT * "
              "WHERE Integer <> 2 "
              "ORDER BY Integer DESC "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "3,C"
                           "\n"
                           "1,A"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはORDER句の後にWHERE句を記述することができます {
  char *sql = "SELECT * "
              "ORDER BY Integer DESC "
              "WHERE Integer <> 2 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "3,C"
                           "\n"
                           "1,A"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)
    testExecuteSQLはFROM句の後にSQLが続いたらERR_SQL_SYNTAXエラーとなります {
  char *sql = "SELECT * "
              "FROM TABLE1 *";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_SQL_SYNTAX, result);
}

- (void)testExecuteSQLはWHERE句を二度記述するとERR_SQL_SYNTAXエラーとなります {
  char *sql = "SELECT * "
              "WHERE Integer <> 2 "
              "WHERE Integer <> 2 "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_SQL_SYNTAX, result);
}

- (void)testExecuteSQLはORDER句を二度記述するとERR_SQL_SYNTAXエラーとなります {
  char *sql = "SELECT * "
              "ORDER BY Integer DESC "
              "ORDER BY Integer DESC "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_SQL_SYNTAX, result);
}

- (void)testExecuteSQLはSELECTキーワードを大文字でも小文字でも識別します {
  char *sql = "select * "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "1,A"
                           "\n"
                           "2,B"
                           "\n"
                           "3,C"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはFROMキーワードを大文字でも小文字でも識別します {
  char *sql = "SELECT * "
              "from TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "1,A"
                           "\n"
                           "2,B"
                           "\n"
                           "3,C"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはORDERキーワードを大文字でも小文字でも識別します {
  char *sql = "SELECT * "
              "order BY String "
              "FROM UNORDERED";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "1,A"
                           "\n"
                           "11,AA"
                           "\n"
                           "12,AB"
                           "\n"
                           "2,B"
                           "\n"
                           "21,BA"
                           "\n"
                           "22,BB"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはBYキーワードを大文字でも小文字でも識別します {
  char *sql = "SELECT * "
              "ORDER by String "
              "FROM UNORDERED";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "1,A"
                           "\n"
                           "11,AA"
                           "\n"
                           "12,AB"
                           "\n"
                           "2,B"
                           "\n"
                           "21,BA"
                           "\n"
                           "22,BB"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはASCキーワードを大文字でも小文字でも識別します {
  char *sql = "SELECT String1, String2 "
              "ORDER BY String1 asc, String2 "
              "FROM UNORDERED2";

  NSString *expectedCsv = @"String1,String2"
                           "\n"
                           "A,A"
                           "\n"
                           "A,B"
                           "\n"
                           "B,A"
                           "\n"
                           "B,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはDESCキーワードを大文字でも小文字でも識別します {
  char *sql = "SELECT String1, String2 "
              "ORDER BY String1 desc, String2 "
              "FROM UNORDERED2";

  NSString *expectedCsv = @"String1,String2"
                           "\n"
                           "B,A"
                           "\n"
                           "B,B"
                           "\n"
                           "A,A"
                           "\n"
                           "A,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはWHEREキーワードを大文字でも小文字でも識別します {
  char *sql = "SELECT * "
              "where Integer = 2 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "2,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはANDキーワードを大文字でも小文字でも識別します {
  char *sql = "SELECT * "
              "WHERE 1 < Integer and Integer < 3 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "2,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}
- (void)testExecuteSQLはORキーワードを大文字でも小文字でも識別します {
  char *sql = "SELECT * "
              "WHERE Integer < 2 or 2 < Integer "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "1,A"
                           "\n"
                           "3,C"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはFrom句のテーブル名を大文字でも小文字でも識別します {
  char *sql = "SELECT * "
              "FROM table1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "1,A"
                           "\n"
                           "2,B"
                           "\n"
                           "3,C"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)
    testExecuteSQLはSELECT句のテーブルと一緒に指定した列名を大文字でも小文字でも識別します {
  char *sql = "SELECT sTRING "
              "FROM table1";

  NSString *expectedCsv = @"String"
                           "\n"
                           "A"
                           "\n"
                           "B"
                           "\n"
                           "C"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはSELECT句のテーブル名を大文字でも小文字でも識別します {
  char *sql = "SELECT table1.String "
              "FROM TABLE1";

  NSString *expectedCsv = @"String"
                           "\n"
                           "A"
                           "\n"
                           "B"
                           "\n"
                           "C"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)
    testExecuteSQLはORDER句のテーブルと一緒に指定した列名を大文字でも小文字でも識別しじます {
  char *sql = "SELECT * "
              "ORDER BY sTRING "
              "FROM UNORDERED";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "1,A"
                           "\n"
                           "11,AA"
                           "\n"
                           "12,AB"
                           "\n"
                           "2,B"
                           "\n"
                           "21,BA"
                           "\n"
                           "22,BB"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはORDER句のテーブル名を大文字でも小文字でも識別しじます {
  char *sql = "SELECT * "
              "ORDER BY unordered.String "
              "FROM UNORDERED";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "1,A"
                           "\n"
                           "11,AA"
                           "\n"
                           "12,AB"
                           "\n"
                           "2,B"
                           "\n"
                           "21,BA"
                           "\n"
                           "22,BB"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLは先頭がSELECTではなかった場合にERR_SQL_SYNTAXを返します {
  char *sql = "a SELECT * "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_SQL_SYNTAX, result);
}

- (void)
    testExecuteSQLはSELECTの次の語が識別子でもアスタリスクでもなかった場合にERR_SQL_SYNTAXを返します {
  char *sql = "SELECT "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_SQL_SYNTAX, result);
}

- (void)
    testExecuteSQLはSELECT句のカンマの後が識別子でもアスタリスクでもなかった場合にERR_SQL_SYNTAXを返します {
  char *sql = "SELECT String, "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_SQL_SYNTAX, result);
}

- (void)
    testExecuteSQLはSELECT句のドットの後にテーブルと一緒に指定した列名の記述がなかった場合にERR_SQL_SYNTAXを返します {
  char *sql = "SELECT TABLE1. "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_SQL_SYNTAX, result);
}

- (void)
    testExecuteSQLはSELECT句のドットの前にテーブル名の記述がなかった場合にERR_SQL_SYNTAXを返します {
  char *sql = "SELECT .String "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_SQL_SYNTAX, result);
}

- (void)testExecuteSQLはORDERの後がBYでなかった場合にERR_SQL_SYNTAXを返します {
  char *sql = "SELECT * "
              "ORDER b String"
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_SQL_SYNTAX, result);
}

- (void)testExecuteSQLはBYの後が識別子でなかった場合にERR_SQL_SYNTAXを返します {
  char *sql = "SELECT * "
              "ORDER BY BY"
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_SQL_SYNTAX, result);
}

- (void)
    testExecuteSQLはORDER句のドットの後にテーブルと一緒に指定した列名の記述がなかった場合にERR_SQL_SYNTAXを返します {
  char *sql = "SELECT * "
              "ORDER BY TABLE1. "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_SQL_SYNTAX, result);
}

- (void)
    testExecuteSQLはORDER句のドットの前にテーブル名の記述がなかった場合にERR_SQL_SYNTAXを返します {
  char *sql = "SELECT * "
              "ORDER BY .String "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_SQL_SYNTAX, result);
}

- (void)
    testExecuteSQLはORDER句のカンマの後がの識別子でなかった場合にERR_SQL_SYNTAXを返します {
  char *sql = "SELECT * "
              "ORDER BY String, "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_SQL_SYNTAX, result);
}

- (void)
    testExecuteSQLはWHEREの後が識別子でもリテラルでもなかった場合にERR_SQL_SYNTAXを返します {
  char *sql = "SELECT * "
              "WHERE * = 2 "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_SQL_SYNTAX, result);
}

- (void)
    testExecuteSQLはWHERE句のドットの後にテーブルと一緒に指定した列名の記述がなかった場合にERR_SQL_SYNTAXを返します {
  char *sql = "SELECT * "
              "WHERE TABLE1. = 2 "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_SQL_SYNTAX, result);
}

- (void)
    testExecuteSQLはWHERE句のドットの前にテーブル名の記述がなかった場合にERR_SQL_SYNTAXを返します {
  char *sql = "SELECT * "
              "WHERE .Integer = 2 "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_SQL_SYNTAX, result);
}

- (void)
    testExecuteSQLはWHERE句の左辺の後が演算子ではなかった場合にERR_SQL_SYNTAXを返します {
  char *sql = "SELECT * "
              "WHERE Integer WHERE 2 "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_SQL_SYNTAX, result);
}

- (void)
    testExecuteSQLはWHERE句の演算子の後が識別子でもリテラルでもなかった場合にERR_SQL_SYNTAXを返します {
  char *sql = "SELECT * "
              "WHERE Integer = "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_SQL_SYNTAX, result);
}

- (void)testExecuteSQLはFROM句がなかった場合にERR_SQL_SYNTAXを返します {
  char *sql = "SELECT * "
              "TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_SQL_SYNTAX, result);
}

- (void)
    testExecuteSQLはFROMの後に識別子がなかった場合にERR_SQL_SYNTAXを返します {
  char *sql = "SELECT * "
              "FROM *";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_SQL_SYNTAX, result);
}

- (void)testExecuteSQLはSELECTの後にスペースがなくても問題なく動きます {
  char *sql = "SELECT* "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "1,A"
                           "\n"
                           "2,B"
                           "\n"
                           "3,C"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはドットの後にスペースがあっても問題なく動きます {
  char *sql = "SELECT TABLE1. Integer "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer"
                           "\n"
                           "1"
                           "\n"
                           "2"
                           "\n"
                           "3"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはドットの後にスペースがなくても問題なく動きます {
  char *sql = "SELECT TABLE1.Integer "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer"
                           "\n"
                           "1"
                           "\n"
                           "2"
                           "\n"
                           "3"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはカンマの後にスペースがあっても問題なく動きます {
  char *sql = "SELECT Integer, String "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "1,A"
                           "\n"
                           "2,B"
                           "\n"
                           "3,C"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはカンマの後にスペースがなくても問題なく動きます {
  char *sql = "SELECT Integer,String "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "1,A"
                           "\n"
                           "2,B"
                           "\n"
                           "3,C"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)
    testExecuteSQLはドットの後にスペースを挟まずに文字が続くとキーワードとして読み込まれません {
  char *sql = "SELECTSTRING "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_SQL_SYNTAX, result);
}

- (void)testExecuteSQLはアスタリスクの後にスペースがなくても問題なく動きます {
  char *sql = "SELECT *"
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "1,A"
                           "\n"
                           "2,B"
                           "\n"
                           "3,C"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)
    testExecuteSQLはORDERの後にスペースを挟まずに文字が続くとキーワードとして読み込まれません {
  char *sql = "SELECT * "
              "ORDERBY Integer "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_SQL_SYNTAX, result);
}

- (void)
    testExecuteSQLはBYの後にスペースを挟まずに文字が続くとキーワードとして読み込まれません {
  char *sql = "SELECT * "
              "ORDER BYInteger "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_SQL_SYNTAX, result);
}

- (void)
    testExecuteSQLはASCの後にスペースを挟まずに文字が続くとキーワードとして読み込まれません {
  char *sql = "SELECT * "
              "ORDER BY Integer ASC"
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_SQL_SYNTAX, result);
}

- (void)
    testExecuteSQLはDESCの後にスペースを挟まずに文字が続くとキーワードとして読み込まれません {
  char *sql = "SELECT * "
              "ORDER BY Integer DESC"
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_SQL_SYNTAX, result);
}

- (void)testExecuteSQLはWHEREの後にスペースがなくても問題なく動きます {
  char *sql = "SELECT *"
              "WHERE\'B\' = String "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "2,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)
    testExecuteSQLはWHEREの後に文字が続くと整数リテラルとして読み込まれません {
  char *sql = "SELECT *"
              "WHEREInteger = 2"
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_TOKEN_CANT_READ, result);
}

- (void)testExecuteSQLは識別子の後にスペースがなくても問題なく動きます {
  char *sql = "SELECT *"
              "WHERE Integer= 2 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "2,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)
    testExecuteSQLは整数リテラルの後に文字が続くと整数リテラルとして読み込まれません {
  char *sql = "SELECT *"
              "WHERE Integer = 2"
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_TOKEN_CANT_READ, result);
}

- (void)testExecuteSQLは文字列リテラルの後にスペースがなくても問題なく動きます {
  char *sql = "SELECT *"
              "WHERE String = \'B\'"
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "2,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLは等しい記号の後にスペースがなくても問題なく動きます {
  char *sql = "SELECT *"
              "WHERE Integer =2 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "2,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLは等しくない記号の後にスペースがなくても問題なく動きます {
  char *sql = "SELECT * "
              "WHERE Integer <>2 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "1,A"
                           "\n"
                           "3,C"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLは大なり記号の後にスペースがなくても問題なく動きます {
  char *sql = "SELECT * "
              "WHERE Integer >2 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "3,C"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLは小なり記号の後にスペースがなくても問題なく動きます {
  char *sql = "SELECT * "
              "WHERE Integer <2 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "1,A"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLは以上記号の後にスペースがなくても問題なく動きます {
  char *sql = "SELECT * "
              "WHERE Integer >=2 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "2,B"
                           "\n"
                           "3,C"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLは以下記号の後にスペースがなくても問題なく動きます {
  char *sql = "SELECT * "
              "WHERE Integer <=2 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "1,A"
                           "\n"
                           "2,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLは加算記号の後にスペースがなくても問題なく動きます {
  char *sql = "SELECT *"
              "WHERE Integer +1 = 3 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "2,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLは減算記号の後にスペースがなくても問題なく動きます {
  char *sql = "SELECT *"
              "WHERE Integer = 3 -1 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "2,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLは乗算記号の後にスペースがなくても問題なく動きます {
  char *sql = "SELECT *"
              "WHERE Integer = 2 *1 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "2,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLは除算記号の後にスペースがなくても問題なく動きます {
  char *sql = "SELECT *"
              "WHERE Integer = 2 /1 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "2,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはAND演算子の後にスペースがなくても問題なく動きます {
  char *sql = "SELECT *"
              "WHERE Integer < 3 AND\'A\' < String "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "2,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}
- (void)
    testExecuteSQLはAND演算子の後にスペースを挟まずに文字が続くとキーワードとして読み込まれません {
  char *sql = "SELECT *"
              "WHERE Integer < 3 ANDInteger > 1 "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_SQL_SYNTAX, result);
}

- (void)testExecuteSQLはOR演算子の後にスペースがなくても問題なく動きます {
  char *sql = "SELECT *"
              "WHERE Integer >= 3  OR\'A\' >= String "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "1,A"
                           "\n"
                           "3,C"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}
- (void)
    testExecuteSQLはOR演算子の後にスペースを挟まずに文字が続くとキーワードとして読み込まれません {
  char *sql = "SELECT *"
              "WHERE Integer >= 3 ORInteger <= 1 "
              "FROM TABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_SQL_SYNTAX, result);
}

- (void)testExecuteSQLはカッコ開くの後にスペースがなくても問題なく動きます {
  char *sql = "SELECT *"
              "WHERE (Integer = 2) "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "2,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはカッコ開くの後にスペースがあっても問題なく動きます {
  char *sql = "SELECT *"
              "WHERE ( Integer = 2) "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "2,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはカッコ閉じるの後にスペースがなくても問題なく動きます {
  char *sql = "SELECT *"
              "WHERE Integer = (2 - 1)* 2 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "2,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)testExecuteSQLはカッコ閉じるの後にスペースがあっても問題なく動きます {
  char *sql = "SELECT *"
              "WHERE Integer = (2 - 1) * 2 "
              "FROM TABLE1";

  NSString *expectedCsv = @"Integer,String"
                           "\n"
                           "2,B"
                           "\n";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(OK, result);
  XCTAssertEqualObjects(expectedCsv, [self readOutput]);
}

- (void)
    testExecuteSQLはFROMの後にスペースを挟まずに文字が続くとキーワードとして読み込まれません {
  char *sql = "SELECT *"
              "FROMTABLE1";

  int result = ExecuteSQL(sql, OUTPUT_PATH);

  XCTAssertEqual(ERR_SQL_SYNTAX, result);
}

@end
