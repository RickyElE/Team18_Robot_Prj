    #include <websocketpp/config/asio_no_tls.hpp>
    #include <websocketpp/server.hpp>
    #include <nlohmann/json.hpp> // JSON 庫
    #include "../Publisher/Publisher.h"
    #include "../Subscriber/Subscriber.h"
    #include "../BatteryMsg/BatteryMsg.h"
    #include "../BatteryMsg/BatteryMsgPubSubTypes.h"

    #include <atomic>
    #include <thread>
    #include <chrono>
    #include <mutex>
    #include <functional>
    using namespace websocketpp;
    using json = nlohmann::json;

    typedef server<config::asio> websocket_server;


    class ppr_remote_node{


    public:
        ppr_remote_node(): ws_server_(), ws_connected_(false) {
            // 配置 WebSocket 伺服器
            ws_server_.clear_access_channels(log::alevel::all);
            ws_server_.clear_error_channels(log::elevel::all);
            
            ws_server_.init_asio();
            
            // 設置回調函數
            ws_server_.set_open_handler(bind(&ppr_remote_node::on_open, this, std::placeholders::_1));
            ws_server_.set_close_handler(bind(&ppr_remote_node::on_close, this, std::placeholders::_1));
        };
        
        ~ppr_remote_node(){
            running_ = false;
            
            if (worker_.joinable()) {
                worker_.join();
            }
            
            if (ws_thread_.joinable()) {
                ws_thread_.join();
            }
        };
        // 返回运行状态
        bool is_running() const {
            return running_;
        }

        // WebSocket 連線處理函數
        void on_open(connection_hdl hdl) {
            std::lock_guard<std::mutex> lock(mutex_);
            ws_hdl_ = hdl;
            ws_connected_ = true;
            std::cout << "WebSocket Client Connected!" << std::endl;
        }

        void on_close(connection_hdl hdl) {
            std::lock_guard<std::mutex> lock(mutex_);
            ws_connected_ = false;
            std::cout << "WebSocket Client Disconnected!" << std::endl;
        }

        // 發送電池數據到 WebSocket
    // 修改 send_battery_status 函数
    void send_battery_status(float voltage, float percentage) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (ws_connected_) {
            try {
                // 直接发送 percentage 和 voltage
                json msg = {
                    {"percentage", percentage},
                    {"voltage", voltage}
                };
                ws_server_.send(ws_hdl_, msg.dump(), frame::opcode::text);
                std::cout << "Sent battery data to WebSocket: " << msg.dump() << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Error sending to WebSocket: " << e.what() << std::endl;
            }
        }
    }

        // 啟動 WebSocket 伺服器
        void start_websocket_server() {
            int bind_attempts = 0;
            const int MAX_BIND_ATTEMPTS = 3;
            while (bind_attempts < MAX_BIND_ATTEMPTS) {
            try {
                ws_server_.listen("0.0.0.0",8082);
                ws_server_.start_accept();
                std::cout << "WebSocket server started on port 8082" << std::endl;
                ws_server_.run();
                break;
            } catch (const std::exception& e) {
                std::cerr << "WebSocket server error: " 
                << e.what() << std::endl;

                if (bind_attempts < MAX_BIND_ATTEMPTS) {
                    std::this_thread::sleep_for(std::chrono::seconds(2));
                } else {
                    std::cerr << "Failed to start WebSocket server after " 
                              << MAX_BIND_ATTEMPTS << " attempts" << std::endl;
                }
            }
         }
        }

        void run();
        
        bool start(){
            running_ = true;
            
            // 啟動 WebSocket 伺服器線程
            ws_thread_ = std::thread(&ppr_remote_node::start_websocket_server, this);
            
            // 啟動電池數據處理線程
            worker_ = std::thread(&ppr_remote_node::run, this);
            
            return true;
        }
        
    private:
        std::atomic<bool> running_{false};
        std::thread worker_;
        std::thread ws_thread_;
        
        websocket_server ws_server_;
        connection_hdl ws_hdl_;
        bool ws_connected_;
        std::mutex mutex_;  // 用於保護 WebSocket 連接狀態
    };

    void ppr_remote_node::run(){    
        std::cout << "Running ppr_remote_node" << std::endl;
        
        // 設置 FastDDS 訂閱
        BatteryMsg battery_msg{};
        eprosima::fastdds::dds::DomainParticipantQos pqos;
        pqos.name("Participant_Subscriber");
        
        Subscriber<BatteryMsgPubSubType, BatteryMsg> battery_subscriber(
            BatteryMsgPubSubType(),
            &battery_msg,
            "Battery_Topic", 
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(0, pqos)
        );
        
        if (!battery_subscriber.init()) {
            std::cerr << "Failed to initialize battery subscriber" << std::endl;
            return;
        }
        
        std::cout << "Battery subscriber initialized successfully" << std::endl;
        
        // 主循環: 接收電池數據並轉發到 WebSocket
        while(running_){
            try {
                // 等待新的電池數據
                battery_subscriber.listener_->wait_for_data();
                
                // 獲取最新的電池數據
                float voltage = battery_msg.voltage();
                float percentage = battery_msg.percentage();
                
                std::cout << "Received battery data: Voltage=" << voltage 
                        << ", Percentage=" << percentage << std::endl;
                
                // 將數據發送到 WebSocket
                send_battery_status(voltage, percentage);
                
                // 短暫休眠以減少CPU使用率
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            } catch (const std::exception& e) {
                std::cerr << "Error in battery data handling: " << e.what() << std::endl;
            }
        }
        
        std::cout << "ppr_remote_node run loop ended" << std::endl;
    }
    int main() {
        ppr_remote_node remote_node;

        // 启动节点
        if (!remote_node.start()) {
            std::cerr << "Failed to start remote node" << std::endl;
            return -1;
        }

        // 保持主线程运行
        while (remote_node.is_running()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        return 0;
    }
