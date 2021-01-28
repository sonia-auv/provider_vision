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
 *  @example Trigger_C.c
 *
 *  @brief Trigger_C.c shows how to trigger the camera. It relies on
 *  information provided in the Enumeration_C, Acquisition_C, and NodeMapInfo_C
 *  examples.
 *
 *  It can also be helpful to familiarize yourself with the
 *  ImageFormatControl_C and Exposure_C examples as these provide a strong
 *  introduction to camera customization.
 *
 *  This example shows the process of configuring, using, and cleaning up a
 *  camera for use with both a software and a hardware trigger.
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

// Use the following enum and global static variable to select whether a
// software or hardware trigger is used.
typedef enum _triggerType
{
    SOFTWARE,
    HARDWARE
} triggerType;

const triggerType chosenTrigger = SOFTWARE;

// This function configures the camera to use a trigger. First, trigger mode
// is set to off in order to select the trigger source. Trigger mode is
// then enabled, which has the camera capture only a single image upon the
// execution of the chosen trigger.
spinError ConfigureTrigger(spinNodeMapHandle hNodeMap)
{
    spinError err = SPINNAKER_ERR_SUCCESS;

    printf("\n\n*** TRIGGER CONFIGURATION ***\n\n");

    printf("Note that if the application / user software triggers faster than frame time, the trigger may be dropped / "
           "skipped by the camera.\n");
    printf("If several frames are needed per trigger, a more reliable alternative for such case, is to use the "
           "multi-frame mode.\n\n");

    if (chosenTrigger == SOFTWARE)
    {
        printf("Software trigger chosen...\n\n");
    }
    else if (chosenTrigger == HARDWARE)
    {
        printf("Hardware trigger chosen...\n\n");
    }

    //
    // Ensure trigger mode off
    //
    // *** NOTES ***
    // The trigger must be disabled in order to configure whether the source
    // is software or hardware.
    //
    spinNodeHandle hTriggerMode = NULL;
    spinNodeHandle hTriggerModeOff = NULL;
    int64_t triggerModeOff = 0;

    err = spinNodeMapGetNode(hNodeMap, "TriggerMode", &hTriggerMode);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to disable trigger mode. Aborting with error %d...\n\n", err);
        return err;
    }

    // check if available and writable
    if (!IsAvailableAndReadable(hTriggerMode, "TriggerMode"))
    {
        PrintRetrieveNodeFailure("node", "TriggerMode");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }

    // fetch entry
    err = spinEnumerationGetEntryByName(hTriggerMode, "Off", &hTriggerModeOff);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to disable trigger mode. Aborting with error %d...\n\n", err);
        return err;
    }

    // check if available and readable
    if (!IsAvailableAndReadable(hTriggerModeOff, "TriggerModeOff"))
    {
        PrintRetrieveNodeFailure("entry", "TriggerMode 'Off'");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }

    err = spinEnumerationEntryGetIntValue(hTriggerModeOff, &triggerModeOff);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to disable trigger mode. Aborting with error %d...\n\n", err);
        return err;
    }

    // turn trigger mode off
    if (!IsAvailableAndWritable(hTriggerMode, "TriggerMode"))
    {
        PrintRetrieveNodeFailure("node", "TriggerMode");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }

    err = spinEnumerationSetIntValue(hTriggerMode, triggerModeOff);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to disable trigger mode. Aborting with error %d...\n\n", err);
        return err;
    }

    printf("Trigger mode disabled...\n");

    //
    // Set TriggerSelector to FrameStart
    //
    // *** NOTES ***
    // For this example, the trigger selector should be set to frame start.
    // This is the default for most cameras.
    //
    spinNodeHandle hTriggerSelector = NULL;
    spinNodeHandle hTriggerSelectorChoice = NULL;
    int64_t triggerSelectorChoice = 0;

    // Retrieve enumeration node from nodemap
    err = spinNodeMapGetNode(hNodeMap, "TriggerSelector", &hTriggerSelector);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to choose trigger selector. Aborting with error %d...\n\n", err);
        return err;
    }

    // check if readable
    if (!IsAvailableAndReadable(hTriggerSelector, "TriggerSelector"))
    {
        PrintRetrieveNodeFailure("node", "TriggerSelector");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }

    // Retrieve entry node from enumeration node to set selector
    err = spinEnumerationGetEntryByName(hTriggerSelector, "FrameStart", &hTriggerSelectorChoice);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to choose trigger selector. Aborting with error %d...\n\n", err);
        return err;
    }

    if (!IsAvailableAndReadable(hTriggerSelectorChoice, "TriggerSelectorChoice"))
    {
        PrintRetrieveNodeFailure("entry", "TriggerSelector 'Choice'");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }

    // Retrieve integer value from entry node
    err = spinEnumerationEntryGetIntValue(hTriggerSelectorChoice, &triggerSelectorChoice);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to choose trigger selector. Aborting with error %d...\n\n", err);
        return err;
    }

    // set trigger source choice
    if (!IsAvailableAndWritable(hTriggerSelector, "TriggerSelector"))
    {
        PrintRetrieveNodeFailure("node", "TriggerSelector");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }

    err = spinEnumerationSetIntValue(hTriggerSelector, triggerSelectorChoice);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to choose trigger selector. Aborting with error %d...\n\n", err);
        return err;
    }

    printf("Trigger selector set to frame start...\n");

    //
    // Choose trigger source
    //
    // *** NOTES ***
    // The trigger source must be set to hardware or software while trigger
    // mode is off.
    //
    spinNodeHandle hTriggerSource = NULL;
    spinNodeHandle hTriggerSourceChoice = NULL;
    int64_t triggerSourceChoice = 0;

    // Retrieve enumeration node from nodemap
    err = spinNodeMapGetNode(hNodeMap, "TriggerSource", &hTriggerSource);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to choose trigger source. Aborting with error %d...\n\n", err);
        return err;
    }

    // check if readable
    if (!IsAvailableAndReadable(hTriggerSource, "TriggerSource"))
    {
        PrintRetrieveNodeFailure("node", "TriggerSource");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }

    if (chosenTrigger == SOFTWARE)
    {
        // Retrieve entry node from enumeration node to set software
        err = spinEnumerationGetEntryByName(hTriggerSource, "Software", &hTriggerSourceChoice);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to choose trigger source. Aborting with error %d...\n\n", err);
            return err;
        }
    }
    else if (chosenTrigger == HARDWARE)
    {
        // Retrieve entry node from enumeration node to set hardware ('Line0')
        err = spinEnumerationGetEntryByName(hTriggerSource, "Line0", &hTriggerSourceChoice);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to choose trigger source. Aborting with error %d...\n\n", err);
            return err;
        }
    }

    if (!IsAvailableAndReadable(hTriggerSourceChoice, "TriggerSourceChoice"))
    {
        PrintRetrieveNodeFailure("entry", "TriggerSource 'Choice'");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }

    // Retrieve integer value from entry node
    err = spinEnumerationEntryGetIntValue(hTriggerSourceChoice, &triggerSourceChoice);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to choose trigger source. Aborting with error %d...\n\n", err);
        return err;
    }

    // set trigger source choice
    if (!IsAvailableAndWritable(hTriggerSource, "TriggerSource"))
    {
        PrintRetrieveNodeFailure("node", "TriggerSource");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }

    err = spinEnumerationSetIntValue(hTriggerSource, triggerSourceChoice);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to choose trigger source. Aborting with error %d...\n\n", err);
        return err;
    }

    if (chosenTrigger == SOFTWARE)
    {
        printf("Trigger source set to software...\n");
    }
    else if (chosenTrigger == HARDWARE)
    {
        printf("Trigger source set to line 0...\n");
    }

    //
    // Turn trigger mode on
    //
    // *** LATER ***
    // Once the appropriate trigger source has been set, turn trigger mode
    // in order to retrieve images using the trigger.
    //

    spinNodeHandle hTriggerModeOn = NULL;
    int64_t triggerModeOn = 0;

    // NOTE: Blackfly and Flea3 GEV cameras need 1 second delay after trigger mode is
    // turned on
    err = spinEnumerationGetEntryByName(hTriggerMode, "On", &hTriggerModeOn);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to enable trigger mode. Aborting with error %d...\n\n", err);
        return err;
    }

    if (!IsAvailableAndReadable(hTriggerModeOn, "TriggerModeOn"))
    {
        PrintRetrieveNodeFailure("entry", "Trigger Mode 'On'");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }

    err = spinEnumerationEntryGetIntValue(hTriggerModeOn, &triggerModeOn);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to enable trigger mode. Aborting with error %d...\n\n", err);
        return err;
    }

    if (!IsAvailableAndWritable(hTriggerSource, "TriggerSource"))
    {
        PrintRetrieveNodeFailure("node", "TriggerSource");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }

    err = spinEnumerationSetIntValue(hTriggerMode, triggerModeOn);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to enable trigger mode. Aborting with error %d...\n\n", err);
        return err;
    }

    printf("Trigger mode enabled...\n\n");

    return err;
}

