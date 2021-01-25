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

static void printer(string s)
{
	cout << s << endl;
}

int main(int argc, char** argv)
{
	argcrack args(argc, argv);

	if (argc < 2 || args.isHelpRequested()) {
		cout << "Usage:" << endl << "  " << path(argv[0]).filename().string() << " COMMAND path-to-dsk-file" << endl << endl << "Where command looks like:" << endl;
		cout << "  dir" << endl;
		cout << "  extract * (TO path-to-directory)" << endl;
		cout << "  extract filename (TO path-to-directory)" << endl;
		cout << "  insert path-to-file" << endl;
		cout << "  insert path-to-directory" << endl;
		cout << "  dumpdos path-to-file" << endl;
		cout << "  new path-to-file" << endl;
		exit(1);
	}

	einsteindsk einyDisk;

	string dskName(argv[argc - 1]);

	if (args.ispresent("new")) {

		einyDisk.init(40, 10, 512);
	}
	else if (!isFile(dskName) || !einyDisk.load(dskName)) {

		cout << "Invalid DSK file " << dskName << endl;
		return 1;
	}

	if (args.ispresent("dir")) {

		for (auto file : einyDisk._files) {
			cout << file->_name << "  (" << file->_size << ")" << endl;
		}
		cout << einyDisk._files.size() << " found" << endl;
	}
	if (args.ispresent("diag")) {

		einyDisk.diag(printer);
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

					if (einyDisk._files.size())
						einyDisk._files.erase(remove_if(einyDisk._files.begin(), einyDisk._files.end(), [&](einyfile* f) -> bool { return nameOnly.compare(f->_name) == 0; }));

					einyDisk._files.push_back(new einyfile(nameOnly, disk::loadBytes(entry.path().string())));
				}
			}
		}
		else {

			auto nameOnly = path(insertPath).filename().string();
			transform(nameOnly.begin(), nameOnly.end(), nameOnly.begin(), ::toupper);

			if (einyDisk._files.size())
				einyDisk._files.erase(remove_if(einyDisk._files.begin(), einyDisk._files.end(), [&](einyfile* f) -> bool { return nameOnly.compare(f->_name) == 0; }));

			einyDisk._files.push_back(new einyfile(nameOnly, disk::loadBytes(insertPath)));
		}

		if (!einyDisk.save(dskName)) {
			cerr << "insert failed." << endl;
			return 1;
		}
	}
	else if (args.ispresent("dumpdos")) {

		string fileToExtract;
		args.getstring("dumpdos", fileToExtract);

		auto dosSectors = einyDisk.readSectors(0, 20);
		if (!einyDisk.saveBytes(fileToExtract, dosSectors)) {
			cerr << "dumpdos failed." << endl;
			return 1;
		}
	}
	else if (args.ispresent("new")) {

		string dosFile;
		args.getstring("new", dosFile);

		auto dos = einyDisk.loadBytes(dosFile);

		einyDisk.writeSectors(0, 20, dos);
		if (!einyDisk.save(dskName)) {
			cerr << "new disk creation failed." << endl;
			return 1;
		}
	}
	else {
		cerr << "Unrecognised command." << endl;
		return 1;
	}

	return 0;
}
