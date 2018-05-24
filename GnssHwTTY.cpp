/*
 * Copyright (C) 2017 GlobalLogic
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

#define LOG_TAG "GnssSalvatorHAL"

#include <termios.h>
#include <unistd.h>
#include <inttypes.h>
#include <memory.h>
#include <sys/param.h> /* for MIN(x,y) */

#include <cutils/log.h>
#include <cutils/properties.h>

#include <android/hardware/gnss/1.0/IGnss.h>

#include "GnssHw.h"

enum SvidValues : uint16_t {
    GLONASS_SVID_OFFSET = 64,
    GLONASS_SVID_COUNT = 24,
    BEIDOU_SVID_OFFSET = 200,
    BEIDOU_SVID_COUNT = 35,
    SBAS_SVID_MIN = 33,
    SBAS_SVID_MAX = 64,
    SBAS_SVID_ADD = 87,
    QZSS_SVID_MIN = 193,
    QZSS_SVID_MAX = 200
};

GnssHwTTY::GnssHwTTY(void) :
    mFd(-1)
{
    memset(&mGnssLocation, 0, sizeof(GnssLocation));
    memset(&mSvStatus, 0, sizeof(IGnssCallback::GnssSvStatus));
}

GnssHwTTY::~GnssHwTTY(void)
{
}

bool GnssHwTTY::start(void)
{
    char prop_tty_dev[PROPERTY_VALUE_MAX], prop_tty_baudrate[PROPERTY_VALUE_MAX];

    ALOGD("Start HW");

    property_get("ro.boot.gps.tty_dev", prop_tty_dev, "/dev/ttyUSB0");
    property_get("ro.boot.gps.tty_baudrate", prop_tty_baudrate, "9600");

    uint32_t baudrate = atoi(prop_tty_baudrate);

     /* Open the serial tty device */
    do {
        mFd = ::open(prop_tty_dev, O_RDWR | O_NOCTTY);
    } while(mFd < 0 && errno == EINTR);

    if (mFd < 0) {
        ALOGE("Could not open TTY device %s, error: %s", prop_tty_dev, strerror(errno));
        return false;
    }

    ALOGI("TTY %s@%d fd=%d", prop_tty_dev, baudrate, mFd);

    /* Setup serial port */
    struct termios  ios;
    ::tcgetattr(mFd, &ios);

    ios.c_cflag = CS8 | CLOCAL | CREAD;
    ios.c_iflag = IGNPAR;
    ios.c_oflag = 0;
    ios.c_lflag = 0;  /* disable ECHO, ICANON, etc... */

    /* Set baudrate */
    switch(baudrate) {
        case 2400: ios.c_cflag |= B2400; break;
        case 4800: ios.c_cflag |= B4800; break;
        case 9600: ios.c_cflag |= B9600; break;
        case 19200: ios.c_cflag |= B19200; break;
        case 38400: ios.c_cflag |= B38400; break;
        default: {
            ios.c_cflag |= B9600;
            ALOGW("Unsupported baud rate %d.. setting default 9600", baudrate);
        }
    }

    ::tcsetattr(mFd, TCSANOW, &ios);
    ::tcflush(mFd, TCIOFLUSH);

    return true;
}

bool GnssHwTTY::stop(void)
{
    ALOGD("Stop HW");

    if (mFd != -1) {
        ::close(mFd);
        mFd = -1;
    }

    return true;
}

void GnssHwTTY::GnssHwHandleThread(void)
{
    ALOGD("GnssHwHandleThread() ->");

    while (!mThreadExit)
    {
        if (mFd == -1) {
            usleep(1000000); /* Sleeping if TTY device not accessible */
            continue;
        }

        char ch;
        int ret = read(mFd, &ch, sizeof(ch));

        if (ret < 0 && errno == EAGAIN) {
            continue;
        }

        if(ret > 0) {
            NMEA_ReaderPushChar(ch);
        } else {
            ALOGE("TTY read error: %s", strerror(errno));
            break;
        }
    }

    ALOGD("GnssHwHandleThread() <-");
}

