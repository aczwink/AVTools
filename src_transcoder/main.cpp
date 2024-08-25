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
//Local
#include "SinkNode.hpp"
#include "SourceNode.hpp"
#include "FilterGraph.hpp"
#include "FilterGraphBuilder.hpp"

static bool ParseAudioFilter(const String& filter, FilterGraphBuilder& builder, const BinaryTreeMap<String, CodingFormatId>& codecStringMap)
{
	if(filter == u8"decode")
	{
		builder.InsertDecoder(DataType::Audio);
	}
	else if(filter.StartsWith(u8"encode="))
	{
		String codec = filter.SubString(7);
		builder.InsertEncoder(DataType::Audio, codecStringMap.Get(codec));
	}
	else
		return false;
	return true;
}

static bool ParseFilters(const FixedArray<String>& args, FilterGraphBuilder& builder, const BinaryTreeMap<String, CodingFormatId>& codecStringMap)
{
	for(uint32 i = 1; i < args.GetNumberOfElements() - 1; i++)
	{
		const auto& arg = args[i];

		if(arg == u8"--f:a")
		{
			if(!ParseAudioFilter(args[i+1], builder, codecStringMap))
				return false;
			i++;
		}
	}

	return true;
}

static void PrintManual()
{
	stdOut
			<< u8"Usage: " << endl
			<< u8"  transcoder" << " inputFile outputFile" << endl << endl;
}

int32 Main(const String& programName, const FixedArray<String>& args)
{
    PrintManual();

    BinaryTreeMap<String, CodingFormatId> codecStringMap;
    codecStringMap.Insert(u8"pcm_f32le", CodingFormatId::PCM_Float32LE);

    FilterGraph filterGraph;
	FilterGraphBuilder builder(filterGraph);

	if(!builder.LoadSource(args[0]))
	    return EXIT_FAILURE;
	if(!ParseFilters(args, builder, codecStringMap))
		return EXIT_FAILURE;
    if(!builder.LoadSink(args[args.GetNumberOfElements() - 1]))
        return EXIT_FAILURE;

	filterGraph.Run();
	return EXIT_SUCCESS;
}