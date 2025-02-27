#ifndef _BATTEY_SUBSCRIBER_H_
#define _BATTEY_SUBSCRIBER_H_

#include "ChassisMsgPubSubType.h"

#include <chrono>
#include <thread>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

class ChassisSubscriber
{
public:
    eprosima::fastdds::dds::DomainParticipant* participant_ = nullptr;

    eprosima::fastdds::dds::Subscriber* subscriber_ = nullptr;

    eprosima::fastdds::dds::DataReader* reader_ = nullptr;

    eprosima::fastdds::dds::Topic* topic_ = nullptr;    

    eprosima::fastdds::dds::TypeSupport type_;

public:
    ChassisSubscriber() : type_(new ChassisMsgPubSubType()) {}
    virtual ~ChassisSubscriber(){
        if (reader_ != nullptr)
        {
            subscriber_->delete_datareader(reader_);
        }

        if (subscriber_ != nullptr)
        {
            participant_->delete_subscriber(subscriber_);
        }

        if (topic_ != nullptr)
        {
            participant_->delete_topic(topic_);
        }

        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
    bool subscribe(){
        class SubListener : public eprosima::fastdds::dds::DataReaderListener
        {
        public:
            SubListener(){};
            ~SubListener() override {};

            // void on_subscription_matched(
            //     eprosima::fastdds::dds::DataReader* reader,
            //     const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override
            // {
            //     if (info.current_count_change == 1)
            //     {
            //         return;
            //         // std::cout << "Subscriber matched" << std::endl;
            //     }
            //     else 
            //     if (info.current_count_change == -1)
            //     {
            //         std::cout << "Subscriber unmatched" << std::endl;
            //     }
            //     else
            //     {
            //         std::cout << info.total_count_change << "Unknown subscription matched change" << std::endl;
            //     }
            
            // }
        
            void on_data_available(eprosima::fastdds::dds::DataReader* reader)override{
                ChassisMsg msg;
                eprosima::fastdds::dds::SampleInfo info;
                if (reader->take_next_sample(&msg, &info) == ReturnCode_t::RETCODE_OK)
                {
                    if (info.valid_data)
                    {
                        std::cout << "BatteryMsg: " << msg.getBatteryPercentage() << " " << msg.getBatteryVoltage() << std::endl;
                    }
                }
            }

            ChassisMsg msg_;

        } listener_;
        eprosima::fastdds::dds::DataReaderQos readerQos;
        reader_ = subscriber_->create_datareader(topic_, readerQos, &listener_);
        if (reader_ == nullptr)
        {
            return false;
        }
        return true;
        
    }
    bool init(){
        std::cout << "Initializing subscriber" << std::endl;
        eprosima::fastdds::dds::DomainParticipantQos pqos;
        pqos.name("Participant_subscriber");
        participant_ = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(0, pqos);
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

        subscriber_ = participant_->create_subscriber(eprosima::fastdds::dds::SUBSCRIBER_QOS_DEFAULT, nullptr);

        if (subscriber_ == nullptr)
        {
            return false;
        }

        return true;
    }
};

#endif