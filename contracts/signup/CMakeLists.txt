add_contract(signup signup
   ${CMAKE_CURRENT_SOURCE_DIR}/src/signup.cpp
   #${CMAKE_CURRENT_SOURCE_DIR}/src/extra.cpp
)

target_include_directories(signup
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include
   ${CMAKE_CURRENT_SOURCE_DIR}/../eosio.system/include
   ${CMAKE_CURRENT_SOURCE_DIR}/../eosio.system) #src/exchange_state.cpp

set_target_properties(signup
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

#add_contract(rex.results rex.results ${CMAKE_CURRENT_SOURCE_DIR}/src/rex.results.cpp)

#target_include_directories(rex.results
#   PUBLIC
#   ${CMAKE_CURRENT_SOURCE_DIR}/include)

#set_target_properties(rex.results
#   PROPERTIES
#   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/.rex")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/signup.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/signup.contracts.md @ONLY )

target_compile_options( signup PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
