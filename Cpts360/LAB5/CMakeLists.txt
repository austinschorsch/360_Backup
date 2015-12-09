cmake_minimum_required(VERSION 3.3)
project(aschorsch)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    CLion/bin/cmake/share/cmake-3.3/include/cmCPluginAPI.h
    CLion/bin/cmake/share/cmake-3.3/Modules/FortranCInterface/Verify/main.c
    CLion/bin/cmake/share/cmake-3.3/Modules/FortranCInterface/Verify/VerifyC.c
    CLion/bin/cmake/share/cmake-3.3/Modules/FortranCInterface/Verify/VerifyCXX.cxx
    CLion/bin/cmake/share/cmake-3.3/Modules/FortranCInterface/my_module_.c
    CLion/bin/cmake/share/cmake-3.3/Modules/FortranCInterface/mymodule_.c
    CLion/bin/cmake/share/cmake-3.3/Modules/CheckForPthreads.c
    CLion/bin/cmake/share/cmake-3.3/Modules/CheckFunctionExists.c
    CLion/bin/cmake/share/cmake-3.3/Modules/CheckVariableExists.c
    CLion/bin/cmake/share/cmake-3.3/Modules/CMakeCCompilerABI.c
    CLion/bin/cmake/share/cmake-3.3/Modules/CMakeCompilerABI.h
    CLion/bin/cmake/share/cmake-3.3/Modules/CMakeCXXCompilerABI.cpp
    CLion/bin/cmake/share/cmake-3.3/Modules/CMakeTestGNU.c
    CLion/bin/cmake/share/cmake-3.3/Modules/CMakeTestWatcomVersion.c
    CLion/bin/cmake/share/cmake-3.3/Modules/DummyCXXFile.cxx
    CLion/bin/cmake/share/cmake-3.3/Modules/TestForAnsiForScope.cxx
    CLion/bin/cmake/share/cmake-3.3/Modules/TestForANSIStreamHeaders.cxx
    CLion/bin/cmake/share/cmake-3.3/Modules/TestForSSTREAM.cxx
    CLion/bin/cmake/share/cmake-3.3/Modules/TestForSTDNamespace.cxx
    CLion/bin/gdb/lib/python2.7/distutils/tests/xxmodule.c
    CLion/ClionProjects/test/main.cpp
    "Cpts360/ASS1/Homework 1/t.c"
    Cpts360/LAB1/Lab1/t.c
    Cpts360/LAB2/t.c
    Cpts360/LAB3/t.c
    Cpts360/LAB3/t1.c
    Cpts360/LAB4/client.c
    Cpts360/LAB4/server.c
    Downloads/mysh2.c
    etc/ext2fs/inode.c
    etc/ext2fs/time.c
    etc/initrd-tree/lib/firmware/zd1211/WS11Ub.h
    etc/initrd-tree/lib/firmware/zd1211/WS11UPh.h
    etc/initrd-tree/lib/firmware/zd1211/WS11UPhm.h
    etc/initrd-tree/lib/firmware/zd1211/WS11UPhR.h
    etc/initrd-tree/lib/firmware/zd1211/WS11UPhR_Turbo.h
    etc/initrd-tree/lib/firmware/zd1211/WS11Ur.h)

add_executable(aschorsch ${SOURCE_FILES})