isEmpty(LLVM_INSTALL_DIR):LLVM_INSTALL_DIR=$$(LLVM_INSTALL_DIR)

DEFINES += CLANG_COMPLETION
DEFINES += CLANG_HIGHLIGHTING

## Unfortunately, CLANG_INDEXING takes too much time.
## May be it will be enabled by default in next month.
#DEFINES += CLANG_INDEXING

#DEFINES += CLANG_LEXER

win32 {
    LLVM_INCLUDEPATH = $$LLVM_INSTALL_DIR/include
    LLVM_LIBS = -L$$LLVM_INSTALL_DIR/bin \
        -L$$LLVM_INSTALL_DIR/lib \
        -lclang

    LLVM_LIBS += -ladvapi32 -lshell32
}

unix {
    LLVM_CONFIG=llvm-config
    !isEmpty(LLVM_INSTALL_DIR):LLVM_CONFIG=$$LLVM_INSTALL_DIR/bin/llvm-config-3.2

    LLVM_INCLUDEPATH = $$system($$LLVM_CONFIG --includedir)
    LLVM_LIBDIR = $$LLVM_INSTALL_DIR/lib

    LLVM_LIBS = -L$$LLVM_LIBDIR

    exists ($${LLVM_LIBDIR}/libclang.*) {
        #message("LLVM was build with autotools")
        CLANG_LIB = clang
    } else {
        exists ($${LLVM_LIBDIR}/liblibclang.*) {
            #message("LLVM was build with CMake")
            CLANG_LIB = libclang
        } else {
            error("Cannot find Clang shared library!")
        }
    }

    LLVM_LIBS += -l$${CLANG_LIB}
}

contains(DEFINES, CLANG_LEXER) {
    LLVM_LIBS += \
        -lclangLex \
        -lclangBasic \
        -lclangCodeGen \
        -lclangAnalysis \
        -lclangRewrite \
        -lclangSema \
        -lclangDriver \
        -lclangAST \
        -lclangParse \
        -lLLVMCore \
        -lLLVMSupport \
        -lLLVMMC


    LLVM_DEFINES += __STDC_LIMIT_MACROS __STDC_CONSTANT_MACROS
}
