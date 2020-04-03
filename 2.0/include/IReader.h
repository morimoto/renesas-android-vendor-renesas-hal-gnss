/*
 * Copyright (C) 2019 GlobalLogic
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
#pragma once

#include "include/GnssTransport.h"

enum class RDError : uint8_t {
    Success,
    TransportError,
    InvalidInput,
    InternalError,
    Capturing,
    Reset,
};

typedef struct FakeLocationPoints {
    double latitude;
    double longitude;
    float speed;
} fakeLocationPoint_t;

//TODO(g.chabukiani): change return type,
//specify it with specified callback class/function
typedef int (*cbPtr)();


class IReader {
public:
    virtual ~IReader() = 0;

    virtual RDError Start() = 0;
    virtual RDError Stop() = 0;
    virtual RDError SetUpNotificationCallback(cbPtr notificationCb) = 0;
};
