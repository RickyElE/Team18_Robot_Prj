#ifndef _CHASSIS_MSG_H_
#define _CHASSIS_MSG_H_

#include <stdint.h>
#include <array>
#include <string>
#include <vector>
#include <map>
#include <bitset>

#if defined(_WIN32)
#if defined(EPROSIMA_USER_DLL_EXPORT)
#define eProsima_user_DllExport __declspec( dllexport )
#else
#define eProsima_user_DllExport
#endif  // EPROSIMA_USER_DLL_EXPORT
#else
#define eProsima_user_DllExport
#endif  // _WIN32

#if defined(_WIN32)
#if defined(EPROSIMA_USER_DLL_EXPORT)
#if defined(HelloWorldMsg_SOURCE)
#define HelloWorldMsg_DllAPI __declspec( dllexport )
#else
#define HelloWorldMsg_DllAPI __declspec( dllimport )
#endif // HelloWorldMsg_SOURCE
#else
#define HelloWorldMsg_DllAPI
#endif  // EPROSIMA_USER_DLL_EXPORT
#else
#define HelloWorldMsg_DllAPI
#endif // _WIN32

namespace eprosima {
namespace fastcdr {
class Cdr;
}
}


class ChassisMsg {
public:
    eProsima_user_DllExport ChassisMsg();
    eProsima_user_DllExport virtual ~ChassisMsg();
    eProsima_user_DllExport ChassisMsg(const ChassisMsg &x);
    eProsima_user_DllExport ChassisMsg(ChassisMsg &&x);
    eProsima_user_DllExport ChassisMsg& operator=(const ChassisMsg &x);
    eProsima_user_DllExport ChassisMsg& operator=(ChassisMsg &&x);

    eProsima_user_DllExport void serialize(eprosima::fastcdr::Cdr &scdr) const;
    eProsima_user_DllExport void deserialize(eprosima::fastcdr::Cdr &dcdr);
    eProsima_user_DllExport bool operator==(const ChassisMsg &x) const;
    eProsima_user_DllExport bool operator!=(const ChassisMsg &x) const;

    eProsima_user_DllExport void setBatteryPercentage(double batteryPercentage);
    eProsima_user_DllExport double getBatteryPercentage() const;
    eProsima_user_DllExport void setBatteryVoltage(double batteryVoltage);
    eProsima_user_DllExport double getBatteryVoltage() const;
    eProsima_user_DllExport static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);
    eProsima_user_DllExport static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);
    eProsima_user_DllExport static size_t getCdrSerializedSize(const ChassisMsg& data, size_t current_alignment = 0);
    eProsima_user_DllExport void serializeKey(eprosima::fastcdr::Cdr &scdr) const;

    eProsima_user_DllExport static bool isKeyDefined();
private:
    uint8_t m_batterypercentage;
    float m_batteryVoltage;
};

#endif