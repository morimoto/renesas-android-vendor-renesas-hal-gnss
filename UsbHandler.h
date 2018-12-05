/*
 * Copyright (C) 2018 GlobalLogic
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <vector>
#include <cstdint>
#include <string>
#include <usbhost/usbhost.h>

#include <log/log.h>

typedef struct  deviceDescriptor
{
   uint16_t idVendor;
   uint16_t idProduct;
} devDescriptor_t;

class UsbHandler
{
public:
    UsbHandler() {}
    ~UsbHandler() {}

    /**
     * @brief ScanUsbDevices
     * @brief Check each connected usb device for product id and vendor id
     * @return true if ublox device exists, otherwise false
     */
    bool ScanUsbDevices();

private:

    /**
     * @brief ScanSubdir
     * @param subDir - a pointer to directory name
     * @param listEntries - a reference to the vector with path to existing usb devices
     */
    void ScanSubdir(const char * subDir, std::vector<std::string> &listDeviceNames);

    /**
     * @brief PrepareUsbDeviceList - fill usbDeviceList vector with path to the connected usb devices
     */
    void PrepareUsbDeviceList();

    /**
     * @brief GetDeviceInfo - retrieve product id and vendor id
     * @param name - a pointer to  the full path to usb device
     */
    void GetDeviceInfo(const char* name);

    std::vector<deviceDescriptor> usbDeviceList;
};
