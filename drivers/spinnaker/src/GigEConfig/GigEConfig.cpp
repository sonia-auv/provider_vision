//=============================================================================
// Copyright (c) 2001-2020 FLIR Systems, Inc. All Rights Reserved.
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
 *  @example GigEConfig.cpp
 *
 *  @brief GigEConfig.cpp shows how to enumerate GigE adapters and GigE cameras
 *  and display information about the cameras. This example also demonstrates
 *  how to force the cameras to be in the proper subnet or individually configure
 *  a camera's current/persistent IP address, subnet mask and default gateway given
 *  the camera's serial number.
 */

#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <cmath>

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;
using namespace std;

// Constants for validating proper user specified inputs
const std::string zeroSerial = "0";
const std::string zeroIPAddress = "0.0.0.0";
const std::string broadcastAddress = "255.255.255.255";

void Usage()
{
    cout << endl << "Usage:" << endl << endl;
    cout << "GigEConfig [-a | -s <serial number> | -m <MAC address>]" << endl;
    cout << "GigEConfig [-s <serial number> -i <IP address> -n <netmask> -g <gateway>]" << endl;
    cout << endl << "Options:" << endl;
    cout << "No arguments will list all discoverable cameras" << endl;
    cout << "-a : Auto-configure all discoverable cameras" << endl;
    cout << "-s : Serial number is used to specify camera" << endl;
    cout << "-m : MAC address is used to specify camera" << endl;
    cout << "     Bytes of MAC address must be separated by either a ':' or '-' character" << endl;
    cout << "-i : IP address to assign to camera. Cannot be " << zeroIPAddress << " or " << broadcastAddress << "."
         << endl;
    cout << "-n : Subnet mask to assign to camera. Cannot be " << zeroIPAddress << "." << endl;
    cout << "-g : Default gateway to assign to camera. Cannot be " << zeroIPAddress << " or " << broadcastAddress << "."
         << endl;
    cout << endl << endl;
}

gcstring GetDottedAddress(int64_t value)
{
    // Helper function for formatting IP Address into the following format
    // x.x.x.x
    unsigned int inputValue = static_cast<unsigned int>(value);
    ostringstream convertValue;
    convertValue << ((inputValue & 0xFF000000) >> 24);
    convertValue << ".";
    convertValue << ((inputValue & 0x00FF0000) >> 16);
    convertValue << ".";
    convertValue << ((inputValue & 0x0000FF00) >> 8);
    convertValue << ".";
    convertValue << (inputValue & 0x000000FF);
    return convertValue.str().c_str();
}

gcstring GetMACAddress(int64_t value)
{
    // Helper function for formatting MAC Address into the following format
    // XX:XX:XX:XX:XX:XX
    long long inputValue = static_cast<long long>(value);
    ostringstream convertValue;
    convertValue << std::uppercase << std::setfill('0') << std::setw(2) << std::hex
                 << ((inputValue & 0xFF0000000000) >> 40);
    convertValue << ":";
    convertValue << std::uppercase << std::setfill('0') << std::setw(2) << std::hex
                 << ((inputValue & 0x00FF00000000) >> 32);
    convertValue << ":";
    convertValue << std::uppercase << std::setfill('0') << std::setw(2) << std::hex
                 << ((inputValue & 0x0000FF000000) >> 24);
    convertValue << ":";
    convertValue << std::uppercase << std::setfill('0') << std::setw(2) << std::hex
                 << ((inputValue & 0x000000FF0000) >> 16);
    convertValue << ":";
    convertValue << std::uppercase << std::setfill('0') << std::setw(2) << std::hex
                 << ((inputValue & 0x00000000FF00) >> 8);
    convertValue << ":";
    convertValue << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << (inputValue & 0x0000000000FF);
    return convertValue.str().c_str();
}

std::string GetMACAddress(const char* macAddrString)
{
    // Helper function for replacing - with : in the MAC Address
    std::string macAddress = macAddrString;
    std::replace(macAddress.begin(), macAddress.end(), '-', ':');
    return macAddress;
}

