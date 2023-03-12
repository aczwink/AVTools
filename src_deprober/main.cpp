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
//Local
#include "Prober.hpp"

int32 Main(const String &programName, const FixedArray<String> &args)
{
	bool headerOnly = false;

	switch(args.GetNumberOfElements())
	{
		case 2:
			if(args[0] == u8"-header")
				headerOnly = true;
			else
				NOT_IMPLEMENTED_ERROR;
			//fall through
		case 1:
		{
			FileSystem::Path path = FileSystem::FileSystemsManager::Instance().OSFileSystem().FromNativePath(args[args.GetNumberOfElements()-1]);
			FileSystem::File file(path);

			if(!file.Exists())
			{
				stdErr << u8"Input file doesn't exist." << endl;
				return EXIT_FAILURE;
			}
			if(file.Type() == FileSystem::FileType::Directory)
			{
				stdErr << u8"Input file is a directory." << endl;
				return EXIT_FAILURE;
			}

			Prober prober(path);

			prober.Probe(headerOnly);
			return EXIT_SUCCESS;
		}
	}

	stdOut << u8"deprober is a tool for testing demuxing and decoding using Std++." << endl
		   << u8"It is not designed to do anything useful but aid in debugging." << endl << endl
		   << u8"usage: deprober [options] container" << endl
			<< u8"options can be:" << endl
			<< u8"-header\t\tdisplay header info only. Skip payload..." << endl;

	return EXIT_SUCCESS;
}