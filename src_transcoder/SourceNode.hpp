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
#include <StdXX.hpp>
//Local
#include "Node.hpp"
//Namespaces
using namespace StdXX;
using namespace StdXX::Multimedia;

class SourceNode : public Node
{
public:
    //Constructor
    SourceNode(FileInputStream *fileInputStream, Demuxer* demuxer);

    //Destructor
    ~SourceNode();

    //Methods
    bool CanProcess() const override;
    PortFormat GetInputFormat(uint32 inputPortNumber) const override;
    PortFormat GetOutputFormat(uint32 outputPortNumber) const;
    void ProcessNextEntity() override;

    //Inline
    inline Demuxer *GetDemuxer()
    {
        return this->demuxer;
    }

private:
    //Members
    bool endOfPacketsReached;
    FileInputStream* fileInputStream;
    Demuxer* demuxer;
};