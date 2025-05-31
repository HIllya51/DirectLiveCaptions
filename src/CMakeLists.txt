cmake_minimum_required(VERSION 3.16)

project(DirectLiveCaptions)
add_definitions(-DUNICODE -D_UNICODE)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
include(FetchContent)
FetchContent_Declare(CognitiveServices 
	URL https://www.nuget.org/api/v2/package/Microsoft.CognitiveServices.Speech/1.41.1
	DOWNLOAD_EXTRACT_TIMESTAMP true
)
FetchContent_Declare(CognitiveServicesEmbed 
	URL https://www.nuget.org/api/v2/package/Microsoft.CognitiveServices.Speech.extension.embedded.sr/1.41.1
	DOWNLOAD_EXTRACT_TIMESTAMP true
)
FetchContent_Declare(CognitiveServicestelemetry
	URL https://www.nuget.org/api/v2/package/Microsoft.CognitiveServices.Speech.extension.telemetry/1.41.1
	DOWNLOAD_EXTRACT_TIMESTAMP true
)
FetchContent_Declare(CognitiveServicesonnxruntime
	URL https://www.nuget.org/api/v2/package/Microsoft.CognitiveServices.Speech.extension.onnx.runtime/1.41.1
	DOWNLOAD_EXTRACT_TIMESTAMP true
)
FetchContent_MakeAvailable(CognitiveServices CognitiveServicesEmbed CognitiveServicestelemetry CognitiveServicesonnxruntime)

add_library(CognitiveServices INTERFACE)
target_include_directories(CognitiveServices INTERFACE ${cognitiveservices_SOURCE_DIR}/build/native/include/c_api)
target_include_directories(CognitiveServices INTERFACE ${cognitiveservices_SOURCE_DIR}/build/native/include/cxx_api)

target_link_libraries(CognitiveServices INTERFACE ${cognitiveservices_SOURCE_DIR}/build/native/x64/Release/Microsoft.CognitiveServices.Speech.core.lib) 
target_link_options(CognitiveServices INTERFACE "/DELAYLOAD:Microsoft.CognitiveServices.Speech.core.dll")

add_library(pch pch.cpp)
target_precompile_headers(pch PUBLIC pch.h)

add_executable(DirectLiveCaptions WIN32 main.cpp appx.cpp window.cpp controls.cpp app.manifest LoopbackCapture.cpp)
target_precompile_headers(DirectLiveCaptions REUSE_FROM pch)
target_link_libraries(DirectLiveCaptions CognitiveServices)
target_link_libraries(DirectLiveCaptions windowsapp comctl32 Mmdevapi mfuuid)
set(CMAKE_FINAL_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/Release)
add_custom_command(TARGET DirectLiveCaptions
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${cognitiveservices_SOURCE_DIR}/runtimes/win-x64/native/Microsoft.CognitiveServices.Speech.core.dll
        ${CMAKE_FINAL_OUTPUT_DIRECTORY}/Microsoft.CognitiveServices.Speech.core.dll
    )
add_custom_command(TARGET DirectLiveCaptions
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${cognitiveservices_SOURCE_DIR}/runtimes/win-x64/native/Microsoft.CognitiveServices.Speech.extension.audio.sys.dll
        ${CMAKE_FINAL_OUTPUT_DIRECTORY}/Microsoft.CognitiveServices.Speech.extension.audio.sys.dll
    )
add_custom_command(TARGET DirectLiveCaptions
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${cognitiveservicesembed_SOURCE_DIR}/runtimes/win-x64/native/Microsoft.CognitiveServices.Speech.extension.embedded.sr.dll
        ${CMAKE_FINAL_OUTPUT_DIRECTORY}/Microsoft.CognitiveServices.Speech.extension.embedded.sr.dll
    )
add_custom_command(TARGET DirectLiveCaptions
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${cognitiveservicesembed_SOURCE_DIR}/runtimes/win-x64/native/Microsoft.CognitiveServices.Speech.extension.embedded.sr.runtime.dll
        ${CMAKE_FINAL_OUTPUT_DIRECTORY}/Microsoft.CognitiveServices.Speech.extension.embedded.sr.runtime.dll
    )
add_custom_command(TARGET DirectLiveCaptions
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${cognitiveservices_SOURCE_DIR}/runtimes/win-x64/native/Microsoft.CognitiveServices.Speech.extension.lu.dll
        ${CMAKE_FINAL_OUTPUT_DIRECTORY}/Microsoft.CognitiveServices.Speech.extension.lu.dll
    )
add_custom_command(TARGET DirectLiveCaptions
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${cognitiveservicesonnxruntime_SOURCE_DIR}/runtimes/win-x64/native/Microsoft.CognitiveServices.Speech.extension.onnxruntime.dll
        ${CMAKE_FINAL_OUTPUT_DIRECTORY}/Microsoft.CognitiveServices.Speech.extension.onnxruntime.dll
    )
add_custom_command(TARGET DirectLiveCaptions
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${cognitiveservicestelemetry_SOURCE_DIR}/runtimes/win-x64/native/Microsoft.CognitiveServices.Speech.extension.telemetry.dll
        ${CMAKE_FINAL_OUTPUT_DIRECTORY}/Microsoft.CognitiveServices.Speech.extension.telemetry.dll
    )