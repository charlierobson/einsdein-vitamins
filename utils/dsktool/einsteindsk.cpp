#include "einsteindsk.h"

typedef unsigned char BYTE;

einsteindsk::einsteindsk(dsk& srcDSK) :
	_dsk(srcDSK)
{
	getfiles();
}

einsteindsk::~einsteindsk() {
}


static char* cpUpTo(char* dest, const char* src, int max)
{
	int i;
	for (i = 0; i < max; ++i)
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


vector<string> einsteindsk::dir() {
	vector<string> fileNames;

	// directory starts at sector 20, is 4 sectors (2k) in length
	for (auto i = 20; i < 24; ++i)
	{
		char bfr[13] = { 0 };
		typedef struct
		{
			BYTE user;
			char filename[8 + 3];
			BYTE extent;
			BYTE x, y;
			BYTE blockCount;
			short blockIDs[8];
		}
		dirent;

		auto sectorBytes = _dsk.readSectors(i, 1);
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
				char* pp = cpUpTo(bfr, &p->filename[0], 8);
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
				pp = cpUpTo(pp, &p->filename[8], 3);
				*pp = 0;

				fileNames.push_back(string(bfr));
			}
		}
	}

	return fileNames;
}




void einsteindsk::getfiles()
{
	char outname[256];
	char outname2[256];

	char* outp;

	std::vector<einyfile*> files;

	int i, res;

	einyfile* outfile = NULL;

	for (i = 20; i < 24; ++i)
	{
		int j, k;
		char bfr[13] = { 0 };
		typedef struct
		{
			BYTE user;
			char filename[8 + 3];
			BYTE extent;
			BYTE x, y;
			BYTE blockCount;
			short blockIDs[8];
		}
		dirent;

		vector<BYTE> sectorBytes = _dsk.readSectors(i, 1);
		dirent* p = (dirent*)(sectorBytes.data());

		for (j = 0; j < 16; ++j, ++p)
		{
			// for each directory entry in this sector:

			if (p->user == 0 && p->blockCount > 0 && p->blockCount <= 0x80)
			{
				// directory entry has passed some very rudimentary sanity check, so see if we have an object for this file already

				// create an 8.3 filename from something of the form 'NAME    EXT'
				//
				// copy up to 8 characters, but stop at a space, leaving a pointer
				// to the next character in the output buffer to be filled.
				char* pp = cpUpTo(bfr, &p->filename[0], 8);
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
				pp = cpUpTo(pp, &p->filename[8], 3);
				*pp = 0;

				einyfile* outfile = NULL;

				std::vector<einyfile*>::iterator it;
				for (it = files.begin(); it != files.end(); ++it)
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

					strcpy(outname2, outname);
					strcat(outname2, bfr);

					outfile = new einyfile(outname2, bfr);
					files.push_back(outfile);
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
					auto sectors = _dsk.readSectors(base, 4);
					outfile->write128(sectors.data(), nb > 16 ? 16 : nb, p->extent, block);

					nb -= 16;
					++block;
				}
			}
		}
	}

	// end of disk. close off any open file
	std::vector<einyfile*>::iterator it;
	for (it = files.begin(); it != files.end(); ++it)
	{
		einyfile* tfo = *it;
		tfo->close();
	}

	printf("\n");
}
