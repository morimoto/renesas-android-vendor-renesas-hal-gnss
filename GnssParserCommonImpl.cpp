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

#define LOG_TAG "GnssParserCommon"
#define LOG_NDEBUG 1

#include <log/log.h>

#include "GnssParserCommonImpl.h"

bool GnssParserCommonImpl::isBigEndian()
{
    union
    {
        uint32_t dec;
        char literal[4];
    } checker = {0x01020304};
    return 0x01 == checker.literal[0] ? true : false;
}

void GnssParserCommonImpl::dumpDebug()
{
    ALOGV("[%s, line %d]", __func__, __LINE__);
}

void GnssParserCommonImpl::hexdump(const char* file, void* ptr, size_t buflen)
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);
    if (nullptr == file || nullptr == ptr) {
        return;
    }

    FILE* pfile = fopen(file, "a");
    if (nullptr == pfile) {
        return;
    }

    fprintf(pfile, "_______________________________________\n");
    uint8_t* buf = static_cast<uint8_t*>(ptr);
    size_t i, j;
    for (i = 0; i < buflen; i += 16) {
        fprintf(pfile, "%06zx: ", i);
        for (j = 0; j < 16; j++){
            if (i + j < buflen) {
                fprintf(pfile, "%02x ", buf[i + j]);
            } else {
                fprintf(pfile,"   ");
            }
        }
        fprintf(pfile," ");
        for (j = 0; j < 16; j++) {
            if (i + j < buflen) {
                fprintf(pfile, "%c", isprint(buf[i + j]) ? buf[i + j] : '.');
            }
        }
        fprintf(pfile, "\n");
    }
    fprintf(pfile, "---------------------------------------\n");
    fclose(pfile);
}

GnssIParser::~GnssIParser() {}
