file(GLOB random_SRC random/*.cpp random/*.hpp)

include_directories(
        ${top_srcdir}/src
        ${top_srcdir}/src/book
        ${top_srcdir}/src/hex
        ${top_srcdir}/src/util
        ${top_srcdir}/src/commonengine
        ${top_srcdir}/src/solver
        ${top_srcdir}/src/smartgame
        ${top_srcdir}/src/gtpengine
)

link_directories(/usr/local/lib)

add_executable(random ${random_SRC})

target_link_libraries(
        random
        hex 
        util 
        book 
        fuego_smartgame
        fuego_gtpengine
        solver 
        commonengine
        ${Boost_LIBRARIES}
        db
)
