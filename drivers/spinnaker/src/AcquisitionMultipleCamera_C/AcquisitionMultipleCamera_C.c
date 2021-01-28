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
 *  @example AcquisitionMultipleCamera_C.c
 *
 *  @brief AcquisitionMultipleCamera_C.c shows how to capture images from
 *  multiple cameras simultaneously. It relies on information provided in the
 *  Enumeration_C, Acquisition_C, and NodeMapInfo_C examples.
 *
 *  This example reads similarly to the Acquisition_C example, except that
 *  loops are used to allow for pseudo-simultaneous acquisitions.
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

// This function helps to check if a node is available and readable
bool8_t IsAvailableAndReadable(spinNodeHandle hNode, char nodeName[])
{
    bool8_t pbAvailable = False;
    spinError err = SPINNAKER_ERR_SUCCESS;
    err = spinNodeIsAvailable(hNode, &pbAvailable);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve node availability (%s node), with error %d...\n\n", nodeName, err);
    }

    bool8_t pbReadable = False;
    err = spinNodeIsReadable(hNode, &pbReadable);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve node readability (%s node), with error %d...\n\n", nodeName, err);
    }
    return pbReadable && pbAvailable;
}

// This function helps to check if a node is available and writable
bool8_t IsAvailableAndWritable(spinNodeHandle hNode, char nodeName[])
{
    bool8_t pbAvailable = False;
    spinError err = SPINNAKER_ERR_SUCCESS;
    err = spinNodeIsAvailable(hNode, &pbAvailable);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve node availability (%s node), with error %d...\n\n", nodeName, err);
    }

    bool8_t pbWritable = False;
    err = spinNodeIsWritable(hNode, &pbWritable);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve node writability (%s node), with error %d...\n\n", nodeName, err);
    }
    return pbWritable && pbAvailable;
}

// This function handles the error prints when a node or entry is unavailable or
// not readable/writable on the connected camera
void PrintRetrieveNodeFailure(char node[], char name[])
{
    printf("Unable to get %s (%s %s retrieval failed).\n\n", node, name, node);
}

#ifdef _DEBUG
// Disables heartbeat on GEV cameras so debugging does not incur timeout errors
spinError DisableHeartbeatGigE(spinCamera hCam, spinNodeMapHandle hNodeMap)
{
    spinNodeHandle hHeartbeatDisable = NULL;
    bool8_t desiredHeartbeatMode = True;
    spinError err = SPINNAKER_ERR_SUCCESS;
    spinNodeHandle hDeviceType;
    char deviceType[MAX_BUFF_LEN];
    size_t lenDeviceType = MAX_BUFF_LEN;

    // Retrieve TL device nodemap
    spinNodeMapHandle hNodeMapTLDevice = NULL;

    err = spinCameraGetTLDeviceNodeMap(hCam, &hNodeMapTLDevice);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf(
            "Unable to retrieve TL device nodemap, error %d occurred. Require this for debug, so aborting...\n\n", err);
    }

    printf("Checking device type to see if we need to disable the camera's heartbeat...\n\n");

    //
    // Write to boolean node controlling the camera's heartbeat
    //
    // *** NOTES ***
    // This applies only to GEV cameras and only applies when in DEBUG mode.
    // GEV cameras have a heartbeat built in, but when debugging applications the
    // camera may time out due to its heartbeat. Disabling the heartbeat prevents
    // this timeout from occurring, enabling us to continue with any necessary debugging.
    // This procedure does not affect other types of cameras and will prematurely exit
    // if it determines the device in question is not a GEV camera.
    //
    // *** LATER ***
    // Since we only disable the heartbeat on GEV cameras during debug mode, it is better
    // to power cycle the camera after debugging. A power cycle will reset the camera
    // to its default settings.
    //

    err = spinNodeMapGetNode(hNodeMapTLDevice, "DeviceType", &hDeviceType);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Error with reading the device's type, error %d occurred. Aborting...\n\n", err);
        return err;
    }
    else
    {
        err = spinNodeToString(hDeviceType, deviceType, &lenDeviceType);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Error with reading the device's type (value), error %d occurred. Aborting...\n\n", err);
            return err;
        }
        else
        {
            if (strcmp(deviceType, "GigEVision") == 0)
            {
                printf("Working with a GigE camera. Attempting to disable heartbeat before continuing...\n\n");
                err = spinNodeMapGetNode(hNodeMap, "GevGVCPHeartbeatDisable", &hHeartbeatDisable);
                if (err != SPINNAKER_ERR_SUCCESS)
                {
                    printf(
                        "Unable to disable heartbeat on camera, error %d occurred. Continuing with execution as this "
                        "may be non-fatal...\n\n",
                        err);
                    return SPINNAKER_ERR_SUCCESS;
                }

                // Set desired boolean as value of heartbeat disable node
                if (IsAvailableAndWritable(hHeartbeatDisable, "GevGVCPHeartbeatDisable"))
                {
                    err = spinBooleanSetValue(hHeartbeatDisable, desiredHeartbeatMode);
                    if (err != SPINNAKER_ERR_SUCCESS)
                    {
                        printf(
                            "Unable to disable heartbeat on camera (boolean node), error %d occured. Continuing with "
                            "execution as this may be non-fatal...\n\n",
                            err);
                        return err;
                    }
                    else
                    {
                        printf("WARNING: Heartbeat on GigE camera disabled for the rest of Debug Mode.\n");
                        printf("         Power cycle camera when done debugging to re-enable the heartbeat...\n\n");
                    }
                }
                else
                {
                    PrintRetrieveNodeFailure("entry", "GevGVCPHeartbeatDisable");
                }
                return err;
            }
            else
            {
                printf("Camera does not use GigE interface. Resuming normal execution...\n\n");
            }
        }
    }

    return err;
}
#endif

