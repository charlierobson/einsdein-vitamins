#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <vector>

#include "dsk.h"

typedef unsigned char BYTE;
typedef unsigned short WORD;


typedef struct
{
	char headerString[34];
	char creatorName[14];
	BYTE nTracks;
	BYTE nSides;
	BYTE unused[2];
	BYTE trackSizeTable[256-52];
}
DISK_INFORMATION_BLOCK;

typedef struct
{
	char headerString[13];
	BYTE unused[3];
	BYTE trackNumber;
	BYTE sideNumber;
	BYTE unused2[2];
	BYTE sectorSize;
	BYTE nSectors;
	BYTE GAPHASH3Length;
	BYTE filler;
}
TRACK_INFORMATION_BLOCK;

typedef struct
{
	BYTE track;
	BYTE side;
	BYTE sectorID;
	BYTE sectorSize;
	BYTE FDCStatusRegister1;
	BYTE FDCStatusRegister2;
	WORD dataLength;
}
SECTOR_INFORMATION_BLOCK;



int dsk::parseDSK()
{
	int i, j;
	BYTE* fptr = _buffer;

	_tracks = 0;
	_sectors = 0;

	TRACK_INFORMATION_BLOCK* tib;
	SECTOR_INFORMATION_BLOCK* sib;

	DISK_INFORMATION_BLOCK* dib = (DISK_INFORMATION_BLOCK*)fptr;
	fptr += sizeof(DISK_INFORMATION_BLOCK);

	if (!memcmp(dib, "EXTENDED", sizeof("EXTENDED")))
	{
		return dsk::error_invalid_format;
	}

	//!!!! only handling single sided disks ATM

	if (dib->nSides == 2)
	{
		return dsk::error_invalid_format;
	}

	_tracks = dib->nTracks;

	//!!!! only handling 10 sectors per track
	// 0x15(00)  =  21  =  256+20*256  =  sizeof(header)+10*512

	for (i = 0; i < _tracks; ++i)
	{
		if (dib->trackSizeTable[i] != 0x15)
		{
			return dsk::error_invalid_format;
		}
	}

	// ok, work out the sector offsets in the file

	for (i = 0; i < _tracks; ++i)
	{
		tib = (TRACK_INFORMATION_BLOCK*)fptr;
		sib = (SECTOR_INFORMATION_BLOCK*)(fptr + sizeof(TRACK_INFORMATION_BLOCK));

		fptr += 256;
		BYTE* sectorBase = fptr;

		for(j = 0; j < 10; ++j)
		{
			_sectorOffsets[i * 10 + sib->sectorID] = fptr;
			fptr += 512;

			++_sectors;
			++sib;
		}
	}

	return dsk::error_none;
}



dsk::dsk()
{
	_diskSize = 0;
	_buffer = NULL;
}

dsk::~dsk()
{
}

int dsk::load(string fileName)
{
	int result = disk::error_none;

	FILE* src = fopen(fileName.c_str(), "rb");
	if (src != NULL)
	{
		fseek(src, 0, SEEK_END);
		_diskSize = ftell(src);
		rewind(src);

		_buffer = new BYTE[_diskSize];

		fread_s(_buffer, _diskSize, 1, _diskSize, src);
		fclose(src);

		result = disk::error_invalid_format;
		if (memcmp(_buffer, "EXTENDED CPC DSK", 16) == 0 && _diskSize == 215296)
		{
			result = parseDSK();
		}
	}

	return result;
}

vector<unsigned char> dsk::sector(int sector) {
	return vector<unsigned char>(_sectorOffsets[sector], _sectorOffsets[sector] + 512);
}


int dsk::readSectors(void* dest, int startSector, int sectorCount)
{
	BYTE* p = (BYTE*)dest;
	if (startSector + sectorCount >= 400)
	{
		return disk::error_invalid_sector;
	}
	for (int i = startSector; i < startSector + sectorCount; ++i)
	{
		memcpy(p, _sectorOffsets[i], 512);
		p += 512;
	}

	return 0;
}

int dsk::writeSectors(void* src, int startSector, int sectorCount)
{
	BYTE* p = (BYTE*)src;
	if (startSector + sectorCount >= 400)
	{
		return disk::error_invalid_sector;
	}

	for (int i = startSector; i < startSector + sectorCount; ++i)
	{
		memcpy(_sectorOffsets[i], p, 512);
		p += 512;
	}

	return 0;
}
