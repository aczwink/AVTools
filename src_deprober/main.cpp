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
//Local
#include "Prober.hpp"

int32 Main(const String &programName, const LinkedList<String> &args)
{
	bool headerOnly = false;

	switch(args.GetNumberOfElements())
	{
		case 2:
			if(args[0] == "-header")
				headerOnly = true;
			else
				NOT_IMPLEMENTED_ERROR;
			//fall through
		case 1:
		{
			Path path;

			path = args[args.GetNumberOfElements()-1];
			if(!path.Exists())
			{
				stdErr << "Input file doesn't exist." << endl;
				return EXIT_FAILURE;
			}
			if(path.IsDirectory())
			{
				stdErr << "Input file is a directory." << endl;
				return EXIT_FAILURE;
			}

			Prober prober(path);

			prober.Probe(headerOnly);
			return EXIT_SUCCESS;
		}
	}

	stdOut << "deprober is a tool for testing demuxing and decoding using ACStdLib." << endl
		   << "It is not designed to do anything useful but aid in debugging." << endl << endl
		   << "usage: deprober [options] container" << endl
			<< "options can be:" << endl
			<< "-header\t\tdisplay header info only. Skip payload..." << endl;

	return EXIT_SUCCESS;
}