int GnssHwTTY::NMEA_Checksum(const char *s)
{
    int crc = 0;

    while(*s) {
        crc ^= *s++;
    }

    return crc;
}

void GnssHwTTY::NMEA_ReaderSplitMessage(std::string msg, std::vector<std::string> & out)
{
    int end = 0;
    bool separator = true;

    out.clear();

    while (!msg.empty())
    {
        if ((end = msg.find(",")) < 0) {
            separator = false;
            end = (int)msg.length();
        }
        out.push_back(std::string(msg.c_str(), end));
        msg.erase(0, end + (separator ? 1 : 0));
    }
}

void GnssHwTTY::NMEA_ReaderPushChar(char ch)
{
    if (mNmeaReaderBufPos >= sizeof(mNmeaReaderBuf)) {
        mNmeaReaderBufPos = 0;
        mNmeaReaderInCapture = false;
    }

    if (ch == '$' && !mNmeaReaderInCapture) {
        mNmeaReaderInCapture = true;
    } else if (ch == '$' && mNmeaReaderInCapture) {
        /* Missed end of message CR LF ? Reset the reader */
        mNmeaReaderBufPos = 0;
    } else if ((ch == '\r' || ch == '\n') && mNmeaReaderInCapture) {
        /* End of message */
        mNmeaReaderBuf[mNmeaReaderBufPos] = 0;

        /* Parse NMEA */
        NMEA_ReaderParse(mNmeaReaderBuf);

        /* Reset the reader  */
        mNmeaReaderBufPos = 0;
        mNmeaReaderInCapture = false;
        return;
    }

    if (mNmeaReaderInCapture) {
        mNmeaReaderBuf[mNmeaReaderBufPos++] = ch;
    }
}

void GnssHwTTY::NMEA_ReaderParse(char *msg)
{
    int crc = 0;
    char * p = strstr(msg, "*");

    if (p == NULL) /* No CRC field found? Drop message. */
        return;

    crc = strtol(p + 1, NULL, 16);
    *p = '\0';

    /* Checking for message integrity */
    if (NMEA_Checksum(msg + 1) ^ crc) {
        ALOGW("Drop message due invalid CRC: %s", msg);
        return;
    }

    /* Push RAW NMEA message to system */
    android::hardware::hidl_string nmeaString;
    nmeaString.setToExternal(msg, strlen(msg));

    auto ret = mGnssCb->gnssNmeaCb(time(NULL), nmeaString);
    if (!ret.isOk()) {
        ALOGE("%s: Unable to invoke gnssNmeaCb", __func__);
    }

    /* */
    if (strncmp(msg, "$GPRMC", 6) == 0)  {
        NMEA_ReaderParse_GxRMC(msg);
    } if (strncmp(msg, "$GNRMC", 6) == 0)  {
        NMEA_ReaderParse_GxRMC(msg);
    } else if (strncmp(msg, "$GPGGA", 6) == 0) {
        NMEA_ReaderParse_GxGGA(msg);
    } else if (strncmp(msg, "$GNGGA", 6) == 0) {
        NMEA_ReaderParse_GxGGA(msg);
    } else if (strncmp(msg, "$GPGSV", 6) == 0) {
        NMEA_ReaderParse_GPGSV(msg);
    }
}

