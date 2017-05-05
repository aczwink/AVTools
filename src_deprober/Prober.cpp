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
#include "Prober.hpp"

//Constructor
Prober::Prober(const Path &path) : path(path), input(path)
{
	this->packetCounter = 0;
}

//Destructor
Prober::~Prober()
{
	delete this->pDemuxer;
}

//Public methods
void Prober::Probe()
{
	//first find format
	this->pFormat = IFormat::Find(this->input);
	if(!this->pFormat)
	{
		//second try by extension
		this->pFormat = IFormat::FindByExtension(this->path.GetFileExtension());
	}

	if(!this->pFormat)
	{
		stdErr << "No format could be matched with the input." << endl;
		return;
	}
}