// This function retrieves a single image using the trigger. In this example,
// only a single image is captured and made available for acquisition - as such,
// attempting to acquire two images for a single trigger execution would cause
// the example to hang. This is different from other examples, whereby a
// constant stream of images are being captured and made available for image
// acquisition.
spinError GrabNextImageByTrigger(spinNodeMapHandle hNodeMap, spinCamera hCam)
{
    spinError err = SPINNAKER_ERR_SUCCESS;

    //
    // Use trigger to capture image
    //
    // *** NOTES ***
    // The software trigger only feigns being executed by the Enter key;
    // what might not be immediately apparent is that there is no
    // continuous stream of images being captured; in other examples that
    // acquire images, the camera captures a continuous stream of images.
    // When an image is then retrieved, it is plucked from the stream;
    // there are many more images captured than retrieved. However, while
    // trigger mode is activated, there is only a single image captured at
    // the time that the trigger is activated.
    //
    if (chosenTrigger == SOFTWARE)
    {
        // Get user input
        printf("Press the Enter key to initiate software trigger...\n");
        getchar();

        // Execute software trigger
        spinNodeHandle hTriggerSoftware = NULL;

        err = spinNodeMapGetNode(hNodeMap, "TriggerSoftware", &hTriggerSoftware);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to execute software trigger. Aborting with error %d...\n\n", err);
            return err;
        }

        err = spinCommandExecute(hTriggerSoftware);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to execute software trigger. Aborting with error %d...\n\n", err);
            return err;
        }

        // NOTE: Blackfly and Flea3 GEV cameras need 2 second delay after software trigger
    }
    else if (chosenTrigger == HARDWARE)
    {
        // Execute hardware trigger
        printf("Use the hardware to trigger image acquisition.\n");
    }

    return err;
}

