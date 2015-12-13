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

//! �J�����g�f�B���N�g���ɂ���CSV�ɑ΂��A�ȈՓI��SQL�����s���A���ʂ��t�@�C���ɏo�͂��܂��B
//! @param [in] sql ���s����SQL�ł��B
//! @param[in] outputFileName SQL�̎��s���ʂ�CSV�Ƃ��ďo�͂���t�@�C�����ł��B�g���q���܂݂܂��B
//! @return ���s�������ʂ̏�Ԃł��B
int ExecuteSQL(const string, const string);

//! ExecuteSQL�̖߂�l�̎�ނ�\���܂��B
enum class ResultValue
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

//! ���͂�o�́A�o�߂̌v�Z�ɗ��p����f�[�^�̃f�[�^�^�̎�ނ�\���܂��B
enum class DataType
{
	STRING,   //!< ������^�ł��B
	INTEGER,  //!< �����^�ł��B
	BOOLEAN   //!< �^�U�l�^�ł��B
};

//! �g�[�N���̎�ނ�\���܂��B
enum class TokenKind
{
	NOT_TOKEN,              //!< �g�[�N����\���܂���B
	ASC,                    //!< ASC�L�[���[�h�ł��B
	AND,                    //!< AND�L�[���[�h�ł��B
	BY,                     //!< BY�L�[���[�h�ł��B
	DESC,                   //!< DESC�L�[���[�h�ł��B
	FROM,                   //!< FROM�L�[���[�h�ł��B
	OR,                     //!< OR�L�[���[�h�ł��B
	ORDER,                  //!< ORDER�L�[���[�h�ł��B
	SELECT,                 //!< SELECT�L�[���[�h�ł��B
	WHERE,                  //!< WHERE�L�[���[�h�ł��B
	ASTERISK,               //!< �� �L���ł��B
	COMMA,                  //!< �C �L���ł��B
	CLOSE_PAREN,            //!< �j �L���ł��B
	DOT,                    //!< �D �L���ł��B
	EQUAL,                  //!< �� �L���ł��B
	GREATER_THAN,           //!< �� �L���ł��B
	GREATER_THAN_OR_EQUAL,  //!< ���� �L���ł��B
	LESS_THAN,              //!< �� �L���ł��B
	LESS_THAN_OR_EQUAL,     //!< ���� �L���ł��B
	MINUS,                  //!< �| �L���ł��B
	NOT_EQUAL,              //!< ���� �L���ł��B
	OPEN_PAREN,             //!< �i �L���ł��B
	PLUS,                   //!< �{ �L���ł��B
	SLASH,                  //!< �^ �L���ł��B
	IDENTIFIER,             //!< ���ʎq�ł��B
	INT_LITERAL,            //!< �������e�����ł��B
	STRING_LITERAL          //!< �����񃊃e�����ł��B
};

//! ��̒l�����f�[�^�ł��B
class Data
{
	string m_string; //!< �f�[�^��������^�̏ꍇ�̒l�ł��B

	//! ���ۂ̃f�[�^���i�[���鋤�p�̂ł��B
	union
	{
		int integer;                  //!< �f�[�^�������^�̏ꍇ�̒l�ł��B
		bool boolean;                 //!< �f�[�^���^�U�l�^�̏ꍇ�̒l�ł��B
	} m_value;
public:
	DataType type = DataType::STRING; //!< �f�[�^�̌^�ł��B

	//! Data�N���X�̐V�����C���X�^���X�����������܂��B
	Data();

	//! Data�N���X�̐V�����C���X�^���X�����������܂��B
	//! @param [in] value �f�[�^�̒l�ł��B
	Data(const string value);

	//! Data�N���X�̐V�����C���X�^���X�����������܂��B
	//! @param [in] value �f�[�^�̒l�ł��B
	Data(const int value);

	//! Data�N���X�̐V�����C���X�^���X�����������܂��B
	//! @param [in] value �f�[�^�̒l�ł��B
	Data(const bool value);

	//! �f�[�^��������^�̏ꍇ�̒l���擾���܂��B
	//! @return �f�[�^��������^�̏ꍇ�̒l�ł��B
	const string& string() const;

	//! �f�[�^�������^�̏ꍇ�̒l���擾���܂��B
	//! @return �f�[�^�������^�̏ꍇ�̒l�ł��B
	const int& integer() const;

	//! �f�[�^���^�U�l�^�̏ꍇ�̒l���擾���܂��B
	//! @return �f�[�^���^�U�l�^�̏ꍇ�̒l�ł��B
	const bool& boolean() const;
};

//! WHERE��Ɏw�肷�鉉�Z�q�̏���\���܂��B
class Operator
{
public:
	TokenKind kind = TokenKind::NOT_TOKEN; //!< ���Z�q�̎�ނ��A���Z�q���L�q����g�[�N���̎�ނŕ\���܂��B
	int order = 0; //!< ���Z�q�̗D�揇�ʂł��B

	//! Operator�N���X�̐V�����C���X�^���X�����������܂��B
	Operator();

	//! Operator�N���X�̐V�����C���X�^���X�����������܂��B
	//! @param [in] kind ���Z�q�̎�ނ��A���Z�q���L�q����g�[�N���̎�ނŕ\���܂��B
	//! @param [in] order ���Z�q�̗D�揇�ʂł��B
	Operator(const TokenKind kind, const int order);
};

//! �g�[�N����\���܂��B
class Token
{
public:
	TokenKind kind; //!< �g�[�N���̎�ނł��B
	string word; //!< �L�^����Ă���g�[�N���̕�����ł��B�L�^�̕K�v���Ȃ���΋󔒂ł��B

	//! Token�N���X�̐V�����C���X�^���X�����������܂��B
	Token();

	//! Token�N���X�̐V�����C���X�^���X�����������܂��B
	//! @param [in] kind �g�[�N���̎�ނł��B
	Token(const TokenKind kind);

	//! Token�N���X�̐V�����C���X�^���X�����������܂��B
	//! @param [in] kind �g�[�N���̎�ނł��B
	//! @param [in] word �L�^����Ă���g�[�N���̕�����ł��B�L�^�̕K�v���Ȃ���΋󔒂ł��B
	Token(const TokenKind kind, const string word);
};

//! �w�肳�ꂽ��̏��ł��B�ǂ̃e�[�u���ɏ������邩�̏����܂݂܂��B
class Column
{
public:
	string tableName; //!< �񂪏�������e�[�u�����ł��B�w�肳��Ă��Ȃ��ꍇ�͋󕶎���ƂȂ�܂��B
	string columnName; //!< �w�肳�ꂽ��̗񖼂ł��B

	//! Column�N���X�̐V�����C���X�^���X�����������܂��B
	Column();

	//! Column�N���X�̐V�����C���X�^���X�����������܂��B
	//! @param [in] columnName �w�肳�ꂽ��̗񖼂ł��B
	Column(const string columnName);

	//! Column�N���X�̐V�����C���X�^���X�����������܂��B
	//! @param [in] tableName �񂪏�������e�[�u�����ł��B�w�肳��Ă��Ȃ��ꍇ�͋󕶎���ƂȂ�܂��B
	//! @param [in] columnName �w�肳�ꂽ��̗񖼂ł��B
	Column(const string tableName, const string columnName);
};