// This function acquires and saves 10 images from each device.
spinError AcquireImages(spinCameraList hCameraList, size_t numCameras)
{
    spinError err = SPINNAKER_ERR_SUCCESS;

    spinCamera hCam = NULL;
    unsigned int i = 0;

    printf("\n*** IMAGE ACQUISITION ***\n\n");

    //
    // Prepare each camera to acquire images
    //
    // *** NOTES ***
    // For simultaneous streaming, each camera is prepared as if it were just
    // one, but in a loop. However, there are a few differences. First, each
    // camera must be selected at the beginning of the loop using its index.
    //
    // For every time a camera is selected, it must be released as well. This
    // is true in all C examples, but made clear in none more than this. Notice
    // the bookend calls to select and release each camera in every loop
    // throughout the example.
    //
    // Please notice that each camera is selected at the beginning of each loop
    // when it is needed and released at the end when it isn't. Every time a
    // camera is retrieved from the system or an interface, it needs to be
    // released; otherwise, an error will be returned when the system is
    // released.
    //
    for (i = 0; i < numCameras; i++)
    {
        // Select camera
        err = spinCameraListGet(hCameraList, i, &hCam);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to select camera. Aborting with error %d...\n\n", err);
            return err;
        }

        // Retrieve GenICam nodemap
        spinNodeMapHandle hNodeMap = NULL;

        err = spinCameraGetNodeMap(hCam, &hNodeMap);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to retrieve GenICam nodemap. Aborting with error %d...\n\n", err);
            return err;
        }

        // Set acquisition mode to continuous
        spinNodeHandle hAcquisitionMode = NULL;
        spinNodeHandle hAcquisitionModeContinuous = NULL;
        int64_t acquisitionModeContinuous = 0;

        err = spinNodeMapGetNode(hNodeMap, "AcquisitionMode", &hAcquisitionMode);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to set acquisition mode to continuous (node retrieval). Aborting with error %d...\n\n", err);
            return err;
        }
        // Check if AcquisitionMode available and readable
        if (IsAvailableAndReadable(hAcquisitionMode, "AcquisitionMode"))
        {
            err = spinEnumerationGetEntryByName(hAcquisitionMode, "Continuous", &hAcquisitionModeContinuous);
            if (err != SPINNAKER_ERR_SUCCESS)
            {
                printf(
                    "Unable to set acquisition mode to continuous (enum entry retrieval). Aborting with error "
                    "%d...\n\n",
                    err);
                return err;
            }
        }
        else
        {
            PrintRetrieveNodeFailure("node", "AcquisitionMode");
            return SPINNAKER_ERR_ACCESS_DENIED;
        }

        // Check if AcquisitionModeContinuous is available and readable
        if (IsAvailableAndReadable(hAcquisitionModeContinuous, "AcquisitionModeContinuous"))
        {
            err = spinEnumerationEntryGetIntValue(hAcquisitionModeContinuous, &acquisitionModeContinuous);
            if (err != SPINNAKER_ERR_SUCCESS)
            {
                printf(
                    "Unable to set acquisition mode to continuous (enum entry int value retrieval). Aborting with "
                    "error %d...\n\n",
                    err);
                return err;
            }
        }
        else
        {
            PrintRetrieveNodeFailure("entry", "AcquisitionMode 'Continuous'");
        }

        // Check if AcquisitionMode is writable, change to continuous
        if (IsAvailableAndWritable(hAcquisitionMode, "AcquisitionMode"))
        {
            err = spinEnumerationSetIntValue(hAcquisitionMode, acquisitionModeContinuous);
            if (err != SPINNAKER_ERR_SUCCESS)
            {
                printf(
                    "Unable to set acquisition mode to continuous (enum entry setting). Aborting with error %d...\n\n",
                    err);
                return err;
            }
        }
        else
        {
            PrintRetrieveNodeFailure("node", "AcquisitionMode");
            return SPINNAKER_ERR_ACCESS_DENIED;
        }
        printf("Camera %d acquisition mode set to continuous...\n", i);

