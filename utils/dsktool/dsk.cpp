#include <stdio.h>
#include <ctype.h>
#include <istream>
#include <fstream>
#include <iostream>

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


bool dsk::parseDSK(vector<char>& rawDSK)
{
	BYTE* fptr = (BYTE*)rawDSK.data();

	TRACK_INFORMATION_BLOCK* tib;
	SECTOR_INFORMATION_BLOCK* sib;

	DISK_INFORMATION_BLOCK* dib = (DISK_INFORMATION_BLOCK*)fptr;
	fptr += sizeof(DISK_INFORMATION_BLOCK);

	if (memcmp(dib, "EXTENDED", 8) != 0)
	{
		return false;
	}

	//!!!! only handling single sided disks ATM

	if (dib->nSides == 2)
	{
		return false;
	}

	_trackCount = dib->nTracks;
	_sectorsPerTrack = 0;

	//!!!! only handling 10 sectors per track
	// 0x15(00)  =  21  =  256+20*256  =  sizeof(header)+10*512

	for (auto i = 0; i < _trackCount; ++i)
	{
		if (dib->trackSizeTable[i] != 0x15)
		{
			return false;
		}
	}

	// ok, work out the sector offsets in the file

	for (auto i = 0; i < _trackCount; ++i)
	{
		tib = (TRACK_INFORMATION_BLOCK*)fptr;
		sib = (SECTOR_INFORMATION_BLOCK*)(fptr + sizeof(TRACK_INFORMATION_BLOCK));

		fptr += 256;
		BYTE* sectorBase = fptr;

		for (auto j = 0; j < 10; ++j)
		{
			_sectorOffsets[i * 10 + sib->sectorID] = fptr;
			fptr += 512;

			++_sectorsPerTrack;
			++sib;
		}
	}

	return true;
}



dsk::dsk() : disk()
{
}

dsk::~dsk()
{
}

bool dsk::load(string fileName)
{
	size_t size;
	std::vector<char> raw;
    ifstream dskFile(fileName, ios::binary|ios::ate);

	if (dskFile.good()) {
		size = dskFile.tellg();
		raw.reserve(size);
		dskFile.seekg(0, ios::beg);
		dskFile.read(raw.data(), size);
	}

	return memcmp(raw.data(), "EXTENDED CPC DSK", 16) == 0
		&& size == 215296
		&& parseDSK(raw);
}
