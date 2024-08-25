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
//Namespaces
using namespace StdXX;
using namespace StdXX::Multimedia;

struct NodeData
{
    UniquePointer<Frame> frame;
    UniquePointer<IPacket> packet;
};
struct PortFormat
{
    bool packetsOrFrames;
    DecodingParameters frameParameters;
};

class Node
{
public:
    //Destructor
    virtual ~Node(){}

    //Abstract
    virtual bool CanProcess() const = 0;
    virtual PortFormat GetInputFormat(uint32 inputPortNumber) const = 0;
    virtual PortFormat GetOutputFormat(uint32 outputPortNumber) const = 0;
    virtual void ProcessNextEntity() = 0;

    //Inline
    inline void AddData(NodeData&& data)
    {
        this->inputPortQueue.InsertTail(Move(data));
    }

    inline void ConnectOutputPortTo(uint32 outputPortNumber, Node* target, uint32 inputPortNumber)
    {
        this->outputPorts[outputPortNumber] = target;

        if(target->inputPorts.GetNumberOfElements() <= inputPortNumber)
        {
            target->inputPorts.Resize(inputPortNumber + 1);
            target->inputPorts[inputPortNumber] = this;
        }
    }

    inline Node* GetOutputLinkTarget(uint32 index)
    {
        return this->outputPorts[index];
    }

    inline uint32 GetOutputPortCount() const
    {
        return this->outputPorts.GetNumberOfElements();
    }

protected:
    //Members
    DynamicArray<Node *> outputPorts;

    //Inline
    inline NodeData GetNextData()
    {
        return this->inputPortQueue.PopFront();
    }

    inline bool IsDataAvailable() const
    {
        return !this->inputPortQueue.IsEmpty();
    }

    inline bool IsMoreInputFromInputsPortsExpected() const
    {
        for(const auto& input : this->inputPorts)
        {
            if(input->IsMoreInputExpected())
                return true;
        }
        return false;
    }


private:
    //State
    DynamicArray<Node *> inputPorts;
    LinkedList<NodeData> inputPortQueue;

    //Inline
    inline bool IsMoreInputExpected() const
    {
        if(this->CanProcess())
            return true;
        return this->IsMoreInputFromInputsPortsExpected();
    }
};