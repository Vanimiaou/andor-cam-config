//---------------------------------------------------------------------------
// Author: Hoang Van Do
// Description: Adapt the SDK examples with modifications on the acquisition mode, EM gain, external trigger etc
//---------------------------------------------------------------------------
#include "rearrangement_dev.h"

#define _OPEN_SYS_ITOA_EXT
#include <stdio.h>
#include <stdlib.h>
#include "Camera.h"
#include <thread>
#include <chrono>
//---------------------------------------------------------------------------

unsigned int i_x = 21, i_y = 27, atompixel_x = 231, atompixel_y = 299;
unsigned int HStartPixel = atompixel_x - (i_x - 1) / 2, HEndPixel = atompixel_x + (i_x - 1) / 2, VStartPixel = atompixel_y - (i_y - 1) / 2, VEndPixel = atompixel_y + (i_y - 1) / 2;

bool b_gblerrorFlag = false;
float f_saturationLevel;
long l_handle;

std::chrono::steady_clock::time_point end_WaitForAcquisition2;
std::chrono::steady_clock::time_point end_GetAcquisition2;
long long duration_test;


//Returns True if function call was successful
bool CheckError(unsigned int _ui_err, const char* _cp_func)
{
    bool b_ret;

    if (_ui_err == DRV_SUCCESS) {
        b_ret = true;
    }
    else {
        printf("ERROR - %s -- %i\n\n",_cp_func,_ui_err);
        b_gblerrorFlag = true;
        b_ret = false;
    }
    return b_ret;
}

//Get the Handle for each Camera
void GetHandles()
{
    unsigned int ui_error;

    ui_error = GetCameraHandle(0, &l_handle);
    CheckError(ui_error, "GetCameraHandle");
}

//Initialize each camera
void InitializeCameras()
{
    unsigned int ui_error;
    ui_error = SetCurrentCamera(l_handle);
    CheckError(ui_error, "SetCurrentCamera");

    ui_error = Initialize("");
    CheckError(ui_error, "Initialize");

    ui_error = CoolerON();
    CheckError(ui_error, "CoolerON");

}

//Set Acquisition Paramaters
void SetUpCamera()
{
    unsigned int ui_error;

    ui_error = SetTemperature(-60);
    CheckError(ui_error, "SetTemperature");

    ui_error = SetCoolerMode(1);
    CheckError(ui_error, "SetCoolerMode");

    ui_error = SetCurrentCamera(l_handle);
    CheckError(ui_error, "SetCurrentCamera");

    ui_error = SetReadMode(4);
    CheckError(ui_error, "SetReadMode");

    ui_error = SetImage(1, 1, HStartPixel, HEndPixel, VStartPixel, VEndPixel); 
    CheckError(ui_error, "SetImage");

    ui_error = SetAcquisitionMode(1);
    CheckError(ui_error, "SetAcquisitionMode");

    ui_error = SetNumberKinetics(1);
    CheckError(ui_error, "SetNumberKinetics");

    ui_error = SetBaselineClamp(1);
    CheckError(ui_error, "SetBaselineClamp");

    ui_error = SetVSSpeed(4);
    CheckError(ui_error, "SetVSSpeed");

    ui_error = SetVSAmplitude(0);
    CheckError(ui_error, "SetVSAmplitude");

    ui_error = SetHSSpeed(0, 2);
    CheckError(ui_error, "SetHSSpeed");

    ui_error = SetPreAmpGain(0);
    CheckError(ui_error, "SetPreAmpGain");

    ui_error = SetEMCCDGain(50);
    CheckError(ui_error, "SetEMCCDGain");

    ui_error = SetCountConvertMode(1);
    CheckError(ui_error, "SetCountConvertMode");

    //float f_keepcleantime;
    //ui_error = GetKeepCleanTime(&f_keepcleantime);
    //printf("%f clean time\n", f_keepcleantime);

    //float f_readouttime;
    //ui_error = GetReadOutTime(&f_readouttime);
    //printf("%f read out time\n", f_readouttime);
    //Use Andor Capibilities to test if camera has a shutter
    AndorCapabilities* caps = new AndorCapabilities();
    caps->ulSize = sizeof(AndorCapabilities);

    ui_error = GetCapabilities(caps);
    CheckError(ui_error, "GetCapabilities");

    if (caps->ulFeatures & AC_FEATURES_SHUTTER) {

        //If a shutter is present set is to always open
        ui_error = SetShutter(1, 1, 10, 10);
        CheckError(ui_error, "SetShutter");
    }

    if (caps->ulTriggerModes & AC_TRIGGERMODE_EXTERNALEXPOSURE) {
        //trying to put external exposure
        ui_error = SetTriggerMode(7);
        CheckError(ui_error, "SetTriggerMode");
    }


}

