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

#include "liblibreoffice.hxx"

using namespace std;

#define LO_PATH "/opt/libreoffice4.2/program"

static const char * program = "lloconv";

static void
usage()
{
    cerr << "Usage: " << program << " [-f FORMAT] INPUT_FILE OUTPUT_FILE" << endl;
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
    if (argv[1][0] == '-' && argv[1][1] == 'f') {
	if (argv[1][2]) {
	    format = argv[1] + 2;
	    ++argv;
	} else {
	    format = argv[2];
	    argv += 2;
	}
    }

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

    if (project_major < 420) {
        cerr << program << ": LibreOffice >= 4.2 required for liblibreoffice feature (found ProductMajor " << project_major << " is < 420)" << endl;
	_Exit(1);
    }

    LibLibreOffice * llo = lo_cpp_init(lo_path);
    if (!llo || !llo->initialize(lo_path)) {
        cerr << program << ": Failed to initialise liblibreoffice" << endl;
	_Exit(1);
    }

    LODocument * lodoc = llo->documentLoad(input);
    if (!lodoc) {
	const char * errmsg = llo->getError();
        cerr << program << ": liblibreoffice failed to load document (" << errmsg << ")" << endl;
	_Exit(1);
    }

    if (!lodoc->saveAs(output, format)) {
	const char * errmsg = llo->getError();
        cerr << program << ": liblibreoffice failed to export (" << errmsg << ")" << endl;
	_Exit(1);
    }

    // Avoid segfault from LibreOffice by terminating swiftly.
    _Exit(0);
} catch (const exception & e) {
    cerr << program << ": liblibreoffice threw exception (" << e.what() << ")" << endl;
    _Exit(1);
}
