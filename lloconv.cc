/* lloconv.cc - Convert a document using liblibreoffice/LibreOfficeKit
 *
 * Copyright (C) 2014,2015 Olly Betts
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cstdlib>
#include <iostream>

#include <sysexits.h>

#include "convert.h"

using namespace std;

static void
usage()
{
    cerr << "Usage: " << program << " [-f OUTPUT_FORMAT] [-o OPTIONS] INPUT_FILE OUTPUT_FILE\n\n";
    cerr << "Specifying options requires LibreOffice >= 4.3.0rc1\n\n";
    cerr << "Known values for OUTPUT_FORMAT include:\n";
    cerr << "  For text documents: doc docx fodt html odt ott pdf txt xhtml\n\n";
    cerr << "Known OPTIONS include: SkipImages\n";
    cerr << flush;
}

int
main(int argc, char **argv)
{
    program = argv[0];

    if (argc < 3) {
	usage();
	_Exit(EX_USAGE);
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
	_Exit(EX_USAGE);
    }

    const char * input = argv[1];
    const char * output = argv[2];
    const char * lo_path = get_lo_path();
    int rc = convert(format, lo_path, input, output, options);

    // Avoid segfault from LibreOffice by terminating swiftly.
    _Exit(rc);
}
