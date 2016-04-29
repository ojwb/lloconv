/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Changes made for lloconv:
 *  + Added <stdlib.h> for malloc() and free()
 *  + Avoid having to include libreoffice headers 
 */

#ifdef LINUX

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <LibreOfficeKit.h>

#include <dlfcn.h>
#ifdef AIX
#  include <sys/ldr.h>
#endif

#define TARGET_LIB  "lib" "sofficeapp" ".so"

// If LO is built with --enable-mergelibs then we want to look in this library
// instead.  This is the case on Ubuntu.
#define TARGET_LIB2 "lib" "mergedlo" ".so"

#define TARGET_LIB_MAX_LEN \
    (sizeof(TARGET_LIB) > sizeof(TARGET_LIB2) ? \
         sizeof(TARGET_LIB) : sizeof(TARGET_LIB2))

typedef LibreOfficeKit *(HookFunction)(const char*);

int lok_init( const char *install_path, LibreOfficeKit** lok )
{
    char *imp_lib;
    void *dlhandle;
    HookFunction *pSym;
    size_t len;

    if( !install_path )
        return 0;
    len = strlen( install_path );
    imp_lib = (char *) malloc( len + TARGET_LIB_MAX_LEN + 2 );
    if( imp_lib == NULL )
    {
        fprintf( stderr, "failed to open library : not enough memory\n");
        return 0;
    }

    strcpy( imp_lib, install_path );
    imp_lib[len] = '/';
    strcpy( imp_lib + len + 1, TARGET_LIB );

    if( !( dlhandle = dlopen( imp_lib, RTLD_LAZY ) ) )
    {
        strcpy( imp_lib + len + 1, TARGET_LIB2 );
        if( !( dlhandle = dlopen( imp_lib, RTLD_LAZY ) ) )
        {
            imp_lib[len + 1] = '\0';
            fprintf( stderr, "failed to open library '%s"TARGET_LIB"' or "
                             "'%s"TARGET_LIB2"'\n", imp_lib, imp_lib );
            free( imp_lib );
            return 0;
        }
    }

    pSym = (HookFunction *) dlsym( dlhandle, "libreofficekit_hook" );
    if( !pSym ) {
        fprintf( stderr, "failed to find libreofficekit_hook in library '%s'\n", imp_lib );
        dlclose( dlhandle );
        free( imp_lib );
        return 0;
    }

    free( imp_lib );
    *lok = pSym( install_path );
    return 1;
}

#endif // not LINUX => port me !

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
