#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <vector>

#include "disk.h"


disk::disk()
{
	_trackCount = 40;
	_sectorsPerTrack = 10;
	_bytesPerSector = 512;

	_buffer = new unsigned char [size()];
	_sectorOffsets = new unsigned char* [sectorCount()];
}

disk::~disk()
{
	delete[] _buffer;
	delete[] _sectorOffsets;
}


int disk::size() {
	return _trackCount * _sectorsPerTrack * _bytesPerSector;
}

int disk::sectorCount() {
	return _trackCount * _sectorsPerTrack;
}

bool disk::load(string fileName)
{
	auto src = fopen(fileName.c_str(), "rb");
	if (src != NULL)
	{
		fread(_buffer, 1, size(), src);
		fclose(src);

		for (auto i = 0; i < sectorCount(); ++i) {
			_sectorOffsets[i] = _buffer + i * _bytesPerSector;
		}
	}

	return src != NULL;
}

bool disk::save(string fileName)
{
	auto dst = fopen(fileName.c_str(), "wb");
	if (dst != NULL)
	{
		for (auto i = 0; i < sectorCount(); ++i) {
			fwrite(_sectorOffsets[i], 1, _bytesPerSector, dst);
		}

		fclose(dst);
	}

	return dst != NULL;
}

vector<unsigned char> disk::readSectors(int startSector, int sectorCount)
{
	vector<unsigned char> sectors;
	sectors.resize(sectorCount * _bytesPerSector);
	for (auto i = 0; i < sectorCount; ++i) {
		memcpy(sectors.data() + i * _bytesPerSector, _sectorOffsets[startSector + i], _bytesPerSector);
	}

	return sectors;
}

void disk::writeSectors(int startSector, int sectorCount, vector<unsigned char> sectors)
{
	for (auto i = 0; i < sectorCount; ++i) {
		memcpy(_sectorOffsets[startSector+ i], sectors.data() + i * _bytesPerSector, _bytesPerSector);
	}
}
