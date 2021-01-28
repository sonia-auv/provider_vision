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
 *  @example Acquisition_C.c
 *
 *  @brief Acquisition_C.c shows how to acquire images. It relies on
 *  information provided in the Enumeration_C example. Following this, check
 *  out the NodeMapInfo_C example if you haven't already. It explores
 *  retrieving information from various node types.
 *
 *  This example touches on the preparation and cleanup of a camera just
 *  before and just after the acquisition of images. Image retrieval and
 *  conversion, grabbing image data, and saving images are all covered.
 *
 *  Once comfortable with Acquisition_C and NodeMapInfo_C, we suggest checking
 *  out AcquisitionMultipleCamera_C, NodeMapCallback_C, or SaveToAvi_C.
 *  AcquisitionMultipleCamera_C demonstrates simultaneously acquiring images
 *  from a number of cameras, NodeMapCallback_C acts as a good introduction to
 *  programming with callbacks and events, and SaveToAvi_C exhibits video
 *  creation.
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
spinError DisableHeartbeatGigE(spinCamera hCam, spinNodeMapHandle hNodeMap, spinNodeMapHandle hNodeMapTLDevice)
{
    spinNodeHandle hHeartbeatDisable = NULL;
    bool8_t desiredHeartbeatMode = True;
    spinError err = SPINNAKER_ERR_SUCCESS;
    spinNodeHandle hDeviceType;
    char deviceType[MAX_BUFF_LEN];
    size_t lenDeviceType = MAX_BUFF_LEN;

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

// This function acquires and saves 10 images from a device.
spinError AcquireImages(spinCamera hCam, spinNodeMapHandle hNodeMap, spinNodeMapHandle hNodeMapTLDevice)
{
    spinError err = SPINNAKER_ERR_SUCCESS;

    printf("\n*** IMAGE ACQUISITION ***\n\n");

    //
    // Set acquisition mode to continuous
    //
    // *** NOTES ***
    // Because the example acquires and saves 10 images, setting acquisition
    // mode to continuous lets the example finish. If set to single frame
    // or multiframe (at a lower number of images), the example would just
    // hang. This would happen because the example has been written to acquire
    // 10 images while the camera would have been programmed to retrieve
    // less than that.
    //
    // Setting the value of an enumeration node is slightly more complicated
    // than other node types, and especially so in C. It can roughly be broken
    // down into four steps: first, the enumeration node is retrieved from the
    // nodemap; second, the entry node is retrieved from the enumeration node;
    // third, an integer is retrieved from the entry node; and finally, the
    // integer is set as the new value of the enumeration node.
    //
    // It is important to note that there are two sets of functions that might
    // produce erroneous results if they were to be mixed up. The first two
    // functions, spinEnumerationSetIntValue() and
    // spinEnumerationEntryGetIntValue(), use the integer values stored on each
    // individual cameras. The second two, spinEnumerationSetEnumValue() and
    // spinEnumerationEntryGetEnumValue(), use enum values defined in the
    // Spinnaker library. The int and enum values will most likely be
    // different from another.
    //
    spinNodeHandle hAcquisitionMode = NULL;
    spinNodeHandle hAcquisitionModeContinuous = NULL;
    int64_t acquisitionModeContinuous = 0;

    // Retrieve enumeration node from nodemap
    err = spinNodeMapGetNode(hNodeMap, "AcquisitionMode", &hAcquisitionMode);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to set acquisition mode to continuous (node retrieval). Aborting with error %d...\n\n", err);
        return err;
    }

    // Retrieve entry node from enumeration node
    if (IsAvailableAndReadable(hAcquisitionMode, "AcquisitionMode"))
    {
        err = spinEnumerationGetEntryByName(hAcquisitionMode, "Continuous", &hAcquisitionModeContinuous);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf(
                "Unable to set acquisition mode to continuous (entry 'continuous' retrieval). Aborting with error "
                "%d...\n\n",
                err);
            return err;
        }
    }
    else
    {
        PrintRetrieveNodeFailure("entry", "AcquisitionMode");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }

    // Retrieve integer from entry node

    if (IsAvailableAndReadable(hAcquisitionModeContinuous, "AcquisitionModeContinuous"))
    {
        err = spinEnumerationEntryGetIntValue(hAcquisitionModeContinuous, &acquisitionModeContinuous);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf(
                "Unable to set acquisition mode to continuous (entry int value retrieval). Aborting with error "
                "%d...\n\n",
                err);
            return err;
        }
    }
    else
    {
        PrintRetrieveNodeFailure("entry", "AcquisitionMode 'Continuous'");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }
    // Set integer as new value of enumeration node
    if (IsAvailableAndWritable(hAcquisitionMode, "AcquisitionMode"))
    {
        err = spinEnumerationSetIntValue(hAcquisitionMode, acquisitionModeContinuous);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf(
                "Unable to set acquisition mode to continuous (entry int value setting). Aborting with error %d...\n\n",
                err);
            return err;
        }
    }
    else
    {
        PrintRetrieveNodeFailure("entry", "AcquisitionMode");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }

    printf("Acquisition mode set to continuous...\n");

