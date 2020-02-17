#include <assert.h>
#include <string.h>
#include <io.h>
#include <direct.h>
#include <stdio.h>

#include <string>
#include <vector>

typedef unsigned char BYTE;
typedef unsigned short WORD;

BYTE myData[512];

// maximum number of sectors is 10 per track, 80 tracks, single sided disk
// - until the ROM handles different disk types this is all we can handle
int sectorOffsets[10*80*1];

extern WORD crc16_ccitt(const unsigned char *buf, int len, WORD crc);


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

enum
{
	DSK_OK,
	DSK_NOT_DSK_FILE,
	DSK_TOO_MANY_SIDES,
	DSK_TOO_MANY_TRACKS,
	DSK_TOO_MANY_SECTORS
};


char* cpUpTo(char* dest, const char* src, int max)
{
	int i;
	for(i = 0; i < max; ++i)
	{
		if (*src == ' ')
			break;
		if (*src == '/')
			*dest = '-';
		else
			*dest = *src & 127;
		++dest;
		++src;
	}

	return dest;
}


int parseDSK(FILE* infile, int* sectorOffsets, int* tracks, int* sectors)
{
	int i, j;

	DISK_INFORMATION_BLOCK* dib = (DISK_INFORMATION_BLOCK*)myData;
	TRACK_INFORMATION_BLOCK* tib = (TRACK_INFORMATION_BLOCK*)myData;
	SECTOR_INFORMATION_BLOCK* sib;

	*tracks = 0;
	*sectors = 0;

	fread(myData, 1, sizeof(DISK_INFORMATION_BLOCK), infile);

	if (!memcmp(myData, "EXTENDED", sizeof("EXTENDED")))
	{
		return DSK_NOT_DSK_FILE;
	}

	// only handling single sided disks

	if (dib->nSides == 2)
	{
		return DSK_TOO_MANY_SIDES;
	}

	*tracks = dib->nTracks;

	if (*tracks > 80)
	{
		return DSK_TOO_MANY_TRACKS;
	}

	// only handling 10 sectors per track
	// 0x15(00)  =  21  =  256+20*256  =  sizeof(header)+10*512

	for (i = 0; i < *tracks; ++i)
	{
		if (dib->trackSizeTable[i] > 0x15)
		{
			return DSK_TOO_MANY_SECTORS;
		}
	}

	// ok, work out the sector offsets in the file

	for(i = 0; i < *tracks; ++i)
	{
		long sectorBase;

		fread(myData, 1, 256, infile);
		sib = (SECTOR_INFORMATION_BLOCK*)(myData+sizeof(TRACK_INFORMATION_BLOCK));

		sectorBase = ftell(infile);

		for(j = 0; j < 10; ++j)
		{
			if (sib->sectorID > 40)
			{
				return DSK_NOT_DSK_FILE;
			}

			sectorOffsets[i * 10 + sib->sectorID] = sectorBase;
			sectorBase += 512;
			++*sectors;
			++sib;
		}

		fseek(infile, 10*512, SEEK_CUR);
	}

	return DSK_OK;
}


void extractToRaw(char* inname, FILE* infile, int sectors)
{
	int i;
	FILE* outfile;

	char filename[128];

	strcpy_s(filename, 128, inname);
	strcat_s(filename, 128, ".raw");

	if (!fopen_s(&outfile, filename, "wb"))
	{
		for (i = 0; i < sectors; ++i)
		{
			fseek(infile, sectorOffsets[i], SEEK_SET);
			fread(myData, 1, 512, infile);
			fwrite(myData, 1, 512, outfile);
		}

		fclose(outfile);
	}
}



void crcDos(char* inname, FILE* infile)
{
	int i;
	WORD cs = -1;

	for (i = 0; i < 2 * 10; ++i)
	{
		fseek(infile, sectorOffsets[i], SEEK_SET);
		fread(myData, 1, 512, infile);

		cs = crc16_ccitt(myData, 512, cs);
	}

	printf("%04x % 16s\n", cs, inname);
}

// todo:
//
// determine failure reason in file object
// extraction log?]


void extractAll(char* inname, FILE* infile)
{
	char outname[256];
	char outname2[256];

	char* outp;

	std::vector<fileobject*> files;

	int i, res;

	strcpy_s(outname, 256, inname);
	outp = strchr(outname, '.');
	*outp = 0;

	strcat_s(outname, 256, "\\");
	res = _mkdir(outname);

	fileobject* outfile = NULL;

	for (i = 20; i < 24; ++i)
	{
		int j, k;
		char bfr [13]={0};
		typedef struct
		{
			BYTE user;
			char filename[8+3];
			BYTE extent;
			BYTE x, y;
			BYTE blockCount;
			short blockIDs[8];
		}
		dirent;
		dirent* p = (dirent*)myData;

		BYTE sectordata[512*4];

		fseek(infile, sectorOffsets[i], SEEK_SET);
		fread(myData, 1, 512, infile);

		for (j = 0; j < 16; ++j, ++p)
		{
			// for each directory entry in this sector:

			if (p->user == 0 && p->blockCount > 0 && p-> blockCount <= 0x80)
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

				fileobject* outfile = NULL;

				std::vector<fileobject*>::iterator it;
				for (it = files.begin(); it != files.end(); ++it)
				{
					fileobject* tfo = *it;
					if (tfo->nameis(bfr))
					{
						outfile = tfo;
						break;
					}
				}

				if (outfile == NULL)
				{
					// no file object exists for the current directory entry: create one

					strcpy_s(outname2, 256, outname);
					strcat_s(outname2, 256, bfr);

					outfile = new fileobject(outname2, bfr);
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

					for (k = 0; k < 4; ++k)
					{
						fseek(infile, sectorOffsets[base + k], SEEK_SET);
						fread(sectordata + (k * 512), 1, 512, infile);
					}

					outfile->write128(sectordata, nb > 16 ? 16 : nb, p->extent, block);

					nb -= 16;
					++block;
				}
			}
		}
	}

	// end of disk. close off any open file
	std::vector<fileobject*>::iterator it;
	for (it = files.begin(); it != files.end(); ++it)
	{
		fileobject* tfo = *it;
		tfo->close();
	}

	printf("\n");
}




int main(int argc, char** argv)
{
	int ret = 0;

	FILE* infile;

	int tracks, sectors;

	assert(sizeof(DISK_INFORMATION_BLOCK) == 256);
	assert(sizeof(TRACK_INFORMATION_BLOCK) == 24);
	assert(sizeof(SECTOR_INFORMATION_BLOCK) == 8);
	assert(sizeof(int) == 4);

	try
	{

		ret = fopen_s(&infile, argv[1], "rb");
		if (!ret)
		{
			printf("\n\n---- %s ----\n", argv[1]);
			ret = parseDSK(infile, sectorOffsets, &tracks, &sectors);
			if (!ret)
			{
				extractAll(argv[1], infile);
			}

			extractToRaw(argv[1], infile, 20);

			fclose(infile);
		}
	}
	catch(...)
	{
	}
}

