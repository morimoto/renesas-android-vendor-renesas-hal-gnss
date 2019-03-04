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

static bool isBigEndian()
{
    union
    {
        uint32_t  dec;
        char literal[4];
    } checker = {0x01020304};
    return 0x01 == checker.literal[0] ? true : false;
}

uint16_t GnssParserCommonImpl::getUint16(const uint8_t* ptr)
{
    if(isBigEndian())
    {
        uint16_t result = ptr[0] << 8;
        result |= ptr[1];
        return result;
    }

    return *(uint16_t*)ptr;
}

uint32_t GnssParserCommonImpl::getUint32(const uint8_t *ptr)
{
    if(isBigEndian())
    {
        uint32_t result = ptr[0] << 24;
        result |= (ptr[1] << 16);
        result |= (ptr[2] << 8);
        result |= ptr[3];
        return result;
    }
    return *(uint32_t*)ptr;
}

void GnssParserCommonImpl::dumpDebug()
{
    ALOGV("[%s, line %d]", __func__, __LINE__);
}

void GnssParserCommonImpl::hexdump(const char* file, void* ptr, size_t buflen)
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);
    FILE *pfile = fopen(file, "a");
    if (nullptr == pfile) {
        return;
    }
    fprintf(pfile, "_______________________________________\n");
    unsigned char *buf = (unsigned char*)ptr;
    size_t i, j;
    for (i=0; i<buflen; i+=16) {
        fprintf(pfile, "%06zx: ", i);
        for (j=0; j<16; j++){
            if (i+j < buflen) {
                fprintf(pfile, "%02x ", buf[i+j]);
            }else{
                fprintf(pfile,"   ");
            }
        }
        fprintf(pfile," ");
        for (j=0; j<16; j++) {
            if (i+j < buflen) {
                fprintf(pfile, "%c", isprint(buf[i+j]) ? buf[i+j] : '.');
            }
        }
        fprintf(pfile, "\n");
    }
    fprintf(pfile, "---------------------------------------\n");
    fclose(pfile);
}
