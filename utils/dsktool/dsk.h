#pragma once

#include<string>
#include<vector>
using namespace std;

#include "disk.h"

class dsk : public disk
{
private:
	bool parseDSK(vector<char>& rawDSK);

public:
	dsk();
	virtual ~dsk();

	virtual bool load(string fileName);
};
