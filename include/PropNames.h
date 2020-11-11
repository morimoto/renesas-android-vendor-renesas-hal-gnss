/*
 * Copyright (C) 2020 GlobalLogic
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
#ifndef PROPNAMES_H
#define PROPNAMES_H

#include <string>

namespace android::hardware::gnss::V2_1::renesas {
/**
 * @brief BaudRate property
 */
static const std::string propBaudRate = {"ro.boot.gps.tty_baudrate"};

/**
 * @brief Gnss BaudRate property
 */
static const std::string propGnssBaudRate = {"ro.boot.gps.gnss_baudrate"};

/**
 * @brief Requested Receiver property
 */
static const std::string propRequestedReceiver = {"ro.boot.gps.mode"};

/**
 * @brief Sbas property
 */
static const std::string propSbas = {"ro.boot.gps.sbas"};

/**
 * @brief Secmajor property
 */
static const std::string propSecmajor = {"ro.boot.gps.secmajor"};
} // namespace android::hardware::gnss::V2_1::renesas

#endif //PROPNAMES_H
