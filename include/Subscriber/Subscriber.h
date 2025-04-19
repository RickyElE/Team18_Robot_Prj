#ifndef __SUBSCRIBER_H__
#define __SUBSCRIBER_H__

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <condition_variable>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <memory>
#include <mutex>

template <typename MSG_TYPE>
class SubListener : public eprosima::fastdds::dds::DataReaderListener
{
public:
    SubListener(MSG_TYPE *msg):new_data(false){st = msg;};
    ~SubListener() override {};
    void on_subscription_matched(
        eprosima::fastdds::dds::DataReader *reader,
        const eprosima::fastdds::dds::SubscriptionMatchedStatus &info) override{
            if (info.current_count_change == 1){
                std::cout << "Subscriber matched." << std::endl;
                matched_ = info.total_count;
            }
            else
            if (info.current_count_change == -1){
                std::cout << "Subscriber unmatched." << std::endl;
                matched_ = info.total_count;
            }
            else{
                std::cout << info.current_count_change
              << " is not a valid value for SubscriptionMatchedStatus current "
                 "count change"
              << std::endl;
            }
        }

    void on_data_available(eprosima::fastdds::dds::DataReader* reader)override{
        eprosima::fastdds::dds::SampleInfo info;

        if (reader->read_next_sample(st, &info) == ReturnCode_t::RETCODE_OK)
        {
            // std::cout << "Has sampled" << std::endl;
            if (info.valid_data){
                // std::cout << "Got new data!" << std::endl;
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    new_data = true;
                }
                cv.notify_one();
            }
        }
    }

    MSG_TYPE get_data(){
        if (new_data) return *st;
    }
    void wait_for_data(){
        // std::cout << "Wait for data: new data is "<< new_data << std::endl;
        std::unique_lock<std::mutex> lock(mutex_);
        cv.wait(lock,[this]{return new_data;});
        new_data = false;
    }

    int matched_ = 0;
private:
    MSG_TYPE* st;
    bool new_data{false};
    std::mutex mutex_;
    std::condition_variable cv;
};

template <typename MSG_PUBSUB_TYPE, typename MSG_TYPE>
class Subscriber{
public:
    Subscriber(MSG_PUBSUB_TYPE, MSG_TYPE* msg, std::string topic_name, 
                eprosima::fastdds::dds::DomainParticipant* participant)
                :subscriber_(nullptr), reader_(nullptr), topic_(nullptr),
                type_(new MSG_PUBSUB_TYPE)
    {
        topic_name_ = topic_name;
        participant_ = participant;
        listener_ = std::make_unique<SubListener<MSG_TYPE>>(msg);
    }

    ~Subscriber(){
        if (subscriber_ != nullptr)
        {
            participant_->delete_subscriber(subscriber_);
        }
        if (reader_ != nullptr)
        {
            subscriber_->delete_datareader(reader_);
        }
        if (topic_ != nullptr)
        {
            participant_->delete_topic(topic_);
        }
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }

    bool init(){
        std::cout << "Initializing subscriber." << std::endl;
        type_.register_type(participant_);
        subscriber_ = participant_->create_subscriber(eprosima::fastdds::dds::SUBSCRIBER_QOS_DEFAULT, nullptr);
        if (subscriber_ == nullptr)
        {
            return false;
        }

        topic_ = participant_->create_topic(topic_name_, type_.get_type_name(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
        if (topic_ == nullptr)
        {
            return false;
        }
        
        eprosima::fastdds::dds::DataReaderQos rqos = eprosima::fastdds::dds::DATAREADER_QOS_DEFAULT;
        rqos.reliability().kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
        reader_ = subscriber_->create_datareader(topic_, rqos, listener_.get());
        if (reader_ == nullptr)
        {
            return false;
        }
        return true;
    }
    std::unique_ptr<SubListener<MSG_TYPE>> listener_;
private:
    eprosima::fastdds::dds::DomainParticipant *participant_;
    eprosima::fastdds::dds::Subscriber *subscriber_;
    eprosima::fastdds::dds::DataReader *reader_;
    eprosima::fastdds::dds::Topic *topic_;
    eprosima::fastdds::dds::TypeSupport type_;

    
    std::string topic_name_;
};
#endif