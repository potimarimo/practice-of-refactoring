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
	DataType m_type = DataType::STRING; //!< �f�[�^�̌^�ł��B

	const string defaultString = ""; //!< �f�[�^��������������Ȃ��ꍇ��string()���Ԃ��l�B
protected:
	//! Data�N���X�̐V�����C���X�^���X�����������܂��B
	Data();
public:
	//! Data�^�̋�ۃN���X�̃C���X�^���X��Ԃ��܂��B
	//! @param [in] value �f�[�^�̎��ۂ̒l�ł��B
	static shared_ptr<Data> New(const string value);

	//! Data�^�̋�ۃN���X�̃C���X�^���X��Ԃ��܂��B
	//! @param [in] value �f�[�^�̎��ۂ̒l�ł��B
	static shared_ptr<Data> New(const int value);

	//! Data�^�̋�ۃN���X�̃C���X�^���X��Ԃ��܂��B
	//! @param [in] value �f�[�^�̎��ۂ̒l�ł��B
	static shared_ptr<Data> New(const bool value);

	//! �f�[�^�̌^���擾���܂��B
	//! @return �f�[�^�̌^�ł��B
	virtual const DataType type() const = 0;

	//! �f�[�^��������^�̏ꍇ�̒l���擾���܂��B
	//! @return �f�[�^��������^�̏ꍇ�̒l�ł��B
	virtual const string& string() const;

	//! �f�[�^�������^�̏ꍇ�̒l���擾���܂��B
	//! @return �f�[�^�������^�̏ꍇ�̒l�ł��B
	virtual const int integer() const;

	//! �f�[�^���^�U�l�^�̏ꍇ�̒l���擾���܂��B
	//! @return �f�[�^���^�U�l�^�̏ꍇ�̒l�ł��B
	virtual const bool boolean() const;

	//! ���Z���Z���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ���Z�������ʂł��B
	virtual const shared_ptr<const Data> operator+(const shared_ptr<const Data>& right) const = 0;

	//! ���Z���Z���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ���Z�������ʂł��B
	virtual const shared_ptr<const Data> operator-(const shared_ptr<const Data>& right) const = 0;

	//! ��Z���Z���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ��Z�������ʂł��B
	virtual const shared_ptr<const Data> operator*(const shared_ptr<const Data>& right) const = 0;

	//! ���Z���Z���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ���Z�������ʂł��B
	virtual const shared_ptr<const Data> operator/(const shared_ptr<const Data>& right) const = 0;

	//! ���l��r���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ��r�������ʂł��B
	virtual const shared_ptr<const Data> operator==(const shared_ptr<const Data>& right) const = 0;

	//! �s����r���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ��r�������ʂł��B
	virtual const shared_ptr<const Data> operator!=(const shared_ptr<const Data>& right) const = 0;

	//! �ȏ��r���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ��r�������ʂł��B
	virtual const shared_ptr<const Data> operator>=(const shared_ptr<const Data>& right) const = 0;

	//! �傫����r���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ��r�������ʂł��B
	virtual const shared_ptr<const Data> operator>(const shared_ptr<const Data>& right) const = 0;

	//! �ȉ���r���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ��r�������ʂł��B
	virtual const shared_ptr<const Data> operator<=(const shared_ptr<const Data>& right) const = 0;

	//! ��������r���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ��r�������ʂł��B
	virtual const shared_ptr<const Data> operator<(const shared_ptr<const Data>& right) const = 0;

	//! AND���Z���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ���Z�������ʂł��B
	virtual const shared_ptr<const Data> operator&&(const shared_ptr<const Data>& right) const = 0;

	//! OR���Z���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ���Z�������ʂł��B
	virtual const shared_ptr<const Data> operator||(const shared_ptr<const Data>& right) const = 0;
};

//! ������̒l������Data�ł��B
class StringData : public Data
{
	std::string m_string; //!< �f�[�^��������^�̏ꍇ�̒l�ł��B

public:
	//! Data�N���X�̐V�����C���X�^���X�����������܂��B
	//! @param [in] value �f�[�^�̒l�ł��B
	StringData(const std::string value);

	//! �f�[�^�̌^���擾���܂��B
	//! @return �f�[�^	�̌^�ł��B
	const DataType type() const override;

	//! �f�[�^��������^�̏ꍇ�̒l���擾���܂��B
	//! @return �f�[�^��������^�̏ꍇ�̒l�ł��B
	const std::string& string() const override;

	//! ���Z���Z���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ���Z�������ʂł��B
	const shared_ptr<const Data> operator+(const shared_ptr<const Data>& right) const override;

	//! ���Z���Z���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ���Z�������ʂł��B
	const shared_ptr<const Data> operator-(const shared_ptr<const Data>& right) const override;

	//! ��Z���Z���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ��Z�������ʂł��B
	const shared_ptr<const Data> operator*(const shared_ptr<const Data>& right) const override;

	//! ���Z���Z���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ���Z�������ʂł��B
	const shared_ptr<const Data> operator/(const shared_ptr<const Data>& right) const override;

	//! ���l��r���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ��r�������ʂł��B
	const shared_ptr<const Data> operator==(const shared_ptr<const Data>& right) const override;

	//! �s����r���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ��r�������ʂł��B
	const shared_ptr<const Data> operator!=(const shared_ptr<const Data>& right) const override;

	//! �ȏ��r���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ��r�������ʂł��B
	const shared_ptr<const Data> operator>=(const shared_ptr<const Data>& right) const override;

	//! �傫����r���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ��r�������ʂł��B
	const shared_ptr<const Data> operator>(const shared_ptr<const Data>& right) const override;

	//! �ȉ���r���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ��r�������ʂł��B
	const shared_ptr<const Data> operator<=(const shared_ptr<const Data>& right) const override;

	//! ��������r���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ��r�������ʂł��B
	const shared_ptr<const Data> operator<(const shared_ptr<const Data>& right) const override;

	//! AND���Z���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ���Z�������ʂł��B
	const shared_ptr<const Data> operator&&(const shared_ptr<const Data>& right) const override;

	//! OR���Z���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ���Z�������ʂł��B
	const shared_ptr<const Data> operator||(const shared_ptr<const Data>& right) const override;
};

//! �����̒l������Data�ł��B
class IntegerData : public Data
{
	int m_integer;                  //!< �f�[�^�������^�̏ꍇ�̒l�ł��B

public:
	//! Data�N���X�̐V�����C���X�^���X�����������܂��B
	//! @param [in] value �f�[�^�̒l�ł��B
	IntegerData(const int value);

	//! �f�[�^�̌^���擾���܂��B
	//! @return �f�[�^�̌^�ł��B
	const DataType type() const override;

	//! �f�[�^�������^�̏ꍇ�̒l���擾���܂��B
	//! @return �f�[�^�������^�̏ꍇ�̒l�ł��B
	const int integer() const override;

	//! ���Z���Z���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ���Z�������ʂł��B
	const shared_ptr<const Data> operator+(const shared_ptr<const Data>& right) const override;

	//! ���Z���Z���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ���Z�������ʂł��B
	const shared_ptr<const Data> operator-(const shared_ptr<const Data>& right) const override;

	//! ��Z���Z���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ��Z�������ʂł��B
	const shared_ptr<const Data> operator*(const shared_ptr<const Data>& right) const override;

	//! ���Z���Z���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ���Z�������ʂł��B
	const shared_ptr<const Data> operator/(const shared_ptr<const Data>& right) const override;

	//! ���l��r���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ��r�������ʂł��B
	const shared_ptr<const Data> operator==(const shared_ptr<const Data>& right) const override;

	//! �s����r���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ��r�������ʂł��B
	const shared_ptr<const Data> operator!=(const shared_ptr<const Data>& right) const override;

	//! �ȏ��r���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ��r�������ʂł��B
	const shared_ptr<const Data> operator>=(const shared_ptr<const Data>& right) const override;

	//! �傫����r���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ��r�������ʂł��B
	const shared_ptr<const Data> operator>(const shared_ptr<const Data>& right) const override;

	//! �ȉ���r���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ��r�������ʂł��B
	const shared_ptr<const Data> operator<=(const shared_ptr<const Data>& right) const override;

	//! ��������r���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ��r�������ʂł��B
	const shared_ptr<const Data> operator<(const shared_ptr<const Data>& right) const override;

	//! AND���Z���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ���Z�������ʂł��B
	const shared_ptr<const Data> operator&&(const shared_ptr<const Data>& right) const override;

	//! OR���Z���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ���Z�������ʂł��B
	const shared_ptr<const Data> operator||(const shared_ptr<const Data>& right) const override;
};

//! �^�U�l�̒l������Data�ł��B
class BooleanData : public Data
{
	bool m_boolean;                  //!< �f�[�^���^�U�l�^�̏ꍇ�̒l�ł��B

public:
	//! Data�N���X�̐V�����C���X�^���X�����������܂��B
	//! @param [in] value �f�[�^�̒l�ł��B
	BooleanData(const bool value);

	//! �f�[�^�̌^���擾���܂��B
	//! @return �f�[�^�̌^�ł��B
	const DataType type() const override;

	//! �f�[�^�������^�̏ꍇ�̒l���擾���܂��B
	//! @return �f�[�^�������^�̏ꍇ�̒l�ł��B
	const bool boolean() const override;

	//! ���Z���Z���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ���Z�������ʂł��B
	const shared_ptr<const Data> operator+(const shared_ptr<const Data>& right) const override;

	//! ���Z���Z���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ���Z�������ʂł��B
	const shared_ptr<const Data> operator-(const shared_ptr<const Data>& right) const override;

	//! ��Z���Z���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ��Z�������ʂł��B
	const shared_ptr<const Data> operator*(const shared_ptr<const Data>& right) const override;

	//! ���Z���Z���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ���Z�������ʂł��B
	const shared_ptr<const Data> operator/(const shared_ptr<const Data>& right) const override;

	//! ���l��r���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ��r�������ʂł��B
	const shared_ptr<const Data> operator==(const shared_ptr<const Data>& right) const override;

	//! �s����r���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ��r�������ʂł��B
	const shared_ptr<const Data> operator!=(const shared_ptr<const Data>& right) const override;

	//! �ȏ��r���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ��r�������ʂł��B
	const shared_ptr<const Data> operator>=(const shared_ptr<const Data>& right) const override;

	//! �傫����r���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ��r�������ʂł��B
	const shared_ptr<const Data> operator>(const shared_ptr<const Data>& right) const override;

	//! �ȉ���r���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ��r�������ʂł��B
	const shared_ptr<const Data> operator<=(const shared_ptr<const Data>& right) const override;

	//! ��������r���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ��r�������ʂł��B
	const shared_ptr<const Data> operator<(const shared_ptr<const Data>& right) const override;

	//! AND���Z���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ���Z�������ʂł��B
	const shared_ptr<const Data> operator&&(const shared_ptr<const Data>& right) const override;

	//! OR���Z���s���܂��B
	//! @param [in] right �E�ӂł��B
	//! @return ���Z�������ʂł��B
	const shared_ptr<const Data> operator||(const shared_ptr<const Data>& right) const override;
};

//! ���Z���Z���s���܂��B
//! @param [in] left ���ӂł��B
//! @param [in] right �E�ӂł��B
//! @return ���Z�������ʂł��B
const shared_ptr<const Data> operator+(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right);