vector<string> split(const string& str)
{
    // Helper function for splitting string with octets (e.g. x.x.x.x) into
    // a vector of octets with '.' as the delimiter
    string delim = ".";
    vector<string> tokens;
    size_t prev = 0, pos = 0;
    do
    {
        pos = str.find(delim, prev);
        if (pos == string::npos)
        {
            pos = str.length();
        }

        string token = str.substr(prev, pos - prev);
        if (!token.empty())
        {
            tokens.push_back(token);
        }
        prev = pos + delim.length();
    } while (pos < str.length() && prev < str.length());

    return tokens;
}

string IPV4ToLongString(string ipAddress)
{
    // Helper function for converting IPV4 to long address representation. The function
    // expects the input IP string to be in the form of 4 octects (e.g. x.x.x.x).
    vector<string> ipBytes;
    uint64_t num = 0;
    if (!ipAddress.empty())
    {
        ipBytes = split(ipAddress);
        for (int i = ipBytes.size() - 1; i >= 0; i--)
        {
            num += ((std::atoi(ipBytes[i].c_str()) % 256) * pow(256, (3 - i)));
        }
    }

    return std::to_string(num);
}

bool IsDigits(const std::string& str)
{
    return str.find_first_not_of("0123456789") == std::string::npos;
}

bool ValidateIPV4Address(string ipAddress, bool isSubnetMask = false)
{
    vector<string> ipBytes;
    uint64_t num = 0;

    if (ipAddress.empty() || ipAddress.compare(zeroIPAddress) == 0 ||
        (!isSubnetMask && ipAddress.compare(broadcastAddress) == 0))
    {
        return false;
    }

    // Ensure that the IP address has 4 blocks
    ipBytes = split(ipAddress);
    if (ipBytes.size() != 4)
    {
        return false;
    }

    // Ensure that each block is valid
    for (int i = ipBytes.size() - 1; i >= 0; i--)
    {
        if (ipBytes[i].empty() || !IsDigits(ipBytes[i]))
        {
            return false;
        }

        num = std::atoi(ipBytes[i].c_str());
        if (num < 0 || num > 255)
        {
            return false;
        }
    }

    return true;
}

