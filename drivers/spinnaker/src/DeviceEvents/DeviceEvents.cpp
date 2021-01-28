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
 *	@example DeviceEvents.cpp
 *
 *	@brief DeviceEvents.cpp shows how to create a handler to access device
 *	events. It relies on information provided in the Enumeration, Acquisition,
 *	and NodeMapInfo examples.
 *
 *	It can also be helpful to familiarize yourself with the NodeMapCallback
 *	example, as nodemap callbacks follow the same general procedure as
 *	events, but with a few less steps.
 *
 *	Device events can be thought of as camera-related events. This example
 *	creates a user-defined class, DeviceEventHandlerImpl, which allows the user to
 *	define any properties, parameters, and the event handler itself while DeviceEventHandler,
 *	the parent class, allows the child class to appropriately interface with
 *	the Spinnaker SDK.
 */

#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"
#include <iostream>
#include <sstream>

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;
using namespace std;

// Use the following enum and global constant to select whether the device
// event is registered universally to all events or specifically to exposure
// end events.
enum eventType
{
    GENERIC,
    SPECIFIC
};

const eventType chosenEvent = GENERIC;

// This class defines the properties, parameters, and the event itself. Take a
// moment to notice what parts of the class are mandatory, and what have been
// added for demonstration purposes. First, any class used to define device
// event handlers must inherit from DeviceEventHandler. Second, the method signature of
// OnDeviceEvent() must also be consistent. Everything else - including the
// constructor, destructor, properties, and body of OnDeviceEvent() - are
// particular to the example.
class DeviceEventHandlerImpl : public DeviceEventHandler
{
  public:
    // This constructor registers an event name to be used on device events.
    DeviceEventHandlerImpl(gcstring eventName)
    {
        m_eventName = eventName;
        m_count = 0;
    }
    ~DeviceEventHandlerImpl(){};

    // This method defines a device event. It checks that the event is of the
    // correct type and prints its name, ID, and count. It is important to note
    // that device events will be called only if enabled. Alternatively, this
    // example enables all device events and then registers a specific event by
    // name.
    void OnDeviceEvent(gcstring eventName)
    {
        // Check that device event is registered
        if (eventName == m_eventName)
        {
            // Print information on specified device event
            cout << "\tDevice event " << GetDeviceEventName() << " with ID " << GetDeviceEventId() << " number "
                 << ++m_count << "..." << endl;
        }
        else
        {
            // Print no information on non-specified event
            cout << "\tDevice event occurred; not " << m_eventName << "; ignoring..." << endl;
        }
    }

  private:
    int m_count;
    gcstring m_eventName;
};

// This function configures the example to execute device events by enabling all
// types of device events, and then creating and registering a device event that
// only concerns itself with an end of exposure event.
int ConfigureDeviceEvents(INodeMap& nodeMap, CameraPtr pCam, DeviceEventHandlerImpl*& deviceEventHandler)
{
    int result = 0;

    cout << endl << endl << "*** CONFIGURING DEVICE EVENTS ***" << endl << endl;

    try
    {
        //
        // Retrieve device event selector
        //
        // *** NOTES ***
        // Each type of device event must be enabled individually. This is done
        // by retrieving "EventSelector" (an enumeration node) and then enabling
        // the device event on "EventNotification" (another enumeration node).
        //
        // This example only deals with exposure end events. However, instead of
        // only enabling exposure end events with a simpler device event function,
        // all device events are enabled while the device event handler deals with
        // ensuring that only exposure end events are considered. A more standard
        // use-case might be to enable only the events of interest.
        //
        CEnumerationPtr ptrEventSelector = nodeMap.GetNode("EventSelector");
        if (!IsAvailable(ptrEventSelector) || !IsReadable(ptrEventSelector))
        {
            cout << "Unable to retrieve event selector entries. Aborting..." << endl << endl;
            return -1;
        }

        NodeList_t entries;
        ptrEventSelector->GetEntries(entries);

        cout << "Enabling event selector entries..." << endl;

        //
        // Enable device events
        //
        // *** NOTES ***
        // In order to enable a device event, the event selector and event
        // notification nodes (both of type enumeration) must work in unison.
        // The desired event must first be selected on the event selector node
        // and then enabled on the event notification node.
        //
        for (unsigned int i = 0; i < entries.size(); i++)
        {
            // Select entry on selector node
            CEnumEntryPtr ptrEnumEntry = entries.at(i);
            if (!IsAvailable(ptrEnumEntry) || !IsReadable(ptrEnumEntry))
            {
                // Skip if node fails
                continue;
            }

            ptrEventSelector->SetIntValue(ptrEnumEntry->GetValue());

            // Retrieve event notification node (an enumeration node)
            CEnumerationPtr ptrEventNotification = nodeMap.GetNode("EventNotification");
            if (!IsAvailable(ptrEventNotification) || !IsWritable(ptrEventNotification))
            {
                // Skip if node fails
                result = -1;
                continue;
            }

            // Retrieve entry node to enable device event
            CEnumEntryPtr ptrEventNotificationOn = ptrEventNotification->GetEntryByName("On");
            if (!IsAvailable(ptrEventNotification) || !IsReadable(ptrEventNotification))
            {
                // Skip if node fails
                result = -1;
                continue;
            }

            ptrEventNotification->SetIntValue(ptrEventNotificationOn->GetValue());

            cout << "\t" << ptrEnumEntry->GetDisplayName() << ": enabled..." << endl;
        }

        //
        // Create device event handler
        //
        // *** NOTES ***
        // The class has been designed to take in the name of an event. If all
        // events are registered generically, all event types will trigger a
        // device event; on the other hand, if an event is registered
        // specifically, only that event will trigger an event.
        //
        deviceEventHandler = new DeviceEventHandlerImpl("EventExposureEnd");

        //
        // Register device event handler
        //
        // *** NOTES ***
        // Device event handlers are registered to cameras. If there are multiple
        // cameras, each camera must have any device event handlers registered to it
        // separately. Note that multiple device event handlers may be registered to a
        // single camera.
        //
        // *** LATER ***
        // Device event handlers must be unregistered manually. This must be done prior
        // to releasing the system and while the device event handlers are still in
        // scope.
        //
        if (chosenEvent == GENERIC)
        {
            // Device event handlers registered generally will be triggered
            // by any device events.
            pCam->RegisterEventHandler(*deviceEventHandler);

            cout << "Device event handler registered generally..." << endl;
        }
        else if (chosenEvent == SPECIFIC)
        {
            // Device event handlers registered to a specified event will only
            // be triggered by the type of event is it registered to.
            pCam->RegisterEventHandler(*deviceEventHandler, "EventExposureEnd");

            cout << "Device event handler registered specifically to EventExposureEnd events..." << endl;
        }
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }

    return result;
}

