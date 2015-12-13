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

class InputTable;
//! �w�肳�ꂽ��̏��ł��B�ǂ̃e�[�u���ɏ������邩�̏����܂݂܂��B
class Column
{
public:
	string tableName; //!< �񂪏�������e�[�u�����ł��B�w�肳��Ă��Ȃ��ꍇ�͋󕶎���ƂȂ�܂��B
	string columnName; //!< �w�肳�ꂽ��̗񖼂ł��B
	int allColumnsIndex; //!< �S�Ẵe�[�u���̂��ׂĂ̗�̒��ŁA���̗񂪉��Ԗڂ��ł��B
	string outputName; //!< ���̗���o�͂��鎞�̕\�����ł��B

	//! Column�N���X�̐V�����C���X�^���X�����������܂��B
	Column();

	//! Column�N���X�̐V�����C���X�^���X�����������܂��B
	//! @param [in] columnName �w�肳�ꂽ��̗񖼂ł��B
	Column(const string columnName);

	//! Column�N���X�̐V�����C���X�^���X�����������܂��B
	//! @param [in] tableName �񂪏�������e�[�u�����ł��B�w�肳��Ă��Ȃ��ꍇ�͋󕶎���ƂȂ�܂��B
	//! @param [in] columnName �w�肳�ꂽ��̗񖼂ł��B
	Column(const string tableName, const string columnName);

	//! �f�[�^�̌����ɗ��p���邽�߁A�S�Ẵe�[�u���̗�̏���o�^���܂��B
	//! @param [in] queryInfo SQL�ɋL�q���ꂽ���ł��B
	//! @param [in] inputTables �t�@�C������ǂݎ�����f�[�^�ł��B
	void Column::SetAllColumns(const vector<const InputTable> &inputTables);
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

//! SqlQuery�̍\�����������N���X�ł��B
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

//! CSV�Ƃ��ē��͂��ꂽ�t�@�C���̓��e��\���܂��B
class InputTable
{
	const string signNum = "+-0123456789"; //!< �S�Ă̕����Ɛ����ł��B

	const shared_ptr<const vector<const Column>> m_columns; //!< ��̏��ł��B
	const shared_ptr<vector<const vector<const Data>>> m_data; //! �f�[�^�ł��B

	//! �S�Ă����l�ƂȂ��͐��l��ɕϊ����܂��B
	void InputTable::InitializeIntegerColumn();
public:
	//! InputTable�N���X�̐V�����C���X�^���X�����������܂��B
	//! @param [in] columns �ǂݍ��񂾃w�b�_���ł��B
	//! @param [in] data �ǂݍ��񂾃f�[�^�ł��B
	InputTable(const shared_ptr<const vector<const Column>> columns, const shared_ptr<vector<const vector<const Data>>> data);

	//! ��̏����擾���܂��B
	const shared_ptr<const vector<const Column>> columns() const;

	//! �f�[�^���擾���܂��B
	const shared_ptr<vector<const vector<const Data>>> data() const;
};

class TokenReader
{
protected:
	const string alpahUnder = "_abcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPQRSTUVWXYZ"; //!< �S�ẴA���t�@�x�b�g�̑啶���������ƃA���_�[�o�[�ł��B
	const string alpahNumUnder = "_abcdefghijklmnopqrstuvwxzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"; //!< �S�Ă̐����ƃA���t�@�x�b�g�̑啶���������ƃA���_�[�o�[�ł��B
	const string num = "0123456789"; //!< �S�Ă̐����ł��B

