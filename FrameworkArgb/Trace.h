/*++

Module Name:

    Internal.h

Abstract:

    This module contains the local type definitions for the
    driver.

Environment:

    Windows User-Mode Driver Framework 2

--*/

//
// Define the tracing flags.
//
// Tracing GUID - 79b262e6-f592-443a-9b0a-d1bc1184ad50
//

#define WPP_CONTROL_GUIDS                                              \
    WPP_DEFINE_CONTROL_GUID(                                           \
        MyDriver1TraceGuid, (79b262e6,f592,443a,9b0a,d1bc1184ad50),                  \
                                                                       \
        WPP_DEFINE_BIT(EntryExit)                                          \
        WPP_DEFINE_BIT(DataFlow)                                           \
        WPP_DEFINE_BIT(Verbose)                                            \
        WPP_DEFINE_BIT(Information)                                        \
        WPP_DEFINE_BIT(Warning)                                            \
        WPP_DEFINE_BIT(Error)                                              \
        WPP_DEFINE_BIT(Fatal)                                              \
        WPP_DEFINE_BIT(DriverStatus)                                       \
        )                             

#define WPP_FLAG_LEVEL_LOGGER(flag, level)                             \
    WPP_LEVEL_LOGGER(flag)

#define WPP_FLAG_LEVEL_ENABLED(flag, level)                            \
    (WPP_LEVEL_ENABLED(flag) &&                                        \
     WPP_CONTROL(WPP_BIT_ ## flag).Level >= level)

#define WPP_LEVEL_FLAGS_LOGGER(lvl,flags)                              \
           WPP_LEVEL_LOGGER(flags)
               
#define WPP_LEVEL_FLAGS_ENABLED(lvl, flags)                            \
           (WPP_LEVEL_ENABLED(flags) && WPP_CONTROL(WPP_BIT_ ## flags).Level >= lvl)

//
// This comment block is scanned by the trace preprocessor to define our
// Trace function.
//
// begin_wpp config
// FUNC Trace{FLAG=MYDRIVER_ALL_INFO}(LEVEL, MSG, ...);
// FUNC TraceEvents(LEVEL, FLAGS, MSG, ...);
// FUNC TraceFatal{LEVEL=TRACE_LEVEL_FATAL,FLAGS=Fatal}(MSG,...);
// FUNC TraceError{LEVEL=TRACE_LEVEL_ERROR,FLAGS=Error}(MSG,...);
// FUNC TraceWarning{LEVEL=TRACE_LEVEL_WARNING,FLAGS=Warning}(MSG,...);
// FUNC TraceInformation{LEVEL=TRACE_LEVEL_INFORMATION,FLAGS=Information}(MSG,...);
// FUNC TraceVerbose{LEVEL=TRACE_LEVEL_VERBOSE,FLAGS=Verbose}(MSG,...);
// FUNC TracePerformance{PERF=DUMMY,LEVEL=TRACE_LEVEL_PERF}(FLAGS,MSG,...);
//
// FUNC TraceData{LEVEL=TRACE_LEVEL_VERBOSE,FLAGS=DataFlow}(MSG,...);
//
// FUNC TraceDriverStatus{LEVEL=TRACE_LEVEL_INFORMATION,FLAGS=DriverStatus}(MSG,...);

// end_wpp

//
//
// Driver specific #defines
//
#if UMDF_VERSION_MAJOR == 2 && UMDF_VERSION_MINOR == 0
    #define MYDRIVER_TRACING_ID      L"Microsoft\\UMDF2.0\\FrameworkArgb V1.0"
#endif