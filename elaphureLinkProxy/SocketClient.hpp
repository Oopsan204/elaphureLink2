#pragma once
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>

#include <sdkddkver.h>
#include "thirdparty/asio/include/asio.hpp"

#include "pch.h"

using asio::ip::tcp;

class SocketClient
{
    public:
    // Auto-reconnect configuration
    struct ReconnectConfig {
        bool enabled = false;
        int interval_ms = 5000;          // Base interval between reconnect attempts
        int max_attempts = -1;           // Maximum attempts (-1 = unlimited)
        int backoff_multiplier = 1;      // Backoff multiplier (1 = no backoff, 2 = exponential)
        int max_interval_ms = 30000;     // Maximum interval with backoff
    };

    SocketClient()
        : is_running_(false),
          connect_callback_(nullptr),
          disconnect_callback_(nullptr),
          reconnect_attempt_callback_(nullptr),
          reconnect_failed_callback_(nullptr),
          current_reconnect_count_(0),
          is_reconnecting_(false)
    {
    }

    void do_something();

    ~SocketClient()
    {
        // FIXME: access violation

        // ~thread() require to join or detach
        if (main_thread_.joinable()) {
            main_thread_.join();
        }
    }    int init_socket(std::string address, std::string port = "3240")
    {
        socket_.reset(nullptr); // prevent double free

        // Save address and port for reconnection
        saved_address_ = address;
        saved_port_ = port;

        io_context_ = std::make_unique<asio::io_context>();
        tcp::resolver resolver(get_io_context());

        socket_ = std::make_unique<asio::ip::tcp::socket>(get_io_context());

        try {
            endpoint_ = resolver.resolve(address, port);
        } catch (std::exception &e) {
            return -1;
        }

        return 0;
    }
    void kill()
    {
        is_running_ = false;
        is_reconnecting_ = false;
        
        // Cancel reconnect timer if active
        if (reconnect_timer_) {
            reconnect_timer_->cancel();
        }
        
        if (k_is_proxy_init) {
            k_shared_memory_ptr->info_page.is_proxy_ready       = 0;
            k_shared_memory_ptr->consumer_page.command_response = 0xFFFFFFFF; // invalid value
            SetEvent(k_producer_event);                                       // wake up
            SetEvent(k_consumer_event);
        }

        socket_.get()->close();
        io_context_.get()->stop();
        // Resources should not be released immediately, as this will result in a deadlock.
        Sleep(100);
    }

    int start()
    {
        using namespace std::chrono_literals;

        std::unique_lock<std::mutex> lk(running_status_mutex_);
        if (is_running_) {
            return 0; // already start
        }

        is_running_post_done_ = false;
        is_running_           = true;

        do_connect(endpoint_);

        main_thread_ = std::thread([&]() { // start main thread
            try {
                get_io_context().run();
            } catch (std::exception &e) {
                set_running_status(false, e.what());
            }

        });

        running_cv_.wait(lk, [this]() { return is_running_post_done_; });

        return !is_running_;
    }

    void wait_main_thread()
    {
        main_thread_.join();
    }


    //
    //
    // setter/getter
    public:
    asio::io_context &get_io_context()
    {
        return *(io_context_.get());
    }

    asio::ip::tcp::socket &get_socket()
    {
        return *(socket_.get());
    }

    bool is_socket_running()
    {
        return is_running_;
    }

    void set_running_status(bool status, const std::string msg)
    {
        is_running_ = status;
        if (status) {
            if (connect_callback_) {
                connect_callback_(msg.c_str());
            }
        } else {
            if (disconnect_callback_) {
                disconnect_callback_(msg.c_str());
            }
        }
    }

    void set_connect_callback(onSocketConnectCallbackType callback)
    {
        connect_callback_ = callback;
    }    void set_disconnect_callback(onSocketDisconnectCallbackType callback)
    {
        disconnect_callback_ = callback;
    }

    // Auto-reconnect methods
    void set_reconnect_config(const ReconnectConfig& config)
    {
        reconnect_config_ = config;
    }

    ReconnectConfig get_reconnect_config() const
    {
        return reconnect_config_;
    }

    bool is_auto_reconnect_enabled() const
    {
        return reconnect_config_.enabled;
    }

    int get_reconnect_attempt_count() const
    {
        return current_reconnect_count_;
    }

    bool is_reconnecting() const
    {
        return is_reconnecting_;
    }

    void set_reconnect_attempt_callback(onReconnectAttemptCallbackType callback)
    {
        reconnect_attempt_callback_ = callback;
    }

    void set_reconnect_failed_callback(onReconnectFailedCallbackType callback)
    {
        reconnect_failed_callback_ = callback;
    }

