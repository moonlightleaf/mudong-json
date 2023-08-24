set(HEADERS
        noncopyable.hpp
        FileReadStream.hpp
        FileWriteStream.hpp
        StringReadStream.hpp
        StringWriteStream.hpp
        Value.hpp
        Exception.hpp
        Writer.hpp
        Reader.hpp
        Document.hpp
)

add_library(mudong-json STATIC ${HEADERS})
set_target_properties(mudong-json PROPERTIES LINKER_LANGUAGE CXX)

install(TARGETS mudong-json DESTINATION lib)

install(FILES ${HEADERS} DESTINATION include)