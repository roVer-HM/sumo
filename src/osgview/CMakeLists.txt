SET(osgview_STAT_SRCS
   GUIOSGHeader.h
   GUIOSGBuilder.cpp
   GUIOSGBuilder.h
   GUIOSGView.cpp
   GUIOSGView.h
   GUIOSGPerspectiveChanger.h
   GUIOSGPerspectiveChanger.cpp
   GUIOSGManipulator.h
   GUIOSGManipulator.cpp
)

add_library(osgview STATIC ${osgview_STAT_SRCS})
set_property(TARGET osgview PROPERTY PROJECT_LABEL "z_osgview")
