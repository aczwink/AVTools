/*
 * Copyright (c) 2017-2018 Amir Czwink (amir130@hotmail.de)
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
#include "MainWindow.hpp"

int32 Main(const String &programName, const FixedArray<String> &args)
{
	EventHandling::StandardEventQueue eventQueue;
	MainWindow *mainWindow = new MainWindow(eventQueue);

	if (!args.IsEmpty())
		mainWindow->OpenFile(args[0]);
	mainWindow->Show();

	eventQueue.ProcessEvents();
	return EXIT_SUCCESS;
}