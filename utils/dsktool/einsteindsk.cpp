#include <iostream>
#include <algorithm>
#include "einsteindsk.h"

using namespace std;

typedef unsigned char BYTE;


// einstein dos format constants

const int SCT_MAX = 512;				// bytes per sector

const int DOS_MAX = SCT_MAX * 10 * 2;	// size of DOS on disk, 2 tracks of 10 sectors of 512 bytes
const int DOS_SECT = DOS_MAX / SCT_MAX;

const int DIR_MAX = 64 * 32;			// 64 entries at 32 bytes per entry
const int DIR_SECT = DIR_MAX / SCT_MAX;

const int DSK_MAX = 40 * 10 * SCT_MAX;	// 40 tracks of 10 sectors of 512 bytes

const int BLK_MAX = 2048;				// bytes per disk allocation block

typedef struct
{
	BYTE user;
	char filename[8 + 3];
	BYTE extent;
	BYTE reserved1, reserved2;
	BYTE blockCount;
	short blockIDs[8];
}
dirent;


// helper when decoding names in einstein direntry format
static char* fnFromEin(char* dest, const char* src, int max)
{
	for (auto i = 0; i < max; ++i)
	{
		if (*src == ' ')
			break;
		if (*src == '/')
			*dest = '-';
		else
			*dest = toupper(*src & 127);
		++dest;
		++src;
	}

	return dest;
}


// helper when converting names to einstein direntry format
static char* fnToEin(char*dst, char* src, int limit)
{
	while (*src && *src != '.' && limit)
	{
		*dst = toupper(*src);
		++dst;
		++src;

		--limit;
	}

	return src;
}


einsteindsk::einsteindsk() : dsk()
{
}

einsteindsk::~einsteindsk() {
}

bool einsteindsk::load(string pathToDSKFile) {
	bool success = dsk::load(pathToDSKFile);
	return success & getfiles();
}

bool einsteindsk::save(string pathToDSKFile) {
	bool success = putfiles();
	return success & dsk::save(pathToDSKFile);
}


bool einsteindsk::putfiles()
{
	int block = 1;

	int sector = DOS_SECT + DIR_SECT;

	int freeExtents = 64;
	int freeBlocks = (DSK_MAX - DOS_MAX - DIR_MAX) / BLK_MAX;

	dirent directory[64];
	dirent* directoryEntry = directory;
	memset(directory, 0xe5, sizeof(directory));

	for_each(_files.begin(), _files.end(), [&](einyfile* file)
	{
		int recordCount = (file->_size + 127) / 128;		// a record is 128 bytes. all files are a multiple of this size.
		int blockCount = (recordCount + 15) / 16;		// all files are allocated in 2k blocks on disk.
		int extentCount = (blockCount + 7) / 8;		// an extent holds up to 128 records in 8 blocks.

		if (freeExtents < extentCount)
		{
			cout << "Not enough free extents for " << file->_name << ": ignoring" << endl;
			return;
		}
		else if (freeBlocks < blockCount)
		{
			cout << "Not enough free blocks for " << file->_name << ": ignoring" << endl;
			return;
		}

		cout << "Adding " << file->_name << endl;

		memset(directoryEntry, 0, sizeof(dirent) * extentCount);
		file->resetRead();

		for (int extent = 0; extent < extentCount; ++extent)
		{
			directoryEntry->extent = extent;

			directoryEntry->blockCount = recordCount < 128 ? recordCount : 128;
			recordCount -= directoryEntry->blockCount;

			int blocks = blockCount < 8 ? blockCount : 8;
			blockCount -= blocks;

			memset(&directoryEntry->filename[0], 32, 11);

			char* src = fnToEin(&directoryEntry->filename[0], (char*)file->_name.c_str(), 8);
			if (*src == '.')
			{
				++src;
				fnToEin(&directoryEntry->filename[8], src, 3);
			}

			for (int i = 0; i < blocks; ++i)
			{
				directoryEntry->blockIDs[i] = block;

				writeSectors(sector, 4, file->readBytes(BLK_MAX));

				--freeBlocks;
				sector += 4;
				++block;
			}

			++directoryEntry;
			--freeExtents;
		}
	});

	// write the directory after the DOS tracks
	//
	writeSectors(DOS_SECT, DIR_SECT, (unsigned char*)directory);

	return true;
}

bool einsteindsk::getfiles()
{
	_files.clear();

	for (auto i = 20; i < 24; ++i)
	{
		char bfr[13] = { 0 };

		vector<BYTE> sectorBytes = readSectors(i, 1);
		dirent* p = (dirent*)(sectorBytes.data());

		for (auto j = 0; j < 16; ++j, ++p)
		{
			// for each directory entry in this sector:

			if (p->user == 0 && p->blockCount > 0 && p->blockCount <= 0x80)
			{
				// directory entry has passed some very rudimentary sanity check, so see if we have an object for this file already

				// create an 8.3 filename from something of the form 'NAME    EXT'
				//
				// copy up to 8 characters, but stop at a space, leaving a pointer
				// to the next character in the output buffer to be filled.
				char* pp = fnFromEin(bfr, &p->filename[0], 8);
				if (p->filename[8] != ' ')
				{
					// the filename has an extension,
					// so add a separator to the output buffer
					*pp = '.';
					++pp;
				}
				// finally copy up to 3 characters from the extension,
				// again stopping at any whitespace character, then zero terminate the string.
				// a read-only file is indicated by having bit 7 of the first character of
				// the extension set. the copy function strips this.
				//
				pp = fnFromEin(pp, &p->filename[8], 3);
				*pp = 0;

				einyfile* outfile = NULL;

				std::vector<einyfile*>::iterator it;
				for (it = _files.begin(); it != _files.end(); ++it)
				{
					einyfile* tfo = *it;
					if (tfo->nameis(bfr))
					{
						outfile = tfo;
						break;
					}
				}

				if (outfile == NULL)
				{
					// no file object exists for the current directory entry: create one

					outfile = new einyfile(bfr);
					_files.push_back(outfile);
				}

				//printf("%c%s, [%d] $%02x\n", p->filename[9] > 128 ? '*' : ' ', bfr, p->extent, p->blockCount);

				// write all of the blocks associated with this extent

				// each block contains up to 16 x 128byte sectors, or 2kbytes.
				// each extent contains 8 blocks, or 16kbytes.
				//
				int block = 0;
				int nb = p->blockCount;

				while (nb > 0)
				{
					// read the block
					int base = (p->blockIDs[block] - 1) * 4 + 24;
					auto sectors = readSectors(base, 4);
					outfile->write128(sectors.data(), nb > 16 ? 16 : nb, p->extent, block);

					nb -= 16;
					++block;
				}
			}
		}
	}

	return _files.size() > 0;
}
