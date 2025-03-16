#pragma once

#pragma comment(lib, "comctl32")
//      ^^^^^^^^^^^^^^^^^^^^^^^^ for list view
#pragma comment(lib, "msimg32.lib")
//      ^^^^^^^^^^^^^^^^^^^^^^^^^^^ for TransparentBlt

#include "resource.hpp"
#include "targetver.hpp"

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#define NOMINMAX
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#undef VC_EXTRALEAN
#undef NOMINMAX

// drag & drop
#include <shellapi.h>

// list view
#include <commctrl.h>