void PrintDeviceInfo(CameraPtr pCamera)
{
    try
    {
        // Retrieve TL device nodemap; please see NodeMapInfo example for
        // additional comments on transport layer nodemaps
        INodeMap& nodeMapTLDevice = pCamera->GetTLDeviceNodeMap();

        // Print device vendor name and device model name
        //
        // *** NOTES ***
        // Grabbing node information requires first retrieving the node and
        // then retrieving its information. There are two things to keep in
        // mind. First, a node is distinguished by type, which is related
        // to its value's data type. Second, nodes should be checked for
        // availability and readability/writability prior to making an
        // attempt to read from or write to the node.
        //
        CStringPtr ptrStringSerial = nodeMapTLDevice.GetNode("DeviceSerialNumber");
        if (IsAvailable(ptrStringSerial) && IsReadable(ptrStringSerial))
        {
            gcstring deviceSerialNumber = ptrStringSerial->GetValue();
            cout << "DeviceSerialNumber : " << deviceSerialNumber << endl;
        }

        CStringPtr ptrDeviceModelName = nodeMapTLDevice.GetNode("DeviceModelName");
        if (IsAvailable(ptrDeviceModelName) && IsReadable(ptrDeviceModelName))
        {
            gcstring deviceModelName = ptrDeviceModelName->ToString();
            cout << "DeviceModelName : " << deviceModelName << endl;
        }

        CStringPtr ptrDeviceVendorName = nodeMapTLDevice.GetNode("DeviceVendorName");
        if (IsAvailable(ptrDeviceVendorName) && IsReadable(ptrDeviceVendorName))
        {
            gcstring deviceVendorName = ptrDeviceVendorName->ToString();
            cout << "DeviceVendorName : " << deviceVendorName << endl;
        }

        CIntegerPtr ptrIPAddress = nodeMapTLDevice.GetNode("GevDeviceIPAddress");
        if (IsAvailable(ptrIPAddress) && IsReadable(ptrIPAddress))
        {
            gcstring deviceIPAddress = GetDottedAddress(ptrIPAddress->GetValue());
            cout << "GevDeviceIPAddress : " << deviceIPAddress << endl;
        }

        CIntegerPtr ptrSubnetMask = nodeMapTLDevice.GetNode("GevDeviceSubnetMask");
        if (IsAvailable(ptrSubnetMask) && IsReadable(ptrSubnetMask))
        {
            gcstring deviceSubnetMask = GetDottedAddress(ptrSubnetMask->GetValue());
            cout << "GevDeviceSubnetMask : " << deviceSubnetMask << endl;
        }

        CIntegerPtr ptrGateway = nodeMapTLDevice.GetNode("GevDeviceGateway");
        if (IsAvailable(ptrGateway) && IsReadable(ptrGateway))
        {
            gcstring deviceGateway = GetDottedAddress(ptrGateway->GetValue());
            cout << "GevDeviceGateway : " << deviceGateway << endl;
        }

        CIntegerPtr ptrMACAddress = nodeMapTLDevice.GetNode("GevDeviceMACAddress");
        if (IsAvailable(ptrMACAddress) && IsReadable(ptrMACAddress))
        {
            gcstring deviceMACAddress = GetMACAddress(ptrMACAddress->GetValue());
            cout << "GevDeviceMACAddress : " << deviceMACAddress << endl;
        }

        try
        {
            // Initialize Camera
            pCamera->Init();

            // Retrieve device nodemap
            INodeMap& nodeMapDevice = pCamera->GetNodeMap();

            CIntegerPtr ptrPersistentIPAddress = nodeMapDevice.GetNode("GevPersistentIPAddress");
            if (IsAvailable(ptrPersistentIPAddress) && IsReadable(ptrPersistentIPAddress))
            {
                gcstring devicePersistentIPAddress = GetDottedAddress(ptrPersistentIPAddress->GetValue());
                cout << "GevPersistentIPAddress : " << devicePersistentIPAddress << endl;
            }

            CIntegerPtr ptrPersistentSubnetMask = nodeMapDevice.GetNode("GevPersistentSubnetMask");
            if (IsAvailable(ptrPersistentSubnetMask) && IsReadable(ptrPersistentSubnetMask))
            {
                gcstring devicePersistentSubnetMask = GetDottedAddress(ptrPersistentSubnetMask->GetValue());
                cout << "GevPersistentSubnetMask : " << devicePersistentSubnetMask << endl;
            }

            CIntegerPtr ptrPersistentGateway = nodeMapDevice.GetNode("GevPersistentDefaultGateway");
            if (IsAvailable(ptrPersistentGateway) && IsReadable(ptrPersistentGateway))
            {
                gcstring devicePersistentGateway = GetDottedAddress(ptrPersistentGateway->GetValue());
                cout << "GevPersistentDefaultGateway : " << devicePersistentGateway << endl;
            }

            // Deinitialize camera
            pCamera->DeInit();
        }
        catch (Spinnaker::Exception& se)
        {
            if (se == SPINNAKER_ERR_INVALID_ADDRESS)
            {
                cout << "Warning: Camera is on a wrong subnet. Run auto force IP (with -a option) to configure the "
                        "camera correctly."
                     << endl;
            }
            else
            {
                cout << se.what() << endl;
            }
        }
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
    }
}

