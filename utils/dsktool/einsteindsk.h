#pragma once

#include <string>
#include <vector>
using namespace std;

#include "dsk.h"


class einyfile
{
public:
	einyfile(char* name, char* root) :
		_name(name),
		_root(root),
		_size(0),
		_corrupt(false)
	{
		memset(_data, 0, 100 * 1024);
	}

	bool nameis(char* name)
	{
		return _root.compare(name) == 0;
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

	int close()
	{
		int err = -1;

		printf(" %s - ", _root.c_str());

		if (_corrupt)
		{
			puts(" File is corrupt.");
		}
		else
		{
			FILE* outfile;

			err = fopen_s(&outfile, _name.c_str(), "wb");
			if (!err)
			{
				fwrite(_data, _size, 1, outfile);
				fclose(outfile);

				printf(" Wrote %d\n", _size);
			}
			else
			{
				printf(" Failed: err = %d\n", err);
			}
		}

		return err;
	}

private:
	bool _corrupt;

	int _size;

	std::string _name, _root;

	unsigned char _data[100 * 1024];
};


class einsteindsk
{
	dsk& _dsk;
	void getfiles();

public:
	einsteindsk(dsk& srcDSK);
	virtual ~einsteindsk();

	vector<string> dir();
//	vector<einyfile> files();
};