#ifdef _DEBUG
        printf("\n\n*** DEBUG ***\n\n");

        // If using a GEV camera and debugging, should disable heartbeat first to prevent further issues
        err = DisableHeartbeatGigE(hCam, hNodeMap);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            return err;
        }
#endif

        // Begin acquiring images
        err = spinCameraBeginAcquisition(hCam);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to begin image acquisition. Aborting with error %d...\n\n", err);
            return err;
        }

        // Release camera
        err = spinCameraRelease(hCam);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to release camera. Aborting with error %d...\n\n", err);
            return err;
        }

        printf("Camera %d acquiring images...\n\n", i);
    }

    //
    // Retrieve, convert, and save images for each camera
    //
    // *** NOTES ***
    // In order to work with simultaneous camera streams, nested loops are
    // needed. It is important that the loop iterating through the cameras be
    // the inner loop; otherwise, all images will be grabbed from a single
    // camera before grabbing any images from another.
    //
    const unsigned int k_numImages = 10;
    unsigned int imageCnt = 0;

    for (imageCnt = 0; imageCnt < k_numImages; imageCnt++)
    {
        for (i = 0; i < numCameras; i++)
        {
            // Select camera
            err = spinCameraListGet(hCameraList, i, &hCam);
            if (err != SPINNAKER_ERR_SUCCESS)
            {
                printf("Unable to select camera. Aborting with error %d...\n\n", err);
                return err;
            }

            // Retrieve next received image
            spinImage hResultImage = NULL;
            bool8_t isIncomplete = False;
            bool8_t hasFailed = False;

            err = spinCameraGetNextImageEx(hCam, 1000, &hResultImage);
            if (err != SPINNAKER_ERR_SUCCESS)
            {
                printf("Unable to get next image. Non-fatal error %d...\n\n", err);
                // Release camera
                err = spinCameraRelease(hCam);
                if (err != SPINNAKER_ERR_SUCCESS)
                {
                    printf("Unable to release camera. Aborting with error %d...\n\n", err);
                    return err;
                }
                continue;
            }

            // Ensure image completion
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
                    printf("Unable to retrieve image status. Non-fatal error %d...\n\n", imageStatus);
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
                // Release camera
                err = spinCameraRelease(hCam);
                if (err != SPINNAKER_ERR_SUCCESS)
                {
                    printf("Unable to release camera. Aborting with error %d...\n\n", err);
                    return err;
                }
                continue;
            }

            // Print image information
            size_t width = 0;
            size_t height = 0;

            printf("Camera %d grabbed image %d, ", i, imageCnt);

            err = spinImageGetWidth(hResultImage, &width);
            if (err != SPINNAKER_ERR_SUCCESS)
            {
                printf("width = unknown, ");
            }
            else
            {
                printf("width = %u, ", (unsigned int)width);
            }

            err = spinImageGetHeight(hResultImage, &height);
            if (err != SPINNAKER_ERR_SUCCESS)
            {
                printf("height = unknown\n");
            }
            else
            {
                printf("height = %u\n", (unsigned int)height);
            }

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

            // Retrieve TL device nodemap
            spinNodeMapHandle hNodeMapTLDevice = NULL;

            err = spinCameraGetTLDeviceNodeMap(hCam, &hNodeMapTLDevice);
            if (err != SPINNAKER_ERR_SUCCESS)
            {
                printf("Unable to retrieve TL device nodemap. Non-fatal error %d...\n\n", err);
            }

            // Retrieve device serial
            spinNodeHandle hDeviceSerialNumber = NULL;
            char deviceSerialNumber[MAX_BUFF_LEN];
            size_t lenDeviceSerialNumber = MAX_BUFF_LEN;

            err = spinNodeMapGetNode(hNodeMapTLDevice, "DeviceSerialNumber", &hDeviceSerialNumber);
            if (err != SPINNAKER_ERR_SUCCESS)
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

            // Create unique file name
            char filename[MAX_BUFF_LEN];

            if (lenDeviceSerialNumber == 0)
            {
                sprintf(filename, "AcquisitionMultipleCamera-C-%d.jpg", imageCnt);
            }
            else
            {
                sprintf(filename, "AcquisitionMultipleCamera-C-%s-%d.jpg", deviceSerialNumber, imageCnt);
            }

            // Save image
            err = spinImageSave(hConvertedImage, filename, JPEG);
            if (err != SPINNAKER_ERR_SUCCESS)
            {
                printf("Unable to save image. Non-fatal error %d...\n\n", err);
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

            // Release camera
            err = spinCameraRelease(hCam);
            if (err != SPINNAKER_ERR_SUCCESS)
            {
                printf("Unable to release camera. Aborting with error %d...\n\n", err);
                return err;
            }
        }
    }

    //
    // End acquisition for each camera
    //
    // *** NOTES ***
    // Notice that what is usually a one-step process is now three steps
    // because of the additional steps of selecting and releasing the camera.
    // It is worth repeating that each camera must be both selected and
    // released once per loop.
    //
    for (i = 0; i < numCameras; i++)
    {
        // Select camera
        err = spinCameraListGet(hCameraList, i, &hCam);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to select camera. Non-fatal error %d...\n\n", err);
            break;
        }

        err = spinCameraEndAcquisition(hCam);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to end acquisition. Non-fatal error %d...\n\n", err);
        }

        // Release camera
        err = spinCameraRelease(hCam);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to release camera. Aborting with error %d...\n\n", err);
            return err;
        }
    }

    return err;
}