	//! ���ۂɃg�[�N����ǂݍ��܂��B
	//! @param [in] cursol �ǂݍ��݊J�n�ʒu�ł��B
	//! @param [in] end SQL�S�̂̏I���ʒu�ł��B
	//! @return �؂�o���ꂽ�g�[�N���ł��B�ǂݍ��݂����s�����ꍇ��nullptr��Ԃ��܂��B
	virtual const shared_ptr<const Token> ReadCore(string::const_iterator &cursol, const string::const_iterator& end) const = 0;
public:
	//! �g�[�N����ǂݍ��݂܂��B
	//! @param [in] cursol �ǂݍ��݊J�n�ʒu�ł��B
	//! @param [in] end SQL�S�̂̏I���ʒu�ł��B
	//! @return �؂�o���ꂽ�g�[�N���ł��B�ǂݍ��݂����s�����ꍇ��nullptr��Ԃ��܂��B
	const shared_ptr<const Token> Read(string::const_iterator &cursol, const string::const_iterator& end) const;
};

//! ���l���e�����g�[�N����ǂݍ��ދ@�\��񋟂��܂��B
class IntLiteralReader : public TokenReader
{
protected:
	//! ���ۂɃg�[�N����ǂݍ��܂��B
	//! @param [in] cursol �ǂݍ��݊J�n�ʒu�ł��B
	//! @param [in] end SQL�S�̂̏I���ʒu�ł��B
	//! @return �؂�o���ꂽ�g�[�N���ł��B�ǂݍ��݂����s�����ꍇ��nullptr��Ԃ��܂��B
	const shared_ptr<const Token> ReadCore(string::const_iterator &cursol, const string::const_iterator& end) const override;
};

//! �����񃊃e�����g�[�N����ǂݍ��ދ@�\��񋟂��܂��B
class StringLiteralReader : public TokenReader
{
protected:
	//! ���ۂɃg�[�N����ǂݍ��݂܂��B
	//! @param [in] cursol �ǂݍ��݊J�n�ʒu�ł��B
	//! @param [in] end SQL�S�̂̏I���ʒu�ł��B
	//! @return �؂�o���ꂽ�g�[�N���ł��B�ǂݍ��݂����s�����ꍇ��nullptr��Ԃ��܂��B
	const shared_ptr<const Token> ReadCore(string::const_iterator &cursol, const string::const_iterator& end) const override;
};

//! �L�[���[�h�g�[�N����ǂݍ��ދ@�\��񋟂��܂��B
class KeywordReader : public TokenReader
{
protected:
	Token keyword; //!< �ǂݍ��ރL�[���[�h�g�[�N���Ɠ������g�[�N���ł��B

	//! ���ۂɃg�[�N����ǂݍ��݂܂��B
	//! @param [in] cursol �ǂݍ��݊J�n�ʒu�ł��B
	//! @param [in] end SQL�S�̂̏I���ʒu�ł��B
	//! @return �؂�o���ꂽ�g�[�N���ł��B�ǂݍ��݂����s�����ꍇ��nullptr��Ԃ��܂��B
	const shared_ptr<const Token> ReadCore(string::const_iterator &cursol, const string::const_iterator& end) const override;

	//! �L�[���[�h�̎��̕����̃`�F�b�N���s���܂��B
	//! @param [in] next �`�F�b�N�ΏۂƂȂ鎟�̕����̃C�e���[�^�ł��B
	//! @param [in] next end�C�e���[�^�ł��B
	virtual const bool CheckNextChar(const string::const_iterator& next, const string::const_iterator& end) const;
public:
	//! KeywordReader�N���X�̐V�����C���X�^���X�����������܂��B
	//! @param [in] kind �g�[�N���̎�ނł��B
	//! @param [in] word �L�[���[�h�̕�����ł��B
	KeywordReader(const TokenKind kind, const string word);
};

//! �L���g�[�N����ǂݍ��ދ@�\��񋟂��܂��B
class SignReader : public KeywordReader
{
protected:
	//! �L�[���[�h�̎��̕����̃`�F�b�N���s���܂��B
	//! @param [in] next �`�F�b�N�ΏۂƂȂ鎟�̕����̃C�e���[�^�ł��B
	//! @param [in] next end�C�e���[�^�ł��B
	const bool CheckNextChar(const string::const_iterator& next, const string::const_iterator& end) const override;

public:
	//! KeywordReader�N���X�̐V�����C���X�^���X�����������܂��B
	//! @param [in] kind �g�[�N���̎�ނł��B
	//! @param [in] word �L�[���[�h�̕�����ł��B
	SignReader(const TokenKind kind, const string word);
};

//! ���ʎq�g�[�N����ǂݍ��ދ@�\��񋟂��܂��B
class IdentifierReader : public TokenReader
{
protected:
	//! ���ۂɃg�[�N����ǂݍ��݂܂��B
	//! @param [in] cursol �ǂݍ��݊J�n�ʒu�ł��B
	//! @param [in] end SQL�S�̂̏I���ʒu�ł��B
	//! @return �؂�o���ꂽ�g�[�N���ł��B�ǂݍ��݂����s�����ꍇ��nullptr��Ԃ��܂��B
	const shared_ptr<const Token> ReadCore(string::const_iterator &cursol, const string::const_iterator& end) const override;
};

//! �o�͂���f�[�^���Ǘ����܂��B
class OutputData
{
	SqlQueryInfo queryInfo; //!< SQL�ɋL�q���ꂽ���e�ł��B
public:

	//! OutputData�N���X�̐V�����C���X�^���X�����������܂��B
	//! @param [in] queryInfo SQL�̏��ł��B
	OutputData(const SqlQueryInfo queryInfo);

	//! CSV�t�@�C���ɏo�̓f�[�^���������݂܂��B
	//! @param [in] outputFileName ���ʂ��o�͂���t�@�C���̃t�@�C�����ł��B
	//! @param [in] inputTables �t�@�C������ǂݎ�����f�[�^�ł��B
	void WriteCsv(const string outputFileName, const vector<const InputTable> &inputTables);
};

//! SqlQuery��Csv�ɑ΂�����o�͂������܂��B
class Csv
{
	const shared_ptr<const SqlQueryInfo> queryInfo; //!< SQL�ɋL�q���ꂽ���e�ł��B

