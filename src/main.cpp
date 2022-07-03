#include <QCoreApplication>
#include <QObject>
#include <QDebug>
#include <opencv2/opencv.hpp>
#include <ueye.h>
#include <iostream>

using namespace std;
using namespace cv;

bool client_flag = false;
const unsigned int port = 8080;

const int NUMBER_BYTE = 3;
double enable = 1;
double disable = 0;
CAMINFO camera_info;
HIDS camera_id;
SENSORINFO sensor_info;
INT color_mode = IS_CM_BGR8_PACKED;
INT bits_per_pixel = 24;
INT memID[NUMBER_BYTE];
INT width;
INT height;
CHAR* pMem[NUMBER_BYTE];
VOID* mem_data = NULL;
INT mat_mode = CV_8UC3;
UINT nGamma = 160;
UINT nDeviceId = 1;
UINT nRange[3];
IS_DEVICE_INFO deviceInfo;
unsigned int zoom = 1;
const int unsigned zoom_number = 5;
bool send_flag = false;
double frame_rate;
double min_frame_rate;
double max_frame_rate;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // At least one camera must be available ///////////////////////////////////////////////////////
    INT nNumCam;
    if (is_GetNumberOfCameras( &nNumCam ) == IS_SUCCESS) {
        if (nNumCam >= 1) {
            // Create new list with suitable size
            UEYE_CAMERA_LIST* pucl;
            pucl = (UEYE_CAMERA_LIST*) new BYTE [sizeof (DWORD) + nNumCam * sizeof (UEYE_CAMERA_INFO)];
            pucl->dwCount = nNumCam;
            //Retrieve camera info
            if (is_GetCameraList(pucl) == IS_SUCCESS) {
                int iCamera;
                for (iCamera = 0; iCamera < (int)pucl->dwCount; iCamera++) {
                    //Test output of camera info on the screen
                    qDebug() << "Camera:" << iCamera;
                    camera_id = pucl->uci[iCamera].dwCameraID;
                    qDebug() << "\tID:" << camera_id;
                    qDebug() << "\tDevice ID:" << pucl->uci[iCamera].dwDeviceID;
                    qDebug() << "\tSensor ID:" << pucl->uci[iCamera].dwSensorID;
                    qDebug() << "\tIn use:" << pucl->uci[iCamera].dwInUse;
                    qDebug() << "\tSerial No.:" << QString(pucl->uci[iCamera].SerNo);
                    qDebug() << "\tModel:" << pucl->uci[iCamera].Model;
                    qDebug() << "\tStatus:" << pucl->uci[iCamera].dwStatus;
                    qDebug() << "\tFull Model Name:" << pucl->uci[iCamera].FullModelName;
                }
            }
            delete [] pucl;
        }
        else
        {
            qDebug() << "Error: No Camera is connected!";
            return 0;
        }
    }

    // Initialize the camera ///////////////////////////////////////////////////////////////////////
    if (is_InitCamera(&camera_id, NULL) != IS_SUCCESS)
        qDebug() << "Error: Initialize Camera!";
    else
    {
        qDebug() << "Camera initialized!";

        // Enable Auto Exit ////////////////////////////////////////////////////////////////////////
        if (is_EnableAutoExit(camera_id, IS_ENABLE_AUTO_EXIT) != IS_SUCCESS)
            qDebug() << "Error: Enable Auto Exit!";

        // Get Camera Info /////////////////////////////////////////////////////////////////////////
        if (is_GetCameraInfo(camera_id, &camera_info) != IS_SUCCESS)
            qDebug() << "Error: Get Camera Information!";
        else
        {
            qDebug() << "Get Camera Information!";
            qDebug() << "Serial Number:" << camera_info.SerNo;
            qDebug() << "Camera Manufacturer:" << camera_info.ID;
            qDebug() << "Camera Control Date:" << camera_info.Date;
        }

        // Get Sensor Info /////////////////////////////////////////////////////////////////////////
        if (is_GetSensorInfo(camera_id, &sensor_info) != IS_SUCCESS)
            qDebug() << "Error: Get Sensor Information!";
        else
        {
            qDebug() << "Get Sensor Information!";
            width = sensor_info.nMaxWidth;
            qDebug() << "Width =" << width;
            height = sensor_info.nMaxHeight;
            qDebug() << "Height =" << height;
            qDebug() << "Camera Model:" << sensor_info.strSensorName;
            if (sensor_info.nColorMode == IS_COLORMODE_BAYER)
                qDebug() << "Color Mode is BAYER";
            else if (sensor_info.nColorMode == IS_COLORMODE_MONOCHROME)
                qDebug() << "Color Mode is MONOCHROME";
            else if (sensor_info.nColorMode == IS_COLORMODE_CBYCRY)
                qDebug() << "Color Mode is CBYCRY";
            else if (sensor_info.nColorMode == IS_COLORMODE_JPEG)
                qDebug() << "Color Mode is JPEG";
            else if (sensor_info.nColorMode == IS_COLORMODE_INVALID)
                qDebug() << "Color Mode is INVALID";
            qDebug() << "Pixel Size:" << sensor_info.wPixelSize / 100.0 << "μm";
        }

        // Set Color Mode //////////////////////////////////////////////////////////////////////////
        if (is_SetColorMode(camera_id, color_mode) != IS_SUCCESS)
            qDebug() << "Error: Set Color Mode!";

        // Set Auto Gain ///////////////////////////////////////////////////////////////////////////
        if (is_SetAutoParameter(camera_id, IS_SET_ENABLE_AUTO_GAIN, &enable, 0) != IS_SUCCESS)
            qDebug() << "Error: Set Enable Auto Gain";

        // Set Auto Shutter ////////////////////////////////////////////////////////////////////////
        if (is_SetAutoParameter(camera_id, IS_SET_ENABLE_AUTO_SHUTTER, &enable, 0)  != IS_SUCCESS)
            qDebug() << "Error: Set Enable Auto Shutter";

        // set Auto White Balance //////////////////////////////////////////////////////////////////
        if (is_SetAutoParameter(camera_id, IS_SET_ENABLE_AUTO_WHITEBALANCE, &enable, 0) != IS_SUCCESS)
            qDebug() << "Error: Set Enable Auto White Balance";

        // Set Auto Brightness /////////////////////////////////////////////////////////////////////
        if (is_SetAutoParameter(camera_id, IS_SET_AUTO_BRIGHTNESS_ONCE, &enable, 0) != IS_SUCCESS)
            qDebug() << "Error: Set Auto Brightness Once";

        // Set Gamma ///////////////////////////////////////////////////////////////////////////////
        if (is_Gamma(camera_id, IS_GAMMA_CMD_SET, (void*) &nGamma, sizeof(nGamma)) != IS_SUCCESS)
            qDebug() << "Error: Set Gamma";

        // Set the size and position of an area of interest (AOI) within an image //////////////////
        IS_RECT rectAOI;

        rectAOI.s32X     = 0;
        rectAOI.s32Y     = 0;
        rectAOI.s32Width = width;
        rectAOI.s32Height = height;

        if (is_AOI( camera_id, IS_AOI_IMAGE_SET_AOI, (void*)&rectAOI, sizeof(rectAOI)) != IS_SUCCESS)
            qDebug() << "Error: Set Area Of Interest!";

        // Allocate Image Memory and add to the Buffer /////////////////////////////////////////////
        for (int i = 0; i < NUMBER_BYTE; i++)
        {
            if (is_AllocImageMem(camera_id, width, height, bits_per_pixel, &pMem[i], &memID[i]) != IS_SUCCESS)
                qDebug() << "Error: Allocate Image Memory!";
            if (is_AddToSequence(camera_id, pMem[i], memID[i]) != IS_SUCCESS)
                qDebug() << "Error: Add to Sequence!";
        }

        // Get Pixel Clock  Range //////////////////////////////////////////////////////////////////
        ZeroMemory(nRange, sizeof(nRange));
        if (is_PixelClock(camera_id, IS_PIXELCLOCK_CMD_GET_RANGE, (void*)&nRange, sizeof(nRange)) != IS_SUCCESS)
            qDebug() << "Error: Pixel Clock!";
        qDebug() << "Pixel Clock Range =" << nRange[0] << "MHz -" << nRange[1] << "MHz";

        // Get default pixel clock /////////////////////////////////////////////////////////////////
        UINT default_pixel_clock;
        if (is_PixelClock(camera_id, IS_PIXELCLOCK_CMD_GET_DEFAULT, (void*)&default_pixel_clock,
                          sizeof(default_pixel_clock)) != IS_SUCCESS)
            qDebug() << "Error: Camera failed to get default Pixel Clock!";
        qDebug() << "Default Pixel Clock =" << default_pixel_clock << "MHz";

        // Get Frame Rate Range ////////////////////////////////////////////////////////////////////
        double min_frame_duration;
        double max_frame_duration;
        double interval;
        if (is_GetFrameTimeRange(camera_id, &min_frame_duration, &max_frame_duration, &interval) != IS_SUCCESS)
            qDebug() << "Error: Camera failed to get frame rate range!";
        min_frame_rate = 1.0 / max_frame_duration;
        max_frame_rate = 1.0 / min_frame_duration;
        qDebug() << "Frame Rate Range =" << min_frame_rate << "FPS -" << max_frame_rate << "FPS";

        // Get default frame rate //////////////////////////////////////////////////////////////////
        if (is_SetFrameRate(camera_id, IS_GET_DEFAULT_FRAMERATE, &frame_rate) != IS_SUCCESS)
            qDebug() << "Error: Camera failed to set Frame Rate!";
        else
            qDebug() << "Default Frame Rate =" << frame_rate;

        // Set Diplay Mode /////////////////////////////////////////////////////////////////////////
        if (is_SetDisplayMode (camera_id, IS_SET_DM_DIB) != IS_SUCCESS)
            qDebug() << "Error: Set Display Mode!";

        // Set Capture Video ///////////////////////////////////////////////////////////////////////
        if (is_CaptureVideo(camera_id, IS_DONT_WAIT) != IS_SUCCESS)
            qDebug() << "Error: Campure Video!";
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////
    qDebug() << "***   Start grabbing the frames   ***";
    qDebug() << "Press 'x' to terminate";
    qDebug() << "Press 'a' to start/stop frame capturing";
    qDebug() << "Press 'p' to adjust pixel clock";
    qDebug() << "Press 'f' to adjust frame rate";

    Mat frame;
    ////////////////////////////////////////////////////////////////////////////////////////////////
    while(true)
    {
        is_GetImageMem(camera_id, &mem_data);
        Mat frame(height, width, mat_mode, mem_data);

        int width_frame = width/1;
        int height_frame = height/1;

        resize(frame, frame, Size(width_frame, height_frame), 0, 0);

        Rect crop(((width_frame/2)/zoom_number)*(zoom-1), ((height_frame/2)/zoom_number)*(zoom-1),
                  width_frame/((double)zoom_number/((double)zoom_number-zoom+1)),
                  height_frame/((double)zoom_number/((double)zoom_number-zoom+1)));

        frame = frame(crop);
        resize(frame, frame, Size(width_frame, height_frame));

        char key = waitKey(5);
        if (key >= 0)
        {
            ////////////////////////////////////////////////////////////////////////////////////////
            if (key == 'a')
            {
                if (is_CaptureVideo(camera_id, IS_GET_LIVE))
                {
                    if (is_StopLiveVideo(camera_id, IS_FORCE_VIDEO_STOP) != IS_SUCCESS)
                        qDebug() << "ERROR: Camera failed to stop live video!";
                }
                else
                {
                    if (is_CaptureVideo(camera_id, IS_DONT_WAIT) != IS_SUCCESS)
                        qDebug() << "Error: Campure Video!";
                }
            }

            ////////////////////////////////////////////////////////////////////////////////////////
            if (key == 'p')
            {
                UINT _pixel_clock;
                cout << "Enter Pixel Clock Value between " << nRange[0] << "-" << nRange[1] << " MHz: ";
                cin >> _pixel_clock;
                if (_pixel_clock >= nRange[0] && _pixel_clock <= nRange[1])
                {
                    if (is_PixelClock(camera_id, IS_PIXELCLOCK_CMD_SET, (void*)&_pixel_clock,
                                      sizeof(_pixel_clock)) != IS_SUCCESS)
                        qDebug() << "Error: Pixel Clock!";

                }
                else qDebug() << "Error: The Value is out of range!";
            }
            ////////////////////////////////////////////////////////////////////////////////////////
            if (key == 'f')
            {
                cout << "Enter frame rate between " << min_frame_rate << "-" << max_frame_rate << " FPS: ";
                cin >> frame_rate;
                if (is_SetFrameRate(camera_id, frame_rate, &frame_rate) != IS_SUCCESS)
                    qDebug() << "Error: Set Frame Rate";
                qDebug() << "Frame Rate =" << frame_rate << "FPS";
            }
            ////////////////////////////////////////////////////////////////////////////////////////
            if (key == 'x')
            {
                destroyAllWindows();
                if (is_StopLiveVideo(camera_id, IS_FORCE_VIDEO_STOP) != IS_SUCCESS)
                {
                    qDebug() << "ERROR: Camera failed to stop live video!";
                }
                break;
            }
        }

        namedWindow("IDS Camera", WINDOW_NORMAL);
        resizeWindow("IDS Camera", 1500, 1008);
        imshow("IDS Camera, Server", frame);
    }

    return a.exec();
}
