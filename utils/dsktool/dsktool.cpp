#include <iostream>
#include <filesystem>

#include "argcrack.h"
#include "einsteindsk.h"
#include "dsk.h"

using namespace std::filesystem;


int main(int argc, char** argv)
{
	argcrack args(argc, argv);

	if (argc < 2 || args.ispresent("?")) {
		cout << "Usage: " << path(argv[0]).root_name() << " command path-to-dsk-file" << endl << endl << "Where command looks like:" << endl;
		cout << "  DIR" << endl;
		cout << "  EXTRACT * (TO path-to-directory)" << endl;
		cout << "  EXTRACT filename (TO path-to-directory)" << endl;
		cout << "  INSERT filename" << endl;

//		cout << "  CREATE path-to-folder" << endl;

		exit(1);
	}

	string dskName(argv[argc - 1]);
	einsteindsk einyDisk;
	if (!einyDisk.load(dskName)) {
		exit(1);
	}

	if (args.ispresent("dir")) {

		for_each(einyDisk._files.begin(), einyDisk._files.end(), [](einyfile* file)
		{
			cout << file->_name << "  (" << file->_size << ")" << endl;
		});
	}
	else if (args.ispresent("extract")) {

		string to(".");
		args.getstring("to", to);
		path outputFolder(to);

		string fileToExtract;
		if (!args.getstring("extract", fileToExtract)) {
			exit(1);
		}

		std::transform(fileToExtract.begin(), fileToExtract.end(), fileToExtract.begin(), ::toupper);
		for_each(einyDisk._files.begin(), einyDisk._files.end(), [&](einyfile* file)
		{
			if (fileToExtract.compare("*") == 0 || fileToExtract.compare(file->_name) == 0) {
				path name = outputFolder / file->_name;
				name = name.make_preferred();
				cout << "extracting " << name << " ... " << (file->save(name.string()) ? "OK" : "FAIL") << endl;
			}
		});
	}
	else if (args.ispresent("insert")) {

		string fileToInsert;
		if (!args.getstring("insert", fileToInsert)) {
			exit(1);
		}

		auto nameOnly = path(fileToInsert).filename().string();
		std::transform(nameOnly.begin(), nameOnly.end(), nameOnly.begin(), ::toupper);

		einyDisk._files.push_back(new einyfile(nameOnly, disk::loadBytes(fileToInsert)));

		einyDisk.save(dskName);
	}
}
