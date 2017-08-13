/*
 * Copyright (c) 2017 Amir Czwink (amir130@hotmail.de)
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

//Constructor
MainWindow::MainWindow()
{
	this->file = nullptr;
	this->player = nullptr;

	this->SetTitle("Video Playback Software");

	//Main container
	this->SetLayout(new VerticalLayout);

	this->videoWidget = new VideoWidget(this);

	//Playback Control
	GroupBox *playbackControl = new GroupBox(this);

	this->videoPos = new Slider(playbackControl);

	//actions panel
	GroupBox *actionsPanel = new GroupBox(playbackControl);
	actionsPanel->SetLayout(new HorizontalLayout);

	GroupBox *groupBox = new GroupBox(actionsPanel);
	groupBox->SetText("Video stream");
	//new CDropDown(groupBox);

	groupBox = new GroupBox(actionsPanel);
	groupBox->SetText("Audio stream");
	//new CDropDown(groupBox);

	this->playPauseButton = new PushButton(actionsPanel);
	this->playPauseButton->SetText("Play");
	this->playPauseButton->onActivatedHandler = Function<void()>(&MainWindow::TogglePlayPause, this);

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
}

//Public methods
void MainWindow::OpenFile(const Path &path)
{
	this->Reset();

	if(!path.Exists())
	{
		this->ShowErrorBox("File does not exist", "Input file is not existant.");
		return;
	}

	this->file = new FileInputStream(path);
	this->player = new Multimedia::MediaPlayer(*this->file);
	this->player->SetVideoOutput(this->videoWidget);

	this->UpdateControls();
}