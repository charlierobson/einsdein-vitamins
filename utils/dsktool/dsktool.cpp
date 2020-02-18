#include <iostream>
#include <filesystem>

#include "argcrack.h"
#include "einsteindsk.h"
#include "dsk.h"

using namespace std::filesystem;


int main(int argc, char** argv)
{
	if (argc < 2) {
		exit(1);
	}

	argcrack args(argc, argv);

	string to("./");
	args.getstring("to:", to);
	path outputFolder(to);

	string dskName(argv[argc - 1]);

	if (args.ispresent("dir")) {

		auto srcDSK = dsk();
		if (!srcDSK.load(dskName)) {
			exit(1);
		}

		einsteindsk einyDisk(srcDSK);
		for_each(einyDisk._files.begin(), einyDisk._files.end(), [](einyfile* file)
		{
			cout << file->_name << "  (" << file->_size << ")" << endl;
		});
	}
	else if (args.ispresent("extract:")) {

		auto srcDSK = dsk();
		if (!srcDSK.load(dskName)) {
			exit(1);
		}

		einsteindsk einyDisk(srcDSK);

		string fileToExtract;
		if (!args.getstring("extract:", fileToExtract)) {
			exit(1);
		}

		std::transform(fileToExtract.begin(), fileToExtract.end(), fileToExtract.begin(), ::toupper);
		for_each(einyDisk._files.begin(), einyDisk._files.end(), [&](einyfile* file)
		{
			if (fileToExtract.compare("*") == 0 || fileToExtract.compare(file->_name) == 0) {
				path name = outputFolder / file->_name;
				name.make_preferred();
				cout << "extracting " << name << std::endl;
			}
		});
	}
}
