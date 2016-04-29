/* lloconv.cc - Convert a document using LibreOfficeKit
 *
 * Copyright (C) 2014,2015,2016 Olly Betts
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cstdlib>
#include <iostream>

#include <sysexits.h>

#include "convert.h"
#include "urlencode.h"

using namespace std;

static void
usage()
{
    cerr << "Usage: " << program << " [-u] [-f OUTPUT_FORMAT] [-o OPTIONS] INPUT_FILE OUTPUT_FILE\n\n";
    cerr << "  -u  INPUT_FILE is a URL\n";
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
    bool url = false;
    // FIXME: Use getopt() or something.
    ++argv;
    --argc;
    while (argv[0] && argv[0][0] == '-') {
	switch (argv[0][1]) {
	    case '-':
		if (argv[0][2] == '\0') {
		    // End of options.
		    ++argv;
		    --argc;
		    goto last_option;
		}
		break;
	    case 'f':
		if (argv[0][2]) {
		    format = argv[0] + 2;
		    ++argv;
		    --argc;
		} else {
		    format = argv[1];
		    argv += 2;
		    argc -= 2;
		}
		continue;
	    case 'o':
		if (argv[0][2]) {
		    options = argv[0] + 2;
		    ++argv;
		    --argc;
		} else {
		    options = argv[1];
		    argv += 2;
		    argc -= 2;
		}
		continue;
	    case 'u':
		if (argv[0][2] != '\0') {
		    break;
		}
		url = true;
		++argv;
		--argc;
		continue;
	}

	cerr << "Option '" << argv[0] << "' unknown\n\n";
	argc = -1;
	break;
    }
last_option:

    if (argc != 2) {
	usage();
	_Exit(EX_USAGE);
    }

    string input;
    if (url) {
	input = argv[0];
    } else {
	url_encode(input, argv[0]);
    }
    string output;
    url_encode(output, argv[1]);

    void * handle = convert_init();
    int rc = convert(handle, input.c_str(), output.c_str(), format, options);
    convert_cleanup(handle);

    // Avoid segfault from LibreOffice by terminating swiftly.
    _Exit(rc);
}
