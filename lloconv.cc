/* lloconv.cc - Convert a document using liblibreoffice/LibreOfficeKit
 *
 * Copyright (C) 2014 Olly Betts
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cstdlib>
#include <exception>
#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sysexits.h>

#include "liblibreoffice.hxx"
#include "LibreOfficeKit.hxx"

using namespace std;
using namespace lok;

// Install location for Debian packages:
#define LO_PATH_DEBIAN "/usr/lib/libreoffice/program"

// Install location for .deb files from libreoffice.org:
#define LO_PATH_LIBREOFFICEORG(V) "/opt/libreoffice"#V"/program"

static const char * program = "lloconv";

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

// Support for LibreOfficeKit which is in LO >= 4.3.0.
static int
conv_lok(const char * program, const char * format, Office * llo,
	 const char * input, const char * output, const char * options)
try {
    Document * lodoc = llo->documentLoad(input);
    if (!lodoc) {
	const char * errmsg = llo->getError();
	cerr << program << ": LibreOfficeKit failed to load document (" << errmsg << ")" << endl;
	return 1;
    }

    if (!lodoc->saveAs(output, format, options)) {
	const char * errmsg = llo->getError();
	cerr << program << ": LibreOfficeKit failed to export (" << errmsg << ")" << endl;
	delete lodoc;
	delete llo;
	return 1;
    }

    delete lodoc;
    delete llo;

    return 0;
} catch (const exception & e) {
    cerr << program << ": LibreOfficeKit threw exception (" << e.what() << ")" << endl;
    return 1;
}

// Support for the old liblibreoffice code in LO 4.2.x.
static int
conv_llo(const char * program, const char * format, LibLibreOffice * llo,
	 const char * lo_path,
	 const char * input, const char * output)
try {
    if (!llo->initialize(lo_path)) {
	cerr << program << ": Failed to initialise liblibreoffice" << endl;
	return EX_UNAVAILABLE;
    }

    LODocument * lodoc = llo->documentLoad(input);
    if (!lodoc) {
	const char * errmsg = llo->getError();
	cerr << program << ": liblibreoffice failed to load document (" << errmsg << ")" << endl;
	return 1;
    }

    if (!lodoc->saveAs(output, format)) {
	const char * errmsg = llo->getError();
	cerr << program << ": liblibreoffice failed to export (" << errmsg << ")" << endl;
	delete lodoc;
	delete llo;
	return 1;
    }

    delete lodoc;
    delete llo;

    return 0;
} catch (const exception & e) {
    cerr << program << ": liblibreoffice threw exception (" << e.what() << ")" << endl;
    return 1;
}

static int
convert(const char * program, const char * format, const char * lo_path,
	const char * input, const char * output, const char * options)
try {
    LibLibreOffice * llo_old;
    Office * llo;
    if (!lok_cpp_init(lo_path, &llo_old, &llo)) {
	cerr << program << ": Failed to initialise LibreOfficeKit or LibLibreOffice" << endl;
	return EX_UNAVAILABLE;
    }

    if (llo) {
	return conv_lok(program, format, llo, input, output, options);
    }

    if (options) {
	cerr << program << ": LibreOffice >= 4.3.0rc1 required for specifying options" << endl;
	_Exit(1);
    }

    return conv_llo(program, format, llo_old, lo_path, input, output);
} catch (const exception & e) {
    cerr << program << ": LibreOffice threw exception (" << e.what() << ")" << endl;
    return 1;
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

    const char * lo_path = getenv("LO_PATH");
    if (!lo_path) {
	struct stat sb;
#define CHECK_DIR(P) if (!lo_path && stat(P"/versionrc", &sb) == 0 && S_ISREG(sb.st_mode)) lo_path = P
	CHECK_DIR(LO_PATH_DEBIAN);
	CHECK_DIR(LO_PATH_LIBREOFFICEORG(4.4));
	CHECK_DIR(LO_PATH_LIBREOFFICEORG(4.3));

	if (!lo_path) {
	    cerr << program << ": LibreOffice install not found\n"
		"Set LO_PATH in the environment to the 'program' directory - e.g.:\n"
		"LO_PATH=/opt/libreoffice/program\n"
		"export LO_PATH" << endl;
	    _Exit(1);
	}
    }

    int rc = convert(program, format, lo_path, input, output, options);

    // Avoid segfault from LibreOffice by terminating swiftly.
    _Exit(rc);
}
