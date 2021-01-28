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
 *  @example EnumerationEvents_C.c
 *
 *  @brief EnumerationEvents_C.c explores arrival and removal events on
 *  interfaces and the system. It relies on information provided in the
 *  Enumeration_C, Acquisition_C, and NodeMapInfo_C examples.
 *
 *  It can also be helpful to familiarize yourself with the NodeMapCallback_C
 *  example, as a callback can be thought of as a simpler, easier-to-use event.
 *  Although events are more cumbersome, they are also much more flexible and
 *  extensible.
 *
 *  Events generally require a class to be defined as an event handler; however,
 *  because C is not an object-oriented language, a pseudo-class is created
 *  using a function (or two) and a struct whereby the function(s) acts as the
 *  event handler method and the struct acts as its properties.
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
#include "stdlib.h"

// This macro helps with C-strings.
#define MAX_BUFF_LEN 256

// This function represents the arrival event callback function on the system. Together with the function below, this
// makes up a sort of event handler pseudo-class. Notice that the function signatures must match exactly for the
// function to be accepted when creating the event handler, and notice how the user data is cast from the void pointer
// and manipulated.
void onDeviceArrivalSystem(uint64_t deviceSerialNumber, void* pUserData)
{
    spinError err = SPINNAKER_ERR_SUCCESS;

    // Cast void pointer back to system object
    spinSystem hSystem = (spinSystem*)pUserData;

    // Retrieve count
    spinCameraList hCameraList = NULL;

    err = spinCameraListCreateEmpty(&hCameraList);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to create camera list (system arrival). Non-fatal error %d...\n\n", err);
        return;
    }

    err = spinSystemGetCameras(hSystem, hCameraList);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve camera list (system arrival). Non-fatal error %d...\n\n", err);
        return;
    }

    size_t numCameras = 0;

    err = spinCameraListGetSize(hCameraList, &numCameras);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve camera list size (system arrival). Non-fatal error %d...\n\n", err);
        return;
    }

    // Print count
    printf("System event handler:\n");
    printf("\tDevice %u has arrived on the system.\n", (unsigned int)deviceSerialNumber);
    printf(
        "\tThere %s %u %s on the system.\n\n",
        (numCameras == 1 ? "is" : "are"),
        (unsigned int)numCameras,
        (numCameras == 1 ? "device" : "devices"));

    // Clear and destroy camera list while still in scope
    err = spinCameraListClear(hCameraList);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to clear camera list (system arrival). Non-fatal error %d...\n\n", err);
        return;
    }

    err = spinCameraListDestroy(hCameraList);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to destroy camera list (system arrival). Non-fatal error %d...\n\n", err);
        return;
    }
}

// This function represents a removal event callback function on the system.
void onDeviceRemovalSystem(uint64_t deviceSerialNumber, void* pUserData)
{
    spinError err = SPINNAKER_ERR_SUCCESS;

    // Cast void pointer back to system object
    spinSystem hSystem = (spinSystem*)pUserData;

    // Retrieve count
    spinCameraList hCameraList = NULL;

    err = spinCameraListCreateEmpty(&hCameraList);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to create camera list (system removal). Non-fatal error %d...\n\n", err);
        return;
    }

    err = spinSystemGetCameras(hSystem, hCameraList);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve camera list (system removal). Non-fatal error %d...\n\n", err);
        return;
    }

    size_t numCameras = 0;

    err = spinCameraListGetSize(hCameraList, &numCameras);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve camera list size (system removal). Non-fatal error %d...\n\n", err);
        return;
    }

    // Print count
    printf("System event handler:\n");
    printf("\tDevice %u was removed from the system.\n", (unsigned int)deviceSerialNumber);
    printf(
        "\tThere %s %u %s on the system.\n\n",
        (numCameras == 1 ? "is" : "are"),
        (unsigned int)numCameras,
        (numCameras == 1 ? "device" : "devices"));

    // Clear camera list while still in scope
    err = spinCameraListClear(hCameraList);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to clear camera list (system removal). Non-fatal error %d...\n\n", err);
        return;
    }

    err = spinCameraListDestroy(hCameraList);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to destroy camera list (system removal). Non-fatal error %d...\n\n", err);
        return;
    }
}

