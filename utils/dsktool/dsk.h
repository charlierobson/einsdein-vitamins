#pragma once

#include<string>
#include<vector>

#include "disk.h"

using namespace std;

class dsk : public disk
{
	std::vector<unsigned char> _raw;

	bool parseDSK();

public:
	dsk() { }
	virtual ~dsk() { }

	virtual bool load(string fileName);
	virtual bool save(string fileName);
};
