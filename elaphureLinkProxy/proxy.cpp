#include "pch.h"

#include "SocketClient.hpp"


class ProxyManager
{
    public:
    ProxyManager()
        : on_connect_callback_(nullptr),
          on_socket_disconnect_callback_(nullptr),
          on_reconnect_attempt_callback_(nullptr),
          on_reconnect_failed_callback_(nullptr)
    {
    }

    bool is_proxy_running()
    {
        if (client_.get()) {
            return client_.get()->is_socket_running();
        }

        return false;
    }

    void set_on_proxy_connect_callback(onSocketConnectCallbackType callback)
    {
        on_connect_callback_ = callback;

        if (client_.get()) {
            client_.get()->set_connect_callback(callback);
        }
    }

    void set_on_proxy_disconnect_callback(onSocketDisconnectCallbackType callback)
    {
        on_socket_disconnect_callback_ = callback;

        if (client_.get()) {
            client_.get()->set_disconnect_callback(callback);
        }
    }

    int start_with_address(std::string address)
    {
        stop();
        client_ = std::make_unique<SocketClient>();

        if (on_connect_callback_) {
            client_.get()->set_connect_callback(on_connect_callback_);
        }        if (on_socket_disconnect_callback_) {
            client_.get()->set_disconnect_callback(on_socket_disconnect_callback_);
        }
        if (on_reconnect_attempt_callback_) {
            client_.get()->set_reconnect_attempt_callback(on_reconnect_attempt_callback_);
        }        if (on_reconnect_failed_callback_) {
            client_.get()->set_reconnect_failed_callback(on_reconnect_failed_callback_);
        }

        // Apply saved reconnect config
        if (saved_reconnect_config_.enabled) {
            client_.get()->set_reconnect_config(saved_reconnect_config_);
        }

        int ret = client_.get()->init_socket(address, "3240");
        if (ret != 0) {
            return ret;
        }

        return client_.get()->start();
    }

    void stop()
    {
        if (client_.get()) {
            client_.get()->kill();
        }        client_.reset(nullptr);
    }

    // Auto-reconnect methods
    void enable_auto_reconnect(const SocketClient::ReconnectConfig& config)
    {
        if (client_.get()) {
            client_.get()->set_reconnect_config(config);
        }
        saved_reconnect_config_ = config;
    }

    void disable_auto_reconnect()
    {
        SocketClient::ReconnectConfig config;
        config.enabled = false;
        
        if (client_.get()) {
            client_.get()->set_reconnect_config(config);
        }
        saved_reconnect_config_ = config;
    }

    bool is_auto_reconnect_enabled() const
    {
        if (client_.get()) {
            return client_.get()->is_auto_reconnect_enabled();
        }
        return saved_reconnect_config_.enabled;
    }

    int get_reconnect_attempt_count() const
    {
        if (client_.get()) {
            return client_.get()->get_reconnect_attempt_count();
        }
        return 0;
    }

    void set_on_reconnect_attempt_callback(onReconnectAttemptCallbackType callback)
    {
        on_reconnect_attempt_callback_ = callback;
        if (client_.get()) {
            client_.get()->set_reconnect_attempt_callback(callback);
        }
    }

    void set_on_reconnect_failed_callback(onReconnectFailedCallbackType callback)
    {
        on_reconnect_failed_callback_ = callback;
        if (client_.get()) {
            client_.get()->set_reconnect_failed_callback(callback);
        }
    }    private:
    onSocketConnectCallbackType    on_connect_callback_;
    onSocketDisconnectCallbackType on_socket_disconnect_callback_;
    onReconnectAttemptCallbackType on_reconnect_attempt_callback_;
    onReconnectFailedCallbackType  on_reconnect_failed_callback_;
    std::unique_ptr<SocketClient>  client_;
    SocketClient::ReconnectConfig  saved_reconnect_config_;
};

ProxyManager k_manager;


PROXY_DLL_FUNCTION int el_proxy_start_with_address(char *address)
{
    return k_manager.start_with_address(address);
}


PROXY_DLL_FUNCTION void el_proxy_stop()
{
    if (k_is_proxy_init) {
        return k_manager.stop();
    }
}


PROXY_DLL_FUNCTION void el_proxy_set_on_connect_callback(onSocketConnectCallbackType callback)
{
    return k_manager.set_on_proxy_connect_callback(callback);
}


PROXY_DLL_FUNCTION void el_proxy_set_on_disconnect_callback(onSocketDisconnectCallbackType callback)
{
    return k_manager.set_on_proxy_disconnect_callback(callback);
}

// Auto-reconnect API implementations
PROXY_DLL_FUNCTION void el_proxy_enable_auto_reconnect(ProxyReconnectConfig config)
{
    SocketClient::ReconnectConfig socketConfig;
    socketConfig.enabled = config.enabled;
    socketConfig.interval_ms = config.interval_ms;
    socketConfig.max_attempts = config.max_attempts;
    socketConfig.backoff_multiplier = config.backoff_multiplier;
    socketConfig.max_interval_ms = config.max_interval_ms;
    
    k_manager.enable_auto_reconnect(socketConfig);
}

PROXY_DLL_FUNCTION void el_proxy_disable_auto_reconnect()
{
    k_manager.disable_auto_reconnect();
}

PROXY_DLL_FUNCTION void el_proxy_set_on_reconnect_attempt_callback(onReconnectAttemptCallbackType callback)
{
    k_manager.set_on_reconnect_attempt_callback(callback);
}

PROXY_DLL_FUNCTION void el_proxy_set_on_reconnect_failed_callback(onReconnectFailedCallbackType callback)
{
    k_manager.set_on_reconnect_failed_callback(callback);
}

PROXY_DLL_FUNCTION int el_proxy_get_reconnect_attempt_count()
{
    return k_manager.get_reconnect_attempt_count();
}

PROXY_DLL_FUNCTION bool el_proxy_is_auto_reconnect_enabled()
{
    return k_manager.is_auto_reconnect_enabled();
}
