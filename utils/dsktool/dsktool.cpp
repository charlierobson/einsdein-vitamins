#include <iostream>
#include <filesystem>
#include <sys/stat.h>

#include "argcrack.h"
#include "einsteindsk.h"
#include "dsk.h"

using namespace std::filesystem;

bool isFile(string filename)
{
	struct stat _stat;
	stat(filename.c_str(), &_stat);
	return (_stat.st_mode & S_IFREG) != 0;
}

bool isDir(string filename)
{
	struct stat _stat;
	stat(filename.c_str(), &_stat);
	return (_stat.st_mode & S_IFDIR) != 0;
}

int main(int argc, char** argv)
{
	argcrack args(argc, argv);

	if (argc < 2 || args.isHelpRequested()) {
		cout << "Usage:" << endl << "  " << path(argv[0]).filename().string() << " COMMAND path-to-dsk-file" << endl << endl << "Where command looks like:" << endl;
		cout << "  dir" << endl;
		cout << "  extract * (to path-to-directory)" << endl;
		cout << "  extract filename (TO path-to-directory)" << endl;
		cout << "  insert path-to-file" << endl;
		cout << "  insert path-to-directory" << endl;

		exit(1);
	}

	einsteindsk einyDisk;

	string dskName(argv[argc - 1]);
	if (!isFile(dskName) || !einyDisk.load(dskName)) {
		cout << "Invalid DSK file " << dskName << endl;
		return 1;
	}

	if (args.ispresent("dir")) {

		for (auto file : einyDisk._files) {
			cout << file->_name << "  (" << file->_size << ")" << endl;
		}
	}
	else if (args.ispresent("extract")) {

		string fileToExtract;
		args.getstring("extract", fileToExtract);

		string to(".");
		args.getstring("to", to);
		path outputFolder(to);

		std::transform(fileToExtract.begin(), fileToExtract.end(), fileToExtract.begin(), ::toupper);
		for(auto file : einyDisk._files)
		{
			if (fileToExtract.compare("*") == 0 || fileToExtract.compare(file->_name) == 0) {
				path name = outputFolder / file->_name;
				name = name.make_preferred();
				cout << "extracting " << name << " ... " << (file->save(name.string()) ? "OK" : "FAIL") << endl;
			}
		}
	}
	else if (args.ispresent("insert")) {

		string insertPath;
		args.getstring("insert", insertPath);

		if (isDir(insertPath)) {
			for (const auto & entry : directory_iterator(insertPath)) {
				if (entry.is_regular_file()) {

					auto nameOnly = path(entry).filename().string();
					transform(nameOnly.begin(), nameOnly.end(), nameOnly.begin(), ::toupper);

					einyDisk._files.erase(remove_if(einyDisk._files.begin(), einyDisk._files.end(), [&](einyfile* f) -> bool { return nameOnly.compare(f->_name) == 0; }));

					einyDisk._files.push_back(new einyfile(nameOnly, disk::loadBytes(entry.path().string())));
				}
			}
		}
		else {

			auto nameOnly = path(insertPath).filename().string();
			transform(nameOnly.begin(), nameOnly.end(), nameOnly.begin(), ::toupper);

			einyDisk._files.erase(remove_if(einyDisk._files.begin(), einyDisk._files.end(), [&](einyfile* f) -> bool { return nameOnly.compare(f->_name) == 0; }));

			einyDisk._files.push_back(new einyfile(nameOnly, disk::loadBytes(insertPath)));
		}

		einyDisk.save(dskName);
	}
	else {
		cerr << "Unrecognised command." << endl;
		return 1;
	}

	return 0;
}
