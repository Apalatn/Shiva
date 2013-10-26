#ifndef __ERROR__UTILS__H
#define __ERROR__UTILS__H

#include <cstdio>
#include <cstdlib>

void SEVERE(const char *errorString, int errorCode)
{
	fprintf(stderr, "Severe error: %s\nPress any key to quit.", errorString);
	std::getchar();
	exit(errorCode);
}

#endif//__ERROR__UTILS__H