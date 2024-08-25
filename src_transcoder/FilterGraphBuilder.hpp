/*
 * Copyright (c) 2023-2024 Amir Czwink (amir130@hotmail.de)
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
//Local
#include "FilterGraph.hpp"
#include "SourceNode.hpp"
#include "SinkNode.hpp"
//Namespaces
using namespace StdXX;

struct MountPort
{
	Node* node;
	uint32 outputPortNumber;
};

class FilterGraphBuilder
{
public:
	//Constructor
	inline FilterGraphBuilder(FilterGraph& filterGraph) : filterGraph(filterGraph)
	{
	}

	//Methods
	void InsertAudioResampler(DataType dataType, const DecodingParameters& sourceFormat, const AudioSampleFormat& targetFormat);
	void InsertDecoder(DataType dataType);
	void InsertEncoder(DataType dataType, CodingFormatId codingFormatId);
	bool LoadSink(const FileSystem::Path& outputPath);
	bool LoadSource(const FileSystem::Path& inputPath);

private:
	//State
	BinaryTreeMap<DataType, uint32> selectedStreams;
	FilterGraph& filterGraph;
	SinkNode* sinkNode;
	SourceNode* sourceNode;

	//Methods
	void ConnectToSink();
	MountPort Follow(uint32 sourceStreamIndex);
	bool IsCodingFormatSupported(DataType dataType, const ContainerFormat* containerFormat, const CodingFormat* codingFormat) const;
	void PreselectStreams();
	void SmartConnect(uint32 sourceStreamIndex, Node* target, uint32 inputPortNumber);
};