//=============================================================================
// Copyright (c) 2001-2019 FLIR Systems, Inc. All Rights Reserved.
//
// This software is the confidential and proprietary information of FLIR
// Integrated Imaging Solutions, Inc. ("Confidential Information"). You
// shall not disclose such Confidential Information and shall use it only in
// accordance with the terms of the license agreement you entered into
// with FLIR Integrated Imaging Solutions, Inc. (FLIR).
//
// FLIR MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY OF THE
// SOFTWARE, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE, OR NON-INFRINGEMENT. FLIR SHALL NOT BE LIABLE FOR ANY DAMAGES
// SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR DISTRIBUTING
// THIS SOFTWARE OR ITS DERIVATIVES.
//=============================================================================

/**
 *  @example LookupTable_C.c
 *
 *  @brief LookupTable_C.c shows how to configure lookup tables on the
 *  camera. It relies on information provided in the Enumeration_C,
 *  Acquisition_C, and NodeMapInfo_C examples.
 *
 *  It can also be helpful to familiarize yourself with the
 *  ImageFormatControl_C and Exposure_C examples as these provide a strong
 *  introduction to camera customization.
 *
 *  Lookup tables allow for the customization and control of individual pixels.
 *  This can be a very powerful and deeply useful tool; however, because use
 *  cases are context dependent, this example only explores lookup table
 *  configuration.
 *  
 *  *** NOTE ***
 *  When using Visual Studio 2010, our solution will use the /TP flag to
 *  compile this example as C++ code instead of C code. This is because our C
 *  examples adhere to post-C89 standard which is not supported in Visual
 *  Studio 2010. You can still use our 2010 libraries to write your own C
 *  application as long as it follows the Visual Studio 2010 C compiler
 *  standard.
 *  
 */

#include "SpinnakerC.h"
#include "stdio.h"
#include "string.h"

// This macro helps with C-strings.
#define MAX_BUFF_LEN 256

// This function helps to check if a node is available
bool8_t IsAvailable(spinNodeHandle hNode)
{
    bool8_t pbAvailable = False;
    spinError err = SPINNAKER_ERR_ERROR;
    err = spinNodeIsAvailable(hNode, &pbAvailable);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve node availability, with error %d...\n\n", err);
    }
    return pbAvailable;
}

// This function helps to check if a node is readable
bool8_t IsReadable(spinNodeHandle hNode)
{
    bool8_t pbReadable = False;
    spinError err = SPINNAKER_ERR_ERROR;
    err = spinNodeIsReadable(hNode, &pbReadable);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve node readability, with error %d...\n\n", err);
    }
    return pbReadable;
}

// This function helps to check if a node is writable
bool8_t IsWritable(spinNodeHandle hNode)
{
    bool8_t pbWritable = False;
    spinError err = SPINNAKER_ERR_ERROR;
    err = spinNodeIsWritable(hNode, &pbWritable);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve node writability, with error %d...\n\n", err);
    }
    return pbWritable;
}

// This function handles the error prints when a node or entry is unavailable or
// not readable/writable on the connected camera
void PrintRetrieveNodeFailure(char node[], char name[])
{
    printf("Unable to get %s (%s %s retrieval failed).\n\n", node, name, node);
    printf("The %s may not be available on all camera models...\n", node);
    printf("Please try a Blackfly S camera.\n\n");
}

