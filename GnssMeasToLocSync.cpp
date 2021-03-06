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

#define LOG_TAG "GnssRenesasHAL_MeasToLockSync"
#define LOG_NDEBUG 1

#include "GnssMeasToLocSync.h"

GnssMeasToLocSync& GnssMeasToLocSync::getInstance() {
    static GnssMeasToLocSync instance;
    return instance;
}

GnssMeasToLocSync::GnssMeasToLocSync()
{
    mEventsToWait = 0;
}

bool GnssMeasToLocSync::WaitToSend()
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);
    return (mEventsToWait > 0 ? false : true);
}

void GnssMeasToLocSync::UpdateStatus(int8_t numEvents)
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);
    int8_t tmpEventsNum = mEventsToWait + numEvents;
    mEventsToWait = (tmpEventsNum > 0 ? tmpEventsNum : 0);
}

void GnssMeasToLocSync::SetEventsToWait(int8_t numEvents)
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);
    mEventsToWait = ( numEvents < 0 ? 0 : numEvents);
}
