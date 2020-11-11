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

#ifndef IREADER_H
#define IREADER_H

#include <GnssTransport.h>

namespace android::hardware::gnss::V2_1::renesas {

/**
 * @brief return error type
 */
enum class RDError : uint8_t {
    /**
     * @brief Success
     */
    Success,

    /**
     * @brief Transport Error
     */
    TransportError,

    /**
     * @brief Invalid Input
     */
    InvalidInput,

    /**
     * @brief Internal Error
     */
    InternalError,

    /**
     * @brief Capturing
     */
    Capturing,

    /**
     * @brief Reset
     */
    Reset,
};

/**
 * @brief Fake Location Points
 */
typedef struct FakeLocationPoints {
    /**
     * @brief latitude
     */
    double latitude;

    /**
     * @brief longitude
     */
    double longitude;

    /**
     * @brief speed
     */
    float speed;
} fakeLocationPoint_t;

//TODO(g.chabukiani): change return type,
//specify it with specified callback class/function
/**
 * @brief cbPtr
 */
typedef int (*cbPtr)();

/**
 * @brief IReader
 */
class IReader {
public:
    /**
     * @brief Destroy the IReader object
     */
    virtual ~IReader() {};

    /**
     * @brief Start
     *
     * @return RDError
     */
    virtual RDError Start() = 0;

    /**
     * @brief Stop
     *
     * @return RDError
     */
    virtual RDError Stop() = 0;

    /**
     * @brief Set Notification Callback object
     *
     * @param notificationCb
     * @return RDError
     */
    virtual RDError SetUpNotificationCallback(cbPtr notificationCb) = 0;
};

} // namespace android::hardware::gnss::V2_1::renesas

#endif // IREADER_H
