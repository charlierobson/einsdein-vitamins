#pragma once

#include<string>
using namespace std;

#include "disk.h"

class dsk : public disk
{
private:
	int _tracks;
	int _sectors;
	unsigned char* _sectorOffsets[400];

	int parseDSK();

public:
	dsk();
	virtual ~dsk();
	virtual int load(string fileName);

	vector<unsigned char> sector(int sector);

	virtual int readSectors(void* dest, int startSector, int sectorCount);
	virtual int writeSectors(void* src, int startSector, int sectorCount);
};
