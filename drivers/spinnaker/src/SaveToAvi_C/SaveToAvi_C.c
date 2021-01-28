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
 *  @example SaveToAvi_C.c
 *
 *  @brief SaveToAvi_C.c shows how to create an video from a vector of images.
 *  It relies on information provided in the Enumeration_C , Acquisition_C, and
 *  NodeMapInfo_C examples.
 *
 *  This example introduces the SpinVideo class, which is used to quickly and
 *  easily create various types of video files. It demonstrates the creation of
 *  three types: uncompressed, MJPG, and H264.
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
#include "SpinVideoC.h"
#include "stdio.h"
#include "string.h"

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

// These macros helps with C-strings and number of frames in a video.
#define MAX_BUFF_LEN 256
#define NUM_IMAGES   10

// Use the following enum and global constant to select the type of video
// file to be created and saved.
typedef enum _fileType
{
    UNCOMPRESSED,
    MJPG,
    H264
} fileType;

const fileType chosenFileType = UNCOMPRESSED;

// This function prepares, saves, and cleans up an video from a vector of images.
spinError SaveArrayToVideo(spinNodeMapHandle hNodeMap, spinNodeMapHandle hNodeMapTLDevice, spinImage hImages[])
{
    spinError err = SPINNAKER_ERR_SUCCESS;

    printf("\n\n*** CREATING VIDEO ***\n\n");

    //
    // Get the current frame rate; acquisition frame rate recorded in hertz
    //
    // *** NOTES ***
    // The video frame rate can be set to anything; however, in order to
    // have videos play in real-time, the acquisition frame rate can be
    // retrieved from the camera.
    //
    spinNodeHandle hAcquisitionFrameRate = NULL;
    double acquisitionFrameRate = 0.0;
    float frameRateToSet = 0.0;

    err = spinNodeMapGetNode(hNodeMap, "AcquisitionFrameRate", &hAcquisitionFrameRate);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve frame rate (node retrieval). Aborting with error %d...\n\n", err);
        return err;
    }

    if (IsAvailableAndReadable(hAcquisitionFrameRate, "AcquisitionFrameRate"))
    {
        err = spinFloatGetValue(hAcquisitionFrameRate, &acquisitionFrameRate);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to retrieve frame rate (value retrieval). Aborting with error %d...\n\n", err);
            return err;
        }
    }
    else
    {
        err = SPINNAKER_ERR_ACCESS_DENIED;
        printf("Unable to read frame rate. Aborting with error %d...\n\n", err);
        return err;
    }

    frameRateToSet = (float)acquisitionFrameRate;

    printf("Frame rate to be set to %f\n", frameRateToSet);

    // Retrieve device serial number for filename
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
                printf("Unable to get device serial number. Non-fatal error %d...\n\n", err);
                strcpy(deviceSerialNumber, "");
                lenDeviceSerialNumber = 0;
            }
        }
        else
        {
            err = SPINNAKER_ERR_ACCESS_DENIED;
            printf("Unable to get device serial number. Aborting with error %d...\n\n", err);
            strcpy(deviceSerialNumber, "");
            lenDeviceSerialNumber = 0;
            return err;
        }

        printf("Device serial number retrieved as %s...\n", deviceSerialNumber);
    }
    printf("\n");

    //
    // Create a unique filename
    //
    // *** NOTES ***
    // This example creates filenames according to the type of video
    // being created. Notice that '.avi' does not need to be appended to the
    // name of the file. This is because the SpinVideo object takes care
    // of the file extension automatically.
    //
    char filename[MAX_BUFF_LEN];

    if (chosenFileType == UNCOMPRESSED)
    {
        if (lenDeviceSerialNumber == 0)
        {
            sprintf(filename, "SaveToAvi-C-Uncompressed");
        }
        else
        {
            sprintf(filename, "SaveToAvi-C-%s-Uncompressed", deviceSerialNumber);
        }
    }
    else if (chosenFileType == MJPG)
    {
        if (lenDeviceSerialNumber == 0)
        {
            sprintf(filename, "SaveToAvi-C-MJPG");
        }
        else
        {
            sprintf(filename, "SaveToAvi-C-%s-MJPG", deviceSerialNumber);
        }
    }
    else if (chosenFileType == H264)
    {
        if (lenDeviceSerialNumber == 0)
        {
            sprintf(filename, "SaveToAvi-C-H264");
        }
        else
        {
            sprintf(filename, "SaveToAvi-C-%s-H264", deviceSerialNumber);
        }
    }

    //
    // Select option and open video file type
    //
    // *** NOTES ***
    // Depending on the file type, a number of settings need to be set in
    // an object called an option. An uncompressed option only needs to
    // have the video frame rate set whereas videos with MJPG or H264
    // compressions need to have more values set.
    //
    // Once the desired option object is configured, open the video file
    // with the option in order to create the video file.
    //
    // *** LATER ***
    // Once all images have been added, it is important to close the file -
    // this is similar to many other standard file streams.
    //
    spinVideo video = NULL;

    if (chosenFileType == UNCOMPRESSED)
    {
        spinAVIOption option;

        option.frameRate = frameRateToSet;

        err = spinVideoOpenUncompressed(&video, filename, option);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to open uncompressed video file. Aborting with error %d...\n\n", err);
            return err;
        }
    }
    else if (chosenFileType == MJPG)
    {
        spinMJPGOption option;

        option.frameRate = frameRateToSet;
        option.quality = 75;

        err = spinVideoOpenMJPG(&video, filename, option);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to open MJPG video file. Aborting with error %d...\n\n", err);
            return err;
        }
    }
    else if (chosenFileType == H264)
    {
        spinH264Option option;

        option.frameRate = frameRateToSet;
        option.bitrate = 1000000;

        spinNodeHandle hWidth = NULL;
        int64_t width = 0;

        err = spinNodeMapGetNode(hNodeMap, "Width", &hWidth);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to retrieve width (node retrieval). Aborting with error %d...\n\n", err);
            return err;
        }

        if (IsAvailableAndReadable(hWidth, "Width"))
        {
            err = spinIntegerGetValue(hWidth, &width);
            if (err != SPINNAKER_ERR_SUCCESS)
            {
                printf("Unable to retrieve width (value retrieval). Aborting with error %d...\n\n", err);
                return err;
            }
        }
        else
        {
            err = SPINNAKER_ERR_ACCESS_DENIED;
            printf("Unable to read width. Aborting with error %d...\n\n", err);
            return err;
        }

        option.width = (unsigned int)width;

        spinNodeHandle hHeight = NULL;
        int64_t height = 0;

        err = spinNodeMapGetNode(hNodeMap, "Height", &hHeight);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to retrieve height (node retrieval). Aborting with error %d...\n\n", err);
            return err;
        }

        if (IsAvailableAndReadable(hHeight, "Height"))
        {
            err = spinIntegerGetValue(hHeight, &height);
            if (err != SPINNAKER_ERR_SUCCESS)
            {
                printf("Unable to retrieve height (value retrieval). Aborting with error %d...\n\n", err);
                return err;
            }
        }
        else
        {
            err = SPINNAKER_ERR_ACCESS_DENIED;
            printf("Unable to read height. Aborting with error %d...\n\n", err);
            return err;
        }

        option.height = (unsigned int)height;

        err = spinVideoOpenH264(&video, filename, option);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to open H264 video file. Aborting with error %d...\n\n", err);
            return err;
        }
    }

    // Set maximum video file size to 2GB. A new video file is generated when 2GB
    // limit is reached. Setting maximum file size to 0 indicates no limit.
    const unsigned int k_videoFileSize = 2048;

    err = spinVideoSetMaximumFileSize(video, k_videoFileSize);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to set maximum file size. Aborting with error %d...\n\n", err);
        return err;
    }

    //
    // Construct and save video
    //
    // *** NOTES ***
    // Although the video file has been opened, images must be individually
    // appended in order to construct the video.
    //
    unsigned int imageCnt = 0;

    printf("Appending %d images to video file: %s.avi...\n\n", NUM_IMAGES, filename);

    for (imageCnt = 0; imageCnt < NUM_IMAGES; imageCnt++)
    {
        err = spinVideoAppend(video, hImages[imageCnt]);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to append image. Aborting with error %d...\n\n", err);
            return err;
        }

        printf("\tAppended image %d...\n", imageCnt);
    }

    printf("\nVideo saved at %s.avi\n\n", filename);

    //
    // Close video file
    //
    // *** NOTES ***
    // Once all images have been appended, it is important to close the
    // video file. Notice that once an video file has been closed, no more
    // images can be added.
    //
    err = spinVideoClose(video);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to close video file. Aborting with error %d...\n\n", err);
        return err;
    }

    // Destroy images
    for (imageCnt = 0; imageCnt < NUM_IMAGES; imageCnt++)
    {
        err = spinImageDestroy(hImages[imageCnt]);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to destroy image %d. Non-fatal error %d...\n\n", imageCnt, err);
        }
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
        err = SPINNAKER_ERR_ACCESS_DENIED;
        printf("Unable to read device information. Non-fatal error %d...\n\n", err);
        return err;
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

