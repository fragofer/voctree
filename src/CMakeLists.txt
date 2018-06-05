set(NAME engine)

set(SOURCES
        Catalog.cpp
        Catalog.h
        Configuration.cpp
        Configuration.h
        Database.cpp
        Database.h
        ExtKmeans.cpp
        ExtKmeans.h
        FeatureMethod.cpp
        FeatureMethod.h
        FileHelper.cpp
        FileHelper.h
        FileManager.cpp
        FileManager.h
        KeyPointPersistor.cpp
        KeyPointPersistor.h
        KMeans.cpp
        KMeans.h
        main.cpp
        Matching.cpp
        Matching.h
        MatPersistor.cpp
        MatPersistor.h
        Server.cpp
        Server.h
        ShootSegmenter.cpp
        ShootSegmenter.h
        VecPersistor.hpp
        VocTree.cpp
        VocTree.h
) # SOURCES

add_executable(${NAME} 	${SOURCES})
target_link_libraries(${NAME} ${OpenCV_LIBS})