// This function checks if GEV enumeration is enabled on the system.
void CheckGevEnabled(spinSystem system)
{
    spinError err = SPINNAKER_ERR_SUCCESS;

    // Retrieve TL NodeMap from the system
    spinNodeMapHandle hNodeMapTLSystem = NULL;
    err = spinSystemGetTLNodeMap(system, &hNodeMapTLSystem);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve TL system nodemap. Error %d...\n\n", err);
        return;
    }

    // Retrieve EnumerateGEVInterfaces node
    spinNodeHandle hEnumerateGevInterfaces = NULL;
    err = spinNodeMapGetNode(hNodeMapTLSystem, "EnumerateGEVInterfaces", &hEnumerateGevInterfaces);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve EnumerateGEVInterfaces node. Error %d...\n\n", err);
        return;
    }

    // Ensure the node is valid
    bool8_t pbAvailable = False;
    err = spinNodeIsAvailable(hEnumerateGevInterfaces, &pbAvailable);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve node availability (EnumerateGEVInterfaces node), with error %d...\n\n", err);
        return;
    }

    bool8_t pbReadable = False;
    err = spinNodeIsReadable(hEnumerateGevInterfaces, &pbReadable);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve node readability (EnumerateGEVInterfaces node), with error %d...\n\n", err);
        return;
    }

    if (pbAvailable && pbReadable)
    {
        // Read the status of GEV enumeration node
        bool8_t gevEnabled = False;
        err = spinBooleanGetValue(hEnumerateGevInterfaces, &gevEnabled);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to retrieve EnumerateGEVInterfaces node value. Error %d...\n\n", err);
        }
        else if (!gevEnabled)
        {
            printf("WARNING: GEV Enumeration is disabled.\n");
            printf("If you intend to use GigE cameras please run the EnableGEVInterfaces shortcut\n");
            printf("or set EnumerateGEVInterfaces to true and relaunch your application.\n\n");
        }
        else
        {
            printf("EnumerateGEVInterfaces is enabled. Continuing..\n\n");
        }
    }
    else
    {
        printf("EnumerateGEVInterfaces node is unavailable.");
    }
}

// Example entry point; this function sets up the example to act appropriately
// upon arrival and removal events; please see Enumeration example for more
// in-depth comments on preparing and cleaning up the system.
int main(/*int argc, char** argv*/)
{
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

    // Check if GEV enumeration is enabled.
    CheckGevEnabled(hSystem);

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
    size_t numCameras = 0;

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

    err = spinCameraListGetSize(hCameraList, &numCameras);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve number of cameras. Aborting with  error %d...\n\n", err);
        return err;
    }

    printf("Number of cameras detected: %u\n\n", (unsigned int)numCameras);
    printf("\n*** CONFIGURE ENUMERATION EVENTS ***\n\n");

    //
    // Create interface event handler for the system
    //
    // *** NOTES ***
    // The function for the system has been constructed to accept a system in
    // order to print the number of cameras on the system. Notice that there
    // are 3 types of event handlers that can be created: arrival event handlers, removal event handlers,
    // and interface event handlers, which are a combination of arrival and removal
    // event handlers. Here, the an interface event handler is created, which requires both
    // an arrival event handler and a removal event handler object.
    //
    // *** LATER ***
    // In Spinnaker C, every event handler that is created must be destroyed to avoid
    // memory leaks.
    //
    spinInterfaceEventHandler interfaceEventHandlerSystem = NULL;

    err = spinInterfaceEventHandlerCreate(
        &interfaceEventHandlerSystem, onDeviceArrivalSystem, onDeviceRemovalSystem, (void*)hSystem);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to create interface event for system. Aborting with error %d...\n\n", err);
        return err;
    }

    printf("Interface event for system created...\n");

    //
    // Register interface event handler for the system
    //
    // *** NOTES ***
    // Arrival, removal, and interface event handlers can all be registered to
    // interfaces or the system. Do not think that interface event handlers can only be
    // registered to an interface.
    // Registering an interface event handler to the system is basically the same thing
    // as registering that event handler to all interfaces, with the added benefit of
    // not having to manage newly arrived or newly removed interfaces.
    // In order to manually manage newly arrived or removed interfaces, one would need
    // to implement interface arrival/removal event handlers, which are not yet supported
    // in the Spinnaker C API.
    //
    // *** LATER ***
    // Arrival, removal, and interface event handlers must all be unregistered manually.
    // This must be done prior to releasing the system and while they are still
    // in scope.
    //
    err = spinSystemRegisterInterfaceEventHandler(hSystem, interfaceEventHandlerSystem);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to register interface event on system. Aborting with error %d...\n\n", err);
        return err;
    }

    printf("Interface event registered to system...\n");

    // Wait for user to plug in and/or remove camera devices
    printf("Ready! Remove/Plug in cameras to test or press Enter to exit...\n");
    getchar();

    //
    // Unregister system event handler from system object
    //
    // *** NOTES ***
    // It is important to unregister all arrival, removal, and interface event handlers
    // registered to the system.
    //
    err = spinSystemUnregisterInterfaceEventHandler(hSystem, interfaceEventHandlerSystem);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to unregister interface event from system. Aborting with error %d...\n\n", err);
        return err;
    }

    printf("Event handlers unregistered from system...\n");

    //
    // Destroy interface event handlers
    //
    // *** NOTES ***
    // Event handlers must be destroyed in order to avoid memory leaks.
    //
    err = spinInterfaceEventHandlerDestroy(interfaceEventHandlerSystem);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to destroy interface event. Aborting with error %d...\n\n", err);
        return err;
    }

    printf("System event handler destroyed...\n");

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
        printf("Unable to release system instance. Aborting with  error %d...\n\n", err);
        return err;
    }

    printf("\nDone! Press Enter to exit...\n");
    getchar();

    return err;
}