// This function acquires and saves 10 images from a device; please see
// Acquisition_C example for more in-depth comments on the acquisition of
// images.
spinError AcquireImages(
    spinCamera hCam,
    spinNodeMapHandle hNodeMap,
    spinNodeMapHandle hNodeMapTLDevice,
    spinImage hImages[])
{
    spinError err = SPINNAKER_ERR_SUCCESS;

    printf("\n*** IMAGE ACQUISITION ***\n\n");

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
        err = SPINNAKER_ERR_ACCESS_DENIED;
        printf("Unable to read acquisition mode. Aborting with error %d...\n\n", err);
        return err;
    }

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
        err = SPINNAKER_ERR_ACCESS_DENIED;
        printf("Unable to read acquisition mode continuous. Aborting with error %d...\n\n", err);
        return err;
    }

    // set acquisition mode to continuous
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
        printf("Acquisition mode set to continuous...\n");
    }
    else
    {
        err = SPINNAKER_ERR_ACCESS_DENIED;
        printf("Unable to write to acquisition mode. Aborting with error %d...\n\n", err);
        return err;
    }

    // Begin acquiring images
    err = spinCameraBeginAcquisition(hCam);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to begin image acquisition. Aborting with error %d...\n\n", err);
        return err;
    }

    printf("Acquiring images...\n");

    // Retrieve device serial number for filename
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
                printf("Unable to get device serial number. Non-fatal error %d...\n\n", err);
                strcpy(deviceSerialNumber, "");
                lenDeviceSerialNumber = 0;
            }

            printf("Device serial number retrieved as %s...\n", deviceSerialNumber);
        }
        else
        {
            err = SPINNAKER_ERR_ACCESS_DENIED;
            printf("Unable to get device serial number. Non-fatal error %d...\n\n", err);
            strcpy(deviceSerialNumber, "");
            lenDeviceSerialNumber = 0;
        }
    }
    printf("\n");

    // Retrieve, convert, and save images
    unsigned int imageCnt = 0;

    for (imageCnt = 0; imageCnt < NUM_IMAGES; imageCnt++)
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

        printf("Grabbed image %d, ", imageCnt);

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
        hImages[imageCnt] = NULL;

        err = spinImageCreateEmpty(&hImages[imageCnt]);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to create image. Non-fatal error %d...\n\n", err);
            hasFailed = True;
        }

        err = spinImageConvert(hResultImage, PixelFormat_Mono8, hImages[imageCnt]);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to convert image. Non-fatal error %d...\n\n", err);
            hasFailed = True;
        }

        // Release image
        err = spinImageRelease(hResultImage);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to release image. Non-fatal error %d...\n\n", err);
        }
    }

    // End Acquisition
    err = spinCameraEndAcquisition(hCam);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to end acquisition. Non-fatal error %d...\n\n", err);
    }

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
    spinImage hImages[NUM_IMAGES];

    err = AcquireImages(hCam, hNodeMap, hNodeMapTLDevice, hImages);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        return err;
    }

    err = SaveArrayToVideo(hNodeMap, hNodeMapTLDevice, hImages);
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