//! ���Z���Z���s���܂��B
//! @param [in] left ���ӂł��B
//! @param [in] right �E�ӂł��B
//! @return ���Z�������ʂł��B
const shared_ptr<const Data> operator-(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right);


//! ��Z���Z���s���܂��B
//! @param [in] left ���ӂł��B
//! @param [in] right �E�ӂł��B
//! @return ��Z�������ʂł��B
const shared_ptr<const Data> operator*(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right);

//! ���Z���Z���s���܂��B
//! @param [in] left ���ӂł��B
//! @param [in] right �E�ӂł��B
//! @return ���Z�������ʂł��B
const shared_ptr<const Data> operator/(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right);

//! ���l��r���s���܂��B
//! @param [in] left ���ӂł��B
//! @param [in] right �E�ӂł��B
//! @return ��r�������ʂł��B
const shared_ptr<const Data> operator==(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right);

//! �s����r���s���܂��B
//! @param [in] left ���ӂł��B
//! @param [in] right �E�ӂł��B
//! @return ��r�������ʂł��B
const shared_ptr<const Data> operator!=(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right);

//! �ȏ��r���s���܂��B
//! @param [in] left ���ӂł��B
//! @param [in] right �E�ӂł��B
//! @return ��r�������ʂł��B
const shared_ptr<const Data> operator>=(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right);

//! �傫����r���s���܂��B
//! @param [in] left ���ӂł��B
//! @param [in] right �E�ӂł��B
//! @return ��r�������ʂł��B
const shared_ptr<const Data> operator>(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right);

//! �ȉ���r���s���܂��B
//! @param [in] left ���ӂł��B
//! @param [in] right �E�ӂł��B
//! @return ��r�������ʂł��B
const shared_ptr<const Data> operator<=(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right);

//! ��������r���s���܂��B
//! @param [in] left ���ӂł��B
//! @param [in] right �E�ӂł��B
//! @return ��r�������ʂł��B
const shared_ptr<const Data> operator<(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right);

//! AND���Z���s���܂��B
//! @param [in] left ���ӂł��B
//! @param [in] right �E�ӂł��B
//! @return ���Z�������ʂł��B
const shared_ptr<const Data> operator&&(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right);

//! OR���Z���s���܂��B
//! @param [in] left ���ӂł��B
//! @param [in] right �E�ӂł��B
//! @return ���Z�������ʂł��B
const shared_ptr<const Data> operator||(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right);
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
	//! �J�������Ŏw�肳�ꂽ�f�[�^�����m�[�h���ǂ����ł��B
	//! @return �J�������Ŏw�肳�ꂽ�f�[�^�����m�[�h���ǂ����B
	bool ExtensionTreeNode::isDataNodeAsColumnName();
public:
	shared_ptr<ExtensionTreeNode> parent;//!< �e�ƂȂ�m�[�h�ł��B���̎��؂̏ꍇ��nullptr�ƂȂ�܂��B
	shared_ptr<ExtensionTreeNode> left;  //!< ���̎q�ƂȂ�m�[�h�ł��B���g�����[�̗t�ƂȂ鎮�؂̏ꍇ��nullptr�ƂȂ�܂��B
	Operator middleOperator;             //!< ���u����鉉�Z�q�ł��B���g�����[�̂ƂȂ鎮�؂̏ꍇ�̎�ނ�NOT_TOKEN�ƂȂ�܂��B
	shared_ptr<ExtensionTreeNode>right;   //!< �E�̎q�ƂȂ�m�[�h�ł��B���g�����[�̗t�ƂȂ鎮�؂̏ꍇ��nullptr�ƂȂ�܂��B
	bool inParen = false;                //!< ���g���������ɂ���܂�Ă��邩�ǂ����ł��B
	int parenOpenBeforeClose = 0;        //!< �؂̍\�z����0�ȊO�ƂȂ�A���g�̍��ɂ���A�܂����ĂȂ��J�b�R�̊J�n�̐��ƂȂ�܂��B
	int signCoefficient = 1;             //!< ���g���t�ɂ���A�}�C�i�X�P�����Z�q�����Ă���ꍇ��-1�A����ȊO��1�ƂȂ�܂��B
	Column column;                       //!< ���w�肳��Ă���ꍇ�ɁA���̗��\���܂��B��w��ł͂Ȃ��ꍇ��columnName���󕶎���ƂȂ�܂��B
	shared_ptr<const Data> value;                          //!< �w�肳�ꂽ�A�������͌v�Z���ꂽ�l�ł��B

	//! ExtensionTreeNode�N���X�̐V�����C���X�^���X�����������܂��B
	ExtensionTreeNode();

	// left��right��middleOperator�ŉ��Z���܂��B
	void Operate();

	//! ���ۂɏo�͂���s�ɍ��킹�ė�Ƀf�[�^��ݒ肵�܂��B
	//! @param [in] inputTables �t�@�C������ǂݎ�����f�[�^�ł��B
	//! @param [in] ���ۂɏo�͂���s�ł��B
	void SetColumnData(const vector<const shared_ptr<const Data>> &outputRow);
};

//! �����Ƃ��ēn�����m�[�h�y�т��̎q���̃m�[�h���擾���܂��B�����͋A�肪�����ł��B
//! @param [in] �߂�l�̃��[�g�ƂȂ�m�[�h�ł��B
//! @return ���g�y�юq���̃m�[�h�ł��B
const shared_ptr<vector<const shared_ptr<ExtensionTreeNode>>> SelfAndDescendants(shared_ptr<ExtensionTreeNode>);

//! Order��̗�Ə����̎w���\���܂��B
class Order
{
public:
	Column column; //<! ORDER��Ɏw�肳�ꂽ�񖼂ł��B
	const bool isAsc = true; //<! ORDER���w�肳�ꂽ�������������ǂ����ł��B

	//! Order�N���X�̐V�����C���X�^���X�����������܂��B
	//! @param [in] column ORDER��Ɏw�肳�ꂽ�񖼂ł��B
	//! @param [in] isAsc ORDER���w�肳�ꂽ�������������ǂ����ł��B
	Order(Column column, const bool isAsc);
};

//! SqlQuery�̍\�����������N���X�ł��B
class SqlQueryInfo
{
public:
	vector<const string> tableNames; //!< FROM��Ŏw�肵�Ă���e�[�u�����ł��B
	vector<Column> selectColumns; //!< SELECT��Ɏw�肳�ꂽ�񖼂ł��B
	vector<Order> orders; //!< ORDER��Ɏw�肳�ꂽ�����̏��ł��B
	shared_ptr<ExtensionTreeNode> whereTopNode; //!< ���؂̍��ƂȂ�m�[�h�ł��B
};

//! CSV�Ƃ��ē��͂��ꂽ�t�@�C���̓��e��\���܂��B
class InputTable
{
	const string signNum = "+-0123456789"; //!< �S�Ă̕����Ɛ����ł��B

	const shared_ptr<const vector<const Column>> m_columns; //!< ��̏��ł��B
	const shared_ptr<vector<const vector<const shared_ptr<const Data>>>> m_data; //! �f�[�^�ł��B

	//! �S�Ă����l�ƂȂ��͐��l��ɕϊ����܂��B
	void InputTable::InitializeIntegerColumn();
public:
	//! InputTable�N���X�̐V�����C���X�^���X�����������܂��B
	//! @param [in] columns �ǂݍ��񂾃w�b�_���ł��B
	//! @param [in] data �ǂݍ��񂾃f�[�^�ł��B
	InputTable(const shared_ptr<const vector<const Column>> columns, const shared_ptr<vector<const vector<const shared_ptr<const Data>>>> data);

	//! ��̏����擾���܂��B
	//! @return ��̏��ł��B
	const shared_ptr<const vector<const Column>> columns() const;

	//! �f�[�^���擾���܂��B
	//! @return �f�[�^�ł��B
	const shared_ptr<vector<const vector<const shared_ptr<const Data>>>> data() const;
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

class SequenceParser;
//! ���܂��܂ȃp�[�T�[�̊��N���X�ƂȂ钊�ۃN���X�ł��B
class Parser
{
public:
	//! �g�[�N���ɑ΂���p�[�X���s���܂��B
	//! @params [in] cursol ���݂̓ǂݎ��ʒu��\���J�[�\���ł��B
	//! @return �p�[�X�������������ǂ����ł��B
	virtual const bool Parse(vector<const Token>::const_iterator& cursol) const = 0;
};

//! �g�[�N�����ЂƂǂݎ��p�[�T�[�ł��B
class TokenParser : public Parser
{
	TokenKind m_kind; //!< �ǂݎ��g�[�N���̎�ނł��B
	function<void(const Token)> m_action; //!< �ǂݎ�肪������������s���鏈���ł��B
public:
	//! TokenParser�N���X�̐V�����C���X�^���X�����������܂��B
	//! @param [in] �ǂݎ�肪������������s���鏈���ł��B
	//! @params [in] kind �ǂݎ��g�[�N���̎�ނł��B
	TokenParser(const function<void(const Token)> action, const TokenKind kind);

	//! TokenParser�N���X�̐V�����C���X�^���X�����������܂��B
	//! @params [in] kind �ǂݎ��g�[�N���̎�ނł��B
	TokenParser(const TokenKind kind);

	//! �g�[�N���ɑ΂���p�[�X���s���܂��B
	//! @params [in] cursol ���݂̓ǂݎ��ʒu��\���J�[�\���ł��B
	//! @return �p�[�X�������������ǂ����ł��B
	const bool Parse(vector<const Token>::const_iterator& cursol) const override;

	//! �ǂݎ�肪������������s���鏈����o�^���܂��B
	//! @param [in] �ǂݎ�肪������������s���鏈���ł��B
	const shared_ptr<const TokenParser> Action(const function<void(const Token)> action) const;
};

//! �g�[�N���̃p�[�T�[�𐶐����܂��B
//! @params [in] kind �ǂݎ��g�[�N���̎�ނł��B
const shared_ptr<TokenParser> token(TokenKind kind);

//! ��̋K�������Ԃɑg�ݍ��킹���K�������ɓǂݎ��p�[�T�[�ł��B
class SequenceParser : public Parser
{
	const shared_ptr<const Parser> m_parser1; //!< ��ڂ̃p�[�T�[�ł��B
	const shared_ptr<const Parser> m_parser2; //!< ��ڂ̃p�[�T�[�ł��B
	const function<void(void)> m_action; //!< �ǂݎ�肪������������s���鏈���ł��B
public:
	//! SequenceParser�N���X�̐V�����C���X�^���X�����������܂��B
	//! @param [in] �ǂݎ�肪������������s���鏈���ł��B
	//! @params [in] parser1 ��ڂ�Parser�ł��B
	//! @params [in] parser2 ��ږڂ�Parser�ł��B
	SequenceParser(const function<void(void)> action, const shared_ptr<const Parser> parser1, const shared_ptr<const Parser> parser2);

	//! SequenceParser�N���X�̐V�����C���X�^���X�����������܂��B
	//! @params [in] parser1 ��ڂ�Parser�ł��B
	//! @params [in] parser2 ��ڂ�Parser�ł��B
	SequenceParser(const shared_ptr<const Parser> parser1, const shared_ptr<const Parser> parser2);

