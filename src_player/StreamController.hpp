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
#include <StdXX.hpp>
using namespace StdXX;
using namespace StdXX::Type;
using namespace StdXX::UI;

template<typename StreamType>
class StreamController : public ListController
{
public:
	//Constructor
	inline StreamController(const Map<uint32, StreamType*>& streams, Multimedia::MediaPlayer& player) : streams(streams), player(player)
	{
	}

	//Methods
	uint32 GetNumberOfItems() const override
	{
		return this->streams.GetNumberOfElements();
	}
	
	String GetText(uint32 index) const override
	{
		auto it = this->GetIteratorAt(index);
		return String::Number((*it).key);
	}

private:
	//Members
	const Map<uint32, StreamType*>& streams;
	Multimedia::MediaPlayer& player;

	//Methods
	template <typename T = StreamType>
	typename EnableIf<IsSameType<T, Multimedia::AudioStream>::value>::type
		SetStreamIndex(uint32 index) const
	{
		this->player.SetAudioStreamIndex(index);
	}

	template <typename T = StreamType>
	typename EnableIf<IsSameType<T, Multimedia::SubtitleStream>::value>::type
		SetStreamIndex(uint32 index) const
	{
		//TODO
		//this->player.SetSubtitleStreamIndex(index);
	}

	template <typename T = StreamType>
	typename EnableIf<IsSameType<T, Multimedia::VideoStream>::value>::type
		SetStreamIndex(uint32 index) const
	{
		this->player.SetVideoStreamIndex(index);
	}

	//Inline
	auto GetIteratorAt(uint32 index) const
	{
		auto it = this->streams.begin();
		while (index--)
			++it;
		return it;
	}

	//Event handlers
	void OnSelectionChanged() const override
	{
		auto it = this->GetIteratorAt(this->view->SelectionController().GetSelectedIndexes().GetFront().GetRow());
		this->SetStreamIndex((*it).key);
	}

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