	//! �t�@�C���X�g���[������J���}��؂�̈�s��ǂݍ��݂܂��B
	//! @param [in] inputFile �f�[�^��ǂݍ��ރt�@�C���X�g���[���ł��B
	//! @return �t�@�C������ǂݍ��񂾈�s���̃f�[�^�ł��B
	const shared_ptr<const vector<const string>> ReadLineData(ifstream &inputFile) const;

	//! ���̓t�@�C�����J���܂��B
	//! @param [in] filePath �J���t�@�C���̃t�@�C���p�X�ł��B
	//! @return ���̓t�@�C���������X�g���[���ł��B
	ifstream OpenInputFile(const string filePath) const;

	//! ���̓t�@�C������܂��B
	//! @param [in] inputFile ���̓t�@�C���������X�g���[���ł��B
	void CloseInputFile(ifstream &inputFile) const;

	//! ����CSV�̃w�b�_�s��ǂݍ��݂܂��B
	//! @param [in] inputFile ���̓t�@�C���������X�g���[���ł��B�J�����㉽���ǂݍ���ł��܂���B
	//! @param [in] tableName SQL�Ŏw�肳�ꂽ�e�[�u�����ł��B
	//! @return �t�@�C������ǂݎ�����w�b�_���ł��B
	const shared_ptr<const vector<const Column>> ReadHeader(ifstream &inputFile, const string tableName) const;

	//! ����CSV�̃f�[�^�s��ǂݍ��݂܂��B
	//! @param [in] inputFile ���̓t�@�C���������X�g���[���ł��B���łɃw�b�_�݂̂�ǂݍ��񂾌�ł��B
	//! @return �t�@�C������ǂݎ�����f�[�^�ł��B
	const shared_ptr<vector<const vector<const Data>>> ReadData(ifstream &inputFile) const;
public:

	//! Csv�N���X�̐V�����C���X�^���X�����������܂��B
	//! @param [in] queryInfo SQL�ɋL�q���ꂽ���e�ł��B
	Csv(const shared_ptr<const SqlQueryInfo> queryInfo);

	//! CSV�t�@�C��������̓f�[�^��ǂݎ��܂��B
	//! @return �t�@�C������ǂݎ�����f�[�^�ł��B
	const shared_ptr<const vector<const InputTable>> ReadCsv() const;

	//! CSV�t�@�C���ɏo�̓f�[�^���������݂܂��B
	//! @param [in] outputFileName ���ʂ��o�͂���t�@�C���̃t�@�C�����ł��B
	//! @param [in] inputTables �t�@�C������ǂݎ�����f�[�^�ł��B
	void WriteCsv(const string outputFileName, const vector<const InputTable> &inputTables) const;
};

//! �t�@�C���ɑ΂��Ď��s����SQL��\���N���X�ł��B
class SqlQuery
{
	const string space = " \t\r\n"; //!< �S�Ă̋󔒕����ł��B

	const vector<const shared_ptr<const TokenReader>> tokenReaders; //!< �g�[�N���̓ǂݍ��݃��W�b�N�̏W���ł��B
	const vector<const Operator> operators; //!< ���Z�q�̏��ł��B

	shared_ptr<Csv> csv; //!< CSV������Ǘ����܂��B

	//! SQL�̕����񂩂�g�[�N����؂�o���܂��B
	//! @param [in] sql �g�[�N���ɕ������錳�ƂȂ�SQL�ł��B
	//! @return �؂�o���ꂽ�g�[�N���ł��B
	const shared_ptr<const vector<const Token>> GetTokens(const string sql) const;

	//! �g�[�N������͂���SQL�̍\���Ŏw�肳�ꂽ�����擾���܂��B
	//! @param [in] tokens ��͂̑ΏۂƂȂ�g�[�N���ł��B
	//! @return ��͂������ʂ̏��ł��B
	const shared_ptr<const SqlQueryInfo> AnalyzeTokens(const vector<const Token> &tokens) const;
public:
	//! SqlQuery�N���X�̐V�����C���X�^���X�����������܂��B
	//! @param [in] sql ���s����SQL�ł��B
	SqlQuery(const string sql);

