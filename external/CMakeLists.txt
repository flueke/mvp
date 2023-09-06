add_subdirectory(gsl-lite)

if (NOT TARGET mesytec-mvlc)
    message("-- libmvp: using bundled mesytec-mvlc")
    add_subdirectory(mesytec-mvlc)
else()
    message("-- libmvp: using existing mesytec-mvlc target")
endif()
# vim:tw=0
