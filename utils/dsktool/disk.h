#pragma once

#include <string>
#include <vector>
using namespace std;

class disk
{
protected:
	unsigned char* _buffer;
	unsigned char** _sectorOffsets;

	int _trackCount;
	int _sectorsPerTrack;
	int _bytesPerSector;

public:
	disk();
	virtual ~disk();

	int size();
	int sectorCount();

	virtual bool load(string fileName);
	virtual bool save(string fileName);

	vector<unsigned char> readSectors(int startSector, int sectorCount);
	void writeSectors(int startSector, int sectorCount, vector<unsigned char> sectorData);
};
