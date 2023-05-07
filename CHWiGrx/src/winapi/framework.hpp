#pragma once

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
