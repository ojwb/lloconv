/* convert.h - Convert documents using LibreOfficeKit
 *
 * Copyright (C) 2015,2016 Olly Betts
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CONVERT_H
#define INCLUDED_CONVERT_H

extern const char * program;

void * convert_init();
int convert(void * h_void,
	    const char * input, const char * output,
	    const char * format = 0, const char * options = 0);
void convert_cleanup(void * h_void);

#endif