    private:
    void close()
    {
        is_running_ = false;
        if (k_is_proxy_init) {
            k_shared_memory_ptr->info_page.is_proxy_ready       = 0;
            k_shared_memory_ptr->consumer_page.command_response = 0xFFFFFFFF; // invalid value
            SetEvent(k_producer_event);                                       // wake up
            SetEvent(k_consumer_event);
        }

        asio::post(get_io_context(),
                   [this]() {
                       get_socket().close();
                   });
    }    void do_connect(const tcp::resolver::results_type &endpoints)
    {
        asio::async_connect(get_socket(), endpoints,
                            [&](std::error_code ec, tcp::endpoint) {
                                //is_running_post_done_ = true;
                                if (!ec) {
                                    asio::ip::tcp::no_delay option(true);
                                    get_socket().set_option(option);

                                    set_keep_alive();

                                    // Reset reconnect counter on successful connection
                                    current_reconnect_count_ = 0;
                                    is_reconnecting_ = false;

                                    do_handshake();
                                } else {
                                    // Check if we should attempt reconnect
                                    if (reconnect_config_.enabled && should_attempt_reconnect()) {
                                        schedule_reconnect(ec.message());
                                    } else {
                                        notify_connection_status(false, ec.message());
                                        close();
                                    }
                                }
                            });
    }
    void set_keep_alive();

    // handshake phase
    void do_handshake();

    // data phase
    void get_device_info();
    void do_data_process();

    void notify_connection_status(bool status, const std::string msg)
    {
        is_running_post_done_ = true;
        set_running_status(status, msg);
        running_cv_.notify_all();
    }
    private:
    bool                    is_running_;
    bool                    is_running_post_done_;
    std::mutex              running_status_mutex_;
    std::condition_variable running_cv_;

    std::unique_ptr<asio::io_context> io_context_;
    std::unique_ptr<tcp::socket>      socket_;

    tcp::resolver::results_type endpoint_;

    std::thread main_thread_;

    onSocketConnectCallbackType    connect_callback_;
    onSocketDisconnectCallbackType disconnect_callback_;

    // Auto-reconnect members
    ReconnectConfig reconnect_config_;
    std::atomic<int> current_reconnect_count_;
    std::atomic<bool> is_reconnecting_;
    std::unique_ptr<asio::steady_timer> reconnect_timer_;
    std::string saved_address_;
    std::string saved_port_;
    
    onReconnectAttemptCallbackType reconnect_attempt_callback_;
    onReconnectFailedCallbackType reconnect_failed_callback_;

    // Auto-reconnect methods
    bool should_attempt_reconnect() const
    {
        if (reconnect_config_.max_attempts == -1) {
            return true;  // Unlimited attempts
        }
        return current_reconnect_count_ < reconnect_config_.max_attempts;
    }

    int calculate_reconnect_delay() const
    {
        int delay = reconnect_config_.interval_ms;
        
        if (reconnect_config_.backoff_multiplier > 1) {
            // Apply exponential backoff
            for (int i = 0; i < current_reconnect_count_; ++i) {
                delay *= reconnect_config_.backoff_multiplier;
                if (delay >= reconnect_config_.max_interval_ms) {
                    delay = reconnect_config_.max_interval_ms;
                    break;
                }
            }
        }
        
        return delay;
    }

    void schedule_reconnect(const std::string& reason)
    {
        if (!reconnect_timer_) {
            reconnect_timer_ = std::make_unique<asio::steady_timer>(get_io_context());
        }

        current_reconnect_count_++;
        is_reconnecting_ = true;

        // Notify about reconnect attempt
        if (reconnect_attempt_callback_) {
            reconnect_attempt_callback_(current_reconnect_count_, reason.c_str());
        }

        int delay = calculate_reconnect_delay();
        reconnect_timer_->expires_after(std::chrono::milliseconds(delay));

        reconnect_timer_->async_wait([this](std::error_code ec) {
            if (!ec && is_running_) {
                attempt_reconnect();
            }
        });
    }

    void attempt_reconnect()
    {
        if (!is_running_ || !reconnect_config_.enabled) {
            return;
        }

        // Check if we've exceeded max attempts
        if (!should_attempt_reconnect()) {
            is_reconnecting_ = false;
            if (reconnect_failed_callback_) {
                reconnect_failed_callback_("Maximum reconnect attempts reached");
            }
            notify_connection_status(false, "Reconnection failed - max attempts reached");
            close();
            return;
        }

        // Create new socket and attempt reconnection
        socket_ = std::make_unique<asio::ip::tcp::socket>(get_io_context());
        do_connect(endpoint_);
    }

    void on_connection_lost(const std::string& reason)
    {
        if (reconnect_config_.enabled && should_attempt_reconnect() && is_running_) {
            schedule_reconnect(reason);
        } else {
            is_reconnecting_ = false;
            notify_connection_status(false, reason);
        }
    }
};
