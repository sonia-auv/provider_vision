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
 *  @example GenTLInfo_QuickSpin.cpp
 *
 *  @brief GenTLInfo_QuickSpin.cpp shows how to access node information from
 *	interfaces and cameras in C++ with the QuickSpin API. QuickSpin is a subset
 *	of Spinnaker that eases access to camera information via direct node access.
 *	If you're not already familiar with the basics of Spinnaker, we suggest
 *	starting with the Enumeration_QuickSpin example.
 *
 *	The example demonstrates the retrieval of information from interface,
 *	transport layer device, transport layer stream, and application layer nodes.
 *	The retrieval of information of different data types is also touched on.
 *
 *  A much wider range of topics is covered in the full Spinnaker examples than
 *  in the QuickSpin ones. There are only enough QuickSpin examples to
 *  demonstrate node access and to get started with the API; please see full
 *  Spinnaker examples for further or specific knowledge on a topic.
 */

#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace std;

// This function prints device information from the transport layer.
int PrintTransportLayerDeviceInfo(CameraPtr pCamera)
{
    int result = 0;

    try
    {
        //
        // Print device information from the transport layer
        //
        // *** NOTES ***
        // In QuickSpin, accessing device information on the transport layer is
        // accomplished via a camera's TLDevice property. The TLDevice property
        // houses nodes related to general device information such as the three
        // demonstrated below, device access status, XML and GUI paths and
        // locations, and GEV information to name a few. The TLDevice property
        // allows access to nodes that would generally be retrieved through the
        // GenTL nodemap in full Spinnaker.
        //
        // Notice that each node is checked for availability and readability
        // prior to value retrieval. Checking for availability and readability
        // (or writability when applicable) whenever a node is accessed is
        // important in terms of error handling. If a node retrieval error
        // occurs but remains unhandled, an exception is thrown.
        //
        // Print device serial number
        cout << "Device serial number: ";

        if (IsReadable(pCamera->TLDevice.DeviceSerialNumber))
        {
            cout << pCamera->TLDevice.DeviceSerialNumber.ToString() << endl;
        }
        else
        {
            cout << "unavailable" << endl;
            result = -1;
        }

        // Print device vendor name
        cout << "Device vendor name: ";

        if (IsReadable(pCamera->TLDevice.DeviceVendorName))
        {
            cout << pCamera->TLDevice.DeviceVendorName.ToString() << endl;
        }
        else
        {
            cout << "unavailable" << endl;
            result = -1;
        }

        // Print device display name
        cout << "Device display name: ";

        if (IsReadable(pCamera->TLDevice.DeviceDisplayName))
        {
            cout << pCamera->TLDevice.DeviceDisplayName.ToString() << endl << endl;
        }
        else
        {
            cout << "unavailable" << endl << endl;
            result = -1;
        }
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }

    return result;
}

// This function prints stream information from the transport layer.
int PrintTransportLayerStreamInfo(CameraPtr pCamera)
{
    int result = 0;

    try
    {
        //
        // Print stream information from the transport layer
        //
        // *** NOTES ***
        // In QuickSpin, accessing stream information on the transport layer is
        // accomplished via a camera's TLStream property. The TLStream property
        // houses nodes related to streaming such as the two demonstrated below,
        // buffer information, and GEV packet information to name a few. The
        // TLStream property allows access to nodes that would generally be
        // retrieved through the stream nodemap in full Spinnaker.
        //
        // Print stream ID
        cout << "Stream ID: ";

        if (IsReadable(pCamera->TLStream.StreamID))
        {
            cout << pCamera->TLStream.StreamID.ToString() << endl;
        }
        else
        {
            cout << "unavailable" << endl;
            result = -1;
        }

        // Print stream type
        cout << "Stream type: ";

        if (IsReadable(pCamera->TLStream.StreamType))
        {
            cout << pCamera->TLStream.StreamType.ToString() << endl << endl;
        }
        else
        {
            cout << "unavailable" << endl << endl;
            result = -1;
        }
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }

    return result;
}

// This function prints information about the interface.
int PrintTransportLayerInterfaceInfo(InterfacePtr pInterface)
{
    int result = 0;

    try
    {
        //
        // Print stream information from the transport layer
        //
        // *** NOTES ***
        // In QuickSpin, accessing interface information is accomplished via an
        // interface's TLInterface property. The TLInterface property houses
        // nodes that hold information about the interface such as the three
        // demonstrated below, other general interface information, and
        // GEV addressing information. The TLInterface property allows access to
        // nodes that would generally be retrieved through the interface nodemap
        // in full Spinnaker.
        //
        // Interface nodes should also always be checked for availability and
        // readability (or writability when applicable). If a node retrieval
        // error occurs but remains unhandled, an exception is thrown.
        //
        // Print interface display name
        cout << "Interface display name: ";
        if (IsReadable(pInterface->TLInterface.InterfaceDisplayName))
        {
            cout << pInterface->TLInterface.InterfaceDisplayName.ToString() << endl;
        }
        else
        {
            cout << "Unavailable" << endl;
            result = -1;
        }

        // Print interface ID
        cout << "Interface ID: ";
        if (IsReadable(pInterface->TLInterface.InterfaceID))
        {
            cout << pInterface->TLInterface.InterfaceID.ToString() << endl;
        }
        else
        {
            cout << "Unavailable" << endl;
            result = -1;
        }

        // Print interface type
        cout << "Interface type: ";
        if (IsReadable(pInterface->TLInterface.InterfaceType))
        {
            cout << pInterface->TLInterface.InterfaceType.ToString() << endl << endl;
        }
        else
        {
            cout << "Unavailable" << endl << endl;
            result = -1;
        }
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }

    return result;
}