#ifdef _DEBUG
    printf("\n\n*** DEBUG ***\n\n");

    // If using a GEV camera and debugging, should disable heartbeat first to prevent further issues
    err = DisableHeartbeatGigE(hCam, hNodeMap, hNodeMapTLDevice);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        return err;
    }

    printf("\n\n*** END OF DEBUG***\n\n");
#endif

    //
    // Begin acquiring images
    //
    // *** NOTES ***
    // What happens when the camera begins acquiring images depends on the
    // acquisition mode. Single frame captures only a single image, multi
    // frame catures a set number of images, and continuous captures a
    // continuous stream of images. Because the example calls for the retrieval
    // of 10 images, continuous mode has been set.
    //
    // *** LATER ***
    // Image acquisition must be ended when no more images are needed.
    //
    err = spinCameraBeginAcquisition(hCam);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to begin image acquisition. Aborting with error %d...\n\n", err);
        return err;
    }

    printf("Acquiring images...\n");

    //
    // Retrieve device serial number for filename
    //
    // *** NOTES ***
    // The device serial number is retrieved in order to keep cameras from
    // overwriting one another. Grabbing image IDs could also accomplish this.
    //
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
        if (IsAvailableAndReadable(hDeviceSerialNumber, "DeviceSerialNumber"))
        {
            err = spinStringGetValue(hDeviceSerialNumber, deviceSerialNumber, &lenDeviceSerialNumber);
            if (err != SPINNAKER_ERR_SUCCESS)
            {
                strcpy(deviceSerialNumber, "");
                lenDeviceSerialNumber = 0;
            }
        }
        else
        {
            strcpy(deviceSerialNumber, "");
            lenDeviceSerialNumber = 0;
            PrintRetrieveNodeFailure("node", "DeviceSerialNumber");
        }
        printf("Device serial number retrieved as %s...\n", deviceSerialNumber);
    }
    printf("\n");

    // Retrieve, convert, and save images
    const unsigned int k_numImages = 10;
    unsigned int imageCnt = 0;

    for (imageCnt = 0; imageCnt < k_numImages; imageCnt++)
    {
        //
        // Retrieve next received image
        //
        // *** NOTES ***
        // Capturing an image houses images on the camera buffer. Trying to
        // capture an image that does not exist will hang the camera.
        //
        // *** LATER ***
        // Once an image from the buffer is saved and/or no longer needed, the
        // image must be released in orer to keep the buffer from filling up.
        //
        spinImage hResultImage = NULL;

        err = spinCameraGetNextImageEx(hCam, 1000, &hResultImage);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to get next image. Non-fatal error %d...\n\n", err);
            continue;
        }

        //
        // Ensure image completion
        //
        // *** NOTES ***
        // Images can easily be checked for completion. This should be done
        // whenever a complete image is expected or required. Further, check
        // image status for a little more insight into why an image is
        // incomplete.
        //
        bool8_t isIncomplete = False;
        bool8_t hasFailed = False;

        err = spinImageIsIncomplete(hResultImage, &isIncomplete);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to determine image completion. Non-fatal error %d...\n\n", err);
            hasFailed = True;
        }

        // Check image for completion
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

            continue;
        }

        //
        // Print image information; height and width recorded in pixels
        //
        // *** NOTES ***
        // Images have quite a bit of available metadata including things such
        // as CRC, image status, and offset values, to name a few.
        //
        size_t width = 0;
        size_t height = 0;

        printf("Grabbed image %d, ", imageCnt);

        // Retrieve image width
        err = spinImageGetWidth(hResultImage, &width);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("width = unknown, ");
        }
        else
        {
            printf("width = %u, ", (unsigned int)width);
        }

        // Retrieve image height
        err = spinImageGetHeight(hResultImage, &height);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("height = unknown\n");
        }
        else
        {
            printf("height = %u\n", (unsigned int)height);
        }

        //
        // Convert image to mono 8
        //
        // *** NOTES ***
        // Images not gotten from a camera directly must be created and
        // destroyed. This includes any image copies, conversions, or
        // otherwise. Basically, if the image was gotten, it should be
        // released, if it was created, it needs to be destroyed.
        //
        // Images can be converted between pixel formats by using the
        // appropriate enumeration value. Unlike the original image, the
        // converted one does not need to be released as it does not affect the
        // camera buffer.
        //
        // Optionally, the color processing algorithm can also be set using
        // the alternate spinImageConvertEx() function.
        //
        // *** LATER ***
        // The converted image was created, so it must be destroyed to avoid
        // memory leaks.
        //
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

        // Create a unique filename
        char filename[MAX_BUFF_LEN];

        if (lenDeviceSerialNumber == 0)
        {
            sprintf(filename, "Acquisition-C-%d.jpg", imageCnt);
        }
        else
        {
            sprintf(filename, "Acquisition-C-%s-%d.jpg", deviceSerialNumber, imageCnt);
        }

        //
        // Save image
        //
        // *** NOTES ***
        // The standard practice of the examples is to use device serial
        // numbers to keep images of one device from overwriting those of
        // another.
        //
        err = spinImageSave(hConvertedImage, filename, JPEG);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to save image. Non-fatal error %d...\n\n", err);
        }
        else
        {
            printf("Image saved at %s\n\n", filename);
        }

        //
        // Destroy converted image
        //
        // *** NOTES ***
        // Images that are created must be destroyed in order to avoid memory
        // leaks.
        //
        err = spinImageDestroy(hConvertedImage);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to destroy image. Non-fatal error %d...\n\n", err);
        }

        //
        // Release image from camera
        //
        // *** NOTES ***
        // Images retrieved directly from the camera (i.e. non-converted
        // images) need to be released in order to keep from filling the
        // buffer.
        //
        err = spinImageRelease(hResultImage);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to release image. Non-fatal error %d...\n\n", err);
        }
    }
    //
    // End acquisition
    //
    // *** NOTES ***
    // Ending acquisition appropriately helps ensure that devices clean up
    // properly and do not need to be power-cycled to maintain integrity.
    //
    err = spinCameraEndAcquisition(hCam);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to end acquisition. Non-fatal error %d...\n\n", err);
    }
    return err;
}

