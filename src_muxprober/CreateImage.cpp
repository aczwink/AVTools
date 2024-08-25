/*
 * Copyright (c) 2024 Amir Czwink (amir130@hotmail.de)
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
using namespace StdXX;
using namespace StdXX::Multimedia;

void CreateRGB24Image(const Math::Size<uint16>& size, const Math::Vector3S& rgb, Packet& packet)
{
	PixelFormat pixelFormat(NamedPixelFormat::BGR_24);
	Pixmap pixmap(size, pixelFormat);
	RGBPixmapView view(pixmap);

	view.SetAllPixels(rgb);

	uint32 imageSize = pixelFormat.ComputeLineSize(0, size.width) * size.height;
	packet.Allocate(imageSize);

	MemCopy(packet.GetData(), pixmap.GetPlane(0), imageSize);
}