	//! �ǂݎ�肪������������s���鏈����o�^���܂��B
	//! @param [in] �ǂݎ�肪������������s���鏈���ł��B
	const shared_ptr<const SequenceParser> Action(const function<void(void)> action) const;

	//! ��̋K���ɑ΂���p�[�X���s���܂��B
	//! @params [in] cursol ���݂̓ǂݎ��ʒu��\���J�[�\���ł��B
	//! @return �p�[�X�������������ǂ����ł��B
	const bool Parse(vector<const Token>::const_iterator& cursol) const override;
};

//! SequenceParser�N���X�̐V�����C���X�^���X�𐶐����܂��B
//! @params [in] parser1 ��ڂ�Parser�ł��B
//! @params [in] parser2 ��ڂ�Parser�ł��B
const shared_ptr<const SequenceParser> operator>>(const shared_ptr<const Parser> parser1, const shared_ptr<const Parser> parser2);

//! ��̋K���̂ǂ��炩��ǂݎ��p�[�T�[�ł��B
class OrderedChoiceParser : public Parser
{
	const shared_ptr<const Parser> m_parser1; //!< ��ڂ̃p�[�T�[�ł��B
	const shared_ptr<const Parser> m_parser2; //!< ��ڂ̃p�[�T�[�ł��B
	const function<void(void)> m_action; //!< �ǂݎ�肪������������s���鏈���ł��B
public:
	//! OrderedChoiceParser�N���X�̐V�����C���X�^���X�����������܂��B
	//! @param [in] �ǂݎ�肪������������s���鏈���ł��B
	//! @params [in] parser1 ��ڂ�Parser�ł��B
	//! @params [in] parser2 ��ږڂ�Parser�ł��B
	OrderedChoiceParser(const function<void(void)> action, const shared_ptr<const Parser> parser1, const shared_ptr<const Parser> parser2);

	//! OrderedChoiceParser�N���X�̐V�����C���X�^���X�����������܂��B
	//! @params [in] parser1 ��ڂ�Parser�ł��B
	//! @params [in] parser2 ��ڂ�Parser�ł��B
	OrderedChoiceParser(const shared_ptr<const Parser> parser1, const shared_ptr<const Parser> parser2);

	//! �ǂݎ�肪������������s���鏈����o�^���܂��B
	//! @param [in] �ǂݎ�肪������������s���鏈���ł��B
	const shared_ptr<const OrderedChoiceParser> Action(const function<void(void)> action) const;

	//! ��̋K���ɑ΂���p�[�X���s���܂��B
	//! @params [in] cursol ���݂̓ǂݎ��ʒu��\���J�[�\���ł��B
	//! @return �p�[�X�������������ǂ����ł��B
	const bool Parse(vector<const Token>::const_iterator& cursol) const override;
};

//! OrderedChoiceParser�N���X�̐V�����C���X�^���X�𐶐����܂��B
//! @params [in] parser1 ��ڂ�Parser�ł��B
//! @params [in] parser2 ��ڂ�Parser�ł��B
const shared_ptr<const OrderedChoiceParser> operator|(const shared_ptr<const Parser> parser1, const shared_ptr<const Parser> parser2);

//! ���݂��Ȃ��Ă����s�ƂȂ�Ȃ��K����ǂݎ��p�[�T�[�ł��B
class OptionalParser : public Parser
{
	const shared_ptr<const Parser> m_optional; //!< ���݂��Ă����Ȃ��Ă��悢�K���ł��B
	const function<void(void)> m_action; //!< �ǂݎ�肪������������s���鏈���ł��B
public:
	//! OptionalParser�N���X�̐V�����C���X�^���X�����������܂��B
	//! @param [in] �ǂݎ�肪������������s���鏈���ł��B
	//! @params [in] optional ���݂��Ă����Ȃ��Ă��悢�K���ł��B
	OptionalParser(const function<void(void)> action, const shared_ptr<const Parser> optional);

	//! OptionalParser�N���X�̐V�����C���X�^���X�����������܂��B
	//! @params [in] optional ���݂��Ă����Ȃ��Ă��悢�K���ł��B
	OptionalParser(const shared_ptr<const Parser> optional);

	//! �ǂݎ�肪������������s���鏈����o�^���܂��B
	//! @param [in] �ǂݎ�肪������������s���鏈���ł��B
	const shared_ptr<const OptionalParser> Action(const function<void(void)> action) const;

	//! �I�v�V���i���ȃp�[�X���s���܂��B
	//! @params [in] cursol ���݂̓ǂݎ��ʒu��\���J�[�\���ł��B
	//! @return �p�[�X�������������ǂ����ł��B
	const bool Parse(vector<const Token>::const_iterator& cursol) const override;
};

//! OptionalParser�N���X�̐V�����C���X�^���X�𐶐����܂��B
//! @params [in] optional ���݂��Ă����Ȃ��Ă��悢�K���ł��B
const shared_ptr<const OptionalParser> operator-(const shared_ptr<const Parser> optional);

//! 0��ȏ�̌J��Ԃ���ǂݎ��p�[�T�[�ł��B
class ZeroOrMoreParser : public Parser
{
	const shared_ptr<const Parser> m_once; //!< �J��Ԃ��̈�񕪂ƂȂ�K���ł��B
	const function<void(void)> m_action; //!< �ǂݎ�肪������������s���鏈���ł��B
public:
	//! ZeroOrMoreParser�N���X�̐V�����C���X�^���X�����������܂��B
	//! @param [in] �ǂݎ�肪������������s���鏈���ł��B
	//! @params [in] once �J��Ԃ��̈�񕪂ƂȂ�K���ł��B
	ZeroOrMoreParser(const function<void(void)> action, const shared_ptr<const Parser> once);

	//! ZeroOrMoreParser�N���X�̐V�����C���X�^���X�����������܂��B
	//! @params [in] once �J��Ԃ��̈�񕪂ƂȂ�K���ł��B
	ZeroOrMoreParser(const shared_ptr<const Parser> once);

	//! �ǂݎ�肪������������s���鏈����o�^���܂��B
	//! @param [in] �ǂݎ�肪������������s���鏈���ł��B
	const shared_ptr<const ZeroOrMoreParser> Action(const function<void(void)> action) const;

	//! �J��Ԃ��̃p�[�X���s���܂��B
	//! @params [in] cursol ���݂̓ǂݎ��ʒu��\���J�[�\���ł��B
	//! @return �p�[�X�������������ǂ����ł��B
	const bool Parse(vector<const Token>::const_iterator& cursol) const override;
};

////! ZeroOrMoreParser�N���X�̐V�����C���X�^���X�𐶐����܂��B
////! @params [in] once �J��Ԃ��̈�񕪂ƂȂ�K���ł��B
const shared_ptr<const Parser> operator~(const shared_ptr<const Parser> once);

//! ��ǂ݂��s���A�J�[�\����i�߂���ɂ��̋K���̕��@�����݂��邩�ǂ�����Ԃ��p�[�T�[�ł��B
class AndPredicateParser : public Parser
{
	const shared_ptr<const Parser> m_parser; //!< ��ǂ݂���K���ł��B
	const function<void(bool)> m_action; //!< ��ǂ݂����s��������s���鏈���ł��B��ǂ݂������������ǂ������󂯎��܂��B
public:
	//! AndPredicateParser�N���X�̐V�����C���X�^���X�����������܂��B
	//! @param [in] ��ǂ݂����s��������s���鏈���ł��B��ǂ݂������������ǂ������󂯎��܂��B
	//! @params [in] parser ��ǂ݂���K���ł��B
	AndPredicateParser(const function<void(bool)> action, const shared_ptr<const Parser> parser);

	//! AndPredicateParser�N���X�̐V�����C���X�^���X�����������܂��B
	//! @params [in] parser ��ǂ݂���K���ł��B
	AndPredicateParser(const shared_ptr<const Parser> parser);

	//! �ǂݎ�肪������������s���鏈����o�^���܂��B
	//! @param [in] ��ǂ݂����s��������s���鏈���ł��B��ǂ݂������������ǂ������󂯎��܂��B
	const shared_ptr<const AndPredicateParser> Action(const function<void(bool)> action) const;

	//! �J��Ԃ��̃p�[�X���s���܂��B
	//! @params [in] cursol ���݂̓ǂݎ��ʒu��\���J�[�\���ł��B
	//! @return �p�[�X�������������ǂ����ł��B
	const bool Parse(vector<const Token>::const_iterator& cursol) const override;
};

//! �o�͂���f�[�^���Ǘ����܂��B
class OutputData
{
	SqlQueryInfo queryInfo; //!< SQL�ɋL�q���ꂽ���e�ł��B
	vector<Column> allInputColumns; //!< ���͂Ɋ܂܂�邷�ׂĂ̗�̈ꗗ�ł��B

	const vector<const InputTable> &inputTables; //!< �t�@�C������ǂݎ�����f�[�^�ł��B

	//! ���̓t�@�C���ɏ����Ă��������ׂĂ̗��allInputColumns�ɐݒ肵�܂��B
	void InitializeAllInputColumns();

	//! SELECT��̗񖼎w�肪*�������ꍇ�́A����CSV�̗񖼂����ׂđI������܂��B
	void OpenSelectAsterisk();

	//! SELECT��Ŏw�肳�ꂽ�񖼂��A���ڂ̓��̓t�@�C���̉���ڂɑ������邩�𔻕ʂ��܂��B
	void SetAllColumns();

	//! ���͂��ꂽ�e�e�[�u���́A���ݏo�͂��Ă���s���w���J�[�\�����A���������ꂽ��ԂŎ擾���܂��B
	//! @return ���������ꂽ�J�[�\���ł��B
	const shared_ptr<vector<vector<const vector<const shared_ptr<const Data>>>::const_iterator>> OutputData::GetInitializedCurrentRows() const;

	//! WHERE��ORDER BY��K�p���Ă��Ȃ����ׂĂ̍s���擾���܂��B
	//! @return ���ׂẴf�[�^�s�B���͂��ꂽ���ׂĂ̓��̓f�[�^��ۊǂ��܂��B
	const shared_ptr<vector<const vector<const shared_ptr<const Data>>>> GetAllRows() const;

	//! �f�[�^�ɑ΂���WHERE���K�p���܂��B
	//! @params [in] outputRows �K�p�����f�[�^�B
	void ApplyWhere(vector<const vector<const shared_ptr<const Data>>> &outputRows) const;

	//! �f�[�^�ɑ΂���ORDER BY���K�p���܂��B
	//! @params [in] outputRows �K�p�����f�[�^�B
	void ApplyOrderBy(vector<const vector<const shared_ptr<const Data>>> &outputRows) const;
public:

	//! OutputData�N���X�̐V�����C���X�^���X�����������܂��B
	//! @param [in] queryInfo SQL�̏��ł��B
	//! @param [in] inputTables �t�@�C������ǂݎ�����f�[�^�ł��B
	OutputData(const SqlQueryInfo queryInfo, const vector<const InputTable> &inputTables);

