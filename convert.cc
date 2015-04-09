/* convert.cc - Convert documents using liblibreoffice/LibreOfficeKit
 *
 * Copyright (C) 2014,2015 Olly Betts
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "convert.h"

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

const char * program = "<program>";

// Find a LibreOffice installation to use.
static const char *
get_lo_path()
{
    const char * lo_path = getenv("LO_PATH");
    if (!lo_path) {
	struct stat sb;
#define CHECK_DIR(P) if (!lo_path && stat(P"/versionrc", &sb) == 0 && S_ISREG(sb.st_mode)) lo_path = P
	CHECK_DIR(LO_PATH_DEBIAN);
	CHECK_DIR(LO_PATH_LIBREOFFICEORG(4.4));
	CHECK_DIR(LO_PATH_LIBREOFFICEORG(4.3));
	CHECK_DIR(LO_PATH_LIBREOFFICEORG(5.0));

	if (!lo_path) {
	    cerr << program << ": LibreOffice install not found\n"
		"Set LO_PATH in the environment to the 'program' directory - e.g.:\n"
		"LO_PATH=/opt/libreoffice/program\n"
		"export LO_PATH" << endl;
	    _Exit(1);
	}
    }
    return lo_path;
}

struct handle {
    Office * llo;
    LibLibreOffice * llo_old;
    const char * lo_path;

    handle(const char * lo_path_)
	: llo(NULL), llo_old(NULL), lo_path(lo_path_) { }

    ~handle() {
	delete llo;
	delete llo_old;
    }
};

void *
convert_init()
{
    handle * h = NULL;
    try {
	const char * lo_path = get_lo_path();
	handle * h = new handle(lo_path);
	if (!lok_cpp_init(lo_path, &(h->llo_old), &(h->llo))) {
	    delete h;
	    cerr << program << ": Failed to initialise LibreOfficeKit or LibLibreOffice" << endl;
	    return NULL;
	}
	return h;
    } catch (const exception & e) {
	delete h;
	cerr << program << ": LibreOffice threw exception (" << e.what() << ")" << endl;
	return NULL;
    }
}

void
convert_cleanup(void * h_void)
{
    handle * h = reinterpret_cast<handle *>(h_void);
    delete h;
}

// Support for LibreOfficeKit which is in LO >= 4.3.0.
static int
conv_lok(const char * format, Office * llo,
	 const char * input, const char * output, const char * options)
try {
    Document * lodoc = llo->documentLoad(input, options);
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

    return 0;
} catch (const exception & e) {
    cerr << program << ": LibreOfficeKit threw exception (" << e.what() << ")" << endl;
    return 1;
}

// Support for the old liblibreoffice code in LO 4.2.x.
static int
conv_llo(const char * format, LibLibreOffice * llo, const char * lo_path,
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

    return 0;
} catch (const exception & e) {
    cerr << program << ": liblibreoffice threw exception (" << e.what() << ")" << endl;
    return 1;
}

int
convert(void * h_void,
	const char * input, const char * output,
	const char * format, const char * options)
try {
    handle * h = reinterpret_cast<handle *>(h_void);
    if (!h) return 1;

    if (h->llo) {
	return conv_lok(format, h->llo, input, output, options);
    }

    if (options) {
	cerr << program << ": LibreOffice >= 4.3.0rc1 required for specifying options" << endl;
	_Exit(1);
    }

    return conv_llo(format, h->llo_old, h->lo_path, input, output);
} catch (const exception & e) {
    cerr << program << ": LibreOffice threw exception (" << e.what() << ")" << endl;
    return 1;
}