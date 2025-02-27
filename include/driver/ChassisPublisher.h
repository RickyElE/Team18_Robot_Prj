#ifndef _BATTERY_PUBLISHER_H_
#define _BATTERY_PUBLISHER_H_

#include "ChassisMsgPubSubType.h"
// #include "CppTimer.h"

#include <thread>
#include <chrono>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

class ChassisPublisher
{
private:
    eprosima::fastdds::dds::DomainParticipant* participant_ = nullptr;
    eprosima::fastdds::dds::Publisher* publisher_ = nullptr;
    eprosima::fastdds::dds::Topic* topic_ = nullptr;
    eprosima::fastdds::dds::DataWriter* writer_ = nullptr;
    eprosima::fastdds::dds::TypeSupport type_;

    class PubListener : public eprosima::fastdds::dds::DataWriterListener
    {
    public:
        PubListener() : matched_(0){};
        ~PubListener() override {};

        // void on_publication_matched(
        //     eprosima::fastdds::dds::DataWriter* /*writer*/,
        //     const eprosima::fastdds::dds::PublicationMatchedStatus& info) override
        // {
        //     if (info.current_count_change == 1)
        //     {
        //         matched_ = info.total_count;
        //         return;
        //         // std::cout << "Publisher matched" << std::endl;
        //     }
        //     else 
        //     if (info.current_count_change == -1)
        //     {
        //         matched_ = info.total_count;
        //         std::cout << "Publisher unmatched" << std::endl;
        //     }
        //     else
        //     {
        //         std::cout << "Unknown publication matched change" << std::endl;
        //     }
        // }

        std::atomic_int matched_;

    } listener_;
public:
    ChassisPublisher():type_(new ChassisMsgPubSubType()) {}

    virtual ~ChassisPublisher(){
        if (writer_ != nullptr)
        {
            publisher_->delete_datawriter(writer_);
        }

        if (publisher_ != nullptr)
        {
            participant_->delete_publisher(publisher_);
        }

        if (topic_ != nullptr)
        {
            participant_->delete_topic(topic_);
        }
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }

    bool init(){
        std::cout << "Initializing ChassisPublisher" << std::endl;
        eprosima::fastdds::dds::DomainParticipantQos participantQos;
        participantQos.name("Participant_publisher");
        participant_ = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(0, participantQos);
        if (participant_ == nullptr)
        {
            return false;
        }
        type_.register_type(participant_);

        topic_ = participant_->create_topic("ChassisTopic", "ChassisMsg", eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);

        if (topic_ == nullptr)
        {
            return false;
        }

        publisher_ = participant_->create_publisher(eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT, nullptr);

        if (publisher_ == nullptr)
        {
            return false;
        }

        writer_ = publisher_->create_datawriter(topic_, eprosima::fastdds::dds::DATAWRITER_QOS_DEFAULT, &listener_);

        if (writer_ == nullptr)
        {
            return false;
        }

        return true;

    }

    bool publish(ChassisMsg& msg){
        // if (listener_.matched_ > 0)
        {
            writer_->write(&msg);
            return true;
        }
        return false;
    }
};

#endif