// This function configures lookup tables linearly. This involves selecting
// the type of lookup table, finding the appropriate increment calculated from
// the maximum value, and enabling lookup tables on the camera.
int ConfigureLookupTables(spinNodeMapHandle hNodeMap)
{
    spinError err = SPINNAKER_ERR_SUCCESS;

    printf("\n\n*** LOOKUP TABLE CONFIGURATION ***\n\n");

    //
    // Select lookup table type
    //
    // *** NOTES ***
    // Setting the lookup table selector. It is important to note that this
    // does not enable lookup tables.
    //
    spinNodeHandle hLUTSelector = NULL;
    spinNodeHandle hLUTSelectorLUT1 = NULL;
    int64_t lutSelectorLUT1 = 0;
    err = spinNodeMapGetNode(hNodeMap, "LUTSelector", &hLUTSelector);
    if (!IsAvailable(hLUTSelector) || !IsWritable(hLUTSelector))
    {
        PrintRetrieveNodeFailure("node", "LUTSelector");
        return -1;
    }

    err = spinEnumerationGetEntryByName(hLUTSelector, "LUT1", &hLUTSelectorLUT1);
    if (!IsAvailable(hLUTSelectorLUT1) || !IsReadable(hLUTSelectorLUT1))
    {
        PrintRetrieveNodeFailure("entry", "LUTSelector LUT1");
        return -1;
    }

    err = spinEnumerationEntryGetIntValue(hLUTSelectorLUT1, &lutSelectorLUT1);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to set lookup table type (enum entry int value retrieval). Aborting with error %d...\n\n", err);
        return -1;
    }

    err = spinEnumerationSetIntValue(hLUTSelector, lutSelectorLUT1);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to set lookup table type (enum entry setting). Aborting with error %d...\n\n", err);
        return -1;
    }

    printf("Lookup table type set to LUT1...\n");

    //
    // Determine pixel increment and set indexes and values as desired
    //
    // *** NOTES ***
    // To get the pixel increment, the maximum range of the value node must
    // first be retrieved. The value node represents an index, so its value
    // should be one less than a power of 2 (e.g. 511, 1023, etc.). Add 1 to
    // this index to get the maximum range. Divide the maximum range by 512
    // to calculate the pixel increment.
    //
    // Finally, all values (in the value node) and their corresponding
    // indexes (in the index node) need to be set. The goal of this example
    // is to set the lookup table linearly. As such, the slope of the values
    // should be set according to the increment, but the slope of the
    // indexes is inconsequential.
    //
    spinNodeHandle hLUTValue = NULL;
    spinNodeHandle hLUTIndex = NULL;
    int64_t maximumRange = 0;
    int64_t increment = 0;
    int64_t i = 0;

    // Retrieve value node
    err = spinNodeMapGetNode(hNodeMap, "LUTValue", &hLUTValue);
    if (!IsAvailable(hLUTValue) || !IsReadable(hLUTValue))
    {
        PrintRetrieveNodeFailure("node", "LUTValue");
        return -1;
    }

    // Retrieve maximum index
    err = spinIntegerGetMax(hLUTValue, &maximumRange);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to configure lookup table. Aborting with error %d...\n\n", err);
        return -1;
    }

    // Convert maximum index to maximum range
    maximumRange++;
    printf("\tMaximum range: %d\n", (int)maximumRange);

    // Calculate increment
    increment = maximumRange / 512;
    printf("\tIncrement: %d\n", (int)increment);

    // Retrieve index node
    err = spinNodeMapGetNode(hNodeMap, "LUTIndex", &hLUTIndex);
    if (!IsAvailable(hLUTIndex) || !IsReadable(hLUTIndex))
    {
        PrintRetrieveNodeFailure("node", "LUTIndex");
        return -1;
    }

    // Set values and indexes
    for (i = 0; i < maximumRange; i += increment)
    {
        err = spinIntegerSetValue(hLUTIndex, i);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to configure lookup table. Aborting with error %d...\n\n", err);
            return -1;
        }

        err = spinIntegerSetValue(hLUTValue, i);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to configure lookup table. Aborting with error %d...\n\n", err);
            return -1;
        }
    }

    printf("All lookup table values set...\n");

    //
    // Enable lookup tables
    //
    // *** NOTES ***
    // Once lookup tables have been configured, don't forget to enable them
    // with the appropriate node.
    //
    // *** LATER ***
    // Once the images with lookup tables have been collected, turn the
    // feature off with the same node.
    //
    spinNodeHandle hLUTEnable = NULL;

    err = spinNodeMapGetNode(hNodeMap, "LUTEnable", &hLUTEnable);
    if (!IsAvailable(hLUTEnable) || !IsReadable(hLUTEnable))
    {
        PrintRetrieveNodeFailure("node", "LUTEnable");
        return -1;
    }

    err = spinBooleanSetValue(hLUTEnable, True);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to enable lookup table. Aborting with error %d...\n\n", err);
        return -1;
    }

    printf("Lookup table enabled...\n\n");

    return 0;
}

