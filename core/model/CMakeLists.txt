

my_test(model_test    
        model_test.cpp 
     
      )
target_link_libraries(model_test physics   io  parallel utilities )

ADD_EXECUTABLE(geqdsk_convert geqdsk.cpp geqdsk_xdmf.cpp geqdsk_convert.cpp) 
TARGET_LINK_LIBRARIES(geqdsk_convert   Xdmf io  parallel utilities )


add_library(geqdsk   geqdsk.cpp)
TARGET_LINK_LIBRARIES(geqdsk  io parallel utilities)