	//! �o�͂���J�������擾���܂��B
	//! @return �o�͂���J�����ł��B
	const vector<Column> columns() const;

	//! �o�͂��邷�ׂẴf�[�^�s���擾���܂��B
	//! @return �o�͂��邷�ׂẴf�[�^�s�B���͂��ꂽ���ׂĂ̓��̓f�[�^��ۊǂ��܂��B
	const shared_ptr<const vector<const vector<const shared_ptr<const Data>>>> outputRows() const;
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
	const shared_ptr<vector<const vector<const shared_ptr<const Data>>>> ReadData(ifstream &inputFile) const;

	//! �o�̓t�@�C�����J���܂��B
	//! @param [in] filePath �J���t�@�C���̃t�@�C���p�X�ł��B
	//! @return �o�̓t�@�C���������X�g���[���ł��B
	ofstream OpenOutputFile(const string filePath) const;

	//! �o�̓t�@�C������܂��B
	//! @param [in] OutputFile ���̓t�@�C���������X�g���[���ł��B
	void CloseOutputFile(ofstream &outputFile) const;

	//! ����CSV�̃w�b�_�s��ǂݍ��݂܂��B
	//! @param [in] OutputFile �o�̓t�@�C���������X�g���[���ł��B�J�����㉽���ǂݍ���ł��܂���B
	//! @param [in] columns �o�͂���w�b�_���ł��B
	void WriteHeader(ofstream &outputFile, const vector<Column> &columns) const;

	//! ����CSV�̃f�[�^�s��ǂݍ��݂܂��B
	//! @param [in] OutputFile �o�̓t�@�C���������X�g���[���ł��B���łɃw�b�_�݂̂�ǂݍ��񂾌�ł��B
	//! columns [in] �o�͂���f�[�^�ł��B
	void WriteData(ofstream &outputFile, const OutputData &data) const;
public:

	//! Csv�N���X�̐V�����C���X�^���X�����������܂��B
	//! @param [in] queryInfo SQL�ɋL�q���ꂽ���e�ł��B
	Csv(const shared_ptr<const SqlQueryInfo> queryInfo);

	//! CSV�t�@�C��������̓f�[�^��ǂݎ��܂��B
	//! @return �t�@�C������ǂݎ�����f�[�^�ł��B
	const shared_ptr<const vector<const InputTable>> Read() const;