	//! �J�����g�f�B���N�g���ɂ���CSV�ɑ΂��A�ȈՓI��SQL�����s���A���ʂ��t�@�C���ɏo�͂��܂��B
	//! @param[in] outputFileName SQL�̎��s���ʂ�CSV�Ƃ��ďo�͂���t�@�C�����ł��B�g���q���܂݂܂��B
	void Execute(const string outputFileName);
};

// �ȏ�w�b�_�ɑ������镔���B

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

//! �f�[�^�̌����ɗ��p���邽�߁A�S�Ẵe�[�u���̗�̏���o�^���܂��B
//! @param [in] queryInfo SQL�ɋL�q���ꂽ���ł��B
//! @param [in] inputTables �t�@�C������ǂݎ�����f�[�^�ł��B
void Column::SetAllColumns(const vector<const InputTable> &inputTables)
{
	bool found = false;
	int i = 0;
	for (auto &inputTable : inputTables){
		for (auto &inputColumn : *inputTable.columns()){
			if (Equali(columnName, inputColumn.columnName) &&
				(tableName.empty() || // �e�[�u�������ݒ肳��Ă���ꍇ�̂݃e�[�u�����̔�r���s���܂��B
				Equali(tableName, inputColumn.tableName))){

				// ���Ɍ������Ă���̂ɂ��������������G���[�ł��B
				if (found){
					throw ResultValue::ERR_BAD_COLUMN_NAME;
				}
				found = true;
				// ���������l������̃f�[�^�𐶐����܂��B
				allColumnsIndex = i;
				outputName = inputColumn.columnName;
			}
			++i;
		}
	}
	// ���������Ȃ��Ă��G���[�ł��B
	if (!found){
		throw ResultValue::ERR_BAD_COLUMN_NAME;
	}
}

//! ExtensionTreeNode�N���X�̐V�����C���X�^���X�����������܂��B
ExtensionTreeNode::ExtensionTreeNode()
{
}

//! �S�Ă����l�ƂȂ��͐��l��ɕϊ����܂��B
void InputTable::InitializeIntegerColumn()
{
	for (size_t i = 0; i < columns()->size(); ++i){

		// �S�Ă̍s�̂����ɂ��āA�f�[�^�����񂩂畄���Ɛ��l�ȊO�̕�����T���܂��B
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

			// �����Ɛ����ȊO��������Ȃ���ɂ��ẮA���l��ɕϊ����܂��B
			for (auto& inputRow : *data()){
				inputRow[i] = Data(stoi(inputRow[i].string()));
			}
		}
	}
}

//! InputTable�N���X�̐V�����C���X�^���X�����������܂��B
//! @param [in] columns �ǂݍ��񂾃w�b�_���ł��B
//! @param [in] data �ǂݍ��񂾃f�[�^�ł��B
InputTable::InputTable(const shared_ptr<const vector<const Column>> columns, const shared_ptr<vector<const vector<const Data>>> data) : m_columns(columns), m_data(data)
{
	InitializeIntegerColumn();
}

//! ��̏����擾���܂��B
const shared_ptr<const vector<const Column>> InputTable::columns() const
{
	return m_columns;
}

//! �f�[�^���擾���܂��B
const shared_ptr<vector<const vector<const Data>>> InputTable::data() const
{
	return m_data;
}

//! �g�[�N����ǂݍ��݂܂��B
//! @param [in] cursol �ǂݍ��݊J�n�ʒu�ł��B
//! @param [in] end SQL�S�̂̏I���ʒu�ł��B
//! @return �؂�o���ꂽ�g�[�N���ł��B�ǂݍ��݂����s�����ꍇ��nullptr��Ԃ��܂��B
const shared_ptr<const Token> TokenReader::Read(string::const_iterator &cursol, const string::const_iterator &end) const
{
	auto backPoint = cursol;
	auto token = ReadCore(cursol, end);
	if (!token){
		cursol = backPoint;
	}
	return token;
}

//! ���ۂɃg�[�N����ǂݍ��݂܂��B
//! @param [in] cursol �ǂݍ��݊J�n�ʒu�ł��B
//! @param [in] end SQL�S�̂̏I���ʒu�ł��B
//! @return �؂�o���ꂽ�g�[�N���ł��B�ǂݍ��݂����s�����ꍇ��nullptr��Ԃ��܂��B
const shared_ptr<const Token> IntLiteralReader::ReadCore(string::const_iterator &cursol, const string::const_iterator &end) const
{
	auto start = cursol;
	cursol = find_if(cursol, end, [&](char c){return num.find(c) == string::npos; });
	if (start != cursol && (
		alpahUnder.find(*cursol) == string::npos || // �����̌�ɂ����Ɏ��ʎq�������͕̂���킵���̂Ő��l���e�����Ƃ͈����܂���B
		cursol == end)){
		return make_shared<Token>(TokenKind::INT_LITERAL, string(start, cursol));
	}
	else{
		return nullptr;
	}
}