void QueryInterface(InterfacePtr pInterface)
{
    //
    // Retrieve TL nodemap from interface
    //
    // *** NOTES ***
    // Each interface has a nodemap that can be retrieved in order to
    // access information about the interface itself, any devices
    // connected, or addressing information if applicable.
    //
    INodeMap& nodeMapInterface = pInterface->GetTLNodeMap();

    CEnumerationPtr ptrInterfaceType = nodeMapInterface.GetNode("InterfaceType");
    if (IsAvailable(ptrInterfaceType) && IsReadable(ptrInterfaceType))
    {
        if (ptrInterfaceType->GetIntValue() != InterfaceType_GigEVision)
        {
            // Only display GEV interface and devices
            return;
        }
    }

    //
    // Print interface display name
    //
    // *** NOTES ***
    // Grabbing node information requires first retrieving the node and
    // then retrieving its information. There are two things to keep in
    // mind. First, a node is distinguished by type, which is related
    // to its value's data type. Second, nodes should be checked for
    // availability and readability/writability prior to making an
    // attempt to read from or write to the node.
    //
    CStringPtr ptrInterfaceDisplayName = nodeMapInterface.GetNode("InterfaceDisplayName");

    if (IsAvailable(ptrInterfaceDisplayName) && IsReadable(ptrInterfaceDisplayName))
    {
        gcstring interfaceDisplayName = ptrInterfaceDisplayName->GetValue();
        cout << "*** " << interfaceDisplayName << " ***" << endl;
    }
    else
    {
        cout << "*** Unknown Interface (Display name not readable) ***" << endl;
    }

    //
    // Retrieve list of cameras from the interface
    //
    // *** NOTES ***
    // Camera lists can be retrieved from an interface or the system object.
    // Camera lists retrieved from an interface, such as this one, only
    // return cameras attached on that specific interface whereas camera
    // lists retrieved from the system will return all cameras on all
    // interfaces. Cameras inside and outside of the subnet will be enumerated
    // in the list. An exception will be thrown when initializing a camera
    // outside of the subnet.
    //
    // *** LATER ***
    // Camera lists must be cleared manually. This must be done prior to
    // releasing the system and while the camera list is still in scope.
    //
    CameraList camList = pInterface->GetCameras();

    // Retrieve number of cameras
    unsigned int numCameras = camList.GetSize();

    // Return if no cameras detected
    if (numCameras == 0)
    {
        cout << "No devices detected." << endl << endl;
        return;
    }

    // Print device vendor and model name for each camera on the interface
    for (unsigned int i = 0; i < numCameras; i++)
    {
        //
        // Select camera
        //
        // *** NOTES ***
        // Each camera is retrieved from a camera list with an index. If
        // the index is out of range, an exception is thrown.
        //
        CameraPtr pCamera = camList.GetByIndex(i);

        cout << "** Device " << i << " **" << endl;

        PrintDeviceInfo(pCamera);

        cout << endl;
    }

    //
    // Clear camera list before losing scope
    //
    // *** NOTES ***
    // Camera lists (and interface lists) must be cleared manually while in
    // the same scope that the system is released. However, in cases like this
    // where scope is lost, camera lists (and interface lists) will be cleared
    // automatically.
    //
    camList.Clear();
}

void ListAllDeviceInfo()
{
    //
    // Retrieve singleton reference to system object
    //
    // *** NOTES ***
    // Everything originates with the system object. It is important to notice
    // that it has a singleton implementation, so it is impossible to have
    // multiple system objects at the same time. Users can only get a smart
    // pointer (SystemPtr) to the system instance.
    //
    // *** LATER ***
    // The system object should be cleared prior to program completion. If not
    // released explicitly, it will be released automatically when all SystemPtr
    // objects that point to the system go out of scope.
    //
    SystemPtr system = System::GetInstance();

    cout << "---> Listing all discoverable cameras..." << endl << endl;

    //
    // Retrieve list of interfaces from the system
    //
    // *** NOTES ***
    // Interface lists are retrieved from the system object.
    //
    // *** LATER ***
    // Interface lists must be cleared manually. This must be done prior to
    // releasing the system and while the interface list is still in scope.
    //
    InterfaceList interfaceList = system->GetInterfaces();

    unsigned int numInterfaces = interfaceList.GetSize();

    //
    // Create shared pointer interface
    //
    // *** NOTES ***
    // The InterfacePtr object is a smart pointer, and will generally clean
    // itself up upon exiting its scope.
    //
    // *** LATER ***
    // However, if a smart interface pointer is created in the same scope that
    // a system object is explicitly released (i.e. this scope), the reference to
    // the interface must be broken by manually setting the pointer to NULL.
    //
    InterfacePtr interfacePtr = nullptr;

    for (unsigned int i = 0; i < numInterfaces; i++)
    {
        // Select interface
        interfacePtr = interfaceList.GetByIndex(i);

        // Query interface
        QueryInterface(interfacePtr);
    }

    //
    // Release reference to the interface
    //
    // *** NOTES ***
    // Had the InterfacePtr object been created within the for-loop, it would
    // not be necessary to manually break the reference because the smart
    // pointer would have automatically cleaned itself up upon exiting the loop.
    //
    interfacePtr = nullptr;

    //
    // Clear interface list before releasing system
    //
    // *** NOTES ***
    // Interface lists must be cleared manually prior to a system release call.
    //
    interfaceList.Clear();

    //
    // Release system
    //
    // *** NOTES ***
    // The system should be released, but if it is not, it will do so itself.
    // It is often at the release of the system (whether manual or automatic)
    // that unreleased resources and still registered events will throw an
    // exception.
    //
    system->ReleaseInstance();
}