	//! CSV�t�@�C���ɏo�̓f�[�^���������݂܂��B
	//! @param [in] outputFileName ���ʂ��o�͂���t�@�C���̃t�@�C�����ł��B
	//! @param [in] inputTables �t�@�C������ǂݎ�����f�[�^�ł��B
	void Write(const string outputFileName, const vector<const InputTable> &inputTables) const;
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
Data::Data(){}

//! Data�^�̋�ۃN���X�̃C���X�^���X��Ԃ��܂��B
//! @param [in] value �f�[�^�̎��ۂ̒l�ł��B
shared_ptr<Data> Data::New(const std::string value)
{
	return make_shared<StringData>(value);
}

//! Data�^�̋�ۃN���X�̃C���X�^���X��Ԃ��܂��B
//! @param [in] value �f�[�^�̎��ۂ̒l�ł��B
shared_ptr<Data> Data::New(const int value)
{
	return make_shared<IntegerData>(value);
}

//! Data�^�̋�ۃN���X�̃C���X�^���X��Ԃ��܂��B
//! @param [in] value �f�[�^�̎��ۂ̒l�ł��B
shared_ptr<Data> Data::New(const bool value)
{
	return make_shared<BooleanData>(value);
}

//! �f�[�^��������^�̏ꍇ�̒l���擾���܂��B
//! @return �f�[�^��������^�̏ꍇ�̒l�ł��B
const DataType Data::type() const
{
	return m_type;
}

//! �f�[�^��������^�̏ꍇ�̒l���擾���܂��B
//! @return �f�[�^��������^�̏ꍇ�̒l�ł��B
const string& Data::string() const
{
	return defaultString;
}

//! �f�[�^�������^�̏ꍇ�̒l���擾���܂��B
//! @return �f�[�^�������^�̏ꍇ�̒l�ł��B
const int Data::integer() const
{
	return 0;
}

//! �f�[�^���^�U�l�^�̏ꍇ�̒l���擾���܂��B
//! @return �f�[�^���^�U�l�^�̏ꍇ�̒l�ł��B
const bool Data::boolean() const
{
	return false;
}

//! Data�N���X�̐V�����C���X�^���X�����������܂��B
//! @param [in] value �f�[�^�̒l�ł��B
StringData::StringData(const std::string value) : m_string(value){}

//! �f�[�^�̌^���擾���܂��B
//! @return �f�[�^�̌^�ł��B
const DataType StringData::type() const
{
	return DataType::STRING;
}

//! �f�[�^��������^�̏ꍇ�̒l���擾���܂��B
//! @return �f�[�^��������^�̏ꍇ�̒l�ł��B
const string& StringData::string() const
{
	return m_string;
}

//! ���Z���Z���s���܂��B
//! @param [in] right �E�ӂł��B
//! @return ���Z�������ʂł��B
const shared_ptr<const Data> StringData::operator+(const shared_ptr<const Data>& right) const
{
	throw ResultValue::ERR_WHERE_OPERAND_TYPE;
}

//! ���Z���Z���s���܂��B
//! @param [in] right �E�ӂł��B
//! @return ���Z�������ʂł��B
const shared_ptr<const Data> StringData::operator-(const shared_ptr<const Data>& right) const
{
	throw ResultValue::ERR_WHERE_OPERAND_TYPE;
}

//! ��Z���Z���s���܂��B
//! @param [in] right �E�ӂł��B
//! @return ��Z�������ʂł��B
const shared_ptr<const Data> StringData::operator*(const shared_ptr<const Data>& right) const
{
	throw ResultValue::ERR_WHERE_OPERAND_TYPE;
}

//! ���Z���Z���s���܂��B
//! @param [in] right �E�ӂł��B
//! @return ���Z�������ʂł��B
const shared_ptr<const Data> StringData::operator/(const shared_ptr<const Data>& right) const
{
	throw ResultValue::ERR_WHERE_OPERAND_TYPE;
}

//! ���l��r���s���܂��B
//! @param [in] right �E�ӂł��B
//! @return ��r�������ʂł��B
const shared_ptr<const Data> StringData::operator==(const shared_ptr<const Data>& right) const
{
	if (right->type() == DataType::STRING){
		return Data::New(string() == right->string());
	}
	else{
		throw ResultValue::ERR_WHERE_OPERAND_TYPE;
	}
}

//! �s����r���s���܂��B
//! @param [in] right �E�ӂł��B
//! @return ��r�������ʂł��B
const shared_ptr<const Data> StringData::operator!=(const shared_ptr<const Data>& right) const
{
	if (right->type() == DataType::STRING){
		return Data::New(string() != right->string());
	}
	else{
		throw ResultValue::ERR_WHERE_OPERAND_TYPE;
	}
}

//! �ȏ��r���s���܂��B
//! @param [in] right �E�ӂł��B
//! @return ��r�������ʂł��B
const shared_ptr<const Data> StringData::operator>=(const shared_ptr<const Data>& right) const
{
	if (right->type() == DataType::STRING){
		return Data::New(string() >= right->string());
	}
	else{
		throw ResultValue::ERR_WHERE_OPERAND_TYPE;
	}
}

//! �傫����r���s���܂��B
//! @param [in] right �E�ӂł��B
//! @return ��r�������ʂł��B
const shared_ptr<const Data> StringData::operator>(const shared_ptr<const Data>& right) const
{
	if (right->type() == DataType::STRING){
		return Data::New(string() > right->string());
	}
	else{
		throw ResultValue::ERR_WHERE_OPERAND_TYPE;
	}
}

//! �ȉ���r���s���܂��B
//! @param [in] right �E�ӂł��B
//! @return ��r�������ʂł��B
const shared_ptr<const Data> StringData::operator<=(const shared_ptr<const Data>& right) const
{
	if (right->type() == DataType::STRING){
		return Data::New(string() <= right->string());
	}
	else{
		throw ResultValue::ERR_WHERE_OPERAND_TYPE;
	}
}

//! ��������r���s���܂��B
//! @param [in] right �E�ӂł��B
//! @return ��r�������ʂł��B
const shared_ptr<const Data> StringData::operator<(const shared_ptr<const Data>& right) const
{
	if (right->type() == DataType::STRING){
		return Data::New(string() < right->string());
	}
	else{
		throw ResultValue::ERR_WHERE_OPERAND_TYPE;
	}
}

//! AND���Z���s���܂��B
//! @param [in] right �E�ӂł��B
//! @return ���Z�������ʂł��B
const shared_ptr<const Data> StringData::operator&&(const shared_ptr<const Data>& right) const
{
	throw ResultValue::ERR_WHERE_OPERAND_TYPE;
}

//! OR���Z���s���܂��B
//! @param [in] right �E�ӂł��B
//! @return ���Z�������ʂł��B
const shared_ptr<const Data> StringData::operator||(const shared_ptr<const Data>& right) const
{
	throw ResultValue::ERR_WHERE_OPERAND_TYPE;
}

//! Data�N���X�̐V�����C���X�^���X�����������܂��B
//! @param [in] value �f�[�^�̒l�ł��B
IntegerData::IntegerData(const int value) : m_integer(value){}

//! �f�[�^�̌^���擾���܂��B
//! @return �f�[�^�̌^�ł��B
const DataType IntegerData::type() const
{
	return DataType::INTEGER;
}

//! �f�[�^�������^�̏ꍇ�̒l���擾���܂��B
//! @return �f�[�^�������^�̏ꍇ�̒l�ł��B
const int IntegerData::integer() const
{
	return m_integer;
}

//! ���Z���Z���s���܂��B
//! @param [in] right �E�ӂł��B
//! @return ���Z�������ʂł��B
const shared_ptr<const Data> IntegerData::operator+(const shared_ptr<const Data>& right) const
{
	if (right->type() == DataType::INTEGER){
		return Data::New(integer() + right->integer());
	}
	else{
		throw ResultValue::ERR_WHERE_OPERAND_TYPE;
	}
}

//! ���Z���Z���s���܂��B
//! @param [in] right �E�ӂł��B
//! @return ���Z�������ʂł��B
const shared_ptr<const Data> IntegerData::operator-(const shared_ptr<const Data>& right) const
{
	if (right->type() == DataType::INTEGER){
		return Data::New(integer() - right->integer());
	}
	else{
		throw ResultValue::ERR_WHERE_OPERAND_TYPE;
	}
}

//! ��Z���Z���s���܂��B
//! @param [in] right �E�ӂł��B
//! @return ��Z�������ʂł��B
const shared_ptr<const Data> IntegerData::operator*(const shared_ptr<const Data>& right) const
{
	if (right->type() == DataType::INTEGER){
		return Data::New(integer() * right->integer());
	}
	else{
		throw ResultValue::ERR_WHERE_OPERAND_TYPE;
	}
}

//! ���Z���Z���s���܂��B
//! @param [in] right �E�ӂł��B
//! @return ���Z�������ʂł��B
const shared_ptr<const Data> IntegerData::operator/(const shared_ptr<const Data>& right) const
{
	if (right->type() == DataType::INTEGER){
		return Data::New(integer() / right->integer());
	}
	else{
		throw ResultValue::ERR_WHERE_OPERAND_TYPE;
	}
}

//! ���l��r���s���܂��B
//! @param [in] right �E�ӂł��B
//! @return ��r�������ʂł��B
const shared_ptr<const Data> IntegerData::operator==(const shared_ptr<const Data>& right) const
{
	if (right->type() == DataType::INTEGER){
		return Data::New(integer() == right->integer());
	}
	else{
		throw ResultValue::ERR_WHERE_OPERAND_TYPE;
	}
}

//! �s����r���s���܂��B
//! @param [in] right �E�ӂł��B
//! @return ��r�������ʂł��B
const shared_ptr<const Data> IntegerData::operator!=(const shared_ptr<const Data>& right) const
{
	if (right->type() == DataType::INTEGER){
		return Data::New(integer() != right->integer());
	}
	else{
		throw ResultValue::ERR_WHERE_OPERAND_TYPE;
	}
}

//! �ȏ��r���s���܂��B
//! @param [in] right �E�ӂł��B
//! @return ��r�������ʂł��B
const shared_ptr<const Data> IntegerData::operator>=(const shared_ptr<const Data>& right) const
{
	if (right->type() == DataType::INTEGER){
		return Data::New(integer() >= right->integer());
	}
	else{
		throw ResultValue::ERR_WHERE_OPERAND_TYPE;
	}
}

//! �傫����r���s���܂��B
//! @param [in] right �E�ӂł��B
//! @return ��r�������ʂł��B
const shared_ptr<const Data> IntegerData::operator>(const shared_ptr<const Data>& right) const
{
	if (right->type() == DataType::INTEGER){
		return Data::New(integer() > right->integer());
	}
	else{
		throw ResultValue::ERR_WHERE_OPERAND_TYPE;
	}
}

//! �ȉ���r���s���܂��B
//! @param [in] right �E�ӂł��B
//! @return ��r�������ʂł��B
const shared_ptr<const Data> IntegerData::operator<=(const shared_ptr<const Data>& right) const
{
	if (right->type() == DataType::INTEGER){
		return Data::New(integer() <= right->integer());
	}
	else{
		throw ResultValue::ERR_WHERE_OPERAND_TYPE;
	}
}

//! ��������r���s���܂��B
//! @param [in] right �E�ӂł��B
//! @return ��r�������ʂł��B
const shared_ptr<const Data> IntegerData::operator<(const shared_ptr<const Data>& right) const
{
	if (right->type() == DataType::INTEGER){
		return Data::New(integer() < right->integer());
	}
	else{
		throw ResultValue::ERR_WHERE_OPERAND_TYPE;
	}
}

//! AND���Z���s���܂��B
//! @param [in] right �E�ӂł��B
//! @return ���Z�������ʂł��B
const shared_ptr<const Data> IntegerData::operator&&(const shared_ptr<const Data>& right) const
{
	throw ResultValue::ERR_WHERE_OPERAND_TYPE;
}

//! OR���Z���s���܂��B
//! @param [in] right �E�ӂł��B
//! @return ���Z�������ʂł��B
const shared_ptr<const Data> IntegerData::operator||(const shared_ptr<const Data>& right) const
{
	throw ResultValue::ERR_WHERE_OPERAND_TYPE;
}

//! Data�N���X�̐V�����C���X�^���X�����������܂��B
//! @param [in] value �f�[�^�̒l�ł��B
BooleanData::BooleanData(const bool value) : m_boolean(value){}

//! �f�[�^�̌^���擾���܂��B
//! @return �f�[�^�̌^�ł��B
const DataType BooleanData::type() const
{
	return DataType::BOOLEAN;
}

//! �f�[�^�������^�̏ꍇ�̒l���擾���܂��B
//! @return �f�[�^�������^�̏ꍇ�̒l�ł��B
const bool BooleanData::boolean() const
{
	return m_boolean;
}

//! ���Z���Z���s���܂��B
//! @param [in] right �E�ӂł��B
//! @return ���Z�������ʂł��B
const shared_ptr<const Data> BooleanData::operator+(const shared_ptr<const Data>& right) const
{
	throw ResultValue::ERR_WHERE_OPERAND_TYPE;
}

//! ���Z���Z���s���܂��B
//! @param [in] right �E�ӂł��B
//! @return ���Z�������ʂł��B
const shared_ptr<const Data> BooleanData::operator-(const shared_ptr<const Data>& right) const
{
	throw ResultValue::ERR_WHERE_OPERAND_TYPE;
}

//! ��Z���Z���s���܂��B
//! @param [in] right �E�ӂł��B
//! @return ��Z�������ʂł��B
const shared_ptr<const Data> BooleanData::operator*(const shared_ptr<const Data>& right) const
{
	throw ResultValue::ERR_WHERE_OPERAND_TYPE;
}

//! ���Z���Z���s���܂��B
//! @param [in] right �E�ӂł��B
//! @return ���Z�������ʂł��B
const shared_ptr<const Data> BooleanData::operator/(const shared_ptr<const Data>& right) const
{
	throw ResultValue::ERR_WHERE_OPERAND_TYPE;
}

//! ���l��r���s���܂��B
//! @param [in] right �E�ӂł��B
//! @return ��r�������ʂł��B
const shared_ptr<const Data> BooleanData::operator==(const shared_ptr<const Data>& right) const
{
	throw ResultValue::ERR_WHERE_OPERAND_TYPE;
}

//! �s����r���s���܂��B
//! @param [in] right �E�ӂł��B
//! @return ��r�������ʂł��B
const shared_ptr<const Data> BooleanData::operator!=(const shared_ptr<const Data>& right) const
{
	throw ResultValue::ERR_WHERE_OPERAND_TYPE;
}

//! �ȏ��r���s���܂��B
//! @param [in] right �E�ӂł��B
//! @return ��r�������ʂł��B
const shared_ptr<const Data> BooleanData::operator>=(const shared_ptr<const Data>& right) const
{
	throw ResultValue::ERR_WHERE_OPERAND_TYPE;
}

//! �傫����r���s���܂��B
//! @param [in] right �E�ӂł��B
//! @return ��r�������ʂł��B
const shared_ptr<const Data> BooleanData::operator>(const shared_ptr<const Data>& right) const
{
	throw ResultValue::ERR_WHERE_OPERAND_TYPE;
}

//! �ȉ���r���s���܂��B
//! @param [in] right �E�ӂł��B
//! @return ��r�������ʂł��B
const shared_ptr<const Data> BooleanData::operator<=(const shared_ptr<const Data>& right) const
{
	throw ResultValue::ERR_WHERE_OPERAND_TYPE;
}

//! ��������r���s���܂��B
//! @param [in] right �E�ӂł��B
//! @return ��r�������ʂł��B
const shared_ptr<const Data> BooleanData::operator<(const shared_ptr<const Data>& right) const
{
	throw ResultValue::ERR_WHERE_OPERAND_TYPE;
}

//! AND���Z���s���܂��B
//! @param [in] right �E�ӂł��B
//! @return ���Z�������ʂł��B
const shared_ptr<const Data> BooleanData::operator&&(const shared_ptr<const Data>& right) const
{
	if (right->type() == DataType::BOOLEAN){
		return Data::New(boolean() && right->boolean());
	}
	else{
		throw ResultValue::ERR_WHERE_OPERAND_TYPE;
	}
}

//! OR���Z���s���܂��B
//! @param [in] right �E�ӂł��B
//! @return ���Z�������ʂł��B
const shared_ptr<const Data> BooleanData::operator||(const shared_ptr<const Data>& right) const
{
	if (right->type() == DataType::BOOLEAN){
		return Data::New(boolean() || right->boolean());
	}
	else{
		throw ResultValue::ERR_WHERE_OPERAND_TYPE;
	}
}

//! ���Z���Z���s���܂��B
//! @param [in] left ���ӂł��B
//! @param [in] right �E�ӂł��B
//! @return ���Z�������ʂł��B
const shared_ptr<const Data> operator+(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right)
{
	return *left + right;
}

//! ���Z���Z���s���܂��B
//! @param [in] left ���ӂł��B
//! @param [in] right �E�ӂł��B
//! @return ���Z�������ʂł��B
const shared_ptr<const Data> operator-(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right)
{
	return *left - right;
}


//! ��Z���Z���s���܂��B
//! @param [in] left ���ӂł��B
//! @param [in] right �E�ӂł��B
//! @return ��Z�������ʂł��B
const shared_ptr<const Data> operator*(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right)
{
	return *left * right;
}

//! ���Z���Z���s���܂��B
//! @param [in] left ���ӂł��B
//! @param [in] right �E�ӂł��B
//! @return ���Z�������ʂł��B
const shared_ptr<const Data> operator/(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right)
{
	return *left / right;
}

//! ���l��r���s���܂��B
//! @param [in] left ���ӂł��B
//! @param [in] right �E�ӂł��B
//! @return ��r�������ʂł��B
const shared_ptr<const Data> operator==(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right)
{
	return *left == right;
}

//! �s����r���s���܂��B
//! @param [in] left ���ӂł��B
//! @param [in] right �E�ӂł��B
//! @return ��r�������ʂł��B
const shared_ptr<const Data> operator!=(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right)
{
	return *left != right;
}

//! �ȏ��r���s���܂��B
//! @param [in] left ���ӂł��B
//! @param [in] right �E�ӂł��B
//! @return ��r�������ʂł��B
const shared_ptr<const Data> operator>=(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right)
{
	return *left >= right;
}

//! �傫����r���s���܂��B
//! @param [in] left ���ӂł��B
//! @param [in] right �E�ӂł��B
//! @return ��r�������ʂł��B
const shared_ptr<const Data> operator>(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right)
{
	return *left > right;
}

//! �ȉ���r���s���܂��B
//! @param [in] left ���ӂł��B
//! @param [in] right �E�ӂł��B
//! @return ��r�������ʂł��B
const shared_ptr<const Data> operator<=(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right)
{
	return *left <= right;
}

//! ��������r���s���܂��B
//! @param [in] left ���ӂł��B
//! @param [in] right �E�ӂł��B
//! @return ��r�������ʂł��B
const shared_ptr<const Data> operator<(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right)
{
	return *left < right;
}

//! AND���Z���s���܂��B
//! @param [in] left ���ӂł��B
//! @param [in] right �E�ӂł��B
//! @return ���Z�������ʂł��B
const shared_ptr<const Data> operator&&(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right)
{
	return *left && right;
}

//! OR���Z���s���܂��B
//! @param [in] left ���ӂł��B
//! @param [in] right �E�ӂł��B
//! @return ���Z�������ʂł��B
const shared_ptr<const Data> operator||(const shared_ptr<const Data>& left, const shared_ptr<const Data>& right)
{
	return *left || right;
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

// left��right��middleOperator�ŉ��Z���܂��B
void ExtensionTreeNode::Operate()
{
	// ���m�[�h���O�Ɏq�m�[�h�����Z���Ă����܂��B
	if (left){
		left->Operate();
	}
	if (right){
		right->Operate();
	}

	// ���m�[�h�̒l���v�Z���܂��B
	switch (middleOperator.kind){
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

//! �J�������Ŏw�肳�ꂽ�f�[�^�����m�[�h���ǂ����ł��B
//! @return �J�������Ŏw�肳�ꂽ�f�[�^�����m�[�h���ǂ����B
bool ExtensionTreeNode::isDataNodeAsColumnName()
{
	return middleOperator.kind == TokenKind::NOT_TOKEN && !column.columnName.empty();
}

//! ���ۂɏo�͂���s�ɍ��킹�ė�Ƀf�[�^��ݒ肵�܂��B
//! @param [in] ���ۂɏo�͂���s�ł��B
void ExtensionTreeNode::SetColumnData(const vector<const shared_ptr<const Data>> &outputRow)
{
	if (isDataNodeAsColumnName()){
		value = outputRow[column.allColumnsIndex];

		// �������l�����Ēl���v�Z���܂��B
		if (value->type() == DataType::INTEGER){
			value = Data::New(value->integer() * signCoefficient);
		}
	}
}

//! �����Ƃ��ēn�����m�[�h�y�т��̎q���̃m�[�h���擾���܂��B
//! @param [in] �߂�l�̃��[�g�ƂȂ�m�[�h�ł��B�����͋A�肪�����ł��B
//! @return ���g�y�юq���̃m�[�h�ł��B
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

//! Order�N���X�̐V�����C���X�^���X�����������܂��B
//! @param [in] column ORDER��Ɏw�肳�ꂽ�񖼂ł��B
//! @param [in] isAsc ORDER���w�肳�ꂽ�������������ǂ����ł��B
Order::Order(Column column, const bool isAsc) : column(column), isAsc(isAsc){}

//! �S�Ă����l�ƂȂ��͐��l��ɕϊ����܂��B
void InputTable::InitializeIntegerColumn()
{
	for (size_t i = 0; i < columns()->size(); ++i){

		// �S�Ă̍s�̂����ɂ��āA�f�[�^�����񂩂畄���Ɛ��l�ȊO�̕�����T���܂��B
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

			// �����Ɛ����ȊO��������Ȃ���ɂ��ẮA���l��ɕϊ����܂��B
			for (auto& inputRow : *data()){
				inputRow[i] = Data::New(stoi(inputRow[i]->string()));
			}
		}
	}
}

//! InputTable�N���X�̐V�����C���X�^���X�����������܂��B
//! @param [in] columns �ǂݍ��񂾃w�b�_���ł��B
//! @param [in] data �ǂݍ��񂾃f�[�^�ł��B
InputTable::InputTable(const shared_ptr<const vector<const Column>> columns, const shared_ptr<vector<const vector<const shared_ptr<const Data>>>> data) : m_columns(columns), m_data(data)
{
	InitializeIntegerColumn();
}

//! ��̏����擾���܂��B
//! @return �f�[�^�ł��B
const shared_ptr<const vector<const Column>> InputTable::columns() const
{
	return m_columns;
}

//! �f�[�^���擾���܂��B
//! @return ��̏��ł��B
const shared_ptr<vector<const vector<const shared_ptr<const Data>>>> InputTable::data() const
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

//! TokenParser�N���X�̐V�����C���X�^���X�����������܂��B
//! @param [in] �ǂݎ�肪������������s���鏈���ł��B
//! @params [in] kind �ǂݎ��g�[�N���̎�ނł��B
TokenParser::TokenParser(function<void(const Token)> action, TokenKind kind) : m_action(action), m_kind(kind){}

//! TokenParser�N���X�̐V�����C���X�^���X�����������܂��B
//! @params [in] kind �ǂݎ��g�[�N���̎�ނł��B
TokenParser::TokenParser(TokenKind kind) : m_kind(kind){}

//! �g�[�N���ɑ΂���p�[�X���s���܂��B
//! @params [in] cursol ���݂̓ǂݎ��ʒu��\���J�[�\���ł��B
//! @return �p�[�X�������������ǂ����ł��B
const bool TokenParser::Parse(vector<const Token>::const_iterator& cursol) const
{
	if (cursol->kind == m_kind){
		if (m_action){
			m_action(*cursol);
		}
		++cursol;
		return true;
	}
	else{
		return false;
	}
}
//! �ǂݎ�肪������������s���鏈����o�^���܂��B
//! @param [in] �ǂݎ�肪������������s���鏈���ł��B
const shared_ptr<const TokenParser> TokenParser::Action(const function<void(const Token)> action) const
{
	return make_shared<TokenParser>(action, m_kind);
}

//! �g�[�N���̃p�[�T�[�𐶐����܂��B
//! @params [in] kind �ǂݎ��g�[�N���̎�ނł��B
const shared_ptr<TokenParser> token(TokenKind kind)
{
	return make_shared<TokenParser>(kind);
}

//! SequenceParser�N���X�̐V�����C���X�^���X�����������܂��B
//! @param [in] �ǂݎ�肪������������s���鏈���ł��B
//! @params [in] parser1 ��ڂ�Parser�ł��B
//! @params [in] parser1 ��ڂ�Parser�ł��B
SequenceParser::SequenceParser(const function<void(void)> action, const shared_ptr<const Parser> parser1, const shared_ptr<const Parser> parser2) : m_action(action), m_parser1(parser1), m_parser2(parser2){}

//! SequenceParser�N���X�̐V�����C���X�^���X�����������܂��B
//! @params [in] parser1 ��ڂ�Parser�ł��B
//! @params [in] parser1 ��ڂ�Parser�ł��B
SequenceParser::SequenceParser(const shared_ptr<const Parser> parser1, const shared_ptr<const Parser> parser2) : m_parser1(parser1), m_parser2(parser2){}

//! �g�[�N���ɑ΂���p�[�X���s���܂��B
//! @params [in] cursol ���݂̓ǂݎ��ʒu��\���J�[�\���ł��B
//! @return �p�[�X�������������ǂ����ł��B
const bool SequenceParser::Parse(vector<const Token>::const_iterator& cursol) const
{
	auto beforeParse = cursol;
	if (m_parser1->Parse(cursol) && m_parser2->Parse(cursol)){
		if (m_action){
			m_action();
		}
		return true;
	}
	cursol = beforeParse;
	return false;
}

//! �ǂݎ�肪������������s���鏈����o�^���܂��B
//! @param [in] �ǂݎ�肪������������s���鏈���ł��B
const shared_ptr<const SequenceParser> SequenceParser::Action(const function<void(void)> action) const
{
	return make_shared<SequenceParser>(action, m_parser1, m_parser2);
}

//! SequenceParser�N���X�̐V�����C���X�^���X�𐶐����܂��B
//! @params [in] parser1 ��ڂ�Parser�ł��B
//! @params [in] parser2 ��ڂ�Parser�ł��B
const shared_ptr<const SequenceParser> operator>>(const shared_ptr<const Parser> parser1, const shared_ptr<const Parser> parser2)
{
	return make_shared<SequenceParser>(parser1, parser2);
}

//! SequenceParser�N���X�̐V�����C���X�^���X�����������܂��B
//! @param [in] �ǂݎ�肪������������s���鏈���ł��B
//! @params [in] parser1 ��ڂ�Parser�ł��B
//! @params [in] parser2 ��ږڂ�Parser�ł��B
OrderedChoiceParser::OrderedChoiceParser(const function<void(void)> action, const shared_ptr<const Parser> parser1, const shared_ptr<const Parser> parser2) : m_action(action), m_parser1(parser1), m_parser2(parser2){}


//! SequenceParser�N���X�̐V�����C���X�^���X�����������܂��B
//! @params [in] parser1 ��ڂ�Parser�ł��B
//! @params [in] parser2 ��ڂ�Parser�ł��B
OrderedChoiceParser::OrderedChoiceParser(const shared_ptr<const Parser> parser1, const shared_ptr<const Parser> parser2) : m_parser1(parser1), m_parser2(parser2){}

//! �ǂݎ�肪������������s���鏈����o�^���܂��B
//! @param [in] �ǂݎ�肪������������s���鏈���ł��B
const shared_ptr<const OrderedChoiceParser> OrderedChoiceParser::Action(const function<void(void)> action) const
{
	return make_shared<OrderedChoiceParser>(action, m_parser1, m_parser2);
}

//! ��̋K���ɑ΂���p�[�X���s���܂��B
//! @params [in] cursol ���݂̓ǂݎ��ʒu��\���J�[�\���ł��B
//! @return �p�[�X�������������ǂ����ł��B
const bool OrderedChoiceParser::Parse(vector<const Token>::const_iterator& cursol) const
{
	if (m_parser1->Parse(cursol) || m_parser2->Parse(cursol)){
		if (m_action){
			m_action();
		}
		return true;
	}
	return false;
}

//! OrderedChoiceParser�N���X�̐V�����C���X�^���X�𐶐����܂��B
//! @params [in] parser1 ��ڂ�Parser�ł��B
//! @params [in] parser2 ��ڂ�Parser�ł��B
const shared_ptr<const OrderedChoiceParser> operator|(const shared_ptr<const Parser> parser1, const shared_ptr<const Parser> parser2)
{
	return make_shared<OrderedChoiceParser>(parser1, parser2);
}

//! OptionalParser�N���X�̐V�����C���X�^���X�����������܂��B
//! @param [in] �ǂݎ�肪������������s���鏈���ł��B
//! @params [in] optional ���݂��Ă����Ȃ��Ă��悢�K���ł��B
OptionalParser::OptionalParser(const function<void(void)> action, const shared_ptr<const Parser> optional) :m_action(action), m_optional(optional) {}

//! OptionalParser�N���X�̐V�����C���X�^���X�����������܂��B
//! @params [in] optional ���݂��Ă����Ȃ��Ă��悢�K���ł��B
OptionalParser::OptionalParser(const shared_ptr<const Parser> optional) : m_optional(optional){}

//! �ǂݎ�肪������������s���鏈����o�^���܂��B
//! @param [in] �ǂݎ�肪������������s���鏈���ł��B
const shared_ptr<const OptionalParser> OptionalParser::Action(const function<void(void)> action) const
{
	return make_shared<OptionalParser>(action, m_optional);
}

//! �I�v�V���i���ȃp�[�X���s���܂��B
//! @params [in] cursol ���݂̓ǂݎ��ʒu��\���J�[�\���ł��B
//! @return �p�[�X�������������ǂ����ł��B
const bool OptionalParser::Parse(vector<const Token>::const_iterator& cursol) const
{
	auto beforeParse = cursol;
	if (m_optional->Parse(cursol)){
		if (m_action){
			m_action();
		}
	}
	else{
		cursol = beforeParse;
	}
	return true;
}
//! OptionalParser�N���X�̐V�����C���X�^���X�𐶐����܂��B
//! @params [in] optional ���݂��Ă����Ȃ��Ă��悢�K���ł��B
const shared_ptr<const OptionalParser> operator-(const shared_ptr<const Parser> optional)
{
	return make_shared<OptionalParser>(optional);
}

//! ZeroOrMoreParser�N���X�̐V�����C���X�^���X�����������܂��B
//! @param [in] �ǂݎ�肪������������s���鏈���ł��B
//! @params [in] once �J��Ԃ��̈�񕪂ƂȂ�K���ł��B
ZeroOrMoreParser::ZeroOrMoreParser(const function<void(void)> action, const shared_ptr<const Parser> once) :m_action(action), m_once(once){}

//! ZeroOrMoreParser�N���X�̐V�����C���X�^���X�����������܂��B
//! @params [in] once �J��Ԃ��̈�񕪂ƂȂ�K���ł��B
ZeroOrMoreParser::ZeroOrMoreParser(const shared_ptr<const Parser> once): m_once(once){}

//! �ǂݎ�肪������������s���鏈����o�^���܂��B
//! @param [in] �ǂݎ�肪������������s���鏈���ł��B
const shared_ptr<const ZeroOrMoreParser> ZeroOrMoreParser::Action(const function<void(void)> action) const
{
	return make_shared<ZeroOrMoreParser>(action, m_once);
}

//! �J��Ԃ��̃p�[�X���s���܂��B
//! @params [in] cursol ���݂̓ǂݎ��ʒu��\���J�[�\���ł��B
//! @return �p�[�X�������������ǂ����ł��B
const bool ZeroOrMoreParser::Parse(vector<const Token>::const_iterator& cursol) const
{
	while (m_once->Parse(cursol)){}
	if (m_action){
		m_action();
	}
	return true;
}

//! ZeroOrMoreParser�N���X�̐V�����C���X�^���X�𐶐����܂��B
//! @params [in] once �J��Ԃ��̈�񕪂ƂȂ�K���ł��B
const shared_ptr<const Parser> operator~(const shared_ptr<const Parser> once)
{
	return make_shared<ZeroOrMoreParser>(once);
}

//! AndPredicateParser�N���X�̐V�����C���X�^���X�����������܂��B
//! @param [in] ��ǂ݂����s��������s���鏈���ł��B��ǂ݂������������ǂ������󂯎��܂��B
//! @params [in] parser ��ǂ݂���K���ł��B
AndPredicateParser::AndPredicateParser(const function<void(bool)> action, const shared_ptr<const Parser> parser) :m_action(action), m_parser(parser){}

//! ZeroOrMoreParser�N���X�̐V�����C���X�^���X�����������܂��B
//! @params [in] parser ��ǂ݂���K���ł��B
AndPredicateParser::AndPredicateParser(const shared_ptr<const Parser> parser) : m_parser(parser){}

//! �ǂݎ�肪������������s���鏈����o�^���܂��B
//! @param [in] ��ǂ݂����s��������s���鏈���ł��B��ǂ݂������������ǂ������󂯎��܂��B
const shared_ptr<const AndPredicateParser> AndPredicateParser::Action(const function<void(bool)> action) const
{
	return make_shared<AndPredicateParser>(action, m_parser);
}

//! �J��Ԃ��̃p�[�X���s���܂��B
//! @params [in] cursol ���݂̓ǂݎ��ʒu��\���J�[�\���ł��B
//! @return �p�[�X�������������ǂ����ł��B
const bool AndPredicateParser::Parse(vector<const Token>::const_iterator& cursol) const
{
	auto beforeParse = cursol;
	if (m_parser->Parse(cursol)){
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

//! ���̓t�@�C���ɏ����Ă��������ׂĂ̗��allInputColumns�ɐݒ肵�܂��B
void OutputData::InitializeAllInputColumns()
{
	for (auto &inputTable : inputTables){
		copy(
			inputTable.columns()->begin(),
			inputTable.columns()->end(),
			back_inserter(allInputColumns));
	}
}

//! WHERE��ORDER BY��K�p���Ă��Ȃ����ׂĂ̍s���擾���܂��B
//! @return ���ׂẴf�[�^�s�B���͂��ꂽ���ׂĂ̓��̓f�[�^��ۊǂ��܂��B
const shared_ptr<vector<const vector<const shared_ptr<const Data>>>> OutputData::GetAllRows() const
{
	auto outputRows = make_shared<vector<const vector<const shared_ptr<const Data>>>>();
	auto currentRowsPtr = GetInitializedCurrentRows();
	auto &currentRows = *currentRowsPtr;

	// �o�͂���f�[�^��ݒ肵�܂��B
	while (true){
		outputRows->push_back(vector<const shared_ptr<const Data>>());
		auto &outputRow = outputRows->back();// WHERE��ORDER�̂��߂ɂ��ׂĂ̏����܂ލs�Brow�ƃC���f�b�N�X�����L���܂��B

		// outputRow�̗��ݒ肵�܂��B
		for (auto &currentRow : currentRows){
			copy(
				currentRow->begin(),
				currentRow->end(),
				back_inserter(outputRow));
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
	return outputRows;
}

//! �f�[�^�ɑ΂���WHERE���K�p���܂��B
//! @params [in] outputRows �K�p�����f�[�^�B
void OutputData::ApplyWhere(vector<const vector<const shared_ptr<const Data>>> &outputRows) const
{
	// WHERE������K�p���܂��B
	if (queryInfo.whereTopNode){
		auto & newEnd = copy_if(
			outputRows.begin(),
			outputRows.end(),
			outputRows.begin(),
			[&](vector<const shared_ptr<const Data>> row){
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

//! �f�[�^�ɑ΂���ORDER BY���K�p���܂��B
//! @params [in] outputRows �K�p�����f�[�^�B
void OutputData::ApplyOrderBy(vector<const vector<const shared_ptr<const Data>>> &outputRows) const
{
	// ORDER��ɂ����ёւ��̏������s���܂��B
	if (!queryInfo.orders.empty()){
		sort(
			outputRows.begin(),
			outputRows.end(),
			[&](const vector<const shared_ptr<const Data>>& lRow, const vector<const shared_ptr<const Data>>& rRow){
			for (auto &order : queryInfo.orders){
				auto &lData = lRow[order.column.allColumnsIndex]; // �C���f�b�N�X��minIndex�̃f�[�^�ł��B
				auto &rData = rRow[order.column.allColumnsIndex]; // �C���f�b�N�X��j�̃f�[�^�ł��B
				int cmp = 0; // ��r���ʂł��B���������0�A�C���f�b�N�Xj�̍s���傫����΃v���X�A�C���f�b�N�XminIndex�̍s���傫����΃}�C�i�X�ƂȂ�܂��B
				switch (lData->type())
				{
				case DataType::INTEGER:
					cmp = lData->integer() - rData->integer();
					break;
				case DataType::STRING:
					cmp = strcmp(lData->string().c_str(), rData->string().c_str());
					break;
				}

				// �~���Ȃ�cmp�̑召�����ւ��܂��B
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


//! SELECT��̗񖼎w�肪*�������ꍇ�́A����CSV�̗񖼂����ׂđI������܂��B
void OutputData::OpenSelectAsterisk()
{
	if (queryInfo.selectColumns.empty()){
		copy(allInputColumns.begin(), allInputColumns.end(), back_inserter(queryInfo.selectColumns));
	}
}

//! ���p����񖼂��A���ڂ̓��̓t�@�C���̉���ڂɑ������邩�𔻕ʂ��܂��B
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

//! OutputData�N���X�̐V�����C���X�^���X�����������܂��B
//! @param [in] queryInfo SQL�̏��ł��B
OutputData::OutputData(const SqlQueryInfo queryInfo, const vector<const InputTable> &inputTables) : queryInfo(queryInfo), inputTables(inputTables)
{
	InitializeAllInputColumns();
	OpenSelectAsterisk();
	SetAllColumns();
}

//! ���͂��ꂽ�e�e�[�u���́A���ݏo�͂��Ă���s���w���J�[�\�����A���������ꂽ��ԂŎ擾���܂��B
//! @return ���������ꂽ�J�[�\���ł��B
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

//! �o�͂���J���������擾���܂��B
//! @return �o�͂���J�������ł��B
const vector<Column> OutputData::columns() const
{
	return queryInfo.selectColumns;
}

//! �o�͂��邷�ׂẴf�[�^�s���擾���܂��B
//! @return �o�͂��邷�ׂẴf�[�^�s�B���͂��ꂽ���ׂĂ̓��̓f�[�^��ۊǂ��܂��B
const shared_ptr<const vector<const vector<const shared_ptr<const Data>>>> OutputData::outputRows() const
{
	auto outputRows = GetAllRows();
	ApplyWhere(*outputRows);

	ApplyOrderBy(*outputRows);

	return outputRows;
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
const shared_ptr<vector<const vector<const shared_ptr<const Data>>>> Csv::ReadData(ifstream &inputFile) const
{
	auto data = make_shared<vector<const vector<const shared_ptr<const Data>>>>(); // �ǂݍ��񂾃f�[�^�̈ꗗ�B

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

//! �o�̓t�@�C�����J���܂��B
//! @param [in] outputFileName �J���t�@�C���̃t�@�C���p�X�ł��B
//! @return �o�̓t�@�C���������X�g���[���ł��B
ofstream Csv::OpenOutputFile(const string outputFileName) const
{
	ofstream outputFile(outputFileName);
	if (outputFile.bad()){
		throw ResultValue::ERR_FILE_OPEN;
	}
	return outputFile;
}

//! �o�̓t�@�C������܂��B
//! @param [in] OutputFile ���̓t�@�C���������X�g���[���ł��B
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

//! ����CSV�̃w�b�_�s��ǂݍ��݂܂��B
//! @param [in] OutputFile �o�̓t�@�C���������X�g���[���ł��B�J�����㉽���ǂݍ���ł��܂���B
//! @param [in] columns �o�͂���w�b�_���ł��B
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

//! ����CSV�̃f�[�^�s��ǂݍ��݂܂��B
//! @param [in] OutputFile �o�̓t�@�C���������X�g���[���ł��B���łɃw�b�_�݂̂�ǂݍ��񂾌�ł��B
//! columns [in] �o�͂���f�[�^�ł��B
void Csv::WriteData(ofstream &outputFile, const OutputData &data) const
{
	auto &outputRows = data.outputRows();
	for (auto& outputRow : *outputRows){
		size_t i = 0;
		for (const auto &column : data.columns()){
			switch (outputRow[column.allColumnsIndex]->type()){
			case DataType::INTEGER:
				outputFile << outputRow[column.allColumnsIndex]->integer();
				break;
			case DataType::STRING:
				outputFile << outputRow[column.allColumnsIndex]->string();
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

//! Csv�N���X�̐V�����C���X�^���X�����������܂��B
//! @param [in] queryInfo SQL�ɋL�q���ꂽ���e�ł��B
Csv::Csv(const shared_ptr<const SqlQueryInfo> queryInfo) : queryInfo(queryInfo){}

//! CSV�t�@�C��������̓f�[�^��ǂݎ��܂��B
//! @return �t�@�C������ǂݎ�����f�[�^�ł��B
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

//! CSV�t�@�C���ɏo�̓f�[�^���������݂܂��B
//! @param [in] outputFileName ���ʂ��o�͂���t�@�C���̃t�@�C�����ł��B
//! @param [in] queryInfo SQL�̏��ł��B
//! @param [in] inputTables �t�@�C������ǂݎ�����f�[�^�ł��B
void Csv::Write(const string outputFileName, const vector<const InputTable> &inputTables) const
{
	OutputData outputData(*queryInfo, inputTables); // �o�͂���f�[�^�ł��B

	auto outputFile = OpenOutputFile(outputFileName); // �������ރt�@�C���̃t�@�C���X�g���[���ł��B
	
	WriteHeader(outputFile, outputData.columns());

	WriteData(outputFile, outputData);

	CloseOutputFile(outputFile);
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

	auto AND = token(TokenKind::AND);// AND�L�[���[�h�g�[�N���̃p�[�T�[�ł��B
	auto ASC = token(TokenKind::ASC);// ASC�L�[���[�h�g�[�N���̃p�[�T�[�ł��B
	auto BY = token(TokenKind::BY);// BY�L�[���[�h�g�[�N���̃p�[�T�[�ł��B
	auto DESC = token(TokenKind::DESC);// DESC�L�[���[�h�g�[�N���̃p�[�T�[�ł��B
	auto FROM = token(TokenKind::FROM);// FROM�L�[���[�h�g�[�N���̃p�[�T�[�ł��B
	auto OR = token(TokenKind::OR);// OR�L�[���[�h�g�[�N���̃p�[�T�[�ł��B
	auto ORDER = token(TokenKind::ORDER);// ORDER�L�[���[�h�g�[�N���̃p�[�T�[�ł��B
	auto SELECT = token(TokenKind::SELECT);// SELECT�L�[���[�h�g�[�N���̃p�[�T�[�ł��B
	auto WHERE = token(TokenKind::WHERE);// WHERE�L�[���[�h�g�[�N���̃p�[�T�[�ł��B
	auto ASTERISK = token(TokenKind::ASTERISK); // �� �L���g�[�N���̃p�[�T�[�ł��B
	auto COMMA = token(TokenKind::COMMA); // �C �L���ł��g�[�N���̃p�[�T�[�ł��B
	auto CLOSE_PAREN = token(TokenKind::CLOSE_PAREN); // �j �L���g�[�N���̃p�[�T�[�ł��B
	auto DOT = token(TokenKind::DOT); // �D �L���g�[�N���̃p�[�T�[�ł��B
	auto EQUAL = token(TokenKind::EQUAL); // �� �L���g�[�N���̃p�[�T�[�ł��B
	auto GREATER_THAN = token(TokenKind::GREATER_THAN); // �� �L���g�[�N���̃p�[�T�[�ł��B
	auto GREATER_THAN_OR_EQUAL = token(TokenKind::GREATER_THAN_OR_EQUAL); // ���� �L���g�[�N���̃p�[�T�[�ł��B
	auto LESS_THAN = token(TokenKind::LESS_THAN); // �� �L���g�[�N���̃p�[�T�[�ł��B
	auto LESS_THAN_OR_EQUAL = token(TokenKind::LESS_THAN_OR_EQUAL); // ���� �L���g�[�N���̃p�[�T�[�ł��B
	auto MINUS = token(TokenKind::MINUS); // �| �L���g�[�N���̃p�[�T�[�ł��B
	auto NOT_EQUAL = token(TokenKind::NOT_EQUAL); // ���� �L���g�[�N���̃p�[�T�[�ł��B
	auto OPEN_PAREN = token(TokenKind::OPEN_PAREN); // �i �L���g�[�N���̃p�[�T�[�ł��B
	auto PLUS = token(TokenKind::PLUS); // �{ �L���g�[�N���̃p�[�T�[�ł��B
	auto SLASH = token(TokenKind::SLASH); // �^ �L���g�[�N���̃p�[�T�[�ł��B
	auto IDENTIFIER = token(TokenKind::IDENTIFIER); // ���ʎq�g�[�N���̃p�[�T�[�ł��B
	auto INT_LITERAL = token(TokenKind::INT_LITERAL); // �������e�����g�[�N���̃p�[�T�[�ł��B
	auto STRING_LITERAL = token(TokenKind::STRING_LITERAL); // �����񃊃e�����g�[�N���̃p�[�T�[�ł��B

	Column column; // ���ݓǂݍ���ł���ORDER BY��ł̗��ێ����܂��B

	// ��w��̈�ڂ̎��ʎq�̃p�[�T�[�ł��B
	auto FIRST_COLUMN_NAME = IDENTIFIER->Action([&](const Token token){
		// �e�[�u�������w�肳��Ă��Ȃ��ꍇ�Ɖ��肵�ēǂݍ��݂܂��B
		column = Column(token.word);
	});

	// ��w��̓�ڂ̎��ʎq�̃p�[�T�[�ł��B
	auto SECOND_COLUMN_NAME = IDENTIFIER->Action([&](const Token token){
		// �e�[�u�������w�肳��Ă��邱�Ƃ��킩�����̂œǂݑւ��܂��B
		column = Column(column.columnName, token.word);
	});

	auto COLUMN = FIRST_COLUMN_NAME >> -(DOT >> SECOND_COLUMN_NAME); // ��w���̃p�[�T�[�ł��B

	bool isAsc = true; // ���ݓǂݍ���ł���ORDER BY��ł̗񂪏����ł��邩�ǂ����ł��B

	// �����~�����w�肷�邽�߂�DESC�g�[�N���̃p�[�T�[�ł��B
	auto SET_DESC = DESC->Action([&](const Token token){
		isAsc = false;
	});

	shared_ptr<ExtensionTreeNode> currentNode; // ���ݓǂݍ���ł���m�[�h�ł��B
	auto WHERE_OPEN_PAREN = OPEN_PAREN->Action([&](const Token token){
		++currentNode->parenOpenBeforeClose;
	});

	auto WHERE_UNIARY_MINUS = MINUS->Action([&](const Token token){
		currentNode->signCoefficient = -1;
	});

	// �L���̈Ӗ�
	// A >> B		:A�̌��B������
	// -A			:A���C��
	// ~A			:A��0��ȏ㑱��

	// SELECT��̗�w���̃p�[�T�[�ł��B
	auto SELECT_COLUMN = COLUMN->Action([&]{
		queryInfo->selectColumns.push_back(column);
	});

	auto SELECT_COLUMNS = SELECT_COLUMN >> ~(COMMA >> SELECT_COLUMN); // SELECT��̈�ȏ�̗̂�w��̃p�[�T�[�ł��B

	auto SELECT_CLAUSE = SELECT >> (ASTERISK | SELECT_COLUMNS); // SELECT��̃p�[�T�[�ł��B

	auto ORDER_BY_COLUMN = COLUMN >> -(ASC | SET_DESC); // ORDER BY��̗�w���̃p�[�T�[�ł��B

	ORDER_BY_COLUMN = ORDER_BY_COLUMN->Action([&]{
		queryInfo->orders.push_back(Order(column, isAsc));

		// ���̕ϐ��͂܂�������̂ŏ��������܂��B
		isAsc = true;
	});

	auto ORDER_BY_COLUMNS = ORDER_BY_COLUMN >> ~(COMMA >> ORDER_BY_COLUMN); // ORDER BY��̈�ȏ�̗�w��̃p�[�T�[�ł��B

	auto ORDER_BY_CLAUSE = ORDER >> BY >> ORDER_BY_COLUMNS; // ORDER BY��̃p�[�T�[�ł��B

	// �I�y�����h�ɑO�u����� + �� -�̎��̃g�[�N�����ǂ݂����ʂ���p�[�T�[�ł��B
	auto WHERE_UNIALY_NEXT = make_shared<AndPredicateParser>(IDENTIFIER | INT_LITERAL)->Action([&](const bool success){
		if (!success){
			throw ResultValue::ERR_WHERE_OPERAND_TYPE;
		}
	});

	// �I�y�����h�ɑO�u����� + �� - ��ǂݍ��ރp�[�T�[�ł��B
	auto WHERE_UNIAEY_PLUS_MINUS = (PLUS | WHERE_UNIARY_MINUS) >> WHERE_UNIALY_NEXT;

	auto tokenCursol = tokens.begin(); // ���݌��Ă���g�[�N�����w���܂��B

	if (!SELECT_CLAUSE->Parse(tokenCursol)){
		throw ResultValue::ERR_SQL_SYNTAX;
	}


	// ORDER���WHERE���ǂݍ��݂܂��B�ő�e��񂸂������Ƃ��ł��܂��B
	bool readOrder = false; // ���ł�ORDER�傪�ǂݍ��ݍς݂��ǂ����ł��B
	bool readWhere = false; // ���ł�WHERE�傪�ǂݍ��ݍς݂��ǂ����ł��B
	while (true){

		// ��x�ڂ�ORDER��̓G���[�ł��B
		if (readOrder && tokenCursol->kind == TokenKind::ORDER){
			throw ResultValue::ERR_SQL_SYNTAX;
		}

		// ��x�ڂ�WHERE��̓G���[�ł��B
		if (readWhere && tokenCursol->kind == TokenKind::WHERE){
			throw ResultValue::ERR_SQL_SYNTAX;
		}

		// ORDER���ǂݍ��݂܂��B
		if (ORDER_BY_CLAUSE->Parse(tokenCursol)){
			readOrder = true;
			continue;
		}

		// WHERE���ǂݍ��݂܂��B
		if (tokenCursol->kind == TokenKind::WHERE){
			readWhere = true;
			++tokenCursol;
			while (true){
				// �I�y�����h��ǂݍ��݂܂��B

				// �I�y�����h�̃m�[�h��V�����������܂��B
				auto newNode = make_shared<ExtensionTreeNode>();
				if (currentNode){
					// ���݂̃m�[�h���E�̎q�ɂ��炵�A���̈ʒu�ɐV�����m�[�h��}�����܂��B
					currentNode->right = newNode;
					currentNode->right->parent = currentNode;
					currentNode = currentNode->right;
				}
				else{
					// �ŏ��̓J�����g�m�[�h�ɐV�����m�[�h�����܂��B
					currentNode = newNode;
				}

				(~WHERE_OPEN_PAREN)->Parse(tokenCursol);

				WHERE_UNIAEY_PLUS_MINUS->Parse(tokenCursol);

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
					currentNode->value = Data::New(stoi(tokenCursol->word));
					++tokenCursol;
				}
				else if (tokenCursol->kind == TokenKind::STRING_LITERAL){
					// �O��̃V���O���N�H�[�g����苎������������f�[�^�Ƃ��ēǂݍ��݂܂��B
					currentNode->value = Data::New(tokenCursol->word.substr(1, tokenCursol->word.size() - 2));
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
					currentNode = make_shared<ExtensionTreeNode>();
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
			// �������l�̕������v�Z���܂��B
			auto whereNodes = SelfAndDescendants(queryInfo->whereTopNode);
			for (auto &whereNode : *whereNodes){
				if (whereNode->middleOperator.kind == TokenKind::NOT_TOKEN &&
					whereNode->column.columnName.empty() &&
					whereNode->value->type() == DataType::INTEGER){
					whereNode->value = Data::New(whereNode->value->integer() * whereNode->signCoefficient);
				}
			}
			continue;
		}
		break;
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
	csv->Write(outputFileName, *csv->Read());
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