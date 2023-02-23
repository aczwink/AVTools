/*
 * Copyright (c) 2023 Amir Czwink (amir130@hotmail.de)
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
#include <ACStdLib.h>
using namespace ACStdLib;
//Local
#include "ANode.h"
//Namespaces
using namespace ACStdLib::Multimedia;

//Forward declarations
class CSourceNode;

class CSinkNode : public ANode
{
private:
    //Members
    const IFormat *pFormat;
    CFileOutputStream *pFile;
    AMuxer *pMuxer;

public:
    //Constructor
    CSinkNode(const IFormat *pFormat, CFileOutputStream *pFile, AMuxer *pMuxer);

    //Destructor
    ~CSinkNode();

    //Methods
    void ConnectAll(CSourceNode &refSourceNode);
    bool OutputsRaw() const;
    void Run();
};