// This function resets the example by unregistering the device event.
int ResetDeviceEvents(CameraPtr pCam, DeviceEventHandlerImpl*& deviceEventHandler)
{
    int result = 0;

    try
    {
        //
        // Unregister device event handler
        //
        // *** NOTES ***
        // It is important to unregister all device event handlers from all cameras that
        // they are registered to.
        //
        pCam->UnregisterEventHandler(*deviceEventHandler);

        // Delete device event handler (because it is a pointer)
        delete deviceEventHandler;

        cout << "Device event handler unregistered..." << endl << endl;
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }

    return result;
}

// This function prints the device information of the camera from the transport
// layer; please see NodeMapInfo example for more in-depth comments on printing
// device information from the nodemap.
int PrintDeviceInfo(INodeMap& nodeMap)
{
    int result = 0;

    cout << endl << "*** DEVICE INFORMATION ***" << endl << endl;

    try
    {
        FeatureList_t features;
        CCategoryPtr category = nodeMap.GetNode("DeviceInformation");
        if (IsAvailable(category) && IsReadable(category))
        {
            category->GetFeatures(features);

            FeatureList_t::const_iterator it;
            for (it = features.begin(); it != features.end(); ++it)
            {
                CNodePtr pfeatureNode = *it;
                cout << pfeatureNode->GetName() << " : ";
                CValuePtr pValue = (CValuePtr)pfeatureNode;
                cout << (IsReadable(pValue) ? pValue->ToString() : "Node not readable");
                cout << endl;
            }
        }
        else
        {
            cout << "Device control information not available." << endl;
        }
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }

    return result;
}

