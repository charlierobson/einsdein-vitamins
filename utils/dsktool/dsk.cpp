#include <stdio.h>
#include <ctype.h>
#include <fstream>

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
//24

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
// 8

bool dsk::parseDSK()
{
	BYTE* fptr = (BYTE*)_raw.data();

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

	//!!!! only handling 10 sectors per track

	init(dib->nTracks, 10, 512);

	// 0x15(00)  =  21  =  256+20*256  =  sizeof(header)+10*512

	for (auto i = 0; i < _trackCount; ++i)
	{
		if (dib->trackSizeTable[i] != 0x15)
		{
			return false;
		}
	}

	// ok, work out the sector offsets in the file

	for (auto track = 0; track < _trackCount; ++track)
	{
		tib = (TRACK_INFORMATION_BLOCK*)fptr;
		sib = (SECTOR_INFORMATION_BLOCK*)(fptr + sizeof(TRACK_INFORMATION_BLOCK));

		fptr += 256;
		BYTE* sectorBase = fptr;

		for (auto sector = 0; sector < 10; ++sector)
		{
			_sectorOffsets[track * 10 + sib->sectorID] = fptr;
			fptr += 512;

			++_sectorsPerTrack;
			++sib;
		}
	}

	return true;
}

bool dsk::load(string fileName)
{
	_raw = loadBytes(fileName);
	return _raw.size() == 215296
		&& memcmp(_raw.data(), "EXTENDED CPC DSK", 16) == 0
		&& parseDSK();
}

bool dsk::save(string fileName)
{
	ofstream outfile(fileName, ios::out | ios::binary);
	outfile.write((const char*)_raw.data(), 215296);
	return outfile.good();
}

