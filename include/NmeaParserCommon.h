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
#pragma once

#include "include/INmeaParser.h"

#define NP_CH(FUNC) status=FUNC; if(NPError::Success!=status) return status

template <typename T>
class NmeaParserCommon : public INmeaParser<T> {
public:
    NmeaParserCommon() = default;
    virtual ~NmeaParserCommon() override = default;
protected:
    virtual NPError Parse(std::string& in) {
        if (in.empty()) {
            return NPError::IncompletePacket;
        }
        return NPError::Success;
    };
    NPError Split(std::string& in, std::vector<std::string>& out);
};

template <typename T>
NPError NmeaParserCommon<T>::Split(std::string& in,
                                   std::vector<std::string>& out) {
    std::string::size_type end = 0;
    bool separator = true;
    out.clear();

    while (!in.empty()) {
        if ((end = in.find(",")) == std::string::npos) {
            separator = false;
            if ((end = in.find("*")) != std::string::npos) {
                in.erase(end, in.length() - end);
            }
            end = in.length();
        }

        out.push_back(std::string(in.c_str(), end));
        in.erase(0, end + (separator ? 1 : 0));
    }

    if (separator) {
        out.push_back(std::string(in.c_str(), in.length()));
    }

    return NPError::Success;
}
