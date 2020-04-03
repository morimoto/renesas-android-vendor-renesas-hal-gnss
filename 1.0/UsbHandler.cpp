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

#define LOG_TAG "GnssRenesasHAL"
#define LOG_NDEBUG 1

#include <cstring>

#include "UsbHandler.h"

// list of known product ids of u-blox usb devices, represented as decimal integers
static const std::vector<uint16_t> idProductListInt = {420, 421, 422, 423, 424, 4354};

// u-blox vendor id represented as decimal integer
static const uint16_t idVendorUbxInt = 5446;
static const std::string devBusUsbPath("/dev/bus/usb");

void UsbHandler::GetDeviceInfo(const char *name)
{
    auto dev = usb_device_open(name);
    if (dev) {
        devDescriptor_t newDevice = {
            .idVendor = 0,
            .idProduct = 0,
        };

        newDevice.idVendor = usb_device_get_vendor_id(dev);
        newDevice.idProduct = usb_device_get_product_id(dev);
        usbDeviceList.push_back(newDevice);
        usb_device_close(dev);
    }
}

void UsbHandler::ScanSubdir(const char * subDir, std::vector<std::string> &listDeviceNames)
{
    if (nullptr == subDir) {
        ALOGD("[%s, line %d] subDir == null", __func__, __LINE__);
        return;
    }

    std::string curPath(devBusUsbPath + "/" + subDir);

    DIR* devDir = opendir(curPath.c_str());
    if (devDir) {
        struct dirent *dev;
        while ((dev = readdir(devDir)) != nullptr) {
            if (DT_CHR == dev->d_type) {
                listDeviceNames.push_back(curPath + "/" + dev->d_name);
            }
        }
        closedir(devDir);
    }
}

void UsbHandler::PrepareUsbDeviceList()
{
    std::vector<std::string> listDeviceNames;

    DIR* pDir = opendir(devBusUsbPath.c_str());
    if (pDir != nullptr) {
        struct dirent* entry;
        while ((entry = readdir(pDir)) != nullptr) {
            /* skip current and previous directory entries . and .. */
            if (0 != std::strncmp(entry->d_name, ".", 1)) {
                ScanSubdir(entry->d_name, listDeviceNames);
            }
        }

        for (auto &devEntry : listDeviceNames) {
            GetDeviceInfo(devEntry.c_str());
        }

        closedir(pDir);
    }
}

bool UsbHandler::ScanUsbDevices(uint16_t &outProductId)
{
    PrepareUsbDeviceList();

    for (auto dev : usbDeviceList) {
        if (idVendorUbxInt == dev.idVendor) {
            for (auto idProduct : idProductListInt) {
                if (idProduct == dev.idProduct) {
                    ALOGD("[%s, line %d] idVendor=%u, idProduct=%u", __func__, __LINE__, dev.idVendor, dev.idProduct);
                    outProductId = idProduct;
                    return true;
                }
            }
        }
    }

    return false;
}
