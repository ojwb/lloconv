/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_DESKTOP_INC_LIBREOFFICEKIT_HXX
#define INCLUDED_DESKTOP_INC_LIBREOFFICEKIT_HXX

#include "LibreOfficeKit.h"

/*
 * The reasons this C++ code is not as pretty as it could be are:
 *  a) provide a pure C API - that's useful for some people
 *  b) allow ABI stability - C++ vtables are not good for that.
 *  c) avoid C++ types as part of the API.
 */
namespace lok
{

class Document
{
private:
    LibreOfficeKitDocument* mpDoc;

public:
    inline Document(LibreOfficeKitDocument* pDoc) :
        mpDoc(pDoc)
    {}

    inline ~Document()
    {
        mpDoc->pClass->destroy(mpDoc);
    }

    inline bool saveAs(const char* pUrl, const char* pFormat = NULL, const char* pFilterOptions = NULL)
    {
        return mpDoc->pClass->saveAs(mpDoc, pUrl, pFormat, pFilterOptions);
    }
    inline LibreOfficeKitDocument *get() { return mpDoc; }
};

class Office
{
private:
    LibreOfficeKit* mpThis;

public:
    inline Office(LibreOfficeKit* pThis) :
        mpThis(pThis)
    {}

    inline ~Office()
    {
        mpThis->pClass->destroy(mpThis);
    }

    inline Document* documentLoad(const char* pUrl, const char* pOptions)
    {
        LibreOfficeKitDocument* pDoc;
        if (LIBREOFFICEKIT_HAS(mpThis, documentLoadWithOptions))
            pDoc = mpThis->pClass->documentLoadWithOptions(mpThis, pUrl, pOptions);
        else
            pDoc = mpThis->pClass->documentLoad(mpThis, pUrl);

        if (pDoc == NULL)
            return NULL;
        return new Document(pDoc);
    }

    // return the last error as a string, free me.
    inline char* getError()
    {
        return mpThis->pClass->getError(mpThis);
    }
};

inline bool lok_cpp_init(const char* pInstallPath, Office** lok)
{
    LibreOfficeKit* pThis;
    if (!lok_init(pInstallPath, &pThis) || pThis == NULL)
	return false;

    if (pThis->pClass->nSize == 0)
	return false;

    *lok = new ::lok::Office(pThis);
    return true;
}

}
#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
