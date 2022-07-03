QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

INCLUDEPATH += /usr/local/include/opencv4
LIBS += -lopencv_calib3d -lopencv_core -lopencv_dnn -lopencv_features2d \
        -lopencv_flann -lopencv_gapi -lopencv_highgui -lopencv_imgcodecs \
        -lopencv_imgproc -lopencv_ml -lopencv_objdetect -lopencv_photo \
        -lopencv_stitching -lopencv_video -lopencv_videoio \
        -lueye_api
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
