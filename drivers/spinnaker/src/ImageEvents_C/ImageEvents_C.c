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
 *  @example ImageEvents_C.c
 *
 *  @brief ImageEvents_C.c shows how to acquire images using the image event
 *  handler. It relies on information provided in the Enumeration_C,
 *  Acquisition_C, and NodeMapInfo_C examples.
 *
 *  It can also be helpful to familiarize yourself with the NodeMapCallback_C
 *  example, as nodemap callbacks follow the same general procedure as
 *  events, but with a few less steps.
 *
 *  Events generally require a class to be defined as an event handler; however,
 *  because C is not an object-oriented language, an event context is created
 *  using a function and a struct whereby the function acts as the event
 *  method and the struct acts as its properties.
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

// Libraries windows.h or unistd.h included for Sleep()/usleep()
#if defined WIN32 || defined _WIN32 || defined WIN64 || defined _WIN64
#include "windows.h"
#else
#include "unistd.h"
#endif

#include "SpinnakerC.h"
#include "stdio.h"
#include <stdlib.h>
#include "string.h"

// This helper function allows the example to sleep in both Windows and Linux
// systems. Note that Windows sleep takes milliseconds as a parameter while
// Linux systems take microseconds as a parameter.
void SleepyWrapper(int milliseconds)
{
#if defined WIN32 || defined _WIN32 || defined WIN64 || defined _WIN64
    Sleep(milliseconds);
#else
    usleep(1000 * milliseconds);
#endif
}

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

// This struct represents the properties of what would be an image event handler
// were we working in an object-oriented programming language. The struct is
// created with a pointer and passed into the function, which creates persistent
// data, mimicking the properties of a class.
typedef struct _userData
{
    unsigned int numImages;
    unsigned int imageCnt;

    size_t lenDeviceSerialNumber;
    char deviceSerialNumber[MAX_BUFF_LEN];
} userData;

// This function represents what would be the method of an image event handler.
// Together with the struct above, this makes up the image event context.
// Notice that the function signature must match this exactly for the function
// to be accepted when creating the event.
void onImageEvent(spinImage hImage, void* pUserData)
{
    spinError err = SPINNAKER_ERR_SUCCESS;

    // Convert void pointer back to struct
    userData* imageEventInfo = (userData*)pUserData;

    // Only retrieve, convert, and save images if number of expected images has
    // not been exceeded
    if (imageEventInfo->imageCnt < imageEventInfo->numImages)
    {
        printf("Image event occurred...\n");

        // Ensure image completion
        bool8_t isIncomplete = False;

        err = spinImageIsIncomplete(hImage, &isIncomplete);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to determine image completion. Non-fatal error %d...\n\n", err);
            return;
        }

        if (isIncomplete)
        {
            spinImageStatus imageStatus = IMAGE_NO_ERROR;

            err = spinImageGetStatus(hImage, &imageStatus);
            if (err != SPINNAKER_ERR_SUCCESS)
            {
                printf("Unable to retrieve image status. Non-fatal error %d...\n\n", err);
            }
            else
            {
                printf("Image incomplete with image status %d...\n", imageStatus);
            }

            return;
        }

        // Print image information
        size_t width = 0;
        size_t height = 0;

        err = spinImageGetWidth(hImage, &width);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to retrieve image width. Non-fatal error %d...\n", err);
        }

        err = spinImageGetHeight(hImage, &height);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to retrieve image height. Non-fatal error %d...\n", err);
        }

        printf(
            "Grabbed image %u, width = %u, height = %u\n",
            imageEventInfo->imageCnt,
            (unsigned int)width,
            (unsigned int)height);

        // Convert image to mono 8
        spinImage hConvertedImage = NULL;

        err = spinImageCreateEmpty(&hConvertedImage);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to create image. Non-fatal error %d...\n\n", err);
        }

        err = spinImageConvert(hImage, PixelFormat_Mono8, hConvertedImage);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to convert image. Non-fatal error %d...\n\n", err);
            return;
        }

        // Create unique file name
        char filename[MAX_BUFF_LEN];

        if (imageEventInfo->lenDeviceSerialNumber == 0)
        {
            sprintf(filename, "ImageEvents-C-%d.jpg", imageEventInfo->imageCnt);
        }
        else
        {
            sprintf(
                filename,
                "ImageEvents-C-%d-%d.jpg",
                atoi(imageEventInfo->deviceSerialNumber),
                imageEventInfo->imageCnt);
        }

        // Save image
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

        // Increment number of saved images
        imageEventInfo->imageCnt++;
    }
}

