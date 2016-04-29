/* inject-meta.cc - Inject meta-data into a document
 *
 * Provides an example of how to perform multiple conversions.
 *
 * Copyright (C) 2014,2015 Olly Betts
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <string>

#include <fcntl.h>
#include <sysexits.h>
#include <unistd.h>

#include "convert.h"

using namespace std;

// Create a temporary directory called ${TMPDIR-/tmp} + this with XXXXXX
// replaced.
#define TEMP_DIR_TEMPLATE "/inject-meta-XXXXXX"

static void
usage()
{
    cerr << "Usage: " << program << " -mNAME=VALUE[...] INPUT_FILE OUTPUT_FILE\n\n";
    cerr << flush;
}

static void
write_xml_tag(FILE * out, const char * tag, const char * content)
{
    fprintf(out, "<%s>", tag);
    for (const char * p = content; *p; ++p) {
	switch (*p) {
	    case '&':
		fputs("&amp;", out);
		break;
	    case '<':
		fputs("&lt;", out);
		break;
	    case '>':
		fputs("&gt;", out);
		break;
	    default:
		putc(*p, out);
		break;
	}
    }
    fprintf(out, "</%s>", tag);
}

int
main(int argc, char **argv)
{
    program = argv[0];

    if (argc < 3) {
	usage();
	_Exit(EX_USAGE);
    }

    map<string, const char *> meta;

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
	    case 'm': {
		const char * tag = argv[1] + 2;
		const char * eq = strchr(tag, '=');
		if (!eq) {
		    cerr << "Option '" << argv[1] << "' missing '='\n\n";
		    usage();
		    _Exit(EX_USAGE);
		}
		meta[string(tag, eq - tag)] = eq + 1;
		++argv;
		--argc;
		continue;
	    }
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

    void * handle = convert_init();

    // Create a temporary directory.
    const char * p = getenv("TMPDIR");
    if (!p) p = "/tmp";
    char * dir_template = new char[sizeof(TEMP_DIR_TEMPLATE) + strlen(p)];
    strcpy(dir_template, p);
    strcat(dir_template, TEMP_DIR_TEMPLATE);
    p = mkdtemp(dir_template);
    if (!p) {
	cerr << program << ": mkdtemp() failed (" << strerror(errno) << ")" << endl;
	_Exit(1);
    }
    string tmpdir(p);

    string odt = tmpdir + "/tmp.odt";
    int rc = convert(handle, false, input, odt.c_str());

    if (!rc) {
	int cwd_fd = open(".", O_RDONLY);
	if (cwd_fd < 0) {
	    cerr << program << ": open(\".\") failed (" << strerror(errno) << ")" << endl;
	    _Exit(1);
	}
	if (chdir(tmpdir.c_str()) < 0) {
	    cerr << program << ": chdir() failed (" << strerror(errno) << ")" << endl;
	    _Exit(1);
	}
	FILE * f = popen("unzip -p tmp.odt meta.xml", "r");
	if (!f) {
	    cerr << program << ": popen() failed (" << strerror(errno) << ")" << endl;
	    _Exit(1);
	}

	FILE * out = fopen("meta.xml", "w");
	if (!out) {
	    cerr << program << ": fopen() failed (" << strerror(errno) << ")" << endl;
	    _Exit(1);
	}

	char *line = NULL;
	size_t len = 0;
	ssize_t c;

	bool in_meta = false;
	while ((c = getdelim(&line, &len, '>', f)) != -1) {
	    if (in_meta) {
		if (strncmp(line, "</office:meta>", sizeof("</office:meta>") - 1) == 0) {
		    in_meta = false;
		    map<string, const char *>::const_iterator i;
		    for (i = meta.begin(); i != meta.end(); ++i) {
			const char * tag = i->first.c_str();
			write_xml_tag(out, tag, i->second);
		    }
		    meta.clear();
		} else if (line[0] == '<') {
		    map<string, const char *>::iterator i;
		    for (i = meta.begin(); i != meta.end(); ++i) {
			const char * tag = i->first.c_str();
			if (strncmp(line + 1, tag, i->first.size()) == 0 &&
			    line[1 + i->first.size()] == '>') {
			    write_xml_tag(out, tag, i->second);
			    meta.erase(i);
			    if ((c = getdelim(&line, &len, '>', f)) == -1) break;
			    goto next;
			}
		    }
		}
	    } else {
		if (strncmp(line, "<office:meta>", sizeof("<office:meta>") - 1) == 0) {
		    in_meta = true;
		}
	    }
	    fwrite(line, c, 1, out);
next:;
	}

	if (fclose(out) < 0) {
	    cerr << program << ": fclose() failed (" << strerror(errno) << ")" << endl;
	    _Exit(1);
	}

	if (pclose(f) < 0) {
	    cerr << program << ": pclose() failed (" << strerror(errno) << ")" << endl;
	    _Exit(1);
	}

	int r = system("zip tmp.odt meta.xml");
	unlink("meta.xml");
	if (!WIFEXITED(r) || WEXITSTATUS(r) != 0) {
	    cerr << program << ": failed to run zip command" << endl;
	    _Exit(1);
	}

	if (fchdir(cwd_fd) < 0) {
	    cerr << program << ": fchdir() failed (" << strerror(errno) << ")" << endl;
	    _Exit(1);
	}
	rc = convert(handle, false, odt.c_str(), output);
	unlink(odt.c_str());
    }
    convert_cleanup(handle);

    // Avoid segfault from LibreOffice by terminating swiftly.
    _Exit(rc);
}
