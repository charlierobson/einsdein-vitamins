#pragma once

#include <string>
#include <vector>

using namespace std;

class disk
{
protected:
	vector<unsigned char> _raw;
	vector<unsigned char*> _sectorOffsets;

	int _trackCount;
	int _sectorsPerTrack;
	int _bytesPerSector;

public:
	disk() { }
	virtual ~disk() { }

	static vector<unsigned char> loadBytes(string filePath);

	int size();
	int sectorCount();

	void init(int tracks, int sectors, int bytesPerSector);

	virtual bool load(string fileName);
	virtual bool save(string fileName);

	vector<unsigned char> readSectors(int startSector, int sectorCount);
	void writeSectors(int startSector, int sectorCount, unsigned char* sectorData);
};