// This function resets the camera by disabling lookup tables.
int ResetLookupTables(spinNodeMapHandle hNodeMap)
{
    //
    // Disable lookup tables
    //
    // *** NOTES ***
    // It is recommended to keep lookup tables off when they are not
    // explicitly required.
    //
    spinNodeHandle hLUTEnable = NULL;

    spinNodeMapGetNode(hNodeMap, "LUTEnable", &hLUTEnable);
    if (!IsAvailable(hLUTEnable) || !IsReadable(hLUTEnable))
    {
        PrintRetrieveNodeFailure("node", "LUTEnable");
        return -1;
    }

    spinBooleanSetValue(hLUTEnable, False);

    printf("Lookup tables disabled...\n\n");

    return 0;
}

// This function prints the device information of the camera from the transport
// layer; please see NodeMapInfo_C example for more in-depth comments on
// printing device information from the nodemap.
int PrintDeviceInfo(spinNodeMapHandle hNodeMap)
{
    spinError err = SPINNAKER_ERR_SUCCESS;
    unsigned int i = 0;

    printf("\n*** DEVICE INFORMATION ***\n\n");

    // Retrieve device information category node
    spinNodeHandle hDeviceInformation = NULL;

    err = spinNodeMapGetNode(hNodeMap, "DeviceInformation", &hDeviceInformation);
    if (!IsAvailable(hDeviceInformation) || !IsReadable(hDeviceInformation))
    {
        printf("Unable to retrieve node. Non-fatal error %d...\n\n", err);
        return -1;
    }

    // Retrieve number of nodes within device information node
    size_t numFeatures = 0;

    err = spinCategoryGetNumFeatures(hDeviceInformation, &numFeatures);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve number of nodes. Non-fatal error %d...\n\n", err);
        return -1;
    }

    // Iterate through nodes and print information
    for (i = 0; i < numFeatures; i++)
    {
        spinNodeHandle hFeatureNode = NULL;

        err = spinCategoryGetFeatureByIndex(hDeviceInformation, i, &hFeatureNode);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to retrieve node. Non-fatal error %d...\n\n", err);
            continue;
        }

        spinNodeType featureType = UnknownNode;

        // Get feature node name
        char featureName[MAX_BUFF_LEN];
        size_t lenFeatureName = MAX_BUFF_LEN;
        err = spinNodeGetName(hFeatureNode, featureName, &lenFeatureName);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            strcpy(featureName, "Unknown name");
        }

        if (IsAvailable(hFeatureNode) && IsReadable(hFeatureNode))
        {
            err = spinNodeGetType(hFeatureNode, &featureType);
            if (err != SPINNAKER_ERR_SUCCESS)
            {
                printf("Unable to retrieve node type. Non-fatal error %d...\n\n", err);
                continue;
            }
        }
        else
        {
            printf("%s: Node not readable\n", featureName);
            continue;
        }

        char featureValue[MAX_BUFF_LEN];
        size_t lenFeatureValue = MAX_BUFF_LEN;
        err = spinNodeToString(hFeatureNode, featureValue, &lenFeatureValue);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            strcpy(featureValue, "Unknown value");
        }

        printf("%s: %s\n", featureName, featureValue);
    }

    return 0;
}