//! WHERE��̏����̎��؂�\���܂��B
class ExtensionTreeNode
{
public:
	shared_ptr<ExtensionTreeNode> parent;//!< �e�ƂȂ�m�[�h�ł��B���̎��؂̏ꍇ��nullptr�ƂȂ�܂��B
	shared_ptr<ExtensionTreeNode> left;  //!< ���̎q�ƂȂ�m�[�h�ł��B���g�����[�̗t�ƂȂ鎮�؂̏ꍇ��nullptr�ƂȂ�܂��B
	Operator middleOperator;             //!< ���u����鉉�Z�q�ł��B���g�����[�̂ƂȂ鎮�؂̏ꍇ�̎�ނ�NOT_TOKEN�ƂȂ�܂��B
	shared_ptr<ExtensionTreeNode>right;   //!< �E�̎q�ƂȂ�m�[�h�ł��B���g�����[�̗t�ƂȂ鎮�؂̏ꍇ��nullptr�ƂȂ�܂��B
	bool inParen = false;                //!< ���g���������ɂ���܂�Ă��邩�ǂ����ł��B
	int parenOpenBeforeClose = 0;        //!< �؂̍\�z����0�ȊO�ƂȂ�A���g�̍��ɂ���A�܂����ĂȂ��J�b�R�̊J�n�̐��ƂȂ�܂��B
	int signCoefficient = 1;             //!< ���g���t�ɂ���A�}�C�i�X�P�����Z�q�����Ă���ꍇ��-1�A����ȊO��1�ƂȂ�܂��B
	Column column;                       //!< ���w�肳��Ă���ꍇ�ɁA���̗��\���܂��B��w��ł͂Ȃ��ꍇ��columnName���󕶎���ƂȂ�܂��B
	bool calculated = false;             //!< ���̒l���v�Z���ɁA�v�Z�ς݂��ǂ����ł��B
	Data value;                          //!< �w�肳�ꂽ�A�������͌v�Z���ꂽ�l�ł��B

	//! ExtensionTreeNode�N���X�̐V�����C���X�^���X�����������܂��B
	ExtensionTreeNode();
};

//! �s�̏�����͂̃e�[�u���C���f�b�N�X�A��C���f�b�N�X�̌`�Ŏ����܂��B
class ColumnIndex
{
public:
	int table;  //!< �񂪓��͂̉��e�[�u���ڂ̗񂩂ł��B
	int column; //!< �񂪓��͂̃e�[�u���̉���ڂ��ł��B

	//! Column�N���X�̐V�����C���X�^���X�����������܂��B
	ColumnIndex();

	//! Column�N���X�̐V�����C���X�^���X�����������܂��B
	//! @param [in] table �񂪓��͂̉��e�[�u���ڂ̗񂩂ł��B
	//! @param [in] column �񂪓��͂̃e�[�u���̉���ڂ��ł��B
	ColumnIndex(const int table, const int column);
};

class SqlQueryInfo
{
public:
	vector<const string> tableNames; //!< FROM��Ŏw�肵�Ă���e�[�u�����ł��B
	vector<Column> selectColumns; //!< SELECT��Ɏw�肳�ꂽ�񖼂ł��B
	vector<Column> orderByColumns; //!< ORDER��Ɏw�肳�ꂽ�񖼂ł��B
	vector<TokenKind> orders; //!< �����C���f�b�N�X��orderByColumns�ɑΉ����Ă���A�����A�~���̎w��ł��B
	vector<shared_ptr<ExtensionTreeNode>> whereExtensionNodes; //!< WHERE�Ɏw�肳�ꂽ�؂̃m�[�h���A�؍\���Ƃ͖��֌W�Ɋi�[���܂��B
	shared_ptr<ExtensionTreeNode> whereTopNode; //!< ���؂̍��ƂȂ�m�[�h�ł��B
};

//! �t�@�C���ɑ΂��Ď��s����SQL��\���N���X�ł��B
class SqlQuery
{
	vector<ifstream> inputTableFiles;                       //!< �ǂݍ��ޓ��̓t�@�C���̑S�Ẵt�@�C���|�C���^�ł��B
	ofstream outputFile;                                    //!< �������ރt�@�C���̃t�@�C���|�C���^�ł��B
	bool found = false;                                     //!< �������Ɍ����������ǂ����̌��ʂ��ꎞ�I�ɕۑ����܂��B
	vector<vector<vector<Data>>> inputData;                 //!< ���̓f�[�^�ł��B
	vector<vector<Data>> outputData;                        //!< �o�̓f�[�^�ł��B
	vector<vector<Data>> allColumnOutputData;               //!< �o�͂���f�[�^�ɑΉ�����C���f�b�N�X�������A���ׂĂ̓��̓f�[�^��ۊǂ��܂��B

	const string alpahUnder = "_abcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPQRSTUVWXYZ"; //!< �S�ẴA���t�@�x�b�g�̑啶���������ƃA���_�[�o�[�ł��B
	const string alpahNumUnder = "_abcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"; //!< �S�Ă̐����ƃA���t�@�x�b�g�̑啶���������ƃA���_�[�o�[�ł��B
	const string signNum = "+-0123456789"; //!< �S�Ă̕����Ɛ����ł��B
	const string num = "0123456789"; //!< �S�Ă̐����ł��B
	const string space = " \t\r\n"; //!< �S�Ă̋󔒕����ł��B

	// keywordConditions��signConditions�͐擪���珇�Ɍ��������̂ŁA�O����v�ƂȂ��̍��ڂ͏��ԂɋC�����ēo�^���Ȃ��Ă͂����܂���B
	const vector<const Token> keywordConditions;//!< �L�[���[�h���g�[�N���Ƃ��ĔF�����邽�߂̃L�[���[�h�ꗗ���ł��B
	const vector<const Token> signConditions;//!< �L�����g�[�N���Ƃ��ĔF�����邽�߂̋L���ꗗ���ł��B
	const vector<const Operator> operators; //!< ���Z�q�̏��ł��B

	vector<const Token> tokens; //!< SQL�𕪊������g�[�N���ł��B

	SqlQueryInfo queryInfo; //! SQL����͂������ʂ̏��ł��B

	vector<vector<Column>> inputColumns; //!< ���͂��ꂽCSV�̍s�̏��ł��B

	string m_outputFileName; //!< outputFileName SQL�̎��s���ʂ�CSV�Ƃ��ďo�͂���t�@�C�����ł��B�g���q���܂݂܂��B

	//! SQL�̕����񂩂�g�[�N����؂�o���܂��B
	//! @param [in] sql �g�[�N���ɕ������錳�ƂȂ�SQL�ł��B
	//! @return �؂�o���ꂽ�g�[�N���ł��B
	const shared_ptr<const vector<const Token>> GetTokens(const string sql) const;

	//! �g�[�N������͂���SQL�̍\���Ŏw�肳�ꂽ�����擾���܂��B
	void AnalyzeTokens();

	//! CSV�t�@�C��������̓f�[�^��ǂݎ��܂��B
	void ReadCsv();

	//! CSV�t�@�C���ɏo�̓f�[�^���������݂܂��B
	void WriteCsv();

	//! �t�@�C����Close�������s���A����ɍs��ꂽ���m�F���܂��B
	void CheckClosingFiles();
public:
	//! SqlQuery�N���X�̐V�����C���X�^���X�����������܂��B
	SqlQuery();

