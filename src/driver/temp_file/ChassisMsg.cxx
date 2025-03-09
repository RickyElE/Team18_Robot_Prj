#ifdef _WIN32
// Remove linker warning LNK4221 on Visual Studio
namespace {
char dummy;
}  // namespace
#endif  // _WIN32

#include "ChassisMsg.h"
#include <fastcdr/Cdr.h>

#include <fastcdr/exceptions/BadParamException.h>
using namespace eprosima::fastcdr::exception;

#include <utility>

ChassisMsg::ChassisMsg()
{
    m_batterypercentage = 0.0;
    m_batteryVoltage = 0.0;
}

ChassisMsg::~ChassisMsg()
{
}

ChassisMsg::ChassisMsg(const ChassisMsg &x)
{
    m_batterypercentage = x.m_batterypercentage;
    m_batteryVoltage = x.m_batteryVoltage;
}

ChassisMsg::ChassisMsg(ChassisMsg &&x)
{
    m_batterypercentage = x.m_batterypercentage;
    m_batteryVoltage = x.m_batteryVoltage;
}

ChassisMsg& ChassisMsg::operator=(const ChassisMsg &x)
{
    m_batterypercentage = x.m_batterypercentage;
    m_batteryVoltage = x.m_batteryVoltage;
    
    return *this;
}

ChassisMsg& ChassisMsg::operator=(ChassisMsg &&x)
{
    m_batterypercentage = x.m_batterypercentage;
    m_batteryVoltage = x.m_batteryVoltage;
    
    return *this;
}

bool ChassisMsg::operator==(const ChassisMsg &x) const
{
    if(m_batterypercentage == x.m_batterypercentage && m_batteryVoltage == x.m_batteryVoltage)
    {
        return true;
    }
    return false;
}

bool ChassisMsg::operator!=(const ChassisMsg &x) const
{
    return !(*this == x);
}

void ChassisMsg::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_batterypercentage;
    scdr << m_batteryVoltage;
}

void ChassisMsg::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_batterypercentage;
    dcdr >> m_batteryVoltage;
}

void ChassisMsg::setBatteryPercentage(double batteryPercentage)
{
    m_batterypercentage = batteryPercentage;
}

void ChassisMsg::setBatteryVoltage(double batteryVoltage)
{
    m_batteryVoltage = batteryVoltage;
}

double ChassisMsg::getBatteryPercentage() const
{
    return m_batterypercentage;
}

double ChassisMsg::getBatteryVoltage() const
{
    return m_batteryVoltage;
}

size_t ChassisMsg::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;
            
    current_align += 1 + eprosima::fastcdr::Cdr::alignment(current_align, 1);
    current_align += 4 + eprosima::fastcdr::Cdr::alignment(current_align, 4);

    return current_align;
}

bool ChassisMsg::isKeyDefined()
{
    return false;
}


size_t ChassisMsg::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;
            
    current_align += 1 + eprosima::fastcdr::Cdr::alignment(current_align, 1);
    current_align += 4 + eprosima::fastcdr::Cdr::alignment(current_align, 4);

    return current_align;
}

size_t ChassisMsg::getCdrSerializedSize(const ChassisMsg& data, size_t current_alignment)
{
    (void)data;
    size_t current_align = current_alignment;
            
    current_align += 1 + eprosima::fastcdr::Cdr::alignment(current_align, 1);
    current_align += 4 + eprosima::fastcdr::Cdr::alignment(current_align, 4);

    return current_align;
}

void ChassisMsg::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
}