// ==========================================================
// FreeImage implementation
//
// Design and implementation by
// - Floris van den Berg (flvdberg@wxs.nl)
// - Herv� Drolon (drolon@infonie.fr)
// - Karl-Heinz Bussian (khbussian@moss.de)
//
// This file is part of FreeImage 3
//
// COVERED CODE IS PROVIDED UNDER THIS LICENSE ON AN "AS IS" BASIS, WITHOUT WARRANTY
// OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, WITHOUT LIMITATION, WARRANTIES
// THAT THE COVERED CODE IS FREE OF DEFECTS, MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE
// OR NON-INFRINGING. THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE COVERED
// CODE IS WITH YOU. SHOULD ANY COVERED CODE PROVE DEFECTIVE IN ANY RESPECT, YOU (NOT
// THE INITIAL DEVELOPER OR ANY OTHER CONTRIBUTOR) ASSUME THE COST OF ANY NECESSARY
// SERVICING, REPAIR OR CORRECTION. THIS DISCLAIMER OF WARRANTY CONSTITUTES AN ESSENTIAL
// PART OF THIS LICENSE. NO USE OF ANY COVERED CODE IS AUTHORIZED HEREUNDER EXCEPT UNDER
// THIS DISCLAIMER.
//
// Use at your own risk!
// ==========================================================


#ifdef WIN32
#include <windows.h>
#endif

#include "FreeImage.h"
#include "Utilities.h"

//----------------------------------------------------------------------

static const char *s_version = "3.3.0";
static const char *s_copyright = "This program uses FreeImage, a free, open source image library supporting all common bitmap formats. See http://freeimage.sourceforge.net for details";

//----------------------------------------------------------------------

#ifdef WIN32
#ifndef FREEIMAGE_LIB
BOOL APIENTRY
DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH :
			FreeImage_Initialise(FALSE);
			break;

		case DLL_PROCESS_DETACH :
			FreeImage_DeInitialise();
			break;

		case DLL_THREAD_ATTACH :
		case DLL_THREAD_DETACH :
			break;
    }

    return TRUE;
}
#endif // FREEIMAGE_LIB
#endif // WIN32

//----------------------------------------------------------------------

const char * DLL_CALLCONV
FreeImage_GetVersion() {
	return s_version;
}

const char * DLL_CALLCONV
FreeImage_GetCopyrightMessage() {
	return s_copyright;
}

//----------------------------------------------------------------------

BOOL DLL_CALLCONV
FreeImage_IsLittleEndian() {
	union {
		long i;
		char c[4];
	} u;
	u.i = 1;
	return (u.c[0] != 0);
}

//----------------------------------------------------------------------

FreeImage_OutputMessageFunction FreeImage_OutputMessageProcProc = NULL;

void DLL_CALLCONV
FreeImage_SetOutputMessage(FreeImage_OutputMessageFunction omf) {
	FreeImage_OutputMessageProcProc = omf;
}

void DLL_CALLCONV
FreeImage_OutputMessageProc(int fif, const char *fmt, ...) {
	const int MSG_SIZE = 512; // 512 bytes should be more than enough for a short message

	if ((fmt != NULL) && (FreeImage_OutputMessageProcProc != NULL)) {
		char message[MSG_SIZE];
		memset(message, 0, MSG_SIZE);

		// initialize the optional parameter list

		va_list arg;
		va_start(arg, fmt);

		// check the length of the format string

		int str_length = (strlen(fmt) > MSG_SIZE) ? MSG_SIZE : strlen(fmt);

		// parse the format string and put the result in 'message'

		for (int i = 0, j = 0; i < str_length; ++i) {
			if (fmt[i] == '%') {
				if (i + 1 < str_length) {
					switch(tolower(fmt[i + 1])) {
						case '%' :
							message[j++] = '%';
							break;

						case 'o' : // octal numbers
						{
							char tmp[16];

							_itoa(va_arg(arg, int), tmp, 8);

							strcat(message, tmp);

							j += strlen(tmp);

							++i;

							break;
						}

						case 'i' : // decimal numbers
						case 'd' :
						{
							char tmp[16];

							_itoa(va_arg(arg, int), tmp, 10);

							strcat(message, tmp);

							j += strlen(tmp);

							++i;

							break;
						}

						case 'x' : // hexadecimal numbers
						{
							char tmp[16];

							_itoa(va_arg(arg, int), tmp, 16);

							strcat(message, tmp);

							j += strlen(tmp);

							++i;

							break;
						}

						case 's' : // strings
						{
							char *tmp = va_arg(arg, char*);

							strcat(message, tmp);

							j += strlen(tmp);

							++i;

							break;
						}
					};
				} else {
					message[j++] = fmt[i];
				}
			} else {
				message[j++] = fmt[i];
			};
		}

		// deinitialize the optional parameter list

		va_end(arg);

		// output the message to the user program

		FreeImage_OutputMessageProcProc((FREE_IMAGE_FORMAT)fif, message);
	}
}