	//! �J�����g�f�B���N�g���ɂ���CSV�ɑ΂��A�ȈՓI��SQL�����s���A���ʂ��t�@�C���ɏo�͂��܂��B
	//! @param [in] sql ���s����SQL�ł��B
	//! @param[in] outputFileName SQL�̎��s���ʂ�CSV�Ƃ��ďo�͂���t�@�C�����ł��B�g���q���܂݂܂��B
	//! @return ���s�������ʂ̏�Ԃł��B 
	int Execute(const string sql, const string outputFileName);
};

// �ȏ�w�b�_�ɑ������镔���B

//! Data�N���X�̐V�����C���X�^���X�����������܂��B
Data::Data() :m_value({ 0 })
{
}

//! Data�N���X�̐V�����C���X�^���X�����������܂��B
//! @param [in] value �f�[�^�̒l�ł��B
Data::Data(const std::string value) : m_value({ 0 })
{
	m_string = value;
}

//! Data�N���X�̐V�����C���X�^���X�����������܂��B
//! @param [in] value �f�[�^�̒l�ł��B
Data::Data(const int value) : type(DataType::INTEGER)
{
	m_value.integer = value;
}

//! Data�N���X�̐V�����C���X�^���X�����������܂��B
//! @param [in] value �f�[�^�̒l�ł��B
Data::Data(const bool value) : type(DataType::BOOLEAN)
{
	m_value.boolean = value;
}


//! �f�[�^��������^�̏ꍇ�̒l���擾���܂��B
//! @return �f�[�^��������^�̏ꍇ�̒l�ł��B
const string& Data::string() const
{
	return m_string;
}

//! �f�[�^�������^�̏ꍇ�̒l���擾���܂��B
//! @return �f�[�^�������^�̏ꍇ�̒l�ł��B
const int& Data::integer() const
{
	return m_value.integer;
}

//! �f�[�^���^�U�l�^�̏ꍇ�̒l���擾���܂��B
//! @return �f�[�^���^�U�l�^�̏ꍇ�̒l�ł��B
const bool& Data::boolean() const
{
	return m_value.boolean;
}

//! Operator�N���X�̐V�����C���X�^���X�����������܂��B
Operator::Operator()
{
}

//! Operator�N���X�̐V�����C���X�^���X�����������܂��B
//! @param [in] kind ���Z�q�̎�ނ��A���Z�q���L�q����g�[�N���̎�ނŕ\���܂��B
//! @param [in] order ���Z�q�̗D�揇�ʂł��B
Operator::Operator(const TokenKind kind, const int order) : kind(kind), order(order)
{
}

//! Token�N���X�̐V�����C���X�^���X�����������܂��B
Token::Token() : Token(TokenKind::NOT_TOKEN, "")
{
}

//! Token�N���X�̐V�����C���X�^���X�����������܂��B
//! @param [in] kind �g�[�N���̎�ނł��B
Token::Token(const TokenKind kind) : Token(kind, "")
{
}

//! Token�N���X�̐V�����C���X�^���X�����������܂��B
//! @param [in] kind �g�[�N���̎�ނł��B
//! @param [in] word �L�^����Ă���g�[�N���̕�����ł��B�L�^�̕K�v���Ȃ���΋󔒂ł��B
Token::Token(const TokenKind kind, const string word) :kind(kind)
{
	this->word = word;
}

//! Column�N���X�̐V�����C���X�^���X�����������܂��B
Column::Column() : Column("", "")
{

}

//! Column�N���X�̐V�����C���X�^���X�����������܂��B
//! @param [in] columnName �w�肳�ꂽ��̗񖼂ł��B
Column::Column(const string columnName) : Column("", columnName)
{
}

//! Column�N���X�̐V�����C���X�^���X�����������܂��B
//! @param [in] tableName �񂪏�������e�[�u�����ł��B�w�肳��Ă��Ȃ��ꍇ�͋󕶎���ƂȂ�܂��B
//! @param [in] columnName �w�肳�ꂽ��̗񖼂ł��B
Column::Column(const string tableName, const string columnName)
{
	this->tableName = tableName;
	this->columnName = columnName;
}

//! ExtensionTreeNode�N���X�̐V�����C���X�^���X�����������܂��B
ExtensionTreeNode::ExtensionTreeNode()
{
}

//! Column�N���X�̐V�����C���X�^���X�����������܂��B
ColumnIndex::ColumnIndex() : ColumnIndex(0, 0)
{
}

//! Column�N���X�̐V�����C���X�^���X�����������܂��B
//! @param [in] table �񂪓��͂̉��e�[�u���ڂ̗񂩂ł��B
//! @param [in] column �񂪓��͂̃e�[�u���̉���ڂ��ł��B
ColumnIndex::ColumnIndex(const int table, const int column) : table(table), column(column)
{
}

//! ��̕�������A�啶������������ʂ����ɔ�r���A���������ǂ����ł��B
//! @param [in] str1 ��r������ڂ̕�����ł��B
//! @param [in] str2 ��r������ڂ̕�����ł��B
//! @return ��r�������ʁA���������ǂ����ł��B
bool Equali(const string str1, const string str2){
	return
		str1.size() == str2.size() &&
		equal(str1.begin(), str1.end(), str2.begin(),
		[](const char &c1, const char &c2){return toupper(c1) == toupper(c2); });
}