void GnssHwTTY::NMEA_ReaderParse_GxRMC(char *msg)
{
    int degree;
    double raw, minutes, polarity;
    struct tm t;

    std::vector<std::string> rmc;
    NMEA_ReaderSplitMessage(std::string(msg), rmc);

    /* Message parts count must be 13 */
    if (rmc.size() != 13 || (rmc[0] != "$GPRMC" && rmc[0] != "$GNRMC")) {
        return;
    }

    // Status A=active or V=Void
    if (rmc[2] != "A") {
        ALOGD("GPRMC: No valid fix coordinates, wait...");

        /* Invalidate location data*/
        memset(&mGnssLocation, 0, sizeof(GnssLocation));

        return; /* Only when Active continue parse */
    }

    /* Parse time/date fields */
    memset(&t, 0, sizeof(struct tm));

    // UTC fix time
    if (rmc[1].length() > 0) {
        sscanf(rmc[1].c_str(), "%02u%02u%02u", &t.tm_hour, &t.tm_min, &t.tm_sec);
    }

    // Date
    if (rmc[9].length() > 0) {
        sscanf(rmc[9].c_str(), "%02u%02u%02u", &t.tm_mday, &t.tm_mon, &t.tm_year);

        // fix month (1-12 -> 0-11)
        t.tm_mon -= 1;
        // fix year (1917 -> 2017)
        t.tm_year += 100;
    }

    mGnssLocation.timestamp = mktime(&t) * 1000;

    /* Parse longtitude and latitude */
    mGnssLocation.gnssLocationFlags |= static_cast<uint16_t>(GnssLocationFlags::HAS_LAT_LONG);

    raw = atof(rmc[3].c_str()); // Latitude
    polarity = (rmc[4] == "S" ? -1.f : 1.f);

    degree = (raw / 100.f);
    minutes = ((raw / 100) - degree) * 100;
    mGnssLocation.latitudeDegrees = ((minutes / 60) + degree) * polarity;

    raw = atof(rmc[5].c_str()); // Longitude
    polarity = (rmc[6] == "W" ? -1.f : 1.f);

    degree = (raw / 100);
    minutes = ((raw / 100) - degree) * 100;
    mGnssLocation.longitudeDegrees = ((minutes / 60) + degree) * polarity;

    // Speed over the ground in knots
    if (rmc[7].length() > 0) {
        mGnssLocation.speedMetersPerSec = (atof(rmc[7].c_str()) * 1.852) / 3.6; // knots -> m/s
        mGnssLocation.gnssLocationFlags |= static_cast<uint16_t>(GnssLocationFlags::HAS_SPEED);
    } else {
        mGnssLocation.speedMetersPerSec = 0.f;
        mGnssLocation.gnssLocationFlags &= ~static_cast<uint16_t>(GnssLocationFlags::HAS_SPEED);
    }

    // Track angle in degrees True
    if (rmc[8].length() > 0) {
        mGnssLocation.bearingDegrees = atof(rmc[8].c_str());
        mGnssLocation.gnssLocationFlags |= static_cast<uint16_t>(GnssLocationFlags::HAS_BEARING);
    } else {
        mGnssLocation.bearingDegrees = 0.f;
        mGnssLocation.gnssLocationFlags &= ~static_cast<uint16_t>(GnssLocationFlags::HAS_BEARING);
    }

    // Magnetic Variation
    mGnssLocation.gnssLocationFlags |= static_cast<uint16_t>(GnssLocationFlags::HAS_HORIZONTAL_ACCURACY);
    mGnssLocation.horizontalAccuracyMeters = 1.f;

    if (rmc[10].length() > 0) {
        mGnssLocation.horizontalAccuracyMeters = atof(rmc[10].c_str());
    }

    auto ret = mGnssCb->gnssLocationCb(mGnssLocation);
    if (!ret.isOk()) {
        ALOGE("%s: Unable to invoke gnssLocationCb", __func__);
    }
}

void GnssHwTTY::NMEA_ReaderParse_GxGGA(char *msg)
{
    std::vector<std::string> gga;
    NMEA_ReaderSplitMessage(std::string(msg), gga);

    /* Message parts count must be 15 */
    if (gga.size() != 15 ||
            (gga[0] != "$GPGGA" && gga[0] != "$GNGGA")) {
        return;
    }

    // Altitude, Meters, above mean sea level
    if (gga[9].length() > 0) {
        mGnssLocation.altitudeMeters = atof(gga[9].c_str());
        mGnssLocation.gnssLocationFlags |= static_cast<uint16_t>(GnssLocationFlags::HAS_ALTITUDE);
    } else {
        mGnssLocation.altitudeMeters = 0;
        mGnssLocation.gnssLocationFlags &= ~static_cast<uint16_t>(GnssLocationFlags::HAS_ALTITUDE);
    }
}

