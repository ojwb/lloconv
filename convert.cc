/* convert.cc - Convert documents using LibreOfficeKit
 *
 * Copyright (C) 2014,2015,2016 Olly Betts
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "convert.h"

#include <cstdlib>
#include <exception>
#include <iostream>

#include <sys/stat.h>
#include <sys/types.h>
#include <sysexits.h>
#include <unistd.h>

#include <LibreOfficeKit/LibreOfficeKit.hxx>

#include "urlencode.h"

using namespace std;
using namespace lok;

// Install location for Debian packages:
#define LO_PATH_DEBIAN "/usr/lib/libreoffice/program"

// Install location for .deb files from libreoffice.org:
#define LO_PATH_LIBREOFFICEORG(V) "/opt/libreoffice" #V "/program"

const char *program = "<program>";

// Find a LibreOffice installation to use.
static const char *get_lo_path() {
  const char *lo_path = getenv("LO_PATH");
  if (!lo_path) {
    struct stat sb;
#define CHECK_DIR(P)                                                           \
  if (!lo_path && stat(P "/versionrc", &sb) == 0 && S_ISREG(sb.st_mode))       \
  lo_path = P
    CHECK_DIR(LO_PATH_DEBIAN);
    CHECK_DIR(LO_PATH_LIBREOFFICEORG(5.4));
    CHECK_DIR(LO_PATH_LIBREOFFICEORG(5.3));
    CHECK_DIR(LO_PATH_LIBREOFFICEORG(5.2));
    CHECK_DIR(LO_PATH_LIBREOFFICEORG(5.1));
    CHECK_DIR(LO_PATH_LIBREOFFICEORG(5.0));
    CHECK_DIR(LO_PATH_LIBREOFFICEORG(4.4));
    CHECK_DIR(LO_PATH_LIBREOFFICEORG(4.3));

    if (!lo_path) {
      cerr << program
           << ": LibreOffice install not found\n"
              "Set LO_PATH in the environment to the 'program' directory - "
              "e.g.:\n"
              "LO_PATH=/opt/libreoffice/program\n"
              "export LO_PATH"
           << endl;
      _Exit(1);
    }
  }
  return lo_path;
}

void *convert_init() {
  Office *llo = NULL;
  try {
    const char *lo_path = get_lo_path();
    llo = lok_cpp_init(lo_path);
    if (!llo) {
      cerr << program << ": Failed to initialise LibreOfficeKit" << endl;
      return NULL;
    }
    return static_cast<void *>(llo);
  } catch (const exception &e) {
    delete llo;
    cerr << program << ": LibreOfficeKit threw exception (" << e.what() << ")"
         << endl;
    return NULL;
  }
}

void convert_cleanup(void *h_void) {
  Office *llo = static_cast<Office *>(h_void);
  delete llo;
}

int convert(void *h_void, bool url, const char *input, const char *output,
            const char *format, const char *options) try {
  if (!h_void)
    return 1;
  Office *llo = static_cast<Office *>(h_void);

  string input_url;
  if (url) {
    input_url = input;
  } else {
    url_encode_path(input_url, input);
  }
  Document *lodoc = llo->documentLoad(input_url.c_str(), options);
  if (!lodoc) {
    const char *errmsg = llo->getError();
    cerr << program << ": LibreOfficeKit failed to load document (" << errmsg
         << ")" << endl;
    return 1;
  }

  string output_url;
  url_encode_path(output_url, output);
  if (!lodoc->saveAs(output_url.c_str(), format, options)) {
    const char *errmsg = llo->getError();
    cerr << program << ": LibreOfficeKit failed to export (" << errmsg << ")"
         << endl;
    delete lodoc;
    return 1;
  }

  delete lodoc;

  return 0;
} catch (const exception &e) {
  cerr << program << ": LibreOfficeKit threw exception (" << e.what() << ")"
       << endl;
  return 1;
}