void ListDeviceInfoBySerial(std::string serialNumber)
{
    cout << "---> Listing device info from serial number..." << endl << endl;

    // Retrieve singleton reference to system object
    SystemPtr system = System::GetInstance();

    // Retrieve list of cameras from the system. Cameras inside and outside of
    // the subnet will be enumerated in the list. An exception will be thrown
    // when initializing a camera outside of the subnet.
    CameraList camList = system->GetCameras();

    // Select camera
    CameraPtr pCamera = camList.GetBySerial(serialNumber);
    if (pCamera != nullptr)
    {
        PrintDeviceInfo(pCamera);
        cout << endl;
    }
    else
    {
        cout << "--> Could not discover camera with serial number: " << serialNumber.c_str() << endl;
    }
}

void ListDeviceInfoByMAC(std::string macAddress)
{
    cout << "---> Listing device info from MAC Address..." << endl << endl;

    // Retrieve singleton reference to system object
    SystemPtr system = System::GetInstance();

    // Retrieve list of cameras from the system. Cameras inside and outside of
    // the subnet will be enumerated in the list. An exception will be thrown
    // when initializing a camera outside of the subnet.
    CameraList camList = system->GetCameras();

    unsigned int numCameras = camList.GetSize();

    //
    // Create shared pointer to camera
    //
    // *** NOTES ***
    // The CameraPtr object is a shared pointer, and will generally clean itself
    // up upon exiting its scope. However, if a shared pointer is created in the
    // same scope that a system object is explicitly released (i.e. this scope),
    // the reference to the shared pointer must be broken manually.
    //
    // *** LATER ***
    // Shared pointers can be terminated manually by assigning them to NULL.
    // This prevents exceptions to be thrown when cleaning up the whole system.
    //
    CameraPtr pCamera = nullptr;

    bool bCameraFound = false;
    for (unsigned int i = 0; i < numCameras; i++)
    {
        //
        // Select camera
        //
        // *** NOTES ***
        // Each camera is retrieved from a camera list with an index. If
        // the index is out of range, an exception is thrown.
        //
        CameraPtr pCamera = camList.GetByIndex(i);

        // Retrieve TL device nodemap; please see NodeMapInfo example for
        // additional comments on transport layer nodemaps
        INodeMap& nodeMapTLDevice = pCamera->GetTLDeviceNodeMap();

        CIntegerPtr ptrMACAddress = nodeMapTLDevice.GetNode("GevDeviceMACAddress");
        if (IsAvailable(ptrMACAddress) && IsReadable(ptrMACAddress))
        {
            gcstring deviceMACAddress = GetMACAddress(ptrMACAddress->GetValue());
            if (deviceMACAddress.compare(macAddress.c_str()) == 0)
            {
                PrintDeviceInfo(pCamera);
                bCameraFound = true;
                cout << endl;
            }
        }
    }

    if (!bCameraFound)
    {
        cout << "--> Could not discover camera with MAC address: " << macAddress.c_str() << endl;
    }
}