// This function acquires and saves 10 images from a device; please see
// Acquisition_C example for additional information on the steps in this
// function.
int AcquireImages(spinCamera hCam, spinNodeMapHandle hNodeMap, spinNodeMapHandle hNodeMapTLDevice)
{
    spinError err = SPINNAKER_ERR_SUCCESS;

    printf("\n*** IMAGE ACQUISITION ***\n\n");

    // Set acquisition mode to continuous
    spinNodeHandle hAcquisitionMode = NULL;
    spinNodeHandle hAcquisitionModeContinuous = NULL;
    int64_t acquisitionModeContinuous = 0;

    err = spinNodeMapGetNode(hNodeMap, "AcquisitionMode", &hAcquisitionMode);
    if (!IsAvailable(hAcquisitionMode) || !IsWritable(hAcquisitionMode))
    {
        printf("Unable to set acquisition mode to continuous (node retrieval). Aborting with error %d...\n\n", err);
        return -1;
    }

    err = spinEnumerationGetEntryByName(hAcquisitionMode, "Continuous", &hAcquisitionModeContinuous);
    if (!IsAvailable(hAcquisitionModeContinuous) || !IsReadable(hAcquisitionModeContinuous))
    {
        printf(
            "Unable to set acquisition mode to continuous (entry 'continuous' retrieval). Aborting with error "
            "%d...\n\n",
            err);
        return -1;
    }

    err = spinEnumerationEntryGetIntValue(hAcquisitionModeContinuous, &acquisitionModeContinuous);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf(
            "Unable to set acquisition mode to continuous (entry int value retrieval). Aborting with error %d...\n\n",
            err);
        return -1;
    }

    err = spinEnumerationSetIntValue(hAcquisitionMode, acquisitionModeContinuous);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf(
            "Unable to set acquisition mode to continuous (entry int value setting). Aborting with error %d...\n\n",
            err);
        return -1;
    }

    printf("Acquisition mode set to continuous...\n");

    // Begin acquiring images
    err = spinCameraBeginAcquisition(hCam);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to begin image acquisition. Aborting with error %d...\n\n", err);
        return -1;
    }

    printf("Acquiring images...\n");

    // Retrieve device serial number for filename
    spinNodeHandle hDeviceSerialNumber = NULL;
    char deviceSerialNumber[MAX_BUFF_LEN];
    size_t lenDeviceSerialNumber = MAX_BUFF_LEN;

    err = spinNodeMapGetNode(hNodeMapTLDevice, "DeviceSerialNumber", &hDeviceSerialNumber);
    if (!IsAvailable(hDeviceSerialNumber) || !IsReadable(hDeviceSerialNumber))
    {
        strcpy(deviceSerialNumber, "");
        lenDeviceSerialNumber = 0;
    }
    else
    {
        err = spinStringGetValue(hDeviceSerialNumber, deviceSerialNumber, &lenDeviceSerialNumber);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            strcpy(deviceSerialNumber, "");
            lenDeviceSerialNumber = 0;
        }

        printf("Device serial number retrieved as %s...\n", deviceSerialNumber);
    }
    printf("\n");

    // Retrieve, convert, and save images
    const unsigned int k_numImages = 10;
    unsigned int imageCnt = 0;

    for (imageCnt = 0; imageCnt < k_numImages; imageCnt++)
    {
        // Retrieve next received image
        spinImage hResultImage = NULL;

        err = spinCameraGetNextImageEx(hCam, 1000, &hResultImage);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to get next image. Non-fatal error %d...\n\n", err);
            continue;
        }

        // Ensure image completion
        bool8_t isIncomplete = False;
        bool8_t hasFailed = False;

        err = spinImageIsIncomplete(hResultImage, &isIncomplete);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to determine image completion. Non-fatal error %d...\n\n", err);
            hasFailed = True;
        }

        if (isIncomplete)
        {
            spinImageStatus imageStatus = IMAGE_NO_ERROR;

            err = spinImageGetStatus(hResultImage, &imageStatus);
            if (err != SPINNAKER_ERR_SUCCESS)
            {
                printf("Unable to retrieve image status. Non-fatal error %d...\n\n", err);
            }
            else
            {
                printf("Image incomplete with image status %d...\n", imageStatus);
            }

            hasFailed = True;
        }

        // Release incomplete or failed image
        if (hasFailed)
        {
            err = spinImageRelease(hResultImage);
            if (err != SPINNAKER_ERR_SUCCESS)
            {
                printf("Unable to release image. Non-fatal error %d...\n\n", err);
            }

            continue;
        }

        // Print image information
        size_t width = 0;
        size_t height = 0;

        err = spinImageGetWidth(hResultImage, &width);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to retrieve image width. Non-fatal error %d...\n", err);
        }

        err = spinImageGetHeight(hResultImage, &height);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to retrieve image height. Non-fatal error %d...\n", err);
        }

        printf("Grabbed image %u, width = %u, height = %u\n", imageCnt, (unsigned int)width, (unsigned int)height);

        // Convert image to mono 8
        spinImage hConvertedImage = NULL;

        err = spinImageCreateEmpty(&hConvertedImage);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to create image. Non-fatal error %d...\n\n", err);
            hasFailed = True;
        }

        err = spinImageConvert(hResultImage, PixelFormat_Mono8, hConvertedImage);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to convert image. Non-fatal error %d...\n\n", err);
            hasFailed = True;
        }

        // Create unique file name and save image
        char filename[MAX_BUFF_LEN];

        if (lenDeviceSerialNumber == 0)
        {
            sprintf(filename, "LookupTable-C-%d.jpg", imageCnt);
        }
        else
        {
            sprintf(filename, "LookupTable-C-%s-%d.jpg", deviceSerialNumber, imageCnt);
        }

        err = spinImageSave(hConvertedImage, filename, JPEG);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to save image. Non-fatal error %d...\n", err);
        }
        else
        {
            printf("Image saved at %s\n\n", filename);
        }

        // Destroy converted image
        err = spinImageDestroy(hConvertedImage);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to destroy image. Non-fatal error %d...\n\n", err);
        }

        // Release image
        err = spinImageRelease(hResultImage);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to release image. Non-fatal error %d...\n\n", err);
        }
    }

    // End acquisition
    err = spinCameraEndAcquisition(hCam);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to end acquisition. Non-fatal error %d...\n\n", err);
    }

    return 0;
}

