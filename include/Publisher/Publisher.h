#ifndef __PUBLISHER_H__
#define __PUBLISHER_H__

#include <cstddef>
#include <fastdds/dds/core/policy/QosPolicies.hpp>

#include <fastdds/dds/core/status/PublicationMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

template <typename MSG_PUBSUB_TYPE, typename MSG_TYPE>
class Publisher{
public:
    Publisher(MSG_TYPE, std::string topic_name, eprosima::fastdds::dds::DomainParticipant *participant):
        publisher_(nullptr), writer_(nullptr), topic_(nullptr), type_(new MSG_PUBSUB_TYPE)
    {
        topic_name_ = topic_name;
        participant_ = participant;
    }

    virtual ~Publisher(){
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
    };

    class PubListener : public eprosima::fastdds::dds::DataWriterListener
    {
    public:
        PubListener() : matched_(0){};
        ~PubListener() override {};
        const float matched() const {return matched_;}
        
    private:
        std::atomic_int matched_;
        void on_publication_matched(eprosima::fastdds::dds::DataWriter* ,
        const eprosima::fastdds::dds::PublicationMatchedStatus &info)override{
            if (info.current_count_change == 1) {
                matched_ = info.total_count;
                std::cout << "DataWriter matched." << std::endl;
              } else if (info.current_count_change == -1) {
                matched_ = info.total_count;
                std::cout << "DataWriter unmatched." << std::endl;
              } else {
                std::cout << info.current_count_change
                          << " is not a valid value for PublicationMatchedStatus current "
                             "count change"
                          << std::endl;
              }
        }
    } listener_;

    bool init(){
        type_.register_type(participant_);
        publisher_ = participant_->create_publisher(eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT, nullptr);
        if (publisher_ == nullptr)
        {
            return false;
        }

        topic_ = participant_->create_topic(topic_name_, type_.get_type_name(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
        if (topic_ == nullptr)
        {
            return false;
        }

        writer_ = publisher_->create_datawriter(topic_, eprosima::fastdds::dds::DATAWRITER_QOS_DEFAULT, &listener_);
        if (writer_ == nullptr)
        {
            return false;
        }

        reliability_qos.kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
        writer_qos.reliability(reliability_qos);
        writer_->set_qos(writer_qos);
        return true;
    }

    void publish(MSG_TYPE& msg){
        while(listener_.matched() == 0){
            std::this_thread::sleep_for(
                std::chrono::milliseconds(250)); // Sleep 250 ms
        }
        writer_->write(&msg);
        
    }
private:
    eprosima::fastdds::dds::DomainParticipant *participant_;
    eprosima::fastdds::dds::Publisher *publisher_;
    eprosima::fastdds::dds::DataWriter *writer_;
    eprosima::fastdds::dds::Topic *topic_;
    eprosima::fastdds::dds::TypeSupport type_;

    eprosima::fastdds::dds::ReliabilityQosPolicy reliability_qos;
    eprosima::fastdds::dds::DataWriterQos writer_qos;

    std::string topic_name_;
};

#endif