set(foreign_glad_STAT_SRCS
   glad.h
  khrplatform.h
   glad.c
)

add_library(foreign_glad STATIC ${foreign_glad_STAT_SRCS})
set_property(TARGET foreign_glad PROPERTY PROJECT_LABEL "z_foreign_glad")
if (SUMO_UTILS)
    install(TARGETS foreign_glad DESTINATION lib)
endif ()