void GnssHwTTY::NMEA_ReaderParse_GPGSV(char *msg)
{
    std::vector<std::string> gsv;
    NMEA_ReaderSplitMessage(std::string(msg), gsv);

    /* Message must have least 4 parts */
    if (gsv.size() < 4 || gsv[0] != "$GPGSV") {
        return;
    }

    if (gsv.size() % 4) { /* Ommited sattelite c_n0_dbhz param ? */
        gsv.push_back(std::string("0")); /* Append with zero */
    }

    int sentences = atoi(gsv[1].c_str());
    int sentence_idx = atoi(gsv[2].c_str());
    int num_svs = atoi(gsv[3].c_str());

    if( sentence_idx <= 0 || sentence_idx > sentences || num_svs <= 0 ) {
        return;
    }

    /* Per one GPGSV message max 4 sattelite records */
    size_t offset = (sentence_idx - 1) * 4;
    size_t parts = MIN((num_svs - offset), 4);

    for (size_t p = 0; p < parts; p++) {

        if ((offset + p) >= static_cast<int>(GnssMax::SVS_COUNT)) {
            break; /* out of space */
        }

        IGnssCallback::GnssSvInfo * sv = &mSvStatus.gnssSvList[offset + p];

        sv->svFlag = static_cast<uint8_t>(IGnssCallback::GnssSvFlags::HAS_ALMANAC_DATA);

        int idx = 4 + (p * 4);

        sv->svid = atoi(gsv[idx + 0].c_str());

        if (sv->svid >= 1 && sv->svid <= 32) {
            sv->constellation = GnssConstellationType::GPS;
        } else if (sv->svid > GLONASS_SVID_OFFSET && sv->svid <= GLONASS_SVID_OFFSET + GLONASS_SVID_COUNT) {
            sv->constellation = GnssConstellationType::GLONASS;
            sv->svid -= GLONASS_SVID_OFFSET;
        } else if (sv->svid > BEIDOU_SVID_OFFSET && sv->svid <= BEIDOU_SVID_OFFSET + BEIDOU_SVID_COUNT) {
            sv->constellation = GnssConstellationType::BEIDOU;
            sv->svid -= BEIDOU_SVID_OFFSET;
        } else if (sv->svid >= SBAS_SVID_MIN && sv->svid <= SBAS_SVID_MAX) {
            sv->constellation = GnssConstellationType::SBAS;
            sv->svid += SBAS_SVID_ADD;
        } else if (sv->svid >= QZSS_SVID_MIN && sv->svid <= QZSS_SVID_MAX) {
            sv->constellation = GnssConstellationType::QZSS;
        } else {
            ALOGW("Unknown constellation type with Svid = %d", sv->svid);
            sv->constellation = GnssConstellationType::UNKNOWN;
        }

        sv->elevationDegrees    = atof(gsv[idx + 1].c_str());
        sv->azimuthDegrees      = atof(gsv[idx + 2].c_str());
        sv->cN0Dbhz             = atof(gsv[idx + 3].c_str());

        if (sv->cN0Dbhz > 1.f) {
            sv->svFlag |= static_cast<uint8_t>(IGnssCallback::GnssSvFlags::USED_IN_FIX);
        }

        ALOGV("GPGSV: [%zu] svid=%d, elevation=%f, azimuth=%f, c_n0_dbhz=%f", offset + p,
              sv->svid, sv->elevationDegrees, sv->azimuthDegrees, sv->cN0Dbhz);
    }

    mSvStatus.numSvs = (num_svs > static_cast<int>(GnssMax::SVS_COUNT) ?
               static_cast<int>(GnssMax::SVS_COUNT) : num_svs);

    if (sentences == sentence_idx) { /* Last SVS received and parsed */
        auto ret = mGnssCb->gnssSvStatusCb(mSvStatus);
        if (!ret.isOk()) {
            ALOGE("%s: Unable to invoke gnssSvStatusCb", __func__);
        }
    }
}
