/*
 * Copyright (c) 2017-2023 Amir Czwink (amir130@hotmail.de)
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
#include <StdXX.hpp>
using namespace StdXX;
using namespace StdXX::Type;
using namespace StdXX::UI;

class StreamController : public ListController
{
public:
	//Constructor
	inline StreamController(const BinaryTreeMap<uint32, Multimedia::Stream*>& streams, Multimedia::MediaPlayer& player) : streams(streams), player(player)
	{
		if(!streams.IsEmpty())
			this->dataType = (*streams.begin()).value->codingParameters.dataType;
	}

	//Methods
	uint32 GetNumberOfItems() const override
	{
		return this->streams.GetNumberOfElements();
	}
	
	String GetText(uint32 index) const override
	{
		auto it = this->streams.begin();
		while (index--)
			++it;
		return String::Number((*it).key);
	}

private:
	//Members
	Multimedia::DataType dataType;
	const BinaryTreeMap<uint32, Multimedia::Stream*>& streams;
	Multimedia::MediaPlayer& player;

	//Methods
	void SetStreamIndex(uint32 index) const
	{
		if(this->dataType == Multimedia::DataType::Audio)
			this->player.SetAudioStreamIndex(index);
		else if(this->dataType == Multimedia::DataType::Video)
			this->player.SetVideoStreamIndex(index);
	}

	//Event handlers
	/*void OnSelectionChanged() const override
	{
		auto it = this->GetIteratorAt(this->view->SelectionController().GetSelectedIndexes().GetFront().GetRow());
		this->SetStreamIndex((*it).key);
	}*/

	void OnViewChanged() override
	{
		//auto select first stream
		if (!this->streams.IsEmpty())
		{
			ControllerIndex first = this->GetChildIndex(0, 0, ControllerIndex());
			this->view->Select(first);
		}
	}
};