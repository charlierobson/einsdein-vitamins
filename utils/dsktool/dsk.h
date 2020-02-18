#pragma once

#include<string>
#include<vector>
using namespace std;

#include "disk.h"

class dsk : public disk
{
	std::vector<char> _raw;

	bool parseDSK();

public:
	dsk();
	virtual ~dsk();

	virtual bool load(string fileName);
};
