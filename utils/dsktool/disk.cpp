#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "disk.h"


disk::disk()
{
	_diskSize = 400 * 512;
	_buffer = new unsigned char [_diskSize];
}

disk::~disk()
{
	delete[] _buffer;
}

void disk::format(disk* dosSrc)
{
	unsigned char _sectr[512];
	if (dosSrc != NULL)
	{
		for (int i = 0; i < 20; ++i)
		{
			dosSrc->readSectors(_sectr, i, 1);
			writeSectors(_sectr, i, 1);
		}
	}
	memset(_sectr, 0xe5, 512);
	for (int i = 20; i < 400; ++i)
	{
		writeSectors(_sectr, i, 1);
	}
}

int disk::load(string fileName)
{
	FILE* src;
	int result = fopen_s(&src, fileName.c_str(), "rb");
	if (!result)
	{
		fread_s(_buffer, 400*512, 1, 400*512, src);
		fclose(src);
	}
	return result;
}

int disk::save(string fileName)
{
	FILE* dst;
	int result = fopen_s(&dst, fileName.c_str(), "wb");
	if (!result)
	{
		fwrite(_buffer, 1, _diskSize, dst);
		fclose(dst);
	}
	return result;
}

int disk::readSectors(void* dest, int startSector, int sectorCount)
{
	if (startSector + sectorCount >= 400)
	{
		return disk::error_invalid_sector;
	}

	memcpy(dest, &_buffer[startSector * 512], sectorCount * 512);
	return 0;
}

int disk::writeSectors(void* src, int startSector, int sectorCount)
{
	if (startSector + sectorCount >= 400)
	{
		return disk::error_invalid_sector;
	}

	memcpy(&_buffer[startSector * 512], src, sectorCount * 512);
	return 0;
}
