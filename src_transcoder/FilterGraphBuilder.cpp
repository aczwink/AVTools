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
//Class header
#include "FilterGraphBuilder.hpp"
//Local
#include "DecoderNode.hpp"
#include "EncoderNode.hpp"
#include "AudioResampleNode.hpp"

//Public methods
void FilterGraphBuilder::InsertAudioResampler(DataType dataType, const DecodingParameters& sourceFormat, const AudioSampleFormat& targetFormat)
{
	uint32 sourceStreamIndex = this->selectedStreams.Get(dataType);

	AudioResampleNode* resampleNode = new AudioResampleNode(sourceFormat, targetFormat);
	this->filterGraph.AddNode(resampleNode);
	this->SmartConnect(sourceStreamIndex, resampleNode, 0);
}

void FilterGraphBuilder::InsertDecoder(DataType dataType)
{
	uint32 sourceStreamIndex = this->selectedStreams.Get(dataType);
	Demuxer* demuxer = this->sourceNode->GetDemuxer();
	Stream* sourceStream = demuxer->GetStream(sourceStreamIndex);

	DecoderNode* decoderNode = new DecoderNode(sourceStream);
	this->filterGraph.AddNode(decoderNode);

	this->SmartConnect(sourceStreamIndex, decoderNode, 0);
}

void FilterGraphBuilder::InsertEncoder(DataType dataType, CodingFormatId codingFormatId)
{
	uint32 sourceStreamIndex = this->selectedStreams.Get(dataType);

	EncodingParameters codingParameters = this->sourceNode->GetDemuxer()->GetStream(sourceStreamIndex)->codingParameters;
	codingParameters.codingFormat = FormatRegistry::Instance().FindCodingFormatById(codingFormatId);

	switch(codingParameters.dataType)
	{
		case DataType::Audio:
			codingParameters.audio.sampleFormat->sampleType = codingParameters.codingFormat->GetPreferredSampleType();
			break;
		default:
			NOT_IMPLEMENTED_ERROR; //TODO: implement me
	}

	auto encoder = codingParameters.codingFormat->GetBestMatchingEncoder();
	if(encoder == nullptr)
		NOT_IMPLEMENTED_ERROR;

	EncoderNode* encoderNode = new EncoderNode(encoder->CreateContext(codingParameters));
	this->filterGraph.AddNode(encoderNode);

	this->SmartConnect(sourceStreamIndex, encoderNode, 0);
}

bool FilterGraphBuilder::LoadSink(const FileSystem::Path& outputPath)
{
	const ContainerFormat* format = FormatRegistry::Instance().FindFormatByFileExtension(outputPath.GetFileExtension());
	if(!format)
	{
		stdErr << "No format could be found for file '" << outputPath << "'." << endl;
		return false;
	}

	FileOutputStream* file = new FileOutputStream(outputPath);

	Muxer *pMuxer = format->CreateMuxer(*file);
	if(!pMuxer)
	{
		stdErr << "No muxer is available for format '" << format->GetName() << "'." << endl;
		delete file;
		return false;
	}

	SinkNode *pSink = new SinkNode(format, file, pMuxer);

	this->sinkNode = pSink;
	this->filterGraph.AddNode(pSink);

	this->ConnectToSink();

	return true;
}

bool FilterGraphBuilder::LoadSource(const FileSystem::Path& inputPath)
{
	FileSystem::File file(inputPath);
	if(!file.Exists())
	{
		stdErr << "File '" << inputPath << "' does not exist." << endl;
		return false;
	}

	FileInputStream* fileInputStream = new FileInputStream(inputPath);
	const ContainerFormat *format = FormatRegistry::Instance().ProbeFormat(*fileInputStream);
	if(!format)
	{
		//second try by extension
		format = FormatRegistry::Instance().FindFormatByFileExtension(inputPath.GetFileExtension());
		if(format)
		{
			stdOut << "Warning: File format couldn't be identified else than by extension. This is might be not avoidable but is unsafe." << endl;
		}
		else
		{
			stdErr << "No format could be found for file '" << inputPath << "'. Either the format is not supported or this is not a valid media file." << endl;
			delete fileInputStream;
			return false;
		}
	}

	Demuxer* demuxer = format->CreateDemuxer(*fileInputStream);
	if(!demuxer)
	{
		stdErr << "No demuxer is available for format '" << format->GetName() << "'." << endl;
		delete fileInputStream;
		return false;
	}

	demuxer->ReadHeader();
	if(!demuxer->FindStreamInfo())
	{
		stdErr << "Not all info could be gathered for file '" << inputPath << "'. Expect errors..." << endl;
	}

	SourceNode* sourceNode = new SourceNode(fileInputStream, demuxer);

	this->sourceNode = sourceNode;
	this->filterGraph.AddNode(sourceNode);
	this->PreselectStreams();

	return true;
}