// This function prints the device information of the camera from the transport
// layer; please see NodeMapInfo_C example for more in-depth comments on
// printing device information from the nodemap.
spinError PrintDeviceInfo(spinNodeMapHandle hNodeMap, unsigned int camNum)
{
    spinError err = SPINNAKER_ERR_SUCCESS;
    unsigned int i = 0;

    printf("\n*** DEVICE INFORMATION ***\n\n");

    // Retrieve device information category node
    spinNodeHandle hDeviceInformation = NULL;

    err = spinNodeMapGetNode(hNodeMap, "DeviceInformation", &hDeviceInformation);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve node. Non-fatal error %d...\n\n", err);
        return err;
    }

    // Retrieve number of nodes within device information node
    size_t numFeatures = 0;

    if (IsAvailableAndReadable(hDeviceInformation, "DeviceInformation"))
    {
        err = spinCategoryGetNumFeatures(hDeviceInformation, &numFeatures);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to retrieve number of nodes. Non-fatal error %d...\n\n", err);
            return err;
        }
    }
    else
    {
        PrintRetrieveNodeFailure("node", "DeviceInformation");
        return SPINNAKER_ERR_ACCESS_DENIED;
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

        // get feature node name
        char featureName[MAX_BUFF_LEN];
        size_t lenFeatureName = MAX_BUFF_LEN;
        err = spinNodeGetName(hFeatureNode, featureName, &lenFeatureName);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            strcpy(featureName, "Unknown name");
        }

        if (IsAvailableAndReadable(hFeatureNode, featureName))
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
    printf("\n");

    return err;
}