void AutoConfigure()
{
    cout << "---> Setting all GigE cameras discovered to an IP configuration" << endl
         << "---> that will allow it to work with Spinnaker..." << endl
         << endl;

    //
    // Retrieve singleton reference to system object
    //
    // *** NOTES ***
    // Everything originates with the system object. It is important to notice
    // that it has a singleton implementation, so it is impossible to have
    // multiple system objects at the same time. Users can only get a smart
    // pointer (SystemPtr) to the system instance.
    //
    // *** LATER ***
    // The system object should be cleared prior to program completion. If not
    // released explicitly, it will be released automatically when all SystemPtr
    // objects that point to the system go out of scope.
    SystemPtr pSystem = System::GetInstance();

    //
    // Retrieve list of interfaces from the system
    //
    // *** NOTES ***
    // Interface lists are retrieved from the system object.
    //
    // *** LATER ***
    // Interface lists must be cleared manually. This must be done prior to
    // releasing the system and while the interface list is still in scope.
    //
    InterfaceList interfaceList = pSystem->GetInterfaces();

    // Iterate through available interfaces and attempt to execute force IP
    for (unsigned int i = 0; i < interfaceList.GetSize(); i++)
    {
        InterfacePtr pInterface = interfaceList.GetByIndex(i);

        //
        // Retrieve TL nodemap from interface
        //
        // *** NOTES ***
        // Each interface has a nodemap that can be retrieved in order to
        // access information about the interface itself, any devices
        // connected, or addressing information if applicable.
        //
        INodeMap& nodeMapInterface = pInterface->GetTLNodeMap();

        CEnumerationPtr ptrInterfaceType = nodeMapInterface.GetNode("InterfaceType");
        if (!IsAvailable(ptrInterfaceType) || !IsReadable(ptrInterfaceType))
        {
            cout << "Unable to read InterfaceType for interface at index " << i << endl;
            continue;
        }

        if (ptrInterfaceType->GetIntValue() != InterfaceType_GigEVision)
        {
            // Only force IP on GEV interface
            continue;
        }

        //
        // Print interface display name
        //
        // *** NOTES ***
        // Grabbing node information requires first retrieving the node and
        // then retrieving its information. There are two things to keep in
        // mind. First, a node is distinguished by type, which is related
        // to its value's data type. Second, nodes should be checked for
        // availability and readability/writability prior to making an
        // attempt to read from or write to the node.
        //
        CStringPtr ptrInterfaceDisplayName = nodeMapInterface.GetNode("InterfaceDisplayName");
        if (IsAvailable(ptrInterfaceDisplayName) && IsReadable(ptrInterfaceDisplayName))
        {
            gcstring interfaceDisplayName = ptrInterfaceDisplayName->GetValue();
            cout << "*** " << interfaceDisplayName << " ***" << endl;
        }
        else
        {
            cout << "*** "
                 << "Unknown Interface (Display name not readable)"
                 << " ***" << endl;
        }

        CCommandPtr ptrAutoForceIP = nodeMapInterface.GetNode("GevDeviceAutoForceIP");
        if (IsAvailable(ptrAutoForceIP) && IsWritable(ptrAutoForceIP))
        {
            if (!IsWritable(pInterface->TLInterface.DeviceSelector.GetAccessMode()))
            {
                cout << "Unable to write to the DeviceSelector node while forcing IP" << endl;
            }
            else
            {
                const int cameraCount = pInterface->GetCameras().GetSize();
                for (int i = 0; i < cameraCount; i++)
                {
                    pInterface->TLInterface.DeviceSelector.SetValue(i);
                    pInterface->TLInterface.GevDeviceAutoForceIP.Execute();
                    cout << "AutoForceIP executed for camera at index " << i << endl;
                }
            }
        }
        else
        {
            cout << "Warning : Force IP node not available for this interface" << endl;
        }

        cout << endl;
    }

    interfaceList.Clear();
    pSystem->ReleaseInstance();

    cout << "---> Auto-configuration complete" << endl << endl;
}

