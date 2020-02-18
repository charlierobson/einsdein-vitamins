#pragma once

#include <string>
#include <fstream>
#include <vector>
using namespace std;

#include "dsk.h"


class einyfile
{
public:
	string _name;
	int _size;

	einyfile(char* name) :
		_name(name),
		_size(0),
		_corrupt(false)
	{
		memset(_data, 0, 100 * 1024);
	}

	bool nameis(char* name)
	{
		return _name.compare(name) == 0;
	}

	void write128(unsigned char* ptr, int nsectors, int extent, int block)
	{
		if (_corrupt)
		{
			// no point.
			return;
		}

		int size = nsectors * 128;
		int offset = extent * 16384 + block * 2048;

		if (offset + size > _size)
		{
			_size = offset + size;
		}

		unsigned char* dest = &_data[offset];
		unsigned short* sp = (unsigned short*)ptr;

		memcpy(dest, ptr, size);

		// check for corruption, 1 sector at a time. although we should maybe be checking
		// 512byte device sectors, we'll make do with the reasonable degree of confidence given
		// by checking logical sectors only.

		for (int i = 0; i < nsectors; ++i)
		{
			bool corrupt = true;

			for (int j = 0; j < 64 && corrupt; ++j, ++sp)
			{
				if (*sp != 0xdead)
				{
					corrupt = false;
					break;
				}
			}

			if (corrupt)
			{
				// bail if we find a corrupt sector
				_corrupt = true;
				break;
			}
		}
	}

	bool save(string path)
	{
		if (_corrupt)
			return false;

		ofstream outfile(path, ios::out | ios::binary);
		outfile.write((const char*)_data, _size);

		return outfile.good();
	}

private:
	bool _corrupt;

	unsigned char _data[100 * 1024];
};


class einsteindsk : dsk
{
public:
	vector<einyfile*> _files;

	einsteindsk();
	virtual ~einsteindsk();

	virtual bool load(string pathToDSK);

private:
	bool getfiles();
};
