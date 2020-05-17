/*
 * Copyright (c) 2017-2020 Amir Czwink (amir130@hotmail.de)
 *
 * This file is part of AVTools.
 *
 * AVTools is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * AVTools is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with AVTools.  If not, see <http://www.gnu.org/licenses/>.
 */
//Class header
#include "MainWindow.hpp"
//Local
#include "StreamController.hpp"

//Constructor
MainWindow::MainWindow(EventQueue &eventQueue) : MainAppWindow(eventQueue)
{
	this->file = nullptr;
	this->player = nullptr;

	this->SetTitle(u8"Video Playback Software");

	//Main container
	this->GetContentContainer()->SetLayout(new VerticalLayout);

	WidgetFrameBufferSetup frameBufferSetup;
	frameBufferSetup.nSamples = 1;

	this->videoWidget = new VideoWidget(frameBufferSetup);
	this->AddContentChild(this->videoWidget);

	//Playback Control
	GroupBox *playbackControl = new GroupBox();
	playbackControl->SetTitle(u8"Playback control");
	this->AddContentChild(playbackControl);

	this->videoPos = new Slider();
	playbackControl->AddContentChild(this->videoPos);

	//actions panel
	GroupBox *actionsPanel = new GroupBox();
	actionsPanel->SetTitle(u8"Control");
	actionsPanel->GetContentContainer()->SetLayout(new HorizontalLayout);
	playbackControl->AddContentChild(actionsPanel);

	this->videoStreamChooser = new SelectBox;
	this->videoStreamChooser->SetHint(u8"Video stream");
	actionsPanel->AddContentChild(this->videoStreamChooser);

	this->audioStreamChooser = new SelectBox;
	this->audioStreamChooser->SetHint(u8"Audio stream");
	actionsPanel->AddContentChild(this->audioStreamChooser);

	this->subtitleStreamChooser = new SelectBox;
	this->subtitleStreamChooser->SetHint(u8"Subtitle stream");
	actionsPanel->AddContentChild(this->subtitleStreamChooser);
	
	this->playPauseButton = new PushButton();
	this->playPauseButton->SetText(u8"Play");
	this->playPauseButton->onActivatedHandler = Function<void()>(&MainWindow::TogglePlayPause, this);
	actionsPanel->AddContentChild(this->playPauseButton);

	this->UpdateControls();
}

//Destructor
MainWindow::~MainWindow()
{
	this->Reset();
}

//Private methods
void MainWindow::Reset()
{
	if(this->player)
	{
		delete this->player;
		this->player = nullptr;
	}
	if(this->file)
	{
		delete this->file;
		this->file = nullptr;
	}
}

void MainWindow::TogglePlayPause()
{
	if(this->player->IsPlaying())
	{
		this->player->Pause();
		this->playPauseButton->SetText("Play");
	}
	else
	{
		this->player->Play();
		this->playPauseButton->SetText("Pause");
	}
}

void MainWindow::UpdateControls()
{
	this->playPauseButton->SetEnabled(this->player != nullptr);
}

//Public methods
void MainWindow::OpenFile(const FileSystem::Path &path)
{
	return;
	this->Reset();

	if(!FileSystem::OSFileSystem::GetInstance().Exists(path))
	{
		this->ShowErrorBox("File does not exist", "Input file is not existant.");
		return;
	}

	this->file = new FileInputStream(path);
	this->player = new Multimedia::MediaPlayer(*this->file);
	this->player->SetVideoOutput(this->videoWidget);

	this->videoStreamChooser->SetController(new StreamController(this->player->GetVideoStreams(), *this->player));
	this->audioStreamChooser->SetController(new StreamController(this->player->GetAudioStreams(), *this->player));
	this->subtitleStreamChooser->SetController(new StreamController(this->player->GetSubtitleStreams(), *this->player));

	this->UpdateControls();
}