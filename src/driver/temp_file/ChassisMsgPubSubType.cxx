#include <fastcdr/FastBuffer.h>
#include <fastcdr/Cdr.h>

#include "ChassisMsgPubSubType.h"

using SeralizedPayload_t = eprosima::fastrtps::rtps::SerializedPayload_t;
using InstanceHandle_t = eprosima::fastrtps::rtps::InstanceHandle_t;

ChassisMsgPubSubType::ChassisMsgPubSubType(){
    setName("ChassisMsg");
    auto type_size = ChassisMsg::getMaxCdrSerializedSize();
    type_size += eprosima::fastcdr::Cdr::alignment(type_size, 4);
    m_typeSize = static_cast<uint32_t>(type_size) + 4;
    m_isGetKeyDefined = ChassisMsg::isKeyDefined();
    size_t key_Length = ChassisMsg::getKeyMaxCdrSerializedSize() > 16 ?
                        ChassisMsg::getKeyMaxCdrSerializedSize() : 16;
    m_keyBuffer = reinterpret_cast<unsigned char*>(malloc(key_Length));
    memset(m_keyBuffer, 0, key_Length);
}

ChassisMsgPubSubType::~ChassisMsgPubSubType(){
    if(m_keyBuffer != nullptr){
        free(m_keyBuffer);
    }
}

bool ChassisMsgPubSubType::serialize(void* data, SeralizedPayload_t* payload){
    ChassisMsg* p_type = static_cast<ChassisMsg*>(data);
    eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->max_size);
    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);
    payload->encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
    ser.serialize_encapsulation();

    try{
        p_type->serialize(ser);
    }
    catch (eprosima::fastcdr::exception::NotEnoughMemoryException&){
        return false;
    }

    payload->length = static_cast<uint32_t>(ser.getSerializedDataLength());
    return true;
}

bool ChassisMsgPubSubType::deserialize(SeralizedPayload_t* payload, void* data){
    ChassisMsg* p_type = static_cast<ChassisMsg*>(data);
    eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->length);
    eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);
    deser.read_encapsulation();
    payload->encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

    try{
        p_type->deserialize(deser);
    }
    catch (eprosima::fastcdr::exception::NotEnoughMemoryException&){
        return false;
    }
    return true;
}

std::function<uint32_t()> ChassisMsgPubSubType::getSerializedSizeProvider(void* data){
    return [data]()->uint32_t{
        return static_cast<uint32_t>(type::getCdrSerializedSize(*static_cast<ChassisMsg*>(data))) + 4u;
    };
}

void* ChassisMsgPubSubType::createData(){
    return reinterpret_cast<void*>(new ChassisMsg());
}

void ChassisMsgPubSubType::deleteData(void* data){
    delete(reinterpret_cast<ChassisMsg*>(data));
}

bool ChassisMsgPubSubType::getKey(void* data, InstanceHandle_t* ihandle, bool force_md5){
    if(!m_isGetKeyDefined){
        return false;
    }
    ChassisMsg* p_type = static_cast<ChassisMsg*>(data);
    eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(m_keyBuffer), ChassisMsg::getKeyMaxCdrSerializedSize());
    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);
    p_type->serializeKey(ser);
    if (force_md5 || ChassisMsg::getKeyMaxCdrSerializedSize() > 16){
        m_md5.init();
        m_md5.update(m_keyBuffer, static_cast<unsigned int>(ser.getSerializedDataLength()));
        m_md5.finalize();
        for(int i = 0; i < 16; ++i){
            ihandle->value[i] = m_md5.digest[i];
        }
    }
    else{
        for(int i = 0; i < 16; ++i){
            ihandle->value[i] = m_keyBuffer[i];
        }
    }
    return true;
}