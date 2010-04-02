TEMPLATE = lib
TARGET = QtOpenCL
CONFIG += dll \
    warn_on
win32 { 
    DESTDIR = ../../bin
    !static:DEFINES += QT_MAKEDLL
}
else:DESTDIR = ../../lib

macx {
    LIBS += -framework OpenCL
} else {
    !isEmpty(QMAKE_INCDIR_OPENCL) {
        QMAKE_CXXFLAGS += -I$$QMAKE_INCDIR_OPENCL
    }
    !isEmpty(QMAKE_LIBDIR_OPENCL) {
        LIBS += -L$$QMAKE_LIBDIR_OPENCL
    }
    !isEmpty(QMAKE_LIBS_OPENCL) {
        LIBS += $$QMAKE_LIBS_OPENCL
    } else {
        LIBS += -lOpenCL
    }
}

HEADERS += \
    qclbuffer.h \
    qclcommandqueue.h \
    qclcontext.h \
    qcldevice.h \
    qclevent.h \
    qclglobal.h \
    qclimage.h \
    qclimageformat.h \
    qclkernel.h \
    qclmemoryobject.h \
    qclnamespace.h \
    qclplatform.h \
    qclprogram.h \
    qclsampler.h \
    qclvector.h \
    qclworksize.h

SOURCES += \
    qclbuffer.cpp \
    qclcommandqueue.cpp \
    qclcontext.cpp \
    qcldevice.cpp \
    qclevent.cpp \
    qclimage.cpp \
    qclimageformat.cpp \
    qclkernel.cpp \
    qclmemoryobject.cpp \
    qclplatform.cpp \
    qclprogram.cpp \
    qclsampler.cpp \
    qclvector.cpp

HEADERS += $$PRIVATE_HEADERS
DEFINES += QT_BUILD_CL_LIB
