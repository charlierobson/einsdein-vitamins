#pragma once

#include <string>
using namespace std;

class disk
{
protected:
	unsigned char* _buffer;
	int _diskSize;

public:
	disk();
	virtual ~disk();
	virtual void format(disk* dosSrc);
	virtual int load(string fileName);
	virtual int save(string fileName);
	virtual int readSectors(void* dest, int startSector, int sectorCount);
	virtual int writeSectors(void* src, int startSector, int sectorCount);

	enum
	{
		error_none,
		error_invalid_format,
		error_file_operation,
		error_invalid_sector,
		error_not_implemented,

		error_max
	};
};
