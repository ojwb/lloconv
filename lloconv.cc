/* lloconv.cc - Convert a document using liblibreoffice
 *
 * Copyright (C) 2014 Olly Betts
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>

#include "LibreOfficeKit.hxx"

using namespace std;
using namespace lok;

#define LO_PATH "/opt/libreoffice4.3/program"

static const char * program = "lloconv";

static void
usage()
{
    cerr << "Usage: " << program << " [-f FORMAT] [-o OPTIONS] INPUT_FILE OUTPUT_FILE\n\n";
    cerr << "Known formats include:\n";
    cerr << "  For text documents: doc docx fodt html odt ott pdf txt xhtml\n";
    cerr << "Known options include: SkipImages\n";
    cerr << flush;
}

static int
get_product_major(const char * path)
{
    string versionrc = path;
    versionrc += "/versionrc";
    ifstream vrc(versionrc.c_str());
    if (!vrc)
	return -2;
    string line;
    while (getline(vrc, line)) {
	if (strncmp(line.c_str(), "ProductMajor=", 13) == 0) {
	    return atoi(line.c_str() + 13);
	}
    }
    return -1;
}

int
main(int argc, char **argv)
try {
    program = argv[0];

    if (argc < 3) {
	usage();
	_Exit(1);
    }

    const char * format = NULL;
    const char * options = NULL;
    // FIXME: Use getopt() or something.
    while (argv[1][0] == '-') {
	switch (argv[1][1]) {
	    case '-':
		if (argv[1][2] == '\0') {
		    // End of options.
		    ++argv;
		    --argc;
		    goto last_option;
		}
		break;
	    case 'f':
		if (argv[1][2]) {
		    format = argv[1] + 2;
		    ++argv;
		    --argc;
		} else {
		    format = argv[2];
		    argv += 2;
		    argc -= 2;
		}
		continue;
	    case 'o':
		if (argv[1][2]) {
		    options = argv[1] + 2;
		    ++argv;
		    --argc;
		} else {
		    options = argv[2];
		    argv += 2;
		    argc -= 2;
		}
		continue;
	}

	cerr << "Option '" << argv[1] << "' unknown\n\n";
	argc = 0;
	break;
    }
last_option:

    if (argc != 3) {
	usage();
	_Exit(1);
    }

    const char * input = argv[1];
    const char * output = argv[2];

    const char * lo_path = getenv("LO_PATH");
    if (!lo_path) {
	lo_path = LO_PATH;
    }

    int project_major = get_product_major(lo_path);
    if (project_major < 0) {
        cerr << program << ": LibreOffice install not found in '" << lo_path << "' - you can set LO_PATH in the environment" << endl;
	_Exit(1);
    }

    if (project_major < 430) {
        cerr << program << ": LibreOffice >= 4.3 required for LibreOfficeKit feature (found ProductMajor " << project_major << " is < 430)" << endl;
	_Exit(1);
    }

    Office * llo = lok_cpp_init(lo_path);
    if (!llo) {
        cerr << program << ": Failed to initialise LibreOfficeKit" << endl;
	_Exit(1);
    }

    Document * lodoc = llo->documentLoad(input);
    if (!lodoc) {
	const char * errmsg = llo->getError();
        cerr << program << ": LibreOfficeKit failed to load document (" << errmsg << ")" << endl;
	_Exit(1);
    }

    if (!lodoc->saveAs(output, format, options)) {
	const char * errmsg = llo->getError();
        cerr << program << ": LibreOfficeKit failed to export (" << errmsg << ")" << endl;
	delete lodoc;
	delete llo;
	_Exit(1);
    }

    delete lodoc;
    delete llo;

    // Avoid segfault from LibreOffice by terminating swiftly.
    _Exit(0);
} catch (const exception & e) {
    cerr << program << ": LibreOfficeKit threw exception (" << e.what() << ")" << endl;
    _Exit(1);
}
