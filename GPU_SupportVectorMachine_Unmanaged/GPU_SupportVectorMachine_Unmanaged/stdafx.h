// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "targetver.h"
#include "resource.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <vector>
#include <map>
#include <set>
#include <string>
#include <queue>
#include <fstream>
#include <algorithm>
#include <io.h>

#define BOOST_FILESYSTEM_NO_DEPRECATED
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/timer.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include <d3d11.h>
#include <d3dx11.h>

// TODO: reference additional headers your program requires here
typedef boost::shared_ptr<boost::mutex> MutexPtr;
typedef boost::shared_ptr<boost::thread> ThreadPtr;
typedef boost::shared_ptr<boost::condition_variable> ConditionPtr;

#define TRACE_DEBUG(msg) printf(msg)