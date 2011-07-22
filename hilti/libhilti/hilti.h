// $Id$
//
// Top-level HILTI include file.
//
// Note that all code using libhilti (or part of libhilti) should include
// this file before any other. (This is to make sure GC is setup correctly.

/// \mainpage LibHILTI Reference Documentation
///
/// \section data-types HILTI Data Types
///
/// - \ref bytes
/// - \ref regexp
///
/// \section run-time-support Run-Time Support
///
/// \section debugging Debugging Support
///
/// \section internals Internal Run-time Functions

#ifndef HILTI_H
#define HILTI_H

#ifdef USE_GC
// gc.h must be included library-wide because it overwrites some standard
// functions (such as pthreads_create())
#include <gc.h>
#endif

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>

#include "types.h"

#include "addr.h"
#include "bool.h"
#include "bytes.h"
#include "channel.h"
#include "config.h"
#include "double.h"
#include "enum.h"
#include "hook.h"
#include "exceptions.h"
#include "debug.h"
#include "hilti.h"
#include "init.h"
#include "int.h"
#include "list.h"
#include "memory.h"
#include "net.h"
#include "overlay.h"
#include "port.h"
#include "regexp.h"
#include "str.h"
#include "threading.h"
#include "tuple.h"
#include "utf8proc.h"
#include "vector.h"
#include "continuation.h"
#include "list.h"
#include "tim.h"
#include "interval.h"
#include "timer.h"
#include "map_set.h"
#include "iosrc.h"
#include "context.h"
#include "globals.h"
#include "cmdqueue.h"
#include "file.h"
#include "profiler.h"
#include "classifier.h"
#include "util.h"

#include "module/module.h"

#include <hilti.hlt.h>

#endif
