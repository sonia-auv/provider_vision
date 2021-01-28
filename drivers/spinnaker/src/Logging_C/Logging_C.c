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
 *	@example Logging_C.c
 *
 *	@brief Logging_C.c shows how create and use logging events. It relies on
 *	information provided in the Enumeration_C, Acquisition_C, and NodeMapInfo_C
 *	examples.
 *
 *	It can also be helpful to familiarize yourself with the NodeMapCallback_C
 *	example, as nodemap callbacks follow the same general procedure as
 *	events, but with a few less steps.
 *
 *	Events generally require a class to be defined as an event handler; however,
 *	because C is not an object-oriented language, a pseudo-class is created
 *	using a function and a struct whereby the function acts as the event
 *	handler method and the struct acts as its properties.
 *
 *	** NOTE **
 *  When using Visual Studio 2010, our solution will use the /TP flag to
 *  compile this example as C++ code instead of C code. This is because our C
 *  examples adhere to post-C89 standard which is not supported in Visual
 *  Studio 2010.
 *
 */

#include "SpinnakerC.h"
#include "stdio.h"
#include "string.h"

// This macro helps with C-strings.
#define MAX_BUFF_LEN 256

// Define callback priority threshold; please see documentation for additional
// information on logging level philosophy.
const spinnakerLogLevel k_loggingLevel = LOG_LEVEL_DEBUG;

// This function represents what would be the method of a device event handler.
// Although it may not be immediately evident here, this example demonstrates
// the use of an empty void pointer. Also, notice that the function signature
// must match this exactly for the function to be accepted when creating the
// event.
void onLogEvent(const spinLogEventData hEventData, void* pUserData)
{
    spinError err = SPINNAKER_ERR_SUCCESS;

    printf("--------Log Event Received----------\n");

    //
    // Retrieve string log event data (category name)
    //
    // *** NOTES ***
    // Log data is retrieved by using log event data handle in the signature.
    // Most log data is of type string, needing a character array for the
    // string and an integer for the number of characters.
    //
    char categoryName[MAX_BUFF_LEN];
    size_t lenCategoryName = MAX_BUFF_LEN;

    printf("Category: ");

    err = spinLogDataGetCategoryName(hEventData, categoryName, &lenCategoryName);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unknown - error %d\n", err);
    }
    else
    {
        printf("%s\n", categoryName);
    }

    //
    // Retrieve integer log event data (priority)
    //
    // *** NOTES ***
    // Not all log event data is of type string. For instance, priority delivers
    // an integer.
    //
    int64_t priority = 0;

    printf("Priority: ");

    err = spinLogDataGetPriority(hEventData, &priority);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unknown - error %d\n", err);
    }
    else
    {
        printf("%s\n", categoryName);
    }

    // Retrieve string log event data (priority name)
    char priorityName[MAX_BUFF_LEN];
    size_t lenPriorityName = MAX_BUFF_LEN;

    printf("Priority name: ");

    err = spinLogDataGetPriorityName(hEventData, priorityName, &lenPriorityName);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unknown - error %d\n", err);
    }
    else
    {
        printf("%s\n", priorityName);
    }

    // Retrieve string log event data (timestamp)
    char timestamp[MAX_BUFF_LEN];
    size_t lenTimestamp = MAX_BUFF_LEN;

    printf("Timestamp: ");

    err = spinLogDataGetTimestamp(hEventData, timestamp, &lenTimestamp);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unknown - error %d\n", err);
    }
    else
    {
        printf("%s\n", timestamp);
    }

    // Retrieve string log event data (NDC)
    char NDC[MAX_BUFF_LEN];
    size_t lenNDC = MAX_BUFF_LEN;

    printf("NDC: ");

    err = spinLogDataGetNDC(hEventData, NDC, &lenNDC);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unknown - error %d\n", err);
    }
    else
    {
        printf("%s\n", NDC);
    }

    // Retrieve string log event data (thread name)
    char threadName[MAX_BUFF_LEN];
    size_t lenThreadName = MAX_BUFF_LEN;

    printf("Thread name: ");

    err = spinLogDataGetThreadName(hEventData, threadName, &lenThreadName);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unknown - error %d\n", err);
    }
    else
    {
        printf("%s\n", threadName);
    }

    // Retrieve string log event data (log message)
    char logMessage[MAX_BUFF_LEN];
    size_t lenLogMessage = MAX_BUFF_LEN;

    printf("Log message: ");

    err = spinLogDataGetLogMessage(hEventData, logMessage, &lenLogMessage);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unknown - error %d\n", err);
    }
    else
    {
        printf("%s\n", logMessage);
    }

    printf("------------------------------------\n\n");
}

// Example entry point; notice the volume of data that the logging event handler
// prints out on debug despite the fact that very little really happens in this
// example. Because of this, it may be better to have the logger set to lower
// level in order to provide a more concise, focussed log.
int main(/*int argc, char** argv*/)
{
    spinError err = SPINNAKER_ERR_SUCCESS;

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

    //
    // Create logging event handler
    //
    // *** NOTES ***
    // Logging event handlers need to be created prior to registration; however, the
    // logging level may be set after registration.
    //
    // *** LATER ***
    // In Spinnaker C, every event handler that is created must be destroyed to avoid
    // memory leaks.
    //
    spinLogEventHandler logEventHandler = NULL;

    err = spinLogEventHandlerCreate(&logEventHandler, onLogEvent, NULL);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to create log event. Aborting with error %d...\n\n", err);
        return err;
    }

    //
    // Register logging event handler
    //
    // *** NOTES ***
    // Logging event handlers are registered to the system. Take note that a logging
    // event handler is very verbose when the logging level is set to debug.
    //
    // *** LATER ***
    // Logging event handlers must be unregistered manually. This must be done prior to
    // releasing the system and while the device event handlers are still in scope.
    //
    err = spinSystemRegisterLogEventHandler(hSystem, logEventHandler);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to register log event. Aborting with error %d...\n\n", err);
        return err;
    }

    //
    // Set callback priority level
    //
    // *** NOTES ***
    // Please see documentation for information on the logging philosophies of
    // the Spinnaker SDK.
    //
    err = spinSystemSetLoggingLevel(hSystem, k_loggingLevel);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to set logging level. Aborting with error %d...\n\n", err);
        return err;
    }

    // Retrieve list of cameras from the system
    spinCameraList hCameraList = NULL;
    size_t numCameras = 0;

    err = spinCameraListCreateEmpty(&hCameraList);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve camera list. Aborting with error %d...\n\n", err);
        return err;
    }

    err = spinSystemGetCameras(hSystem, hCameraList);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve camera list. Aborting with error %d...\n\n", err);
        return err;
    }

    err = spinCameraListGetSize(hCameraList, &numCameras);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve number of cameras. Aborting with  error %d...\n\n", err);
        return err;
    }

    printf("Number of cameras detected: %u\n\n", (unsigned int)numCameras);

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

    //
    // Unregister logging event handler
    //
    // *** NOTES ***
    // Logging event handlers may be unregistered altogether or individually. It is
    // important to unregister all logging event handlers from the system.
    //
    err = spinSystemUnregisterAllLogEventHandlers(hSystem);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to unregister log event. Aborting with error %d...\n\n", err);
        return err;
    }

    //
    // Destroy event handlers
    //
    // *** NOTES ***
    // Event handlers must be destroyed in order to avoid memory leaks.
    //
    err = spinLogEventHandlerDestroy(logEventHandler);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to destroy log event. Aborting with error %d...\n\n", err);
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

    return err;
}
