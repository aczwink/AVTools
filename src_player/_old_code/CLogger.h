#pragma once
#include <ACStdLib.h>
using namespace ACStdLib;
using namespace ACStdLib::UI;

class CLogger
{
private:
	//Members
	CTextEdit *pTextBox;

public:
	//Constructor
	inline CLogger()
	{
		this->pTextBox = nullptr;
	}

	//Inline operators
	inline CLogger &operator<<(const CLineBreak &refLB)
	{
		if(this->pTextBox)
			this->pTextBox->SetText(this->pTextBox->GetText() + "\r\n");
		
		return *this;
	}

	inline CLogger &operator<<(uint8 i)
	{
		if(this->pTextBox)
			this->pTextBox->SetText(this->pTextBox->GetText() + ToString((uint64)i));

		return *this;
	}

	inline CLogger &operator<<(uint16 i)
	{
		if(this->pTextBox)
			this->pTextBox->SetText(this->pTextBox->GetText() + ToString((uint64)i));

		return *this;
	}

	inline CLogger &operator<<(uint32 i)
	{
		if(this->pTextBox)
			this->pTextBox->SetText(this->pTextBox->GetText() + ToString((uint64)i));

		return *this;
	}

	inline CLogger &operator<<(uint64 i)
	{
		if(this->pTextBox)
			this->pTextBox->SetText(this->pTextBox->GetText() + ToString(i));

		return *this;
	}

	inline CLogger &operator<<(const CString &refString)
	{
		if(this->pTextBox)
			this->pTextBox->SetText(this->pTextBox->GetText() + refString);

		return *this;
	}

	//Inline
	inline void SetTarget(CTextEdit *pTextBox)
	{
		this->pTextBox = pTextBox;
	}
};