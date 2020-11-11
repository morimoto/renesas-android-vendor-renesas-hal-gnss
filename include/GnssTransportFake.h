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

#ifndef GNSSTRANSPORTFAKE_H
#define GNSSTRANSPORTFAKE_H

#include <fstream>

#include <GnssTransport.h>

namespace android::hardware::gnss::V2_1::renesas {

/**
 * @brief Gnss Transport Fake class implementation
 */
class GnssTransportFake : public Transport {
public:

    /**
     * @brief Construct a new Gnss Transport Fake object
     *
     * @param dataFilePath
     */
    GnssTransportFake(const std::string &dataFilePath);
    /*!
     * \brief Open
     * \return
     */
    TError Open() override;

    /**
     * @brief Close
     *
     * @return TError
     */
    TError Close() override;

    /**
     * @brief Write Data
     *
     * @param toWrite
     * @return TError
     */
    TError WriteData(const std::vector<uint8_t> &toWrite) override;

    /**
     * @brief Read Byte
     *
     * @param errCode
     * @return char
     */
    char ReadByte(TError& errCode) override;
protected:
    /**
     * @brief fake location data Stream
     */
    std::ifstream fakeStream;
};

} // namespace android::hardware::gnss::V2_1::renesas

#endif // GNSSTRANSPORTFAKE_H