void ConfigureCamera(std::string serial, std::string ipAddress, std::string netMask, std::string gateway)
{
    // Retrieve singleton reference to system object
    SystemPtr system = System::GetInstance();

    // Retrieve list of cameras from the system
    CameraList camList = system->GetCameras();

    CameraPtr pCam = nullptr;
    pCam = camList.GetBySerial(serial);

    if (pCam == nullptr)
    {
        cout << "Error: Could not find a camera with serial number: " << serial << endl;
    }
    else
    {
        try
        {
            // Initialize camera
            pCam->Init();

            // Retrieve GenICam nodemap
            INodeMap& nodeMap = pCam->GetNodeMap();

            // Retrieve TL device nodemap
            INodeMap& nodeMapTLDevice = pCam->GetTLDeviceNodeMap();

            CEnumerationPtr ptrDeviceType = nodeMapTLDevice.GetNode("DeviceType");

            if (ptrDeviceType->GetIntValue() != DeviceType_GigEVision)
            {
                cout << "Warning: Persistent IP can only be set for GigE vision cameras" << endl << endl;
            }
            else
            {
                // Find the current IP address
                CIntegerPtr gevCurrentIPAddress = nodeMap.GetNode("GevCurrentIPAddress");
                if (!IsReadable(gevCurrentIPAddress))
                {
                    cout << "Error: Cannot read the current IP address" << endl;
                }
                else
                {
                    cout << "Current IP Address is "
                         << GetDottedAddress(gevCurrentIPAddress->GetValue(gevCurrentIPAddress)) << endl;
                }

                // Enable persistent IP
                CBooleanPtr gevCurrentIPConfigurationPersistentIP =
                    nodeMap.GetNode("GevCurrentIPConfigurationPersistentIP");
                if (!IsWritable(gevCurrentIPConfigurationPersistentIP))
                {
                    cout << "Error: Cannot enable persistent IP address" << endl;
                }
                else
                {
                    gevCurrentIPConfigurationPersistentIP->SetValue(true);
                    cout << "Persistent IP enabled set to " << gevCurrentIPConfigurationPersistentIP->GetValue()
                         << endl;
                }

                // Set and force a persistent IP address
                CIntegerPtr gevPersistentIPAddress = nodeMap.GetNode("GevPersistentIPAddress");
                CIntegerPtr gevDeviceForceIPAddress = nodeMapTLDevice.GetNode("GevDeviceForceIPAddress");
                if (!IsWritable(gevPersistentIPAddress) || !IsWritable(gevDeviceForceIPAddress))
                {
                    cout << "Error: Cannot force a persistent IP address value" << endl;
                }
                else
                {
                    uint64_t ipValue = atol(IPV4ToLongString(ipAddress).c_str());
                    gevPersistentIPAddress->SetValue(ipValue);
                    gevDeviceForceIPAddress->SetValue(ipValue);
                    cout << "Persistent IP address set to " << GetDottedAddress(gevPersistentIPAddress->GetValue())
                         << endl;
                }

                // Set and force a persistent subnet mask
                CIntegerPtr gevPersistentSubnetMask = nodeMap.GetNode("GevPersistentSubnetMask");
                CIntegerPtr gevDeviceForceSubnetMask = nodeMapTLDevice.GetNode("GevDeviceForceSubnetMask");
                if (!IsWritable(gevPersistentSubnetMask) || !IsWritable(gevDeviceForceSubnetMask))
                {
                    cout << "Error: Cannot force a persistent subnet mask value" << endl;
                }
                else
                {
                    uint64_t netMaskValue = atol(IPV4ToLongString(netMask).c_str());
                    gevPersistentSubnetMask->SetValue(netMaskValue);
                    gevDeviceForceSubnetMask->SetValue(netMaskValue);
                    cout << "Persistent subnet mask set to " << GetDottedAddress(gevPersistentSubnetMask->GetValue())
                         << endl;
                }

                // Set and force a persistent default gateway
                CIntegerPtr gevPersistentDefaultGateway = nodeMap.GetNode("GevPersistentDefaultGateway");
                CIntegerPtr gevDeviceForceGateway = nodeMapTLDevice.GetNode("GevDeviceForceGateway");
                if (!IsWritable(gevPersistentDefaultGateway) || !IsWritable(gevDeviceForceGateway))
                {
                    cout << "Error: Cannot set and force a persistent default gateway value" << endl;
                }
                else
                {
                    uint64_t defaultGatewayValue = atol(IPV4ToLongString(gateway).c_str());
                    gevPersistentDefaultGateway->SetValue(defaultGatewayValue);
                    gevDeviceForceGateway->SetValue(defaultGatewayValue);
                    cout << "Persistent default gateway set to "
                         << GetDottedAddress(gevPersistentDefaultGateway->GetValue()) << endl;
                }

                // De-initialize the camera
                pCam->DeInit();

                CCommandPtr gevDeviceForceIP = nodeMapTLDevice.GetNode("GevDeviceForceIP");
                if (!IsWritable(gevDeviceForceIP))
                {
                    cout << "Error: Cannot execute device force IP command" << endl;
                }
                else
                {
                    gevDeviceForceIP->Execute();
                    cout << "Device force IP command executed" << endl;
                }
            }

            // Ensure that the device is de-initialized
            if (pCam->IsInitialized())
            {
                // De-initialize the camera
                pCam->DeInit();
            }
        }
        catch (Spinnaker::Exception& e)
        {
            if (e == SPINNAKER_ERR_INVALID_ADDRESS)
            {
                cout << "Warning: Camera is on a wrong subnet. Run auto force IP (with -a option) to configure the "
                        "camera correctly."
                     << endl;
            }
            else
            {
                cout << "Error: " << e.what() << endl;
            }
        }
    }

    pCam = nullptr;

    // Clear camera list before releasing system
    camList.Clear();

    // Release system
    system->ReleaseInstance();
}

