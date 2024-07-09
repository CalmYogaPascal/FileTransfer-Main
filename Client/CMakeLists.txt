
project (Client)

add_executable(Client Client.cpp MyList.cpp MyListItem.cpp)
find_package(Qt5 COMPONENTS Widgets REQUIRED)
set_target_properties(Client PROPERTIES AUTOMOC TRUE)
target_link_libraries(Client ${Boost_LIBRARIES} Protoses Qt5::Widgets)