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

#define LOG_NDEBUG 1
#define LOG_TAG "GnssRenesasHalNmeaTxt"

#include <NmeaTxt.h>

template <>
NPError NmeaTxt<std::string&>::GetData(std::string& out){
    if (!mIsValid) {
        return NPError::InvalidData;
    }

    out = mParcel.text;
    return NPError::Success;
}

template <>
void NmeaTxt<std::string&>::PrintMsg() {
    std::string msgType = {"Default"};

    switch (mParcel.msgType) {
        case 0u: {
            msgType = "Error";
            break;
        }
        case 1u: {
            msgType = "Warning";
            break;
        }
        case 2u: {
            msgType = "Notice";
            break;
        }
        case 7u: {
            msgType = "User";
            break;
        }
        default:
            break;
    }

    ALOGI("%s: %s", msgType.c_str(), mParcel.text.c_str());
}