// This function acts as the body of the example; please see NodeMapInfo_C
// example for more in-depth comments on setting up cameras.
spinError RunMultipleCameras(spinCameraList hCameraList, size_t numCameras)
{
    spinError err = SPINNAKER_ERR_SUCCESS;
    spinCamera hCam = NULL;
    unsigned int i = 0;

    // Retrieve TL device nodemap, print device information, and initialize each camera
    printf("\n*** DEVICE INFORMATION ***\n\n");

    //
    // Retrieve TL device nodemaps and print device information for each camera
    //
    // *** NOTES ***
    // This example retrieves information from the nodemap twice: once to print
    // device information and once to grab the device serial number. Rather
    // than caching the nodemap twice, each nodemap is retrieved both
    // times as needed.
    //
    for (i = 0; i < numCameras; i++)
    {
        // Select camera
        err = spinCameraListGet(hCameraList, i, &hCam);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to select camera. Non-fatal error %d...\n\n", err);
            break;
        }

        // Retrieve TL device nodemap and print device information
        spinNodeMapHandle hNodeMapTLDevice = NULL;

        err = spinCameraGetTLDeviceNodeMap(hCam, &hNodeMapTLDevice);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to retrieve TL device nodemap. Non-fatal error %d...\n\n", err);
        }
        else
        {
            err = PrintDeviceInfo(hNodeMapTLDevice, i);
        }

        // Release camera
        err = spinCameraRelease(hCam);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to release camera. Aborting with error %d...\n\n", err);
            return err;
        }
    }

    //
    // Initialize each camera
    //
    // *** NOTES ***
    // You may notice that the steps in this function have more loops with less
    // steps per loop; this contrasts the AcquireImages() function which has
    // less loops but more steps per loop. This is done for demonstrative
    // purposes as both work equally well.
    //
    // *** LATER ***
    // Each camera needs to be deinitialized once all images have been acquired.
    //
    for (i = 0; i < numCameras; i++)
    {
        // Select camera
        err = spinCameraListGet(hCameraList, i, &hCam);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to select camera. Non-fatal error %d...\n\n", err);
            break;
        }

        // Initialize camera
        err = spinCameraInit(hCam);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to initialize camera. Aborting with error %d...\n\n", err);
            return err;
        }

        // Release camera
        err = spinCameraRelease(hCam);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to release camera. Aborting with error %d...\n\n", err);
            return err;
        }
    }

    // Acquire images on all cameras
    err = AcquireImages(hCameraList, numCameras);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        return err;
    }

    //
    // Deinitialize each camera
    //
    // *** NOTES ***
    // Again, each camera must be deinitialized separately by selecting,
    // deinitializing, and releasing the camera.
    //
    for (i = 0; i < numCameras; i++)
    {
        // Select camera
        err = spinCameraListGet(hCameraList, i, &hCam);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to select camera. Non-fatal error %d...\n\n", err);
            break;
        }

        // Deinitialize camera
        err = spinCameraDeInit(hCam);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to deinitialize camera. Aborting with error %d...\n\n", err);
            return err;
        }

        // Release camera
        err = spinCameraRelease(hCam);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to release camera. Aborting with error %d...\n\n", err);
            return err;
        }
    }

    return err;
}

// Example entry point; please see Enumeration_C example for more in-depth
// comments on preparing and cleaning up the system.
int main(/*int argc, char** argv*/)
{
    spinError errReturn = SPINNAKER_ERR_SUCCESS;
    spinError err = SPINNAKER_ERR_SUCCESS;

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

    // Retrieve singleton reference to system
    spinSystem hSystem = NULL;

    err = spinSystemGetInstance(&hSystem);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve system instance. Aborting with error %d...\n\n", err);
        return err;
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
        return err;
    }

    err = spinSystemGetCameras(hSystem, hCameraList);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve camera list. Aborting with error %d...\n\n", err);
        return err;
    }

    // Retrieve number of cameras
    size_t numCameras = 0;

    err = spinCameraListGetSize(hCameraList, &numCameras);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve number of cameras. Aborting with error %d...\n\n", err);
        return err;
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
            return err;
        }

        err = spinCameraListDestroy(hCameraList);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to destroy camera list. Aborting with error %d...\n\n", err);
            return err;
        }

        // Release system
        err = spinSystemReleaseInstance(hSystem);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to release system instance. Aborting with error %d...\n\n", err);
            return err;
        }

        printf("Not enough cameras!\n");
        printf("Done! Press Enter to exit...\n");
        getchar();

        return -1;
    }

    // Run example on all cameras
    printf("\nRunning example...\n");

    err = RunMultipleCameras(hCameraList, numCameras);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        errReturn = err;
    }

    printf("Example complete...\n\n");

    // Clear and destroy camera list before releasing system
    err = spinCameraListClear(hCameraList);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to clear camera list. Aborting with error %d...\n\n", err);
        return err;
    }

    err = spinCameraListDestroy(hCameraList);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to destroy camera list. Aborting with error %d...\n\n", err);
        return err;
    }

    // Release system
    err = spinSystemReleaseInstance(hSystem);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to release system instance. Aborting with error %d...\n\n", err);
        return err;
    }

    printf("\nDone! Press Enter to exit...\n");
    getchar();

    return errReturn;
}
