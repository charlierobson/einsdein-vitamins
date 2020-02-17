#include <iostream>

#include "argcrack.h"
#include "einsteindsk.h"
#include "dsk.h"

int main(int argc, char** argv)
{
	if (argc < 2) {
		exit(1);
	}

	argcrack args(argc, argv);

	std::string dskName(argv[argc - 1]);

	if (args.ispresent("dir")) {

		auto srcDSK = dsk();
		if (srcDSK.load(dskName) != dsk::error_none) {
			exit(1);
		}

		auto einyDisk = new einsteindsk(srcDSK);
		auto direntries = einyDisk->dir();
		for (auto name : direntries) {
			cout << name << std::endl;
		}
	}
	else if (args.ispresent("extract:")) {

		auto srcDSK = dsk();
		if (srcDSK.load(dskName) != dsk::error_none) {
			exit(1);
		}

		auto einyDisk = new einsteindsk(srcDSK);
		auto direntries = einyDisk->dir();

		string fileToExtract;
		if (!args.getstring("extract:", fileToExtract)) {
			exit(1);
		}

		if (fileToExtract.compare("*") == 0) {
			// extract all
		}

		std::transform(fileToExtract.begin(), fileToExtract.end(), fileToExtract.begin(), ::toupper);
		if (std::find(direntries.begin(), direntries.end(), fileToExtract) == direntries.end()) {
			exit(1);
		}

		// extract single file

		puts("extracted");
	}
}