//! SQL�̕����񂩂�g�[�N����؂�o���܂��B
//! @param [in] sql �g�[�N���ɕ������錳�ƂȂ�SQL�ł��B
//! @return �؂�o���ꂽ�g�[�N���ł��B
const shared_ptr<const vector<const Token>> SqlQuery::GetTokens(const string sql) const
{
	auto sqlBackPoint = sql.begin(); // SQL���g�[�N���ɕ������ēǂݍ��ގ��ɖ߂�|�C���g���L�^���Ă����܂��B

	auto sqlCursol = sql.begin(); // SQL���g�[�N���ɕ������ēǂݍ��ގ��Ɍ��ݓǂ�ł��镶���̏ꏊ��\���܂��B

	auto sqlEnd = sql.end(); // sql��end���w���܂��B

	auto tokens = make_shared<vector<const Token>>(); //�ǂݍ��񂾃g�[�N���ł��B

	// SQL���g�[�N���ɕ����ēǂݍ��݂܂��B
	while (sqlCursol != sqlEnd){

		// �󔒂�ǂݔ�΂��܂��B
		sqlCursol = find_if(sqlCursol, sqlEnd, [&](char c){return space.find(c) == string::npos; });
		if (sqlCursol == sqlEnd){
			break;
		}

		// ���l���e������ǂݍ��݂܂��B
		sqlBackPoint = sqlCursol;
		sqlCursol = find_if(sqlCursol, sqlEnd, [&](char c){return num.find(c) == string::npos; });
		if (sqlCursol != sqlBackPoint && (
			alpahUnder.find(*sqlCursol) == string::npos || // �����̌�ɂ����Ɏ��ʎq�������͕̂���킵���̂Ő��l���e�����Ƃ͈����܂���B
			sqlCursol == sqlEnd)){
			tokens->push_back(Token(TokenKind::INT_LITERAL, string(sqlBackPoint, sqlCursol)));
			continue;
		}
		else{
			sqlCursol = sqlBackPoint;
		}

		// �����񃊃e������ǂݍ��݂܂��B
		sqlBackPoint = sqlCursol;

		// �����񃊃e�������J�n����V���O���N�H�[�g�𔻕ʂ��A�ǂݍ��݂܂��B
		if (*sqlCursol == "\'"[0]){
			++sqlCursol;
			// ���g���N�X����c�[����cccc�̓V���O���N�H�[�g�̕������e�������̃G�X�P�[�v��F�����Ȃ����߁A�������e�������g��Ȃ����Ƃŉ�����Ă��܂��B
			sqlCursol = find_if_not(sqlCursol, sqlEnd, [](char c){return c != "\'"[0]; });
			if (sqlCursol == sqlEnd){
				throw ResultValue::ERR_TOKEN_CANT_READ;
			}
			++sqlCursol;
			tokens->push_back(Token(TokenKind::STRING_LITERAL, string(sqlBackPoint, sqlCursol)));
			continue;
		}

		// �L�[���[�h��ǂݍ��݂܂��B
		auto keyword = find_if(keywordConditions.begin(), keywordConditions.end(),
			[&](Token keyword){
			auto result =
				mismatch(keyword.word.begin(), keyword.word.end(), sqlCursol,
				[](const char keywordChar, const char sqlChar){return keywordChar == toupper(sqlChar); });

			if (result.first == keyword.word.end() && // �L�[���[�h�̍Ō�̕����܂œ����ł��B
				result.second != sqlEnd && alpahNumUnder.find(*result.second) == string::npos){ //�L�[���[�h�Ɏ��ʎq����؂�Ȃ��ɑ����Ă��Ȃ������m�F���܂��B 
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



		// �L����ǂݍ��݂܂��B
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

		// ���ʎq��ǂݍ��݂܂��B
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

//! �g�[�N������͂���SQL�̍\���Ŏw�肳�ꂽ�����擾���܂��B
void SqlQuery::AnalyzeTokens()
{
	auto tokenCursol = tokens.begin(); // ���݌��Ă���g�[�N�����w���܂��B

	// SELECT���ǂݍ��݂܂��B
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
		bool first = true; // SELECT��ɍŏ��Ɏw�肳�ꂽ�񖼂̓ǂݍ��݂��ǂ����ł��B
		while (tokenCursol->kind == TokenKind::COMMA || first){
			if (tokenCursol->kind == TokenKind::COMMA){
				++tokenCursol;
			}
			if (tokenCursol->kind == TokenKind::IDENTIFIER){
				// �e�[�u�������w�肳��Ă��Ȃ��ꍇ�Ɖ��肵�ēǂݍ��݂܂��B
				queryInfo.selectColumns.push_back(Column(tokenCursol->word));
				++tokenCursol;
				if (tokenCursol->kind == TokenKind::DOT){
					++tokenCursol;
					if (tokenCursol->kind == TokenKind::IDENTIFIER){

						// �e�[�u�������w�肳��Ă��邱�Ƃ��킩�����̂œǂݑւ��܂��B
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

	// ORDER���WHERE���ǂݍ��݂܂��B�ő�e��񂸂������Ƃ��ł��܂��B
	bool readOrder = false; // ���ł�ORDER�傪�ǂݍ��ݍς݂��ǂ����ł��B
	bool readWhere = false; // ���ł�WHERE�傪�ǂݍ��ݍς݂��ǂ����ł��B
	while (tokenCursol->kind == TokenKind::ORDER || tokenCursol->kind == TokenKind::WHERE){

		// ��x�ڂ�ORDER��̓G���[�ł��B
		if (readOrder && tokenCursol->kind == TokenKind::ORDER){
			throw ResultValue::ERR_SQL_SYNTAX;
		}

		// ��x�ڂ�WHERE��̓G���[�ł��B
		if (readWhere && tokenCursol->kind == TokenKind::WHERE){
			throw ResultValue::ERR_SQL_SYNTAX;
		}
		// ORDER���ǂݍ��݂܂��B
		if (tokenCursol->kind == TokenKind::ORDER){
			readOrder = true;
			++tokenCursol;
			if (tokenCursol->kind == TokenKind::BY){
				++tokenCursol;
				bool first = true; // ORDER��̍ŏ��̗񖼂̓ǂݍ��݂��ǂ����ł��B
				while (tokenCursol->kind == TokenKind::COMMA || first){
					if (tokenCursol->kind == TokenKind::COMMA){
						++tokenCursol;
					}
					if (tokenCursol->kind == TokenKind::IDENTIFIER){
						// �e�[�u�������w�肳��Ă��Ȃ��ꍇ�Ɖ��肵�ēǂݍ��݂܂��B
						queryInfo.orderByColumns.push_back(Column(tokenCursol->word));
						++tokenCursol;
						if (tokenCursol->kind == TokenKind::DOT){
							++tokenCursol;
							if (tokenCursol->kind == TokenKind::IDENTIFIER){

								// �e�[�u�������w�肳��Ă��邱�Ƃ��킩�����̂œǂݑւ��܂��B
								queryInfo.orderByColumns.back() = Column(queryInfo.orderByColumns.back().columnName, tokenCursol->word);
								++tokenCursol;
							}
							else{
								throw ResultValue::ERR_SQL_SYNTAX;
							}
						}

						// ���ёւ��̏����A�~�����w�肵�܂��B
						if (tokenCursol->kind == TokenKind::ASC){
							queryInfo.orders.push_back(TokenKind::ASC);
							++tokenCursol;
						}
						else if (tokenCursol->kind == TokenKind::DESC){
							queryInfo.orders.push_back(TokenKind::DESC);
							++tokenCursol;
						}
						else{
							// �w�肪�Ȃ��ꍇ�͏����ƂȂ�܂��B
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

		// WHERE���ǂݍ��݂܂��B
		if (tokenCursol->kind == TokenKind::WHERE){
			readWhere = true;
			++tokenCursol;
			shared_ptr<ExtensionTreeNode> currentNode; // ���ݓǂݍ���ł���m�[�h�ł��B
			while (true){
				// �I�y�����h��ǂݍ��݂܂��B

				// �I�y�����h�̃m�[�h��V�����������܂��B
				queryInfo.whereExtensionNodes.push_back(make_shared<ExtensionTreeNode>());
				if (currentNode){
					// ���݂̃m�[�h���E�̎q�ɂ��炵�A���̈ʒu�ɐV�����m�[�h��}�����܂��B
					currentNode->right = queryInfo.whereExtensionNodes.back();
					currentNode->right->parent = currentNode;
					currentNode = currentNode->right;
				}
				else{
					// �ŏ��̓J�����g�m�[�h�ɐV�����m�[�h�����܂��B
					currentNode = queryInfo.whereExtensionNodes.back();
				}

				// �J�b�R�J����ǂݍ��݂܂��B
				while (tokenCursol->kind == TokenKind::OPEN_PAREN){
					++currentNode->parenOpenBeforeClose;
					++tokenCursol;
				}

				// �I�y�����h�ɑO�u�����+��-��ǂݍ��݂܂��B
				if (tokenCursol->kind == TokenKind::PLUS || tokenCursol->kind == TokenKind::MINUS){

					// +-��O�u����̂͗񖼂Ɛ��l���e�����݂̂ł��B
					if (tokenCursol[1].kind != TokenKind::IDENTIFIER && tokenCursol[1].kind != TokenKind::INT_LITERAL){
						throw ResultValue::ERR_WHERE_OPERAND_TYPE;
					}
					if (tokenCursol->kind == TokenKind::MINUS){
						currentNode->signCoefficient = -1;
					}
					++tokenCursol;
				}

				// �񖼁A�������e�����A�����񃊃e�����̂����ꂩ���I�y�����h�Ƃ��ēǂݍ��݂܂��B
				if (tokenCursol->kind == TokenKind::IDENTIFIER){

					// �e�[�u�������w�肳��Ă��Ȃ��ꍇ�Ɖ��肵�ēǂݍ��݂܂��B
					currentNode->column = Column(tokenCursol->word);
					++tokenCursol;
					if (tokenCursol->kind == TokenKind::DOT){
						++tokenCursol;
						if (tokenCursol->kind == TokenKind::IDENTIFIER){

							// �e�[�u�������w�肳��Ă��邱�Ƃ��킩�����̂œǂݑւ��܂��B
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
					// �O��̃V���O���N�H�[�g����苎������������f�[�^�Ƃ��ēǂݍ��݂܂��B
					currentNode->value = Data(tokenCursol->word.substr(1, tokenCursol->word.size() - 2));
					++tokenCursol;
				}
				else{
					throw ResultValue::ERR_SQL_SYNTAX;
				}

				// �I�y�����h�̉E�̃J�b�R�����ǂݍ��݂܂��B
				while (tokenCursol->kind == TokenKind::CLOSE_PAREN){
					shared_ptr<ExtensionTreeNode> searchedAncestor = currentNode->parent; // �J�b�R����ƑΉ�����J�b�R�J���𗼕��܂ޑc��m�[�h��T�����߂̃J�[�\���ł��B
					while (searchedAncestor){

						// searchedAncestor�̍��̎q�ɑΉ�����J�b�R�J�����Ȃ������������܂��B
						shared_ptr<ExtensionTreeNode> searched = searchedAncestor; // searchedAncestor�̓�������J�b�R�J�����������邽�߂̃J�[�\���ł��B
						while (searched && !searched->parenOpenBeforeClose){
							searched = searched->left;
						}
						if (searched){
							// �Ή��t�����Ă��Ȃ��J�b�R�J������폜���A�m�[�h���J�b�R�Ɉ͂܂�Ă��邱�Ƃ��L�^���܂��B
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


				// ���Z�q(�I�y���[�^�[��ǂݍ��݂܂��B
				auto foundOperator = find_if(operators.begin(), operators.end(), [&](const Operator& op){return op.kind == tokenCursol->kind; }); // ���ݓǂݍ���ł��鉉�Z�q�̏��ł��B

				// ���݌��Ă��鉉�Z�q�̏���T���܂��B
				if (foundOperator != operators.end()){
					// �����������Z�q�̏������ƂɃm�[�h�����ւ��܂��B
					shared_ptr<ExtensionTreeNode> tmp = currentNode; //�m�[�h�����ւ��邽�߂Ɏg���ϐ��ł��B

					shared_ptr<ExtensionTreeNode> searched = tmp; // ����ւ���m�[�h��T�����߂̃J�[�\���ł��B

					//�J�b�R�ɂ������Ă��Ȃ������ꍇ�ɁA���Z�q�̗D�揇�ʂ��Q�l�Ɍ�������m�[�h��T���܂��B
					bool first = true; // ���Z�q�̗D�揇�ʂ���������ŏ��̃��[�v�ł��B
					do{
						if (!first){
							tmp = tmp->parent;
							searched = tmp;
						}
						// ���݂̓ǂݍ��ݏꏊ��������J�b�R���J���ꏊ��T���܂��B
						while (searched && !searched->parenOpenBeforeClose){
							searched = searched->left;
						}
						first = false;
					} while (!searched && tmp->parent && (tmp->parent->middleOperator.order <= foundOperator->order || tmp->parent->inParen));

					// ���Z�q�̃m�[�h��V�����������܂��B
					queryInfo.whereExtensionNodes.push_back(make_shared<ExtensionTreeNode>());
					currentNode = queryInfo.whereExtensionNodes.back();
					currentNode->middleOperator = *foundOperator;

					// ���������ꏊ�ɐV�����m�[�h��z�u���܂��B����܂ł��̈ʒu�ɂ������m�[�h�͍��̎q�ƂȂ�悤�A�e�m�[�h�Ǝq�m�[�h�̃|�C���^���������܂��B
					currentNode->parent = tmp->parent;
					if (currentNode->parent){
						currentNode->parent->right = currentNode;
					}
					currentNode->left = tmp;
					tmp->parent = currentNode;

					++tokenCursol;
				}
				else{
					// ���݌��Ă����ނ����Z�q�̈ꗗ���猩����Ȃ���΁AWHERE��͏I���܂��B
					break;
				}
			}

			// �؂����Ɍ������Ă����̂ڂ�A���̃m�[�h��ݒ肵�܂��B
			queryInfo.whereTopNode = currentNode;
			while (queryInfo.whereTopNode->parent){
				queryInfo.whereTopNode = queryInfo.whereTopNode->parent;
			}
		}
	}

	// FROM���ǂݍ��݂܂��B
	if (tokenCursol->kind == TokenKind::FROM){
		++tokenCursol;
	}
	else{
		throw ResultValue::ERR_SQL_SYNTAX;
	}
	bool first = true; // FROM��̍ŏ��̃e�[�u������ǂݍ��ݒ����ǂ����ł��B
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

	// �Ō�̃g�[�N���܂œǂݍ��݂��i��ł��Ȃ�������G���[�ł��B
	if (tokenCursol != tokens.end()){
		throw ResultValue::ERR_SQL_SYNTAX;
	}


}

//! CSV�t�@�C��������̓f�[�^��ǂݎ��܂��B
void SqlQuery::ReadCsv()
{
	for (size_t i = 0; i < queryInfo.tableNames.size(); ++i){
		// ���̓t�@�C�����J���܂��B
		inputTableFiles.push_back(ifstream(queryInfo.tableNames[i] + ".csv"));
		if (!inputTableFiles.back()){
			throw ResultValue::ERR_FILE_OPEN;
		}

		// ����CSV�̃w�b�_�s��ǂݍ��݂܂��B
		inputColumns.push_back(vector<Column>());
		string inputLine; // �t�@�C������ǂݍ��񂾍s������ł��B
		if (getline(inputTableFiles.back(), inputLine)){
			auto charactorCursol = inputLine.begin(); // �w�b�_���͍s����������J�[�\���ł��B
			auto lineEnd = inputLine.end(); // �w�b�_���͍s��end���w���܂��B

			// �ǂݍ��񂾍s���Ō�܂œǂ݂܂��B
			while (charactorCursol != lineEnd){

				// �񖼂���ǂ݂܂��B
				auto columnStart = charactorCursol; // ���݂̗�̍ŏ����L�^���Ă����܂��B
				charactorCursol = find(charactorCursol, lineEnd, ',');
				inputColumns[i].push_back(Column(queryInfo.tableNames[i], string(columnStart, charactorCursol)));

				// ���͍s�̃J���}�̕���ǂݐi�߂܂��B
				if (charactorCursol != lineEnd){
					++charactorCursol;
				}
			}
		}
		else{
			throw ResultValue::ERR_CSV_SYNTAX;
		}

		// ����CSV�̃f�[�^�s��ǂݍ��݂܂��B

		inputData.push_back(vector<vector<Data>>());

		while (getline(inputTableFiles.back(), inputLine)){
			inputData[i].push_back(vector<Data>()); // ���͂���Ă����s���̃f�[�^�ł��B
			vector<Data> &row = inputData[i].back();

			auto charactorCursol = inputLine.begin(); // �f�[�^���͍s����������J�[�\���ł��B
			auto lineEnd = inputLine.end(); // �f�[�^���͍s��end���w���܂��B

			// �ǂݍ��񂾍s���Ō�܂œǂ݂܂��B
			while (charactorCursol != lineEnd){

				// �ǂݍ��񂾃f�[�^���������ލs�̃J�����𐶐����܂��B
				auto columnStart = charactorCursol; // ���݂̗�̍ŏ����L�^���Ă����܂��B
				charactorCursol = find(charactorCursol, lineEnd, ',');

				row.push_back(Data(string(columnStart, charactorCursol)));

				// ���͍s�̃J���}�̕���ǂݐi�߂܂��B
				if (charactorCursol != lineEnd){
					++charactorCursol;
				}
			}
		}

		// �S�Ă����l�ƂȂ��͐��l��ɕϊ����܂��B
		for (size_t j = 0; j < inputColumns[i].size(); ++j){

			// �S�Ă̍s�̂����ɂ��āA�f�[�^�����񂩂畄���Ɛ��l�ȊO�̕�����T���܂��B
			if (none_of(
				inputData[i].begin(),
				inputData[i].end(),
				[&](const vector<Data> &inputRow){
				return any_of(
					inputRow[j].string().begin(),
					inputRow[j].string().end(),
					[&](const char& c){return signNum.find(c) == string::npos; }); })){

				// �����Ɛ����ȊO��������Ȃ���ɂ��ẮA���l��ɕϊ����܂��B
				for (auto& inputRow : inputData[i]){
					inputRow[j] = Data(stoi(inputRow[j].string()));
				}
			}
		}
	}
}

//! CSV�t�@�C���ɏo�̓f�[�^���������݂܂��B
void SqlQuery::WriteCsv()
{
	vector<Column> allInputColumns; // ���͂Ɋ܂܂�邷�ׂĂ̗�̈ꗗ�ł��B

	// ���̓t�@�C���ɏ����Ă��������ׂĂ̗��allInputColumns�ɐݒ肵�܂��B
	for (size_t i = 0; i < queryInfo.tableNames.size(); ++i){
		transform(
			inputColumns[i].begin(),
			inputColumns[i].end(),
			back_inserter(allInputColumns),
			[&](const Column& column){return Column(queryInfo.tableNames[i], column.columnName); });
	}

	// SELECT��̗񖼎w�肪*�������ꍇ�́A����CSV�̗񖼂����ׂđI������܂��B
	if (queryInfo.selectColumns.empty()){
		copy(allInputColumns.begin(), allInputColumns.end(), back_inserter(queryInfo.selectColumns));
	}

	vector<Column> outputColumns; // �o�͂��邷�ׂĂ̍s�̏��ł��B

	// SELECT��Ŏw�肳�ꂽ�񖼂��A���ڂ̓��̓t�@�C���̉���ڂɑ������邩�𔻕ʂ��܂��B
	vector<ColumnIndex> selectColumnIndexes; // SELECT��Ŏw�肳�ꂽ��́A���̓t�@�C���Ƃ��ẴC���f�b�N�X�ł��B

	for (auto &selectColumn : queryInfo.selectColumns){
		found = false;
		for (size_t i = 0; i < queryInfo.tableNames.size(); ++i){
			int j = 0;
			for (auto &inputColumn : inputColumns[i]){
				if (Equali(selectColumn.columnName, inputColumn.columnName) &&
					(selectColumn.tableName.empty() || // �e�[�u�������ݒ肳��Ă���ꍇ�̂݃e�[�u�����̔�r���s���܂��B
					Equali(selectColumn.tableName, inputColumn.tableName))){

					// ���Ɍ������Ă���̂ɂ��������������G���[�ł��B
					if (found){
						throw ResultValue::ERR_BAD_COLUMN_NAME;
					}
					found = true;
					// ���������l������̃f�[�^�𐶐����܂��B
					selectColumnIndexes.push_back(ColumnIndex(i, j));
				}
				++j;
			}
		}
		// ���������Ȃ��Ă��G���[�ł��B
		if (!found){
			throw ResultValue::ERR_BAD_COLUMN_NAME;
		}
	}

	// �o�͂���񖼂�ݒ肵�܂��B
	transform(
		selectColumnIndexes.begin(),
		selectColumnIndexes.end(),
		back_inserter(outputColumns),
		[&](const ColumnIndex& index){return inputColumns[index.table][index.column]; });

	if (queryInfo.whereTopNode){
		// �������l�̕������v�Z���܂��B
		for (auto &whereExtensionNode : queryInfo.whereExtensionNodes){
			if (whereExtensionNode->middleOperator.kind == TokenKind::NOT_TOKEN &&
				whereExtensionNode->column.columnName.empty() &&
				whereExtensionNode->value.type == DataType::INTEGER){
				whereExtensionNode->value = Data(whereExtensionNode->value.integer() * whereExtensionNode->signCoefficient);
			}
		}
	}

	vector<vector<vector<Data>>::iterator> currentRows; // ���͂��ꂽ�e�e�[�u���́A���ݏo�͂��Ă���s���w���J�[�\���ł��B
	transform(
		inputData.begin(),
		inputData.end(),
		back_inserter(currentRows),
		[](vector<vector<Data>>& rows){return rows.begin(); });

	// �o�͂���f�[�^��ݒ肵�܂��B
	while (true){
		outputData.push_back(vector<Data>());
		vector<Data> &row = outputData.back(); // �o�͂��Ă����s���̃f�[�^�ł��B

		// �s�̊e��̃f�[�^����͂��玝���Ă��Đݒ肵�܂��B
		transform(
			selectColumnIndexes.begin(),
			selectColumnIndexes.end(),
			back_inserter(row),
			[&](const ColumnIndex& index){return (*currentRows[index.table])[index.column]; });

		allColumnOutputData.push_back(vector<Data>());
		vector<Data> &allColumnsRow = allColumnOutputData.back();// WHERE��ORDER�̂��߂ɂ��ׂĂ̏����܂ލs�Brow�ƃC���f�b�N�X�����L���܂��B

		// allColumnsRow�̗��ݒ肵�܂��B
		for (auto &currentRow : currentRows){
			copy(
				currentRow->begin(),
				currentRow->end(),
				back_inserter(allColumnsRow));
		}
		// WHERE�̏����ƂȂ�l���ċA�I�Ɍv�Z���܂��B
		if (queryInfo.whereTopNode){
			shared_ptr<ExtensionTreeNode> currentNode = queryInfo.whereTopNode; // ���݌��Ă���m�[�h�ł��B
			while (currentNode){
				// �q�m�[�h�̌v�Z���I����ĂȂ��ꍇ�́A�܂�������̌v�Z���s���܂��B
				if (currentNode->left && !currentNode->left->calculated){
					currentNode = currentNode->left;
					continue;
				}
				else if (currentNode->right && !currentNode->right->calculated){
					currentNode = currentNode->right;
					continue;
				}

				// ���m�[�h�̒l���v�Z���܂��B
				switch (currentNode->middleOperator.kind){
				case TokenKind::NOT_TOKEN:
					// �m�[�h�Ƀf�[�^���ݒ肳��Ă���ꍇ�ł��B

					// �f�[�^���񖼂Ŏw�肳��Ă���ꍇ�A�������Ă���s�̃f�[�^��ݒ肵�܂��B
					if (!currentNode->column.columnName.empty()){
						found = false;
						for (size_t i = 0; i < allInputColumns.size(); ++i){
							if (Equali(currentNode->column.columnName, allInputColumns[i].columnName) &&
								(currentNode->column.tableName.empty() || // �e�[�u�������ݒ肳��Ă���ꍇ�̂݃e�[�u�����̔�r���s���܂��B
								Equali(currentNode->column.tableName, allInputColumns[i].tableName))){
								// ���Ɍ������Ă���̂ɂ��������������G���[�ł��B
								if (found){
									throw ResultValue::ERR_BAD_COLUMN_NAME;
								}
								found = true;
								currentNode->value = allColumnsRow[i];
							}
						}
						// ���������Ȃ��Ă��G���[�ł��B
						if (!found){
							throw ResultValue::ERR_BAD_COLUMN_NAME;
						}
						;
						// �������l�����Ēl���v�Z���܂��B
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
					// ��r���Z�q�̏ꍇ�ł��B

					// ��r�ł���͕̂�����^�������^�ŁA�����E�̌^�������ꍇ�ł��B
					if (currentNode->left->value.type != DataType::INTEGER && currentNode->left->value.type != DataType::STRING ||
						currentNode->left->value.type != currentNode->right->value.type){
						throw ResultValue::ERR_WHERE_OPERAND_TYPE;
					}
					currentNode->value.type = DataType::BOOLEAN;

					// ��r���ʂ��^�Ɖ��Z�q�ɂ���Čv�Z���@��ς��āA�v�Z���܂��B
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
					// �l�����Z�̏ꍇ�ł��B

					// ���Z�ł���̂͐����^���m�̏ꍇ�݂̂ł��B
					if (currentNode->left->value.type != DataType::INTEGER || currentNode->right->value.type != DataType::INTEGER){
						throw ResultValue::ERR_WHERE_OPERAND_TYPE;
					}
					currentNode->value.type = DataType::INTEGER;

					// ��r���ʂ����Z�q�ɂ���Čv�Z���@��ς��āA�v�Z���܂��B
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
					// �_�����Z�̏ꍇ�ł��B

					// ���Z�ł���̂͐^�U�l�^���m�̏ꍇ�݂̂ł��B
					if (currentNode->left->value.type != DataType::BOOLEAN || currentNode->right->value.type != DataType::BOOLEAN){
						throw ResultValue::ERR_WHERE_OPERAND_TYPE;
					}
					currentNode->value.type = DataType::BOOLEAN;

					// ��r���ʂ����Z�q�ɂ���Čv�Z���@��ς��āA�v�Z���܂��B
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

				// ���g�̌v�Z���I�������͐e�̌v�Z�ɖ߂�܂��B
				currentNode = currentNode->parent;
			}

			// �����ɍ���Ȃ��s�͏o�͂���폜���܂��B
			if (!queryInfo.whereTopNode->value.boolean()){
				allColumnOutputData.pop_back();
				outputData.pop_back();
			}
			// WHERE�����̌v�Z���ʂ����Z�b�g���܂��B
			for (auto &whereExtensionNode : queryInfo.whereExtensionNodes){
				whereExtensionNode->calculated = false;
			}
		}

		// �e�e�[�u���̍s�̂��ׂĂ̑g�ݍ��킹���o�͂��܂��B

		// �Ō�̃e�[�u���̃J�����g�s���C���N�������g���܂��B
		++currentRows[queryInfo.tableNames.size() - 1];

		// �Ō�̃e�[�u�����ŏI�s�ɂȂ��Ă����ꍇ�͐擪�ɖ߂��A���ɑO�̃e�[�u���̃J�����g�s���C���N�������g���܂��B
		for (int i = queryInfo.tableNames.size() - 1; currentRows[i] == inputData[i].end() && 0 < i; --i){
			++currentRows[i - 1];
			currentRows[i] = inputData[i].begin();
		}

		// �ŏ��̃e�[�u�����Ō�̍s�𒴂����Ȃ�o�͍s�̐����͏I���ł��B
		if (currentRows[0] == inputData[0].end()){
			break;
		}
	}

	// ORDER��ɂ����ёւ��̏������s���܂��B
	if (!queryInfo.orderByColumns.empty()){
		// ORDER��Ŏw�肳��Ă���񂪁A�S�Ă̓��͍s�̒��̂ǂ̍s�Ȃ̂����v�Z���܂��B
		vector<int> orderByColumnIndexes; // ORDER��Ŏw�肳�ꂽ��́A���ׂĂ̍s�̒��ł̃C���f�b�N�X�ł��B

		for (auto &orderByColumn : queryInfo.orderByColumns){
			found = false;
			for (size_t i = 0; i < allInputColumns.size(); ++i){
				if (Equali(orderByColumn.columnName, allInputColumns[i].columnName) &&
					(orderByColumn.tableName.empty() || // �e�[�u�������ݒ肳��Ă���ꍇ�̂݃e�[�u�����̔�r���s���܂��B
					Equali(orderByColumn.tableName, allInputColumns[i].tableName))){
					// ���Ɍ������Ă���̂ɂ��������������G���[�ł��B
					if (found){
						throw ResultValue::ERR_BAD_COLUMN_NAME;
					}
					found = true;
					orderByColumnIndexes.push_back(i);
				}
			}
			// ���������Ȃ��Ă��G���[�ł��B
			if (!found){
				throw ResultValue::ERR_BAD_COLUMN_NAME;
			}
		}

		// outputData��allColumnOutputData�̃\�[�g���ꏏ�ɍs���܂��B�ȕւ̂��ߋÂ����\�[�g�͎g�킸�A�I���\�[�g�𗘗p���܂��B
		for (size_t i = 0; i < outputData.size(); ++i){
			int minIndex = i; // ���݂܂łōŏ��̍s�̃C���f�b�N�X�ł��B
			for (size_t j = i + 1; j < outputData.size(); ++j){
				bool jLessThanMin = false; // �C���f�b�N�X��j�̒l���AminIndex�̒l��菬�������ǂ����ł��B
				for (size_t k = 0; k < orderByColumnIndexes.size(); ++k){
					const Data &mData = allColumnOutputData[minIndex][orderByColumnIndexes[k]]; // �C���f�b�N�X��minIndex�̃f�[�^�ł��B
					const Data &jData = allColumnOutputData[j][orderByColumnIndexes[k]]; // �C���f�b�N�X��j�̃f�[�^�ł��B
					int cmp = 0; // ��r���ʂł��B���������0�A�C���f�b�N�Xj�̍s���傫����΃v���X�A�C���f�b�N�XminIndex�̍s���傫����΃}�C�i�X�ƂȂ�܂��B
					switch (mData.type)
					{
					case DataType::INTEGER:
						cmp = jData.integer() - mData.integer();
						break;
					case DataType::STRING:
						cmp = strcmp(jData.string().c_str(), mData.string().c_str());
						break;
					}

					// �~���Ȃ�cmp�̑召�����ւ��܂��B
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

	// �o�̓t�@�C�����J���܂��B
	outputFile = ofstream(m_outputFileName);
	if (outputFile.bad()){
		throw ResultValue::ERR_FILE_OPEN;
	}

	// �o�̓t�@�C���ɗ񖼂��o�͂��܂��B
	for (size_t i = 0; i < queryInfo.selectColumns.size(); ++i){
		outputFile << outputColumns[i].columnName;
		if (i < queryInfo.selectColumns.size() - 1){
			outputFile << ",";
		}
		else{
			outputFile << "\n";
		}
	}

	// �o�̓t�@�C���Ƀf�[�^���o�͂��܂��B
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

//! �t�@�C����Close�������s���A����ɍs��ꂽ���m�F���܂��B
void SqlQuery::CheckClosingFiles()
{
	// �t�@�C�����\�[�X��������܂��B
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

//! SqlQuery�N���X�̐V�����C���X�^���X�����������܂��B
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

//! �J�����g�f�B���N�g���ɂ���CSV�ɑ΂��A�ȈՓI��SQL�����s���A���ʂ��t�@�C���ɏo�͂��܂��B
//! @param [in] sql ���s����SQL�ł��B
//! @param[in] outputFileName SQL�̎��s���ʂ�CSV�Ƃ��ďo�͂���t�@�C�����ł��B�g���q���܂݂܂��B
//! @return ���s�������ʂ̏�Ԃł��B 
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
	catch (ResultValue error) // ���������G���[�̎�ނł��B
	{
		return static_cast<int>(error);
	}
}

//! �J�����g�f�B���N�g���ɂ���CSV�ɑ΂��A�ȈՓI��SQL�����s���A���ʂ��t�@�C���ɏo�͂��܂��B
//! @param [in] sql ���s����SQL�ł��B
//! @param[in] outputFileName SQL�̎��s���ʂ�CSV�Ƃ��ďo�͂���t�@�C�����ł��B�g���q���܂݂܂��B
//! @return ���s�������ʂ̏�Ԃł��B
//! @retval OK=0                      ���Ȃ��I�����܂����B
//! @retval ERR_FILE_OPEN=1           �t�@�C�����J�����ƂɎ��s���܂����B
//! @retval ERR_FILE_WRITE=2          �t�@�C���ɏ������݂��s�����ƂɎ��s���܂����B
//! @retval ERR_FILE_CLOSE=3          �t�@�C������邱�ƂɎ��s���܂����B
//! @retval ERR_TOKEN_CANT_READ=4     �g�[�N����͂Ɏ��s���܂����B
//! @retval ERR_SQL_SYNTAX=5          SQL�̍\����͂����s���܂����B
//! @retval ERR_BAD_COLUMN_NAME=6     �e�[�u���w����܂ޗ񖼂��K�؂ł͂���܂���B
//! @retval ERR_WHERE_OPERAND_TYPE=7  ���Z�̍��E�̌^���K�؂ł͂���܂���B
//! @retval ERR_CSV_SYNTAX=8          CSV�̍\����͂����s���܂����B
//! @retval ERR_MEMORY_ALLOCATE=9     �������̎擾�Ɏ��s���܂����B
//! @retval ERR_MEMORY_OVER=10        �p�ӂ����������̈�̏���𒴂��܂����B
//! @details 
//! �Q�Ƃ���e�[�u���́A�e�[�u����.csv�̌`�ō쐬���܂��B                                                     @n
//! ��s�ڂ̓w�b�_�s�ŁA���̍s�ɗ񖼂������܂��B                                                             @n
//! �O��̃X�y�[�X�ǂݔ�΂���_�u���N�H�[�e�[�V�����ł�����Ȃǂ̋@�\�͂���܂���B                         @n
//! ��̌^�̒�`�͂ł��Ȃ��̂ŁA��̂��ׂẴf�[�^�̒l�����l�Ƃ��ĉ��߂ł����̃f�[�^�𐮐��Ƃ��Ĉ����܂��B @n
//! ���s����SQL�Ŏg����@�\���ȉ��ɗ�Ƃ��Ă����܂��B                                                        @n
//! ��1:                                                                                                     @n
//! SELECT *                                                                                                 @n
//! FROM USERS                                                                                               @n
//!                                                                                                          @n
//! ��2: �啶���������͋�ʂ��܂���B                                                                        @n
//! select *                                                                                                 @n
//! from users                                                                                               @n
//!                                                                                                          @n
//! ��3: ��̎w�肪�ł��܂��B                                                                                @n
//! SELECT Id, Name                                                                                          @n
//! FROM USERS                                                                                               @n
//!                                                                                                          @n
//! ��4: �e�[�u�������w�肵�ė�̎w�肪�ł��܂��B                                                            @n
//! SELECT USERS.Id                                                                                          @n
//! FROM USERS                                                                                               @n
//!                                                                                                          @n
//! ��5: ORDER�傪�g���܂��B                                                                                 @n
//! SELECT *                                                                                                 @n
//! ORDER BY NAME                                                                                            @n
//! FROM USERS                                                                                               @n
//!                                                                                                          @n
//! ��6: ORDER��ɕ�����⏸���A�~���̎w�肪�ł��܂��B                                                       @n
//! SELECT *                                                                                                 @n
//! ORDER BY AGE DESC, Name ASC                                                                              @n
//!                                                                                                          @n
//! ��7: WHERE�傪�g���܂��B                                                                                 @n
//! SELECT *                                                                                                 @n
//! WHERE AGE >= 20                                                                                          @n
//! FROM USERS                                                                                               @n
//!                                                                                                          @n
//! ��8: WHERE��ł͕�����̔�r���g���܂��B                                                                 @n
//! SELECT *                                                                                                 @n
//! WHERE NAME >= 'N'                                                                                        @n
//! FROM USERS                                                                                               @n
//!                                                                                                          @n
//! ��9: WHERE��ɂ͎l�����Z�A�J�b�R�AAND�AOR�Ȃǂ��܂ޕ��G�Ȏ������p�ł��܂��B                              @n
//! SELECT *                                                                                                 @n
//! WHERE AGE >= 20 AND (AGE <= 40 || WEIGHT < 100)                                                          @n
//! FROM USERS                                                                                               @n
//!                                                                                                          @n
//! ��10: FROM��ɕ����̃e�[�u�����w��ł��܂��B���̏ꍇ�̓N���X�Ō������܂��B                               @n
//! SELECT *                                                                                                 @n
//! FROM USERS, CHILDREN                                                                                     @n
//!                                                                                                          @n
//! ��11: WHERE�ŏ��������邱�Ƃɂ��A�e�[�u���̌������ł��܂��B                                          @n
//! SELECT USERS.NAME, CHILDREN.NAME                                                                         @n
//! WHERE USERS.ID = CHILDREN.PARENTID                                                                       @n
//! FROM USERS, CHILDREN                                                                                     @n
int ExecuteSQL(const string sql, const string outputFileName)
{
	return SqlQuery().Execute(sql, outputFileName);
}