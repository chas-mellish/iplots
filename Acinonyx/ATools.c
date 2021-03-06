/*
 *  ATools.c
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 3/2/08.
 *  Copyright 2008 Simon Urbanek. All rights reserved.
 *
 */

#include <stdio.h> /* for vsnprintf for value_printf() */
#include <stdarg.h>

#include "ATools.h"

// common color scheme
AColor backgroundColor = { 1.0, 1.0, 0.7, 1.0 };
AColor pointColor      = { 0.0, 0.0, 0.0, 1.0 };
AColor textColor       = { 0.0, 0.0, 0.0, 1.0 };
AColor hiliteColor     = { 1.0, 0.0, 0.0, 1.0 };
AColor barColor        = { 0.8, 0.8, 0.8, 1.0 };
AColor widgetColor     = { 0.0, 0.0, 0.0, 0.5 };
AColor widgetHoverColor= { 0.9, 0.9, 0.9, 0.5 };

/* FIXME: intitallize NA_xx values */
/* declared in ATypes.h */
double NA_double;
float NA_float;

char value_buf[128];

const char *value_printf(const char *fmt, ...) {
	va_list v;
	va_start(v, fmt);
	vsnprintf(value_buf, sizeof(value_buf), fmt, v);
	va_end(v);
	return (const char*) value_buf;
}

unsigned int current_frame;

#ifdef DEBUG

void AError(const char *fmt, ...) {
	va_list v;
	va_start(v, fmt);
	fprintf(stderr, "*** ERROR (set breakpoint on AError to trace)\n");
	vfprintf(stderr, fmt, v);
	va_end(v);
	fprintf(stderr, "\n");	
}

void ALog(const char *fmt, ...) {
#ifdef PROFILE
	long npt = time_ms();
	if (startupTime == 0L) startupTime = npt;
	fprintf(stderr, "[%4ld.%03ld] ", (npt - startupTime) / 1000, (npt - startupTime) % 1000);
#endif
	va_list v;
	va_start(v, fmt);
	vfprintf(stderr, fmt, v);
	va_end(v);
	fprintf(stderr, "\n");
}
#endif

#ifdef PROFILE
#include <sys/time.h>
#include <stdio.h>

long time_ms() {
#ifdef Win32
	return 0; /* in Win32 we have no gettimeofday :( */
#else
	struct timeval tv;
	gettimeofday(&tv,0);
	return (tv.tv_usec/1000)+(tv.tv_sec*1000);
#endif
}

long profilerTime, startupTime;

void profReport(const char *fmt, ...) {
	long npt = time_ms();
	if (startupTime == 0L) startupTime = profilerTime;
	fprintf(stderr, "[%4ld.%03ld] ", (npt - startupTime) / 1000, (npt - startupTime) % 1000);
	va_list v;
	va_start(v, fmt);
	vfprintf(stderr, fmt, v);
	va_end(v);
	fprintf(stderr, " %ld ms\n", npt - profilerTime);
	profilerTime = npt;
}
#endif
