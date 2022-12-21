#pragma once

#ifdef IS_PLATFORM_WINDOWS
#include "Platforms/Windows/PlatformWindows.h"
#include "Platforms/Windows/PlatformFileDialogWindows.h"
#define SAFE_BREAK __debugbreak()
#elif IS_PLATFORM_LINUX
#define SAFE_BREAK(void)
#endif

#if defined(IS_PLATFORM_WINDOWS) || defined(IS_PLATFORM_OSX) ||defined(IS_PLATFORM_LINUX)
#define IS_PLATFORM_DESKTOP
#endif

namespace Insight
{
#ifdef IS_PLATFORM_WINDOWS
	using Platform = Windows::PlatformWindows;
	using PlatformFileDialog = Platforms::Windows::PlatformFileDialogWindows;
#elif IS_PLATFORM_LINUX
	using Platform = Linux::PlatformLinux;
	using PlatformFileDialog = Windows::PlatformFileDialogLinux;
#endif

#define IS_ASSERT_ENABLED
#ifdef IS_ASSERT_ENABLED

	void IS_CORE AssetPrintError(const char* format, ...);

#define ASSERT(condition)																									\
	if (!(condition))																										\
	{																														\
		AssetPrintError("ASSERT: \n\tFILE: '%s', \n\tLINE: '%i', \n\tCondition: '%s'", __FILE__, __LINE__, #condition);	\
		SAFE_BREAK;																											\
	}

#define ASSERT_MSG(condition, message)																													\
	if (!(condition))																																	\
	{																																					\
		AssetPrintError("ASSERT: \n\tFILE: '%s', \n\tLINE: '%i', \n\tCondition: '%s', \n\tMessage: '%s'", __FILE__, __LINE__, #condition, message);	\
		SAFE_BREAK;																																		\
	}

#define FAIL_ASSERT()																									\
		ASSERT(false)

#define FAIL_ASSERT_MSG(message)																									\
		ASSERT_MSG(false, message)
#endif
}