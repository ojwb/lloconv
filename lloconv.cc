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
#include <exception>
#include <iostream>

#include "liblibreoffice.hxx"

using namespace std;

static const char * lo_path = "/opt/libreoffice4.2/program";

int
main(int argc, char **argv)
try {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " INPUT_FILE OUTPUT_FILE" << endl;
	_Exit(1);
    }

    const char * input = argv[1];
    const char * output = argv[2];

    const char * p = getenv("LO_PATH");
    if (p) {
	lo_path = p;
    }
    LibLibreOffice * llo = lo_cpp_init(lo_path);
    if (!llo || !llo->initialize(lo_path)) {
        cerr << argv[0] << ": Failed to initialise liblibreoffice" << endl;
	_Exit(1);
    }

    LODocument * lodoc = llo->documentLoad(input);
    if (!lodoc) {
	const char * errmsg = llo->getError();
        cerr << argv[0] << ": liblibreoffice failed to load document (" << errmsg << ")" << endl;
	_Exit(1);
    }

    if (!lodoc->saveAs(output, NULL)) {
	const char * errmsg = llo->getError();
        cerr << argv[0] << ": liblibreoffice failed to export (" << errmsg << ")" << endl;
	_Exit(1);
    }

    // Avoid segfault from libreoffice by terminating swiftly.
    _Exit(0);
} catch (const exception & e) {
    cerr << argv[0] << ": liblibreoffice threw exception (" << e.what() << ")" << endl;
    _Exit(1);
}