//Private methods
void FilterGraphBuilder::ConnectToSink()
{
	Demuxer* demuxer = this->sourceNode->GetDemuxer();
	Muxer* muxer = this->sinkNode->Muxer();
	for(const auto& kv : this->selectedStreams)
	{
		Stream* sourceStream = demuxer->GetStream(kv.value);

		//create stream in sink
		Stream* destStream = new Stream(sourceStream->codingParameters.dataType);
		//copy parameters
		destStream->timeScale = sourceStream->timeScale;
		destStream->startTime = sourceStream->startTime;
		destStream->duration = sourceStream->duration;

		muxer->AddStream(destStream);

		MountPort mountPort = this->Follow(kv.value);
		auto format = mountPort.node->GetOutputFormat(mountPort.outputPortNumber);
		bool isTargetFormatSupported = this->IsCodingFormatSupported(sourceStream->codingParameters.dataType, this->sinkNode->Format(), format.frameParameters.codingFormat);
		ASSERT(isTargetFormatSupported, u8"TODO: reencoding necessary");

		destStream->codingParameters = format.frameParameters;

		if(!format.packetsOrFrames)
		{
			NOT_IMPLEMENTED_ERROR; //TODO: encoder required
			//encoding required
			destStream->codingParameters.codingFormat = this->sinkNode->Format()->GetPreferredCodingFormat(sourceStream->codingParameters.dataType);

			EncoderNode* pEncoderNode = new EncoderNode(destStream->GetEncoderContext());
			this->filterGraph.AddNode(pEncoderNode);
			//this->Connect(kv.value, pEncoderNode);
			//MountPort mountPort = this->Follow(sourceStreamIndex);
			//mountPort.node->ConnectOutputPortTo(mountPort.outputPortNumber, target);
		}

		this->SmartConnect(kv.value, this->sinkNode, muxer->GetNumberOfStreams() - 1);
	}
}

MountPort FilterGraphBuilder::Follow(uint32 sourceStreamIndex)
{
	Node* prev = this->sourceNode;
	while(prev->GetOutputLinkTarget(sourceStreamIndex) != nullptr)
	{
		prev = prev->GetOutputLinkTarget(sourceStreamIndex);
		if(prev->GetOutputPortCount() != 1)
			NOT_IMPLEMENTED_ERROR;
		sourceStreamIndex = 0;
	}

	return {
		.node = prev,
		.outputPortNumber = sourceStreamIndex
	};
}

bool FilterGraphBuilder::IsCodingFormatSupported(DataType dataType, const ContainerFormat* containerFormat, const CodingFormat* codingFormat) const
{
	auto supportedFormats = containerFormat->GetSupportedCodingFormats(dataType);
	for(const CodingFormat* supportedFormat : supportedFormats)
	{
		if(supportedFormat == codingFormat)
			return true;
	}
	return false;
}

void FilterGraphBuilder::PreselectStreams()
{
	Demuxer* demuxer = this->sourceNode->GetDemuxer();
	for(uint32 i = 0; i < demuxer->GetNumberOfStreams(); i++)
	{
		Stream* stream = demuxer->GetStream(i);
		if(!this->selectedStreams.Contains(stream->codingParameters.dataType))
			this->selectedStreams[stream->codingParameters.dataType] = i;
	}
}

void FilterGraphBuilder::SmartConnect(uint32 sourceStreamIndex, Node* target, uint32 inputPortNumber)
{
	MountPort mountPort = this->Follow(sourceStreamIndex);
	auto outputFormat = mountPort.node->GetOutputFormat(mountPort.outputPortNumber);
	auto inputFormat = target->GetInputFormat(inputPortNumber);

	ASSERT_EQUALS(inputFormat.frameParameters.dataType, outputFormat.frameParameters.dataType);

	if(inputFormat.packetsOrFrames == outputFormat.packetsOrFrames)
	{
		if(!inputFormat.packetsOrFrames)
		{
			switch(inputFormat.frameParameters.dataType)
			{
				case DataType::Audio:
					if(inputFormat.frameParameters.audio.sampleFormat != outputFormat.frameParameters.audio.sampleFormat)
					{
						this->InsertAudioResampler(DataType::Audio, outputFormat.frameParameters, *inputFormat.frameParameters.audio.sampleFormat);
						return this->SmartConnect(sourceStreamIndex, target, inputPortNumber);
					}
					break;
				default:
					NOT_IMPLEMENTED_ERROR; //TODO: implment me
			}
		}
		mountPort.node->ConnectOutputPortTo(mountPort.outputPortNumber, target, inputPortNumber);
	}
	else
		NOT_IMPLEMENTED_ERROR; //TODO: implment me
}