// Acquire Image 1
std::vector<bool> AcquireImage()
{
    unsigned int ui_error;
    unsigned long ul_size;
    unsigned int num_atoms = 0;
    std::vector<bool> aBoolVector;

    ui_error = SetCurrentCamera(l_handle);
    CheckError(ui_error, "SetCurrentCamera");

    ul_size = i_x * i_y;
    long* lp_data = new long[ul_size];
    
    int i_status;
    GetStatus(&i_status);

    ui_error = StartAcquisition();
    CheckError(ui_error, "StartAcquisition");

    ui_error = WaitForAcquisition();
    CheckError(ui_error, "WaitForAcquisition");

    ui_error = GetAcquiredData(lp_data, ul_size);
    CheckError(ui_error, "GetAcquiredData");

    //bool trap_1 = (bool)(lp_data[94- 63] / countthreshold);
    //aBoolVector.push_back(trap_1);
    //bool trap0 = (bool)(lp_data[94] / countthreshold);
    //aBoolVector.push_back(trap0);
    //bool trap1 = (bool)(lp_data[157] / countthreshold);
    //aBoolVector.push_back(trap1);


    //thsi is the 6traps from -1 to 4
    //bool trap2 = (bool)(lp_data[220] / countthreshold);
    //aBoolVector.push_back(trap2);
    bool trap3 = (bool)(lp_data[283] / countthreshold);
    aBoolVector.push_back(trap3);
    //bool trap4 = (bool)(lp_data[346] / countthreshold);
    //aBoolVector.push_back(trap4);
    //bool trap5 = (bool)(lp_data[346+63] / countthreshold);
    //aBoolVector.push_back(trap5);
    //bool trap6 = (bool)(lp_data[346 + 2*63] / countthreshold);
    //aBoolVector.push_back(trap5);
    //bool trap7 = (bool)(lp_data[346 + 3*63] / countthreshold);
    //aBoolVector.push_back(trap5);

    /* ------ Save Image 1 ----------------------------------------------------------- */
    char buffer[sizeof(image_number) * 4 + 80];
    snprintf(buffer, sizeof(char) * image_number + 80, "data/%lu.fit", 2*image_number);
    ui_error = SaveAsFITS(buffer, 3);
    if (CheckError(ui_error, "SaveAsFITS")) {
        //std::cout << "\nImage " << 2 * image_number << " saved" << std::endl;
    }

    /* ------ Check if we have enough atoms ------------------------------------------ */
    std::for_each(aBoolVector.begin(), aBoolVector.end(), [&](bool n) { num_atoms += n; });     // sum the values of aCameraData
    if (num_atoms >= dwMinNumAtoms) {
        aBoolVector.push_back(1);       // if enough atoms, append 1 to end of aBoolVector
    }
    else {
        aBoolVector.push_back(0);       // if not enough atoms, append 0 to end of aBoolVector
    }

    return aBoolVector;

    //if (lp_data) delete[] lp_data;

}

//Take Acquisition
bool AcquireImage2(ST_SPCM_CARDINFO stCard)
{

    unsigned int ui_error;
    unsigned long ul_size;
    //unsigned int num_atoms;

    //ui_error = SetCurrentCamera(l_handle);
    //CheckError(ui_error, "SetCurrentCamera");

    ul_size = i_x * i_y;
    long* lp_data = new long[ul_size];

    int i_status;
    GetStatus(&i_status);
    //printf("%i status of the camera before AcqImg2\n", i_status);
    
    if (i_status == DRV_IDLE) {
        ui_error = StartAcquisition();
        CheckError(ui_error, "StartAcquisition");
        X0_TTL_pulse(stCard);
        /*startingAcqusition2 = std::chrono::high_resolution_clock::now();
        auto duration10 = std::chrono::duration_cast<std::chrono::milliseconds>(startingAcqusition2 - timestamp4);
        std::cout << "Time between getting bool and starting Acquisition2 = " << duration10.count() << " ms" << std::endl;*/

        //std::cout << "\nAcquisition started" << std::endl;
        unsigned int ui_errorwait;
        ui_errorwait = WaitForAcquisitionTimeOut(dwWaitAcq2_ms);

        if (CheckError(ui_errorwait, "WaitForAcquisitionTimeOut")) {
            ui_error = GetAcquiredData(lp_data, ul_size);
            CheckError(ui_error, "GetAcquiredData");

            char buffer[sizeof(image_number) * 4 + 80];
            snprintf(buffer, sizeof(char) * image_number + 80, "data/%lu.fit", 2 * image_number + 1);
            //snprintf(buffer, sizeof(char) * image_number + 80, "data/%lu.fit", image_number);
            ui_error = SaveAsFITS(buffer, 3);
            if (CheckError(ui_error, "SaveAsFITS")) {
                //std::cout << "Image " << 2*image_number+1 << " saved" << std::endl;
                //std::cout << "Image " << image_number << " saved" << std::endl;
                std::cout << 2 * image_number + 1 << std::endl;
                image_number++;
            }
        }
        else {
            ui_error = AbortAcquisition();
            std::cout << "\nAborting acquisition\n" << std::endl;
            CheckError(ui_error, "AbortAcquisition");
        }
    }
    else {
        printf("\nNot idle");
    }

    return true;

    if (lp_data) delete[] lp_data;

}

//ShutDown Cameras
void ShutDownCameras(long l_numCams, long* lp_cameras)
{
    unsigned int ui_error;
    for (int i = 0; i < l_numCams; i++) {
        ui_error = SetCurrentCamera(lp_cameras[i]);
        CheckError(ui_error, "SetCurrentCamera");

        ShutDown();
        CheckError(ui_error, "ShutDown");
    }
}

//void ABORT() {
//    unsigned int ui_error;
//    ui_error = AbortAcquisition();
//    CheckError(ui_error, "AbortAcquisition");
//}

void StartCamera() {
    unsigned int ui_error;
    long i_numCams;

    ui_error = GetAvailableCameras(&i_numCams);
    CheckError(ui_error, "GetAvailableCameras");
    if (i_numCams == 0) {
        std::cout << "No camera connected" << std::endl;
    }
    else {
        GetHandles();
        InitializeCameras();
        SetUpCamera();
        std::cout << "Camera initialized" << std::endl;
    }
}


//---------------------------------------------------------------------------