// This function acts as the body of the example; please see NodeMapInfo_C
// example for more in-depth comments on setting up cameras.
int RunSingleCamera(spinCamera hCam)
{
    spinError err = SPINNAKER_ERR_SUCCESS;
    int result = 0;

    // Retrieve TL device nodemap and print device information
    spinNodeMapHandle hNodeMapTLDevice = NULL;

    err = spinCameraGetTLDeviceNodeMap(hCam, &hNodeMapTLDevice);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve TL device nodemap. Non-fatal error %d...\n\n", err);
    }
    else
    {
        result = PrintDeviceInfo(hNodeMapTLDevice);
    }

    // Initialize camera
    err = spinCameraInit(hCam);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to initialize camera. Aborting with error %d...\n\n", err);
        return -1;
    }

    // Retrieve GenICam nodemap
    spinNodeMapHandle hNodeMap = NULL;

    err = spinCameraGetNodeMap(hCam, &hNodeMap);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve GenICam nodemap. Aborting with error %d...\n\n", err);
        return -1;
    }

    // Configure lookup tables
    if (ConfigureLookupTables(hNodeMap) != 0)
    {
        return -1;
    }

    // Acquire images
    if (AcquireImages(hCam, hNodeMap, hNodeMapTLDevice) != 0)
    {
        return -1;
    }

    // Reset lookup tables
    if (ResetLookupTables(hNodeMap) != 0)
    {
        return -1;
    }

    // Deinitialize camera
    err = spinCameraDeInit(hCam);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to deinitialize camera. Non-fatal error %d...\n\n", err);
    }

    return result;
}

