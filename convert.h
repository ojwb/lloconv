/* convert.h - Convert documents using liblibreoffice/LibreOfficeKit
 *
 * Copyright (C) 2015 Olly Betts
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CONVERT_H
#define INCLUDED_CONVERT_H

const char * program;
const char * get_lo_path();
int convert(const char * format, const char * lo_path,
	    const char * input, const char * output, const char * options);

#endif
