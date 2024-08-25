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
#pragma once
//Local
#include "Node.hpp"

class SinkNode : public Node
{
public:
    //Constructor
    SinkNode(const ContainerFormat *pFormat, FileOutputStream *pFile, Muxer *pMuxer);

    //Destructor
    ~SinkNode();

    //Properties
	inline const ContainerFormat* Format()
	{
		return this->format;
	}

    inline class Muxer* Muxer()
	{
    	return this->muxer;
	}

    //Methods
	bool CanProcess() const override;
	PortFormat GetInputFormat(uint32 inputPortNumber) const override;
	PortFormat GetOutputFormat(uint32 outputPortNumber) const;
    void ProcessNextEntity() override;

private:
    //Members
    bool headerWritten;
    bool finalized;
    const ContainerFormat *format;
    FileOutputStream *pFile;
    class Muxer *muxer;
};