// This function prints the device information of the camera from the transport
// layer; please see NodeMapInfo_C example for more in-depth comments on
// printing device information from the nodemap.
spinError PrintDeviceInfo(spinNodeMapHandle hNodeMap)
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
spinError RunSingleCamera(spinCamera hCam)
{
    spinError err = SPINNAKER_ERR_SUCCESS;
    // Retrieve TL device nodemap and print device information
    spinNodeMapHandle hNodeMapTLDevice = NULL;

    err = spinCameraGetTLDeviceNodeMap(hCam, &hNodeMapTLDevice);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve TL device nodemap. Non-fatal error %d...\n\n", err);
    }
    else
    {
        err = PrintDeviceInfo(hNodeMapTLDevice);
    }

    // Initialize camera
    err = spinCameraInit(hCam);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to initialize camera. Aborting with error %d...\n\n", err);
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

    // Acquire images
    err = AcquireImages(hCam, hNodeMap, hNodeMapTLDevice);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        return err;
    }

    // Deinitialize camera
    err = spinCameraDeInit(hCam);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to deinitialize camera. Aborting with error %d...\n\n", err);
        return err;
    }

    return err;
}

// Example entry point; please see Enumeration_C example for more in-depth
// comments on preparing and cleaning up the system.
int main(/*int argc, char** argv*/)
{
    spinError errReturn = SPINNAKER_ERR_SUCCESS;
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
        return SPINNAKER_ERR_ACCESS_DENIED;
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
            errReturn = err;
        }
        else
        {
            // Run example
            err = RunSingleCamera(hCamera);
            if (err != SPINNAKER_ERR_SUCCESS)
            {
                errReturn = err;
            }
        }

        // Release camera
        err = spinCameraRelease(hCamera);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            errReturn = err;
        }

        printf("Camera %d example complete...\n\n", i);
    }

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
