// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*!
 * @file BatteryMsg.cpp
 * This source file contains the definition of the described types in the IDL file.
 *
 * This file was generated by the tool gen.
 */

#ifdef _WIN32
// Remove linker warning LNK4221 on Visual Studio
namespace {
char dummy;
}  // namespace
#endif  // _WIN32

#include "BatteryMsg.h"
#include <fastcdr/Cdr.h>

#include <fastcdr/exceptions/BadParamException.h>
using namespace eprosima::fastcdr::exception;

#include <utility>

BatteryMsg::BatteryMsg()
{
    // m_voltage com.eprosima.idl.parser.typecode.PrimitiveTypeCode@7a9273a8
    m_voltage = 0.0;
    // m_percentage com.eprosima.idl.parser.typecode.PrimitiveTypeCode@7ce6a65d
    m_percentage = 0.0;

}

BatteryMsg::~BatteryMsg()
{

}

BatteryMsg::BatteryMsg(
        const BatteryMsg& x)
{
    m_voltage = x.m_voltage;
    m_percentage = x.m_percentage;
}

BatteryMsg::BatteryMsg(
        BatteryMsg&& x) noexcept 
{
    m_voltage = x.m_voltage;
    m_percentage = x.m_percentage;
}

BatteryMsg& BatteryMsg::operator =(
        const BatteryMsg& x)
{

    m_voltage = x.m_voltage;
    m_percentage = x.m_percentage;

    return *this;
}

BatteryMsg& BatteryMsg::operator =(
        BatteryMsg&& x) noexcept
{

    m_voltage = x.m_voltage;
    m_percentage = x.m_percentage;

    return *this;
}

bool BatteryMsg::operator ==(
        const BatteryMsg& x) const
{

    return (m_voltage == x.m_voltage && m_percentage == x.m_percentage);
}

bool BatteryMsg::operator !=(
        const BatteryMsg& x) const
{
    return !(*this == x);
}

size_t BatteryMsg::getMaxCdrSerializedSize(
        size_t current_alignment)
{
    size_t initial_alignment = current_alignment;


    current_alignment += 8 + eprosima::fastcdr::Cdr::alignment(current_alignment, 8);


    current_alignment += 8 + eprosima::fastcdr::Cdr::alignment(current_alignment, 8);



    return current_alignment - initial_alignment;
}

size_t BatteryMsg::getCdrSerializedSize(
        const BatteryMsg& data,
        size_t current_alignment)
{
    (void)data;
    size_t initial_alignment = current_alignment;


    current_alignment += 8 + eprosima::fastcdr::Cdr::alignment(current_alignment, 8);


    current_alignment += 8 + eprosima::fastcdr::Cdr::alignment(current_alignment, 8);



    return current_alignment - initial_alignment;
}

void BatteryMsg::serialize(
        eprosima::fastcdr::Cdr& scdr) const
{

    scdr << m_voltage;
    scdr << m_percentage;

}

void BatteryMsg::deserialize(
        eprosima::fastcdr::Cdr& dcdr)
{

    dcdr >> m_voltage;
    dcdr >> m_percentage;
}

/*!
 * @brief This function sets a value in member voltage
 * @param _voltage New value for member voltage
 */
void BatteryMsg::voltage(
        double _voltage)
{
    m_voltage = _voltage;
}

/*!
 * @brief This function returns the value of member voltage
 * @return Value of member voltage
 */
double BatteryMsg::voltage() const
{
    return m_voltage;
}

/*!
 * @brief This function returns a reference to member voltage
 * @return Reference to member voltage
 */
double& BatteryMsg::voltage()
{
    return m_voltage;
}

/*!
 * @brief This function sets a value in member percentage
 * @param _percentage New value for member percentage
 */
void BatteryMsg::percentage(
        double _percentage)
{
    m_percentage = _percentage;
}

/*!
 * @brief This function returns the value of member percentage
 * @return Value of member percentage
 */
double BatteryMsg::percentage() const
{
    return m_percentage;
}

/*!
 * @brief This function returns a reference to member percentage
 * @return Reference to member percentage
 */
double& BatteryMsg::percentage()
{
    return m_percentage;
}


size_t BatteryMsg::getKeyMaxCdrSerializedSize(
        size_t current_alignment)
{
    size_t current_align = current_alignment;



    return current_align;
}

bool BatteryMsg::isKeyDefined()
{
    return false;
}

void BatteryMsg::serializeKey(
        eprosima::fastcdr::Cdr& scdr) const
{
    (void) scdr;
      
}
