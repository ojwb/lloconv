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
#include <iostream>

#include "liblibreoffice.hxx"

using namespace std;

static const char * lo_path = "/opt/libreoffice4.2/program";

int
main(int argc, char **argv)
{
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " INPUT_FILE OUTPUT_FILE" << endl;
	exit(1);
    }

    const char * input = argv[1];
    const char * output = argv[2];

    const char * p = getenv("LO_PATH");
    if (p) {
	lo_path = p;
    }
    LibLibreOffice * llo = lo_cpp_init(lo_path);
    if (!llo) {
        cerr << argv[0] << ": Failed to initialise liblibreoffice" << endl;
	exit(1);
    }

    LODocument * lodoc = llo->documentLoad(input);
    if (!lodoc) {
	const char * errmsg = llo->getError();
        cerr << argv[0] << ": liblibreoffice failed to load document (" << errmsg << ")" << endl;
	exit(1);
    }

    if (!lodoc->saveAs(output, NULL)) {
	const char * errmsg = llo->getError();
	delete lodoc;
        cerr << argv[0] << ": liblibreoffice failed to export (" << errmsg << ")" << endl;
	exit(1);
    }
}