// Example entry point; please see Enumeration_C example for more in-depth
// comments on preparing and cleaning up the system.
int main(/*int argc, char** argv*/)
{
    int errReturn = 0;
    spinError err = SPINNAKER_ERR_SUCCESS;
    unsigned int i = 0;

    // Since this application saves images in the current folder
    // we must ensure that we have permission to write to this folder.
    // If we do not have permission, fail right away.
    FILE* tempFile;
    tempFile = fopen("test.txt", "w+");
    if (tempFile == NULL)
    {
        printf("Failed to create file in current folder.  Please check "
               "permissions.\n");
        printf("Press Enter to exit...\n");
        getchar();
        return -1;
    }
    fclose(tempFile);
    remove("test.txt");

    // Print application build information
    printf("Application build date: %s %s \n\n", __DATE__, __TIME__);

    // Retrieve singleton reference to system object
    spinSystem hSystem = NULL;

    err = spinSystemGetInstance(&hSystem);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve system instance. Aborting with error %d...\n\n", err);
        printf("\nDone! Press Enter to exit...\n");
        getchar();
        return -1;
    }

    // Print out current library version
    spinLibraryVersion hLibraryVersion;

    spinSystemGetLibraryVersion(hSystem, &hLibraryVersion);
    printf(
        "Spinnaker library version: %d.%d.%d.%d\n\n",
        hLibraryVersion.major,
        hLibraryVersion.minor,
        hLibraryVersion.type,
        hLibraryVersion.build);

    // Retrieve list of cameras from the system
    spinCameraList hCameraList = NULL;

    err = spinCameraListCreateEmpty(&hCameraList);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to create camera list. Aborting with error %d...\n\n", err);
        printf("\nDone! Press Enter to exit...\n");
        getchar();
        return -1;
    }

    err = spinSystemGetCameras(hSystem, hCameraList);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve camera list. Aborting with error %d...\n\n", err);
        printf("\nDone! Press Enter to exit...\n");
        getchar();
        return -1;
    }

    // Retrieve number of cameras
    size_t numCameras = 0;

    err = spinCameraListGetSize(hCameraList, &numCameras);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve number of cameras. Aborting with error %d...\n\n", err);
        printf("\nDone! Press Enter to exit...\n");
        getchar();
        return -1;
    }

    printf("Number of cameras detected: %u\n\n", (unsigned int)numCameras);

    // Finish if there are no cameras
    if (numCameras == 0)
    {
        // Clear and destroy camera list before releasing system
        err = spinCameraListClear(hCameraList);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to clear camera list. Aborting with error %d...\n\n", err);
            printf("\nDone! Press Enter to exit...\n");
            getchar();
            return -1;
        }

        err = spinCameraListDestroy(hCameraList);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to destroy camera list. Aborting with error %d...\n\n", err);
            printf("\nDone! Press Enter to exit...\n");
            getchar();
            return -1;
        }

        // Release system
        err = spinSystemReleaseInstance(hSystem);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to release system instance. Aborting with error %d...\n\n", err);
            printf("\nDone! Press Enter to exit...\n");
            getchar();
            return -1;
        }

        printf("Not enough cameras!\n");
        printf("Done! Press Enter to exit...\n");
        getchar();
        return -1;
    }

    // Run example on each camera
    for (i = 0; i < numCameras; i++)
    {
        printf("\nRunning example for camera %d...\n", i);

        // Select camera
        spinCamera hCamera = NULL;

        err = spinCameraListGet(hCameraList, i, &hCamera);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to retrieve camera from list. Aborting with error %d...\n\n", err);
            errReturn = -1;
        }
        else
        {
            // Run example
            if (RunSingleCamera(hCamera) != 0)
            {
                errReturn = -1;
            }
        }

        // Release camera
        err = spinCameraRelease(hCamera);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            errReturn = -1;
            printf("Unable to release camera instance. Aborting with error %d...\n\n", err);
            continue;
        }

        printf("Camera %d example complete...\n\n", i);
    }

    // Clear and destroy camera list before releasing system
    err = spinCameraListClear(hCameraList);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to clear camera list. Aborting with error %d...\n\n", err);
        printf("\nDone! Press Enter to exit...\n");
        getchar();
        return -1;
    }

    err = spinCameraListDestroy(hCameraList);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to destroy camera list. Aborting with error %d...\n\n", err);
        printf("\nDone! Press Enter to exit...\n");
        getchar();
        return -1;
    }

    // Release system
    err = spinSystemReleaseInstance(hSystem);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to release system instance. Aborting with error %d...\n\n", err);
        printf("\nDone! Press Enter to exit...\n");
        getchar();
        return -1;
    }

    printf("\nDone! Press Enter to exit...\n");
    getchar();
    return errReturn;
}