//! ���ۂɃg�[�N����ǂݍ��݂܂��B
//! @param [in] cursol �ǂݍ��݊J�n�ʒu�ł��B
//! @param [in] end SQL�S�̂̏I���ʒu�ł��B
//! @return �؂�o���ꂽ�g�[�N���ł��B�ǂݍ��݂����s�����ꍇ��nullptr��Ԃ��܂��B
const shared_ptr<const Token> StringLiteralReader::ReadCore(string::const_iterator &cursol, const string::const_iterator &end) const
{
	auto start = cursol;
	// �����񃊃e�������J�n����V���O���N�H�[�g�𔻕ʂ��A�ǂݍ��݂܂��B
	if (*cursol == "\'"[0]){
		++cursol;
		// ���g���N�X����c�[����cccc�̓V���O���N�H�[�g�̕������e�������̃G�X�P�[�v��F�����Ȃ����߁A�������e�������g��Ȃ����Ƃŉ�����Ă��܂��B
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

//! ���ۂɃg�[�N����ǂݍ��݂܂��B
//! @param [in] cursol �ǂݍ��݊J�n�ʒu�ł��B
//! @param [in] end SQL�S�̂̏I���ʒu�ł��B
//! @return �؂�o���ꂽ�g�[�N���ł��B�ǂݍ��݂����s�����ꍇ��nullptr��Ԃ��܂��B
const shared_ptr<const Token> KeywordReader::ReadCore(string::const_iterator &cursol, const string::const_iterator &end) const
{
	auto result =
		mismatch(keyword.word.begin(), keyword.word.end(), cursol,
		[](const char keywordChar, const char sqlChar){return keywordChar == toupper(sqlChar); });

	if (result.first == keyword.word.end() && // �L�[���[�h�̍Ō�̕����܂œ����ł��B
		CheckNextChar(result.second, end)){ 
		cursol = result.second;
		return make_shared<Token>(keyword);
	}
	else{
		return nullptr;
	}
}

//! KeywordReader�N���X�̐V�����C���X�^���X�����������܂��B
//! @param [in] kind �g�[�N���̎�ނł��B
//! @param [in] word �L�[���[�h�̕�����ł��B
KeywordReader::KeywordReader(const TokenKind kind, const string word) : keyword(Token(kind, word)){}

//! �L�[���[�h�̎��̕����̃`�F�b�N���s���܂��B
//! @param [in] next �`�F�b�N�ΏۂƂȂ鎟�̕����̃C�e���[�^�ł��B
//! @param [in] next end�C�e���[�^�ł��B
const bool KeywordReader::CheckNextChar(const string::const_iterator& next, const string::const_iterator& end) const
{
	//�L�[���[�h�Ɏ��ʎq����؂�Ȃ��ɑ����Ă��Ȃ������m�F���܂��B 
	return next != end && alpahNumUnder.find(*next) == string::npos;
}

//! KeywordReader�N���X�̐V�����C���X�^���X�����������܂��B
//! @param [in] kind �g�[�N���̎�ނł��B
//! @param [in] word �L�[���[�h�̕�����ł��B
SignReader::SignReader(const TokenKind kind, const string word) : KeywordReader(kind, word){}

//! �L�[���[�h�̎��̕����̃`�F�b�N���s���܂��B
//! @param [in] next �`�F�b�N�ΏۂƂȂ鎟�̕����̃C�e���[�^�ł��B
//! @param [in] next end�C�e���[�^�ł��B
const bool SignReader::CheckNextChar(const string::const_iterator& next, const string::const_iterator& end) const
{
	// ���̕����̓`�F�b�N�����ɕK��OK�ƂȂ�܂��B
	return true;
}

//! ���ۂɃg�[�N����ǂݍ��݂܂��B
//! @param [in] cursol �ǂݍ��݊J�n�ʒu�ł��B
//! @param [in] end SQL�S�̂̏I���ʒu�ł��B
//! @return �؂�o���ꂽ�g�[�N���ł��B�ǂݍ��݂����s�����ꍇ��nullptr��Ԃ��܂��B
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

//! OutputData�N���X�̐V�����C���X�^���X�����������܂��B
//! @param [in] queryInfo SQL�̏��ł��B
OutputData::OutputData(const SqlQueryInfo queryInfo) : queryInfo(queryInfo)
{
}

//! CSV�t�@�C���ɏo�̓f�[�^���������݂܂��B
//! @param [in] outputFileName ���ʂ��o�͂���t�@�C���̃t�@�C�����ł��B
//! @param [in] inputTables �t�@�C������ǂݎ�����f�[�^�ł��B
void OutputData::WriteCsv(const string outputFileName, const vector<const InputTable> &inputTables)
{
	ofstream outputFile; // �������ރt�@�C���̃t�@�C���|�C���^�ł��B

	vector<Column> allInputColumns; // ���͂Ɋ܂܂�邷�ׂĂ̗�̈ꗗ�ł��B

	vector<vector<Data>> outputData; // �o�̓f�[�^�ł��B
	vector<vector<Data>> allColumnOutputData; // �o�͂���f�[�^�ɑΉ�����C���f�b�N�X�������A���ׂĂ̓��̓f�[�^��ۊǂ��܂��B

	// ���̓t�@�C���ɏ����Ă��������ׂĂ̗��allInputColumns�ɐݒ肵�܂��B
	for (size_t i = 0; i < queryInfo.tableNames.size(); ++i){
		transform(
			inputTables[i].columns()->begin(),
			inputTables[i].columns()->end(),
			back_inserter(allInputColumns),
			[&](const Column& column){return Column(queryInfo.tableNames[i], column.columnName); });
	}

	// SELECT��̗񖼎w�肪*�������ꍇ�́A����CSV�̗񖼂����ׂđI������܂��B
	if (queryInfo.selectColumns.empty()){
		copy(allInputColumns.begin(), allInputColumns.end(), back_inserter(queryInfo.selectColumns));
	}

	// SELECT��Ŏw�肳�ꂽ�񖼂��A���ڂ̓��̓t�@�C���̉���ڂɑ������邩�𔻕ʂ��܂��B
	for (auto &selectColumn : queryInfo.selectColumns){
		selectColumn.SetAllColumns(inputTables);
	}

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

	vector<vector<const vector<const Data>>::const_iterator> currentRows; // ���͂��ꂽ�e�e�[�u���́A���ݏo�͂��Ă���s���w���J�[�\���ł��B
	transform(
		inputTables.begin(),
		inputTables.end(),
		back_inserter(currentRows),
		[](const InputTable& table){return table.data()->begin(); });

	// �o�͂���f�[�^��ݒ肵�܂��B
	while (true){
		outputData.push_back(vector<Data>());
		vector<Data> &row = outputData.back(); // �o�͂��Ă����s���̃f�[�^�ł��B

		allColumnOutputData.push_back(vector<Data>());
		vector<Data> &allColumnsRow = allColumnOutputData.back();// WHERE��ORDER�̂��߂ɂ��ׂĂ̏����܂ލs�Brow�ƃC���f�b�N�X�����L���܂��B

		// allColumnsRow�̗��ݒ肵�܂��B
		for (auto &currentRow : currentRows){
			copy(
				currentRow->begin(),
				currentRow->end(),
				back_inserter(allColumnsRow));
		}

		// �s�̊e��̃f�[�^��ݒ肵�܂��B
		transform(
			queryInfo.selectColumns.begin(),
			queryInfo.selectColumns.end(),
			back_inserter(row),
			[&](const Column& column){return allColumnsRow[column.allColumnsIndex]; });

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
						bool found = false;
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
		for (int i = queryInfo.tableNames.size() - 1; currentRows[i] == inputTables[i].data()->end() && 0 < i; --i){
			++currentRows[i - 1];
			currentRows[i] = inputTables[i].data()->begin();
		}

		// �ŏ��̃e�[�u�����Ō�̍s�𒴂����Ȃ�o�͍s�̐����͏I���ł��B
		if (currentRows[0] == inputTables[0].data()->end()){
			break;
		}
	}

	// ORDER��ɂ����ёւ��̏������s���܂��B
	if (!queryInfo.orderByColumns.empty()){
		// ORDER��Ŏw�肳��Ă���񂪁A�S�Ă̓��͍s�̒��̂ǂ̍s�Ȃ̂����v�Z���܂��B
		vector<int> orderByColumnIndexes; // ORDER��Ŏw�肳�ꂽ��́A���ׂĂ̍s�̒��ł̃C���f�b�N�X�ł��B

		for (auto &orderByColumn : queryInfo.orderByColumns){
			bool found = false;
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
	outputFile = ofstream(outputFileName);
	if (outputFile.bad()){
		throw ResultValue::ERR_FILE_OPEN;
	}

	// �o�̓t�@�C���ɗ񖼂��o�͂��܂��B
	for (size_t i = 0; i < queryInfo.selectColumns.size(); ++i){
		outputFile << queryInfo.selectColumns[i].outputName;
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
	if (outputFile){
		outputFile.close();
		if (outputFile.bad()){
			throw ResultValue::ERR_FILE_CLOSE;
		}
	}
}

//! CSV�t�@�C���ɏo�̓f�[�^���������݂܂��B
//! @param [in] outputFileName ���ʂ��o�͂���t�@�C���̃t�@�C�����ł��B
//! @param [in] queryInfo SQL�̏��ł��B
//! @param [in] inputTables �t�@�C������ǂݎ�����f�[�^�ł��B
void Csv::WriteCsv(const string outputFileName, const vector<const InputTable> &inputTables) const
{
	OutputData output(*queryInfo);
	output.WriteCsv(outputFileName, inputTables);
}

//! �t�@�C���X�g���[������J���}��؂�̈�s��ǂݍ��݂܂��B
//! @param [in] inputFile �f�[�^��ǂݍ��ރt�@�C���X�g���[���ł��B
//! @return �t�@�C������ǂݍ��񂾈�s���̃f�[�^�ł��B
const shared_ptr<const vector<const string>> Csv::ReadLineData(ifstream &inputFile) const
{
	string inputLine; // �t�@�C������ǂݍ��񂾍s������ł��B
	if (getline(inputFile, inputLine)){
		auto lineData = make_shared<vector<const string>>(); // ��s���̃f�[�^�ł��B

		auto charactorCursol = inputLine.begin(); // �w�b�_���͍s����������J�[�\���ł��B
		auto lineEnd = inputLine.end(); // �w�b�_���͍s��end���w���܂��B

		// �ǂݍ��񂾍s���Ō�܂œǂ݂܂��B
		while (charactorCursol != lineEnd){

			// �񖼂���ǂ݂܂��B
			auto columnStart = charactorCursol; // ���݂̗�̍ŏ����L�^���Ă����܂��B
			charactorCursol = find(charactorCursol, lineEnd, ',');
			lineData->push_back(string(columnStart, charactorCursol));

			// ���͍s�̃J���}�̕���ǂݐi�߂܂��B
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

//! ����CSV�̃w�b�_�s��ǂݍ��݂܂��B
//! @param [in] inputFile ���̓t�@�C���������X�g���[���ł��B�J�����㉽���ǂݍ���ł��܂���B
//! @param [in] tableName SQL�Ŏw�肳�ꂽ�e�[�u�����ł��B
//! @return �t�@�C������ǂݎ�����w�b�_���ł��B
const shared_ptr<const vector<const Column>> Csv::ReadHeader(ifstream &inputFile, const string tableName) const
{
	auto columns = make_shared<vector<const Column>>(); // �ǂݍ��񂾗�̈ꗗ�B

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
//! ����CSV�̃f�[�^�s��ǂݍ��݂܂��B
//! @param [in] inputFile ���̓t�@�C���������X�g���[���ł��B���łɃw�b�_�݂̂�ǂݍ��񂾌�ł��B
//! @return �t�@�C������ǂݎ�����f�[�^�ł��B
const shared_ptr<vector<const vector<const Data>>> Csv::ReadData(ifstream &inputFile) const
{
	auto data = make_shared<vector<const vector<const Data>>>(); // �ǂݍ��񂾃f�[�^�̈ꗗ�B

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

//! ���̓t�@�C�����J���܂��B
//! @param [in] filePath �J���t�@�C���̃t�@�C���p�X�ł��B
//! @return ���̓t�@�C���������X�g���[���ł��B
ifstream Csv::OpenInputFile(const string filePath) const
{
	auto inputFile = ifstream(filePath); //���͂���CSV�t�@�C���������X�g���[���ł��B
	if (!inputFile){
		throw ResultValue::ERR_FILE_OPEN;
	}
	return inputFile;
}

//! ���̓t�@�C������܂��B
//! @param [in] inputFile ���̓t�@�C���������X�g���[���ł��B
void Csv::CloseInputFile(ifstream &inputFile) const
{
	inputFile.close();
	if (inputFile.bad()){
		throw ResultValue::ERR_FILE_CLOSE;
	}
}

//! Csv�N���X�̐V�����C���X�^���X�����������܂��B
//! @param [in] queryInfo SQL�ɋL�q���ꂽ���e�ł��B
Csv::Csv(const shared_ptr<const SqlQueryInfo> queryInfo) : queryInfo(queryInfo){}

//! CSV�t�@�C��������̓f�[�^��ǂݎ��܂��B
//! @return �t�@�C������ǂݎ�����f�[�^�ł��B
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

//! SQL�̕����񂩂�g�[�N����؂�o���܂��B
//! @param [in] sql �g�[�N���ɕ������錳�ƂȂ�SQL�ł��B
//! @return �؂�o���ꂽ�g�[�N���ł��B
const shared_ptr<const vector<const Token>> SqlQuery::GetTokens(const string sql) const
{
	auto cursol = sql.begin(); // SQL���g�[�N���ɕ������ēǂݍ��ގ��Ɍ��ݓǂ�ł��镶���̏ꏊ��\���܂��B
	auto end = sql.end(); // sql��end���w���܂��B
	auto tokens = make_shared<vector<const Token>>(); //�ǂݍ��񂾃g�[�N���ł��B

	// SQL���g�[�N���ɕ����ēǂݍ��݂܂��B
	while (cursol != end){

		// �󔒂�ǂݔ�΂��܂��B
		cursol = find_if(cursol, end, [&](char c){return space.find(c) == string::npos; });
		if (cursol == end){
			break;
		}
		// �e��g�[�N����ǂݍ��݂܂��B
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

//! �g�[�N������͂���SQL�̍\���Ŏw�肳�ꂽ�����擾���܂��B
//! @param [in] tokens ��͂̑ΏۂƂȂ�g�[�N���ł��B
//! @return ��͂������ʂ̏��ł��B
const shared_ptr<const SqlQueryInfo> SqlQuery::AnalyzeTokens(const vector<const Token> &tokens) const
{
	auto queryInfo = make_shared<SqlQueryInfo>();
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
				queryInfo->selectColumns.push_back(Column(tokenCursol->word));
				++tokenCursol;
				if (tokenCursol->kind == TokenKind::DOT){
					++tokenCursol;
					if (tokenCursol->kind == TokenKind::IDENTIFIER){

						// �e�[�u�������w�肳��Ă��邱�Ƃ��킩�����̂œǂݑւ��܂��B
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
						queryInfo->orderByColumns.push_back(Column(tokenCursol->word));
						++tokenCursol;
						if (tokenCursol->kind == TokenKind::DOT){
							++tokenCursol;
							if (tokenCursol->kind == TokenKind::IDENTIFIER){

								// �e�[�u�������w�肳��Ă��邱�Ƃ��킩�����̂œǂݑւ��܂��B
								queryInfo->orderByColumns.back() = Column(queryInfo->orderByColumns.back().columnName, tokenCursol->word);
								++tokenCursol;
							}
							else{
								throw ResultValue::ERR_SQL_SYNTAX;
							}
						}

						// ���ёւ��̏����A�~�����w�肵�܂��B
						if (tokenCursol->kind == TokenKind::ASC){
							queryInfo->orders.push_back(TokenKind::ASC);
							++tokenCursol;
						}
						else if (tokenCursol->kind == TokenKind::DESC){
							queryInfo->orders.push_back(TokenKind::DESC);
							++tokenCursol;
						}
						else{
							// �w�肪�Ȃ��ꍇ�͏����ƂȂ�܂��B
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

		// WHERE���ǂݍ��݂܂��B
		if (tokenCursol->kind == TokenKind::WHERE){
			readWhere = true;
			++tokenCursol;
			shared_ptr<ExtensionTreeNode> currentNode; // ���ݓǂݍ���ł���m�[�h�ł��B
			while (true){
				// �I�y�����h��ǂݍ��݂܂��B

				// �I�y�����h�̃m�[�h��V�����������܂��B
				queryInfo->whereExtensionNodes.push_back(make_shared<ExtensionTreeNode>());
				if (currentNode){
					// ���݂̃m�[�h���E�̎q�ɂ��炵�A���̈ʒu�ɐV�����m�[�h��}�����܂��B
					currentNode->right = queryInfo->whereExtensionNodes.back();
					currentNode->right->parent = currentNode;
					currentNode = currentNode->right;
				}
				else{
					// �ŏ��̓J�����g�m�[�h�ɐV�����m�[�h�����܂��B
					currentNode = queryInfo->whereExtensionNodes.back();
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
					queryInfo->whereExtensionNodes.push_back(make_shared<ExtensionTreeNode>());
					currentNode = queryInfo->whereExtensionNodes.back();
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
			queryInfo->whereTopNode = currentNode;
			while (queryInfo->whereTopNode->parent){
				queryInfo->whereTopNode = queryInfo->whereTopNode->parent;
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
			queryInfo->tableNames.push_back(tokenCursol->word);
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

	return queryInfo;
}

//! SqlQuery�N���X�̐V�����C���X�^���X�����������܂��B
//! @param [in] sql ���s����SQL�ł��B
SqlQuery::SqlQuery(const string sql) :
// �擪���珇�Ɍ��������̂ŁA�O����v�ƂȂ��̍��ڂ͏��ԂɋC�����ēo�^���Ȃ��Ă͂����܂���B
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

//! �J�����g�f�B���N�g���ɂ���CSV�ɑ΂��A�ȈՓI��SQL�����s���A���ʂ��t�@�C���ɏo�͂��܂��B
//! @param[in] outputFileName SQL�̎��s���ʂ�CSV�Ƃ��ďo�͂���t�@�C�����ł��B�g���q���܂݂܂��B
void SqlQuery::Execute(const string outputFileName)
{
	csv->WriteCsv(outputFileName, *csv->ReadCsv());
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
	try
	{
		SqlQuery(sql).Execute(outputFileName);
		return static_cast<int>(ResultValue::OK);
	}
	catch (ResultValue error) // ���������G���[�̎�ނł��B
	{
		return static_cast<int>(error);
	}
}