// Example entry point; please see Enumeration example for more in-depth
// comments on preparing and cleaning up the system.
int main(int argc, char** argv)
{
    std::string serialNumber;
    std::string macAddress;
    std::string ipAddress;
    std::string subnetMask;
    std::string gateway;
    bool bCommandIsValid = false;

    cout << endl << "*** FLIR GigE Config Utility ***" << endl << endl;

    switch (argc)
    {
    case 1:
        // List all discovered devices
        ListAllDeviceInfo();
        bCommandIsValid = true;
        break;
    case 2:
        if ((strcmp(argv[1], "-a") == 0) || (strcmp(argv[1], "-A") == 0))
        {
            // Auto configure all devices
            AutoConfigure();
            bCommandIsValid = true;
        }
        break;
    case 3:
        if ((strcmp(argv[1], "-s") == 0) || (strcmp(argv[1], "-S") == 0))
        {
            // List specified device info
            serialNumber = argv[2];
            ListDeviceInfoBySerial(serialNumber);
            bCommandIsValid = true;
        }
        else if ((strcmp(argv[1], "-m") == 0) || (strcmp(argv[1], "-M") == 0))
        {
            // List specified device info
            macAddress = GetMACAddress(argv[2]);
            ListDeviceInfoByMAC(macAddress);
            bCommandIsValid = true;
        }
        break;
    case 9:
        // Get all options from command line
        for (int i = 1; i < argc; i = i + 2)
        {
            if ((strcmp(argv[i], "-s") == 0) || (strcmp(argv[i], "-S") == 0))
            {
                serialNumber = argv[i + 1];
            }
            else if ((strcmp(argv[i], "-i") == 0) || (strcmp(argv[i], "-I") == 0))
            {
                ipAddress = argv[i + 1];
            }
            else if ((strcmp(argv[i], "-n") == 0) || (strcmp(argv[i], "-N") == 0))
            {
                subnetMask = argv[i + 1];
            }
            else if ((strcmp(argv[i], "-g") == 0) || (strcmp(argv[i], "-G") == 0))
            {
                gateway = argv[i + 1];
            }
        }

        // Configure the camera if all options are valid
        if (serialNumber.empty() || serialNumber.compare(zeroSerial) == 0)
        {
            cout << "Error: Please specify a valid serial number" << endl;
            break;
        }

        if (!(ValidateIPV4Address(ipAddress) && ValidateIPV4Address(subnetMask, true) && ValidateIPV4Address(gateway)))
        {
            cout << "Error: Invalid IP address, subnet mask or default gateway" << endl;
            break;
        }

        // Configure the camera with a persistent IP address, subnet mask and default gateway
        ConfigureCamera(serialNumber, ipAddress, subnetMask, gateway);
        bCommandIsValid = true;

        break;
    default:
        break;
    }

    if (!bCommandIsValid)
    {
        Usage();
        return -1;
    }

    return 0;
}