// This function acquires and saves 10 images from a device; please see
// Acquisition example for more in-depth comments on acquiring images.
int AcquireImages(CameraPtr pCam, INodeMap& nodeMap, INodeMap& nodeMapTLDevice)
{
    int result = 0;

    cout << endl << "*** IMAGE ACQUISITION ***" << endl << endl;

    try
    {
        // Set acquisition mode to continuous
        CEnumerationPtr ptrAcquisitionMode = nodeMap.GetNode("AcquisitionMode");
        if (!IsAvailable(ptrAcquisitionMode) || !IsWritable(ptrAcquisitionMode))
        {
            cout << "Unable to set acquisition mode to continuous (node retrieval). Aborting..." << endl << endl;
            return -1;
        }

        CEnumEntryPtr ptrAcquisitionModeContinuous = ptrAcquisitionMode->GetEntryByName("Continuous");
        if (!IsAvailable(ptrAcquisitionModeContinuous) || !IsReadable(ptrAcquisitionModeContinuous))
        {
            cout << "Unable to set acquisition mode to continuous (entry 'continuous' retrieval). Aborting..." << endl
                 << endl;
            return -1;
        }

        int64_t acquisitionModeContinuous = ptrAcquisitionModeContinuous->GetValue();

        ptrAcquisitionMode->SetIntValue(acquisitionModeContinuous);

        cout << "Acquisition mode set to continuous..." << endl;

        // Begin acquiring images
        pCam->BeginAcquisition();

        cout << "Acquiring images..." << endl;

        // Retrieve device serial number for filename
        gcstring deviceSerialNumber("");

        CStringPtr ptrStringSerial = nodeMapTLDevice.GetNode("DeviceSerialNumber");
        if (IsAvailable(ptrStringSerial) && IsReadable(ptrStringSerial))
        {
            deviceSerialNumber = ptrStringSerial->GetValue();

            cout << "Device serial number retrieved as " << deviceSerialNumber << "..." << endl;
        }
        cout << endl;

        // Retrieve, convert, and save images
        const unsigned int k_numImages = 10;

        for (unsigned int imageCnt = 0; imageCnt < k_numImages; imageCnt++)
        {
            try
            {
                // Retrieve next received image and ensure image completion
                ImagePtr pResultImage = pCam->GetNextImage(1000);

                if (pResultImage->IsIncomplete())
                {
                    cout << "Image incomplete with image status " << pResultImage->GetImageStatus() << "..." << endl
                         << endl;
                }
                else
                {
                    // Print image information
                    cout << "Grabbed image " << imageCnt << ", width = " << pResultImage->GetWidth()
                         << ", height = " << pResultImage->GetHeight() << endl;

                    // Convert image to mono 8
                    ImagePtr convertedImage = pResultImage->Convert(PixelFormat_Mono8, HQ_LINEAR);

                    // Create a unique filename
                    ostringstream filename;

                    filename << "DeviceEvents-";
                    if (deviceSerialNumber != "")
                    {
                        filename << deviceSerialNumber.c_str() << "-";
                    }
                    filename << imageCnt << ".jpg";

                    // Save image
                    convertedImage->Save(filename.str().c_str());

                    cout << "Image saved at " << filename.str() << endl;
                }

                // Release image
                pResultImage->Release();

                cout << endl;
            }
            catch (Spinnaker::Exception& e)
            {
                cout << "Error: " << e.what() << endl;
                result = -1;
            }
        }

        // End acquisition
        pCam->EndAcquisition();
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }

    return result;
}

// This function acts as the body of the example; please see NodeMapInfo example
// for more in-depth comments on setting up cameras.
int RunSingleCamera(CameraPtr pCam)
{
    int result = 0;
    int err = 0;

    try
    {
        // Retrieve TL device nodemap and print device information
        INodeMap& nodeMapTLDevice = pCam->GetTLDeviceNodeMap();

        result = PrintDeviceInfo(nodeMapTLDevice);

        // Initialize camera
        pCam->Init();

        // Retrieve GenICam nodemap
        INodeMap& nodeMap = pCam->GetNodeMap();

        // Configure device events
        DeviceEventHandlerImpl* deviceEventHandler = nullptr;

        err = ConfigureDeviceEvents(nodeMap, pCam, deviceEventHandler);
        if (err < 0)
        {
            return err;
        }

        // Acquire images
        result = result | AcquireImages(pCam, nodeMap, nodeMapTLDevice);

        // Reset device events
        result = result | ResetDeviceEvents(pCam, deviceEventHandler);

        // Deinitialize camera
        pCam->DeInit();
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }

    return result;
}

// Example entry point; please see Enumeration example for more in-depth
// comments on preparing and cleaning up the system.
int main(int /*argc*/, char** /*argv*/)
{
    // Since this application saves images in the current folder
    // we must ensure that we have permission to write to this folder.
    // If we do not have permission, fail right away.
    FILE* tempFile = fopen("test.txt", "w+");
    if (tempFile == nullptr)
    {
        cout << "Failed to create file in current folder.  Please check "
                "permissions."
             << endl;
        cout << "Press Enter to exit..." << endl;
        getchar();
        return -1;
    }
    fclose(tempFile);
    remove("test.txt");

    int result = 0;

    // Print application build information
    cout << "Application build date: " << __DATE__ << " " << __TIME__ << endl << endl;

    // Retrieve singleton reference to system object
    SystemPtr system = System::GetInstance();

    // Print out current library version
    const LibraryVersion spinnakerLibraryVersion = system->GetLibraryVersion();
    cout << "Spinnaker library version: " << spinnakerLibraryVersion.major << "." << spinnakerLibraryVersion.minor
         << "." << spinnakerLibraryVersion.type << "." << spinnakerLibraryVersion.build << endl
         << endl;

    // Retrieve list of cameras from the system
    CameraList camList = system->GetCameras();

    unsigned int numCameras = camList.GetSize();

    cout << "Number of cameras detected: " << numCameras << endl << endl;

    // Finish if there are no cameras
    if (numCameras == 0)
    {
        // Clear camera list before relasing system
        camList.Clear();

        // Release system
        system->ReleaseInstance();

        cout << "Not enough cameras!" << endl;
        cout << "Done! Press Enter to exit..." << endl;
        getchar();

        return -1;
    }

    // Run example on each camera
    for (unsigned int i = 0; i < numCameras; i++)
    {
        cout << endl << "Running example for camera " << i << "..." << endl;

        result = result | RunSingleCamera(camList.GetByIndex(i));

        cout << "Camera " << i << " example complete..." << endl << endl;
    }

    // Clear camera list before releasing system
    camList.Clear();

    // Release system
    system->ReleaseInstance();

    cout << endl << "Done! Press Enter to exit..." << endl;
    getchar();

    return result;
}