// This function returns the camera to a normal state by turning off trigger
// mode.
spinError ResetTrigger(spinNodeMapHandle hNodeMap)
{

    spinError err = SPINNAKER_ERR_SUCCESS;

    //
    // Turn trigger mode back off
    //
    // *** NOTES ***
    // Once all images have been captured, it is important to turn trigger
    // mode back off to restore the camera to a clean state.
    //
    spinNodeHandle hTriggerMode = NULL;
    spinNodeHandle hTriggerModeOff = NULL;
    int64_t triggerModeOff = 0;

    err = spinNodeMapGetNode(hNodeMap, "TriggerMode", &hTriggerMode);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to disable trigger mode. Aborting with error %d...\n\n", err);
        return err;
    }

    if (!IsAvailableAndReadable(hTriggerMode, "TriggerMode"))
    {
        PrintRetrieveNodeFailure("node", "TriggerMode");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }
    err = spinEnumerationGetEntryByName(hTriggerMode, "Off", &hTriggerModeOff);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to disable trigger mode. Aborting with error %d...\n\n", err);
        return err;
    }

    if (!IsAvailableAndReadable(hTriggerModeOff, "TriggerModeOff"))
    {
        PrintRetrieveNodeFailure("entry", "TriggerMode 'Off'");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }
    err = spinEnumerationEntryGetIntValue(hTriggerModeOff, &triggerModeOff);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to disable trigger mode. Aborting with error %d...\n\n", err);
        return err;
    }

    if (!IsAvailableAndWritable(hTriggerMode, "TriggerMode"))
    {
        PrintRetrieveNodeFailure("node", "TriggerMode");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }
    err = spinEnumerationSetIntValue(hTriggerMode, triggerModeOff);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to disable trigger mode. Aborting with error %d...\n\n", err);
        return err;
    }

    printf("Trigger mode disabled...\n\n");

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
spinError AcquireImages(spinCamera hCam, spinNodeMapHandle hNodeMap, spinNodeMapHandle hNodeMapTLDevice)
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
        PrintRetrieveNodeFailure("entry", "AcquistionMode 'Continuous'");
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
        PrintRetrieveNodeFailure("entry", "AcquisitionModeContinuous");
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
                strcpy(deviceSerialNumber, "");
                lenDeviceSerialNumber = 0;
            }

            printf("Device serial number retrieved as %s...\n", deviceSerialNumber);
        }
        else
        {
            PrintRetrieveNodeFailure("node", "DeviceSerialNumber");
            strcpy(deviceSerialNumber, "");
            lenDeviceSerialNumber = 0;
        }
    }
    printf("\n");

    // Retrieve, convert, and save images
    const unsigned int k_numImages = 10;
    unsigned int imageCnt = 0;

    for (imageCnt = 0; imageCnt < k_numImages; imageCnt++)
    {
        // Retrieve next image by trigger
        spinImage hResultImage = NULL;

        err = GrabNextImageByTrigger(hNodeMap, hCam);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            return err;
        }

        // Retrieve next received image
        err = spinCameraGetNextImageEx(hCam, 1000, &hResultImage);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to retrieve next image.  Aborting with error %d...\n\n", err);
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

        // Create unique file name
        char filename[MAX_BUFF_LEN];

        if (lenDeviceSerialNumber == 0)
        {
            sprintf(filename, "Trigger-C-%d.jpg", imageCnt);
        }
        else
        {
            sprintf(filename, "Trigger-C-%s-%d.jpg", deviceSerialNumber, imageCnt);
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

        // Release complete image
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

    // Configure trigger
    err = ConfigureTrigger(hNodeMap);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        return err;
    }

    // Acquire images
    err = AcquireImages(hCam, hNodeMap, hNodeMapTLDevice);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        return err;
    }

    // Reset trigger
    err = ResetTrigger(hNodeMap);
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
