# Qt-uEye
Display the images of the IDS cameras by uEye and OpenCV APIs, the code is developed by Qt and Qt Creator

## Install
Install Qt (4.15.2)
    https://www.qt.io/

Install OpenCV (4.2)
    https://opencv.org/

Install IDS Software Suit (4.94)
    https://www.ids-imaging.us/home.html
    
Compile the project with qmake (Qt and Qt Creator).

## Required Modification
In `.pro` file, include the path of the OpenCV (Line 6)
    `INCLUDEPATH += {OpenCV path}`