// This function configures image event handlers by creating and registering them
// to a camera.
spinError ConfigureImageEvents(
    spinCamera hCam,
    spinNodeMapHandle hNodeMapTLDevice,
    spinImageEventHandler* imageEventHandler,
    userData* imageEventInfo)
{
    spinError err = SPINNAKER_ERR_SUCCESS;

    printf("\n\n*** IMAGE EVENTS CONFIGURATION ***\n\n");

    //
    // Prepare user data
    //
    // *** NOTES ***
    // It is important to ensure that all requisite variables are initialized
    // appropriately before creating the image event context.
    //
    // *** LATER ***
    // It is a good idea to keep this data in scope in order to avoid memory
    // leaks.
    //
    // Initialize image count to zero and number of images to 10
    imageEventInfo->imageCnt = 0;
    imageEventInfo->numImages = 10;

    // Initialize device serial number for filename
    spinNodeHandle hDeviceSerialNumber = NULL;
    imageEventInfo->lenDeviceSerialNumber = MAX_BUFF_LEN;

    err = spinNodeMapGetNode(hNodeMapTLDevice, "DeviceSerialNumber", &hDeviceSerialNumber);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        strcpy(imageEventInfo->deviceSerialNumber, "");
        imageEventInfo->lenDeviceSerialNumber = 0;
    }
    else
    {
        if (IsAvailableAndReadable(hDeviceSerialNumber, "DeviceSerialNumber"))
        {
            err = spinStringGetValue(
                hDeviceSerialNumber, imageEventInfo->deviceSerialNumber, &imageEventInfo->lenDeviceSerialNumber);
            if (err != SPINNAKER_ERR_SUCCESS)
            {
                strcpy(imageEventInfo->deviceSerialNumber, "");
                imageEventInfo->lenDeviceSerialNumber = 0;
            }

            printf("Device serial number retrieved as %s...\n", imageEventInfo->deviceSerialNumber);
        }
        else
        {
            PrintRetrieveNodeFailure("node", "DeviceSerialNumber");
            strcpy(imageEventInfo->deviceSerialNumber, "");
            imageEventInfo->lenDeviceSerialNumber = 0;
        }
    }

    //
    // Create image event handler
    //
    // *** NOTES ***
    // The image event handler function has been written to only print convert and save
    // images. This demonstrates an alternative method of acquiring images.
    //
    // *** LATER ***
    // In Spinnaker C, every event handler that is created must be destroyed to avoid
    // memory leaks.
    //
    err = spinImageEventHandlerCreate(imageEventHandler, onImageEvent, (void*)imageEventInfo);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to create event. Aborting with error %d...\n\n", err);
        return err;
    }

    printf("Image event created...\n");

    //
    // Register image event handler
    //
    // *** NOTES ***
    // Image event handlers are registered to cameras. If there are multiple cameras,
    // each camera must have the image event handlers registered to it separately.
    // Also, multiple image event handlers may be registered to a single camera.
    //
    // *** LATER ***
    // Image event handlers must be unregistered manually. This must be done prior to
    // releasing the system and while the image event handlers are still in scope.
    //
    err = spinCameraRegisterImageEventHandler(hCam, *imageEventHandler);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to register event. Aborting with error %d...\n\n", err);
        return err;
    }

    printf("Image event registered...\n\n");

    return err;
}

void WaitForImages(userData* imageEventInfo)
{
    //
    // Wait for images
    //
    // *** NOTES ***
    // In order to passively capture images using image event handlers and
    // automatic polling, the main thread sleeps in increments of 200 ms
    // until 10 images have been acquired and saved.
    //
    const int sleepDuration = 200; // in milliseconds

    while (imageEventInfo->imageCnt < imageEventInfo->numImages)
    {
        printf("\t//\n");
        printf("\t// Sleeping for %d ms. Grabbing images...\n", sleepDuration);
        printf("\t//\n");

        SleepyWrapper(sleepDuration);
    }
}

// This function resets the example by unregistering the image event handler.
spinError ResetImageEvents(spinCamera hCam, spinImageEventHandler imageEventHandler)
{
    spinError err = SPINNAKER_ERR_SUCCESS;

    //
    // Unregister device event handler
    //
    // *** NOTES ***
    // It is important to unregister all image event handlers from all cameras that
    // they are registered to.
    //
    err = spinCameraUnregisterImageEventHandler(hCam, imageEventHandler);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to unregister event. Aborting with error %d...\n\n", err);
        return err;
    }

    printf("Image event unregistered...\n");

    //
    // Destroy event handlers
    //
    // *** NOTES ***
    // Event handlers must be destroyed in order to avoid memory leaks.
    //
    err = spinImageEventHandlerDestroy(imageEventHandler);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to destroy event. Aborting with error %d...\n\n", err);
        return err;
    }

    printf("Image event destroyed...\n\n");

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

// This function acquires and saves 10 images from a device; please see
// Acquisition_C example for more in-depth comments on the acquisition of
// images.
spinError AcquireImages(spinCamera hCam, spinNodeMapHandle hNodeMap, userData* imageEventInfo)
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
        PrintRetrieveNodeFailure("entry", "AcquistionMode");
        return SPINNAKER_ERR_ACCESS_DENIED;
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
        PrintRetrieveNodeFailure("entry", "AcquisitionMode 'Continuous'");
        return SPINNAKER_ERR_ACCESS_DENIED;
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
        PrintRetrieveNodeFailure("node", "AcquisitionMode");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }

    // Begin acquiring images
    err = spinCameraBeginAcquisition(hCam);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to begin image acquisition. Aborting with error %d...\n\n", err);
        return err;
    }

    printf("Acquiring images...\n");

    // Wait for 10 images to be captured by image events
    WaitForImages(imageEventInfo);

    // End acquisition
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

    // Configure device event handlers
    spinImageEventHandler imageEventHandler = NULL;
    userData imageEventInfo;

    err = ConfigureImageEvents(hCam, hNodeMapTLDevice, &imageEventHandler, &imageEventInfo);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        return err;
    }

    // Acquire image handlers
    err = AcquireImages(hCam, hNodeMap, &imageEventInfo);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        return err;
    }

    // Reset device event handlers
    err = ResetImageEvents(hCam, imageEventHandler);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        return err;
    }

    // Deinitialize camera
    err = spinCameraDeInit(hCam);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to deinitialize camera. Non-fatal error %d...\n\n", err);
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
