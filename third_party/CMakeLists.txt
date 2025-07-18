


# Compilar ed25519-donna como librería estática (solo ed25519.c, sin tests)
add_library(ed25519-donna STATIC ed25519-donna/ed25519.c)
target_compile_definitions(ed25519-donna PUBLIC ED25519_REFHASH ED25519_REF_RANDOM)
target_include_directories(ed25519-donna PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ed25519-donna)

# Compilar tweetnacl como librería estática
file(GLOB TWEETNACL_SRC
    tweetnacl/*.c
)
add_library(tweetnacl STATIC ${TWEETNACL_SRC})
target_include_directories(tweetnacl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/tweetnacl)

