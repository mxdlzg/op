# 1.cmake version
cmake_minimum_required(VERSION 3.0.0)



# 3.include path
#INCLUDE_DIRECTORIES(
#    include
#)




set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DEF:op.def /NODEFAULTLIB")

 

# set_target_properties(target PROPERTIES VcpkgEnabled "true")

# 4.source directory
AUX_SOURCE_DIRECTORY(./ DIR_SRCS)

#LINK_DIRECTORIES("/3rd_party/lib/x86")
# 5.set environment variable
# find_package(OpenCV REQUIRED)
# include_directories(libop ${OpenCV_INCLUDE_DIRS})


#find_package(minhook CONFIG REQUIRED)
 #message(STATUS ${minhook_lib})




SET(SRC_FILES 
	"./background/opBackground.cpp" 

  "./background/display/opGDI.cpp" 
  "./background/display/opDXGI.cpp" 
  "./background/display/opDxGL.cpp" 
  "./background/display/IDisplay.cpp" 

  "./background/display/opWGC.cpp" 
  
  
  "./background/mouse/opMouseWin.cpp" 
  "./background/mouse/opMouseDx.cpp" 

  "./background/keypad/bkkeypad.cpp" 
  "./background/keypad/winkeypad.cpp" 
  
  "./background/Hook/DisplayHook.cpp"
  "./background/Hook/InputHook.cpp"
  "./background/Hook/HookExport.cpp"
  "./background/Hook/opDx12Hook.cpp"

 
"./core/globalVar.cpp"
"./core/helpfunc.cpp"
"./core/pipe.cpp"
"./core/opEnv.cpp"


	"./imageProc/imageloc.cpp"
	"./imageProc/imageproc.cpp"
	 "imageProc/tess_ocr.cpp"
	 "imageProc/OcrWrapper.cpp"
"./winapi/injecter.cpp"
"./winapi/memoryEx.cpp"
"./winapi/query_api.cpp"
"./winapi/winapi.cpp"
"../3rd_party/src/kiero.cpp"
"libop.cpp"
"libopEx.cpp"
)

SET(COM_SRC_FILES 
"./core/globalVar.cpp"
"./com/op.cpp"
"./com/opinterface.cpp"
"./com/stdafx.cpp"
"./com/compreg.cpp"
"./com/dllmain.cpp"
"./com/op_i.c"
"./com/op.def")

# 如果是动态，需要额外链接asmjit
set(ASMJIT_FILES "")
if(OP_BUILD_DLL)
  # 获取路径下的所有文件
  SET(ASMJIT_SOURCE_DIR "E:/ExtraLib/Blackbone/src/3rd_party/AsmJit")
  file(GLOB ASMJIT_FILES "${ASMJIT_SOURCE_DIR}/x86/*")

  # 输出文件列表（可选）
  foreach(file ${ASMJIT_FILES})
      message(STATUS "File: ${file}")
  endforeach()

  # 将文件列表保存到一个变量中（可选）
  SET(ASMJIT_SOURCES ${ASMJIT_FILES})
endif()


ADD_DEFINITIONS(-DUNICODE -D_UNICODE)
ADD_DEFINITIONS(-D WIN32)
ADD_DEFINITIONS(-D _WINDOWS)
ADD_DEFINITIONS(-D _USRDLL)
ADD_DEFINITIONS(-D _CRT_NON_CONFORMING_SWPRINTFS)
ADD_DEFINITIONS(-D _CRT_SECURE_NO_WARNINGS)
ADD_DEFINITIONS(-D _SILENCE_CXX17_ADAPTOR_TYPEDEFS_DEPRECATION_WARNING)
ADD_DEFINITIONS(-D _WINDLL)
ADD_DEFINITIONS(-D _UNICODE)
ADD_DEFINITIONS(-D UNICODE)
ADD_DEFINITIONS(-D OP_EXPORTS)
 

# 6.add lib file
#add_library(libop SHARED  ${SRC_FILES})

add_library(libop ${SRC_FILES})

# ----------op_x86/x64

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/com/op_i.c
  COMMAND midl ${CMAKE_CURRENT_SOURCE_DIR}/com/op.idl /iid 
  "./com/op_i.c" /env win32 /h "./com/op_i.h" /W1 /char signed /tlb 
  "./com/op.tlb" /Oicf /target "NT60" /D "NDEBUG" /robust /nologo /proxy "op_p.c"
 # DEPENDS MakeTable
 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )

add_library(${op_com} SHARED ${SRC_FILES} ${COM_SRC_FILES} ${ASMJIT_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/./com/op.rc "imageProc/tess_ocr.cpp")
SET_TARGET_PROPERTIES(${op_com} PROPERTIES LINK_FLAGS "/DEF:\"op.def\" /NODEFAULTLIB:\"mfc110d\"")
# add_custom_command(TARGET ${op_com} PRE_BUILD  COMMAND midl ./com/op.idl /iid "./com/op_i.c" /env win32 /h "./com/op_i.h" /W1 /char signed /tlb "./com/op.tlb" /Oicf /target "NT60" /D "NDEBUG" /robust /nologo /proxy "op_p.c"  WORKING_DIRECTORY  ${CMAKE_CURRENT_SOURCE_DIR})




target_include_directories(${op_com} PUBLIC
"${PROJECT_BINARY_DIR}"
"${CMAKE_CURRENT_SOURCE_DIR}"
)

include_directories(
  ./
  ./core
)

## for tesseract lib
find_package(Tesseract)
include_directories(${Tesseract_INCLUDE_DIRS})
target_link_libraries(${op_com} Tesseract::libtesseract)

# 7.add link library
#TARGET_LINK_LIBRARIES(libop BlackBone.lib)
#TARGET_LINK_LIBRARIES(libop minhook.lib)
TARGET_LINK_LIBRARIES(${op_com} debug ${BLACKBONE_LIB_D} optimized ${BLACKBONE_LIB})
TARGET_LINK_LIBRARIES(${op_com} minhook.lib)

# target_link_libraries(libop  ${OpenCV_LIBS})
# target_link_libraries(libop msvcrt.lib)


 
install(FILES libop.h DESTINATION "${PROJECT_SOURCE_DIR}/include")
install(FILES libopEx.h DESTINATION "${PROJECT_SOURCE_DIR}/include")
IF(CMAKE_CL_64)
  #install(TARGETS libop RUNTIME DESTINATION "${PROJECT_SOURCE_DIR}/bin/x64")
  #install(TARGETS libop ARCHIVE DESTINATION "${PROJECT_SOURCE_DIR}/lib/x64")
  install(TARGETS ${op_com} RUNTIME DESTINATION "${PROJECT_SOURCE_DIR}/bin/x64")
  install(TARGETS ${op_com} ARCHIVE DESTINATION "${PROJECT_SOURCE_DIR}/lib/x64")
ELSE(CMAKE_CL_64)
  #install(TARGETS libop RUNTIME DESTINATION "${PROJECT_SOURCE_DIR}/bin/x86")
  #install(TARGETS libop ARCHIVE DESTINATION "${PROJECT_SOURCE_DIR}/lib/x86")
  install(TARGETS ${op_com} RUNTIME DESTINATION "${PROJECT_SOURCE_DIR}/bin/x86")
  install(TARGETS ${op_com} ARCHIVE DESTINATION "${PROJECT_SOURCE_DIR}/lib/x86")
ENDIF(CMAKE_CL_64)




