#pragma once

#define __NOT_YET_IMPLEMENTED__    0
#define __TEMPORARY_DISABLED__     0

#define __USE_GPGPU__              __NOT_YET_IMPLEMENTED__
#define __USE_EXTRA_HSV_RANGE      __TEMPORARY_DISABLED__

#include <iostream>
#include <algorithm>
#include <cstdint>

#include "Window.h"
#include "GroupBox.h"
#include "Button.h"
#include "Text.h"
#include "Edit.h"
#include "Trackbar.h"

#include "IORoutines.h"

#include <opencv2/core.hpp>

#if __USE_GPGPU__
#include "cudaImpls.h"
#endif

#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/utility.hpp>

#include <chrono>

template<typename TimeT = std::chrono::milliseconds>
struct measure {
    template<typename F, typename ...Args>
    static typename TimeT::rep execution(F func, Args &&... args)
    {
        auto start = std::chrono::system_clock::now();

        func(std::forward<Args>(args)...);

        auto duration = std::chrono::duration_cast<TimeT>
            (std::chrono::system_clock::now() - start);

        return duration.count();
    }
};

#if _DEBUG
#   pragma comment(lib, "opencv_world320d.lib")
//#   pragma comment(lib, "opencv_core320d.lib")
//#   pragma comment(lib, "opencv_highgui320d.lib")
//#   pragma comment(lib, "opencv_imgcodecs320d.lib")
//#   pragma comment(lib, "opencv_imgproc320d.lib")
#elif NDEBUG
#   pragma comment(lib, "opencv_world320.lib")
//#   pragma comment(lib, "opencv_core320.lib")
//#   pragma comment(lib, "opencv_highgui320.lib")
//#   pragma comment(lib, "opencv_imgcodecs320.lib")
//#   pragma comment(lib, "opencv_imgproc320.lib")
#endif

// For using in UI Vista-like buttons, edit boxes and etc.
#if UNICODE
#   if defined _M_X64
#       pragma comment(linker, "/manifestdependency:\"type='win32' \
                                name='Microsoft.Windows.Common-Controls' \
                                version='6.0.0.0' processorArchitecture='amd64' \
                                publicKeyToken='6595b64144ccf1df' \
                                language='*'\"")
#   elif defined _M_IA64
#       pragma comment(linker, "/manifestdependency:\"type='win32' \
                                name='Microsoft.Windows.Common-Controls' \
                                version='6.0.0.0' processorArchitecture='ia64' \
                                publicKeyToken='6595b64144ccf1df' \
                                language='*'\"")
#   elif defined _M_IX86
#       pragma comment(linker, "/manifestdependency:\"type='win32' \
                                name='Microsoft.Windows.Common-Controls' \
                                version='6.0.0.0' processorArchitecture='x86' \
                                publicKeyToken='6595b64144ccf1df' \
                                language='*'\"")
#   else
#       pragma comment(linker, "/manifestdependency:\"type='win32' \
                                name='Microsoft.Windows.Common-Controls' \
                                version='6.0.0.0' processorArchitecture='*' \
                                publicKeyToken='6595b64144ccf1df' \
                                language='*'\"")
#   endif
#endif