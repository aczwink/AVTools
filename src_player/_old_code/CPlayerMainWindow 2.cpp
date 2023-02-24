//Class header
#include "CPlayerMainWindow.h"
//Local
#include "globals.h"

//Constructor
CPlayerMainWindow::CPlayerMainWindow()
{
	AWidgetContainer *pContainer;
	CGroupBox *pPlaybackControlBox, *pGroupBox;

	this->currentDropType = EDropType::None;

	this->EnableDrop();

	this->pVideoWidget->sizingPolicy.vertScale = 20;
	this->pVideoPosSlider->SetMinimum(0);

	//bottom row
	pContainer = new AWidgetContainer(this);

	pContainer->sizingPolicy.SetVerticalPolicy(CSizingPolicy::EPolicy::Expanding);
	pContainer->sizingPolicy.vertScale = 2;

	this->pInfoTextBox = new CTextEdit(pContainer);
	this->pInfoTextBox->SetEditable(false);
	this->logger.SetTarget(this->pInfoTextBox);

	this->pNextFrameButton = new CPushButton(pGroupBox);
	this->pNextFrameButton->SetText("Next Frame");
}

//Eventhandlers
EDropType CPlayerMainWindow::OnDragEnter(const ITransfer &refTransfer)
{
	if(dynamic_cast<const CFileTransfer *>(&refTransfer))
		this->currentDropType = EDropType::Link;

	return this->currentDropType;
}

void CPlayerMainWindow::OnDragLeave()
{
	this->currentDropType = EDropType::None;
}

EDropType CPlayerMainWindow::OnDragMove()
{
	return this->currentDropType;
}

void CPlayerMainWindow::OnDrop(const ITransfer &refTransfer)
{
	if(dynamic_cast<const CFileTransfer *>(&refTransfer))
	{
		CFileTransfer &refFileTransfer = (CFileTransfer &)refTransfer;

		this->OpenFile(refFileTransfer.files[0]);
	}
}

//Private methods
void CPlayerMainWindow::UpdateControls()
{
	bool canPlayAudio, canPlayVideo, canPlay;
	uint32 index;

	this->pVideoStreamSelector->Clear();
	this->pAudioStreamSelector->Clear();

	if(!g_pPlaybackHandler)
	{
		this->pAudioStreamSelector->SetEnabled(false);
		this->pNextFrameButton->SetEnabled(false);
		this->pVideoPosSlider->SetEnabled(false);
		this->pVideoStreamSelector->SetEnabled(false);
		return;
	}

	canPlayAudio = g_pPlaybackHandler->GetAudioStreamIndex() != UINT32_MAX;
	canPlayVideo = g_pPlaybackHandler->GetVideoStreamIndex() != UINT32_MAX;
	canPlay = canPlayAudio || canPlayVideo;

	this->pAudioStreamSelector->SetEnabled(canPlayAudio);
	this->pNextFrameButton->SetEnabled(canPlay);
	this->pPlayPauseButton->SetEnabled(canPlay);
	this->pVideoPosSlider->SetEnabled(canPlay);
	this->pVideoStreamSelector->SetEnabled(canPlayVideo);

	//audio
	for(const auto &refKV : g_pPlaybackHandler->GetAudioStreams())
	{
		index = this->pAudioStreamSelector->AddItem(ToString((uint64)refKV.key));
		if(refKV.key == g_pPlaybackHandler->GetAudioStreamIndex())
			this->pAudioStreamSelector->Select(index);
	}

	//video
	for(const auto &refKV : g_pPlaybackHandler->GetVideoStreams())
	{
		index = this->pVideoStreamSelector->AddItem(ToString((uint64)refKV.key));
		if(refKV.key == g_pPlaybackHandler->GetVideoStreamIndex())
			this->pVideoStreamSelector->Select(index);
	}

	//media pos
	if(g_pPlaybackHandler && g_pPlaybackHandler->GetDemuxer())
		this->pVideoPosSlider->SetMaximum((uint32)g_pPlaybackHandler->GetDemuxer()->GetDuration());
}

//Public methods
void CPlayerMainWindow::OpenFile(const CPath &refPath)
{
	this->pInfoTextBox->SetText(CString());
}
