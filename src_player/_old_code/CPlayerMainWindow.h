#pragma once
#include <ACStdLib.h>
using namespace ACStdLib;
using namespace ACStdLib::UI;
//Local
#include "CVideoWidget.h"
#include "CLogger.h"

class CPlayerMainWindow : public CMainWindow
{
private:
	//Members
	CTextEdit *pInfoTextBox;
	CDropDown *pVideoStreamSelector;
	CDropDown *pAudioStreamSelector;
	CPushButton *pPlayPauseButton;
	CPushButton *pNextFrameButton;

	CLogger logger;
	EDropType currentDropType;

	//Eventhandlers
	EDropType OnDragEnter(const ITransfer &refTransfer);
	void OnDragLeave();
	EDropType OnDragMove();
	void OnDrop(const ITransfer &refTransfer);

public:
	//Constructor
	CPlayerMainWindow();
	
	//Inline
	inline void UpdatePicture()
	{
		this->pVideoWidget->Repaint();
	}

	inline void UpdateMediaPos(float64 fraction)
	{
		this->pVideoPosSlider->SetPos((uint32)fraction);
	}
};
