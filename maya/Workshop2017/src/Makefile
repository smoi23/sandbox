ifndef INCL_BUILDRULES
#
# Include platform specific build settings
#
TOP := /blech/code/maya/devkit/plug-ins/
include $(TOP)/buildrules
all: prebuildPlugins
endif


SRCDIR := ./
DSTDIR := ../build
DEPLOY_PATH := /blech/code/maya/plug-ins/


INCLUDES += -I../include


# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
$(SRCDIR)/pluginMain.cpp \
$(SRCDIR)/HelloWorldCmd.cpp \
$(SRCDIR)/HelloWorldArgsCmd.cpp \
$(SRCDIR)/SceneModifierCmd.cpp \
$(SRCDIR)/HelloWorldNode.cpp \
$(SRCDIR)/BBoxCubeCmd.cpp \
$(SRCDIR)/CopyMeshNode.cpp \
$(SRCDIR)/SphereTransformNode.cpp \
$(SRCDIR)/AttractorDeformerNode.cpp \
$(SRCDIR)/noise.cpp 


OBJS += \
pluginMain.o \
HelloWorldCmd.o \
HelloWorldArgsCmd.o \
SceneModifierCmd.o \
HelloWorldNode.o \
BBoxCubeCmd.o \
CopyMeshNode.o \
SphereTransformNode.o \
AttractorDeformerNode.o \
noise.o 

TARGET = $(DSTDIR)/Workshop2017.$(EXT)


$(OBJS): CFLAGS   :=$(CFLAGS)
$(OBJS): C++FLAGS := $(C++FLAGS)
$(OBJS): INCLUDES := $(INCLUDES)

$(TARGET): LFLAGS := $(LFLAGS)
$(TARGET): LIBS   := $(LIBS) -lOpenMaya -lFoundation -lOpenMayaAnim -lOpenMayaFX -lOpenMayaRender -lOpenMayaUI 

.PHONY: deploy

# Targets
prebuildPlugins: $(TARGET)

$(TARGET): $(OBJS)
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

clean:
	-rm -f $(OBJS)

Clean:
	-rm -f $(OBJS) $(TARGET)

deploy:
	-cp $(TARGET) $(DEPLOY_PATH)


	
# gcc412 -c -I. -I.. -I/usr/autodesk/maya2015-x64/include -I/usr/X11R6/include -DBits64_ -m64 -DUNIX -D_BOOL -DLINUX -DFUNCPROTO -D_GNU_SOURCE -DLINUX_64 -O3 -pthread -pipe -DREQUIRE_IOSTREAM -fPIC -fno-strict-aliasing -Wno-deprecated -Wno-multichar -Wno-comment -Wno-sign-compare -fno-gnu-keywords templateCmd.cc templateNode.cc registerPlugin.cc
# gcc412 -shared -m64 -O3 -pthread -pipe -D_BOOL -DLINUX -DREQUIRE_IOSTREAM -mtune=pentium4 -Wno-deprecated -fno-gnu-keywords -Wl,-Bsymbolic -shared -o template.so templateCmd.o templateNode.o registerPlugin.o -L/usr/autodesk/maya2015-x64/lib -lOpenMaya -lFoundation

	