// This function prints device information from the application layer.
int PrintApplicationLayerDeviceInfo(CameraPtr pCamera)
{
    int result = 0;

    try
    {
        //
        // Print device information from the application layer
        //
        // *** NOTES ***
        // Most camera interaction happens on the application layer. The
        // advantages of these nodes is that there is a lot more of them, they
        // allow for a much deeper level of interaction with a camera, and
        // no intermediate property (i.e. TLDevice or TLStream) is required.
        // The disadvantage is that they require initialization.
        //
        // Print exposure time; exposure time recorded in microseconds
        cout << "Exposure time: ";

        if (IsReadable(pCamera->ExposureTime))
        {
            cout << pCamera->ExposureTime.ToString() << endl;
        }
        else
        {
            cout << "unavailable" << endl;
            result = -1;
        }

        // Print black level; black level recorded as a percentage
        cout << "Black level: ";

        if (IsReadable(pCamera->BlackLevel))
        {
            cout << pCamera->BlackLevel.ToString() << endl;
        }
        else
        {
            cout << "unavailable" << endl;
            result = -1;
        }

        // Print height; height recorded in pixels
        cout << "Height: ";

        if (IsReadable(pCamera->Height))
        {
            cout << pCamera->Height.ToString() << endl << endl;
        }
        else
        {
            cout << "unavailable" << endl << endl;
            result = -1;
        }
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }

    return result;
}

// Example entry point; this function prints transport layer information from
// each interface and transport and application layer information from each
// camera.
int main(int /*argc*/, char** /*argv*/)
{
    int result = 0;

    // Print application build information
    cout << "Application build date: " << __DATE__ << " " << __TIME__ << endl << endl;

    //
    // Retrieve singleton reference to system object
    //
    // *** NOTES ***
    // Everything originates from the system. Notice that it is implemented as
    // a singleton object, making it impossible to have more than one system.
    //
    // *** LATER ***
    // The system object should be cleared prior to program completion. If not
    // released explicitly, it will release itself automatically.
    //
    SystemPtr system = System::GetInstance();

    // Retrieve list of interfaces from the system
    CameraList camList = system->GetCameras();

    unsigned int numCameras = camList.GetSize();

    cout << "Number of cameras detected: " << numCameras << endl << endl;

    // Retrieve list of cameras from the system
    InterfaceList interfaceList = system->GetInterfaces();

    unsigned int numInterfaces = interfaceList.GetSize();

    cout << "Number of interfaces detected: " << numInterfaces << endl << endl;

    //
    // Print information on each interface
    //
    // *** NOTES ***
    // All USB 3 Vision and GigE Vision interfaces should enumerate for
    // Spinnaker.
    //
    cout << endl << "*** PRINTING INTERFACE INFORMATION ***" << endl << endl;

    for (unsigned int i = 0; i < numInterfaces; i++)
    {
        result = result | PrintTransportLayerInterfaceInfo(interfaceList.GetByIndex(i));
    }

    //
    // Print general device information on each camera from transport layer
    //
    // *** NOTES ***
    // Transport layer nodes do not require initialization in order to interact
    // with them.
    //
    cout << endl << "*** PRINTING TRANSPORT LAYER DEVICE INFORMATION ***" << endl << endl;

    for (unsigned int i = 0; i < numCameras; i++)
    {
        result = result | PrintTransportLayerDeviceInfo(camList.GetByIndex(i));
    }

    //
    // Print streaming information on each camera from transport layer
    //
    // *** NOTES ***
    // Again, initialization is not required to print information from the
    // transport layer; this is equally true of streaming information.
    //
    cout << endl << "*** PRINTING TRANSPORT LAYER STREAMING INFORMATION ***" << endl << endl;

    for (unsigned int i = 0; i < numCameras; i++)
    {
        result = result | PrintTransportLayerStreamInfo(camList.GetByIndex(i));
    }

    //
    // Print device information on each camera from application layer
    //
    // *** NOTES ***
    // Application layer nodes require initialization in order to interact with
    // them; as such, this loop initializes the camera, prints some information
    // from the application layer, and then deinitializes it. If the camera were
    // not initialized, node availability would fail.
    //
    cout << endl << "*** PRINTING APPLICATION LAYER INFORMATION ***" << endl << endl;

    for (unsigned int i = 0; i < numCameras; i++)
    {
        cout << "Device: " << i << endl;
        try
        {
            // Initialize camera
            camList.GetByIndex(i)->Init();
        }
        catch (Spinnaker::Exception& e)
        {
            // Report the error and continue to the next device
            cout << "Error: " << e.what() << endl << endl;
            result = -1;
            continue;
        }

        // Print information
        result = result | PrintApplicationLayerDeviceInfo(camList.GetByIndex(i));

        // Deinitialize camera
        camList.GetByIndex(i)->DeInit();
    }

    // Clear camera list before releasing system
    camList.Clear();

    // Clear interface list before releasing system
    interfaceList.Clear();

    // Release system
    system->ReleaseInstance();

    cout << endl << "Done! Press Enter to exit..." << endl;
    getchar();

    return result;
}
