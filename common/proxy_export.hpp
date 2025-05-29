#pragma once

#ifndef PROXY_DLL_USE_IMPORT
#define PROXY_DLL_FUNCTION extern "C" __declspec(dllexport)
#else
#define PROXY_DLL_FUNCTION extern "C" __declspec(dllimport)
#endif


/*
 * Note: If not specified, all functions are thread unsafe.
 */

using onSocketConnectCallbackType    = void (*)(const char *);
using onSocketDisconnectCallbackType = void (*)(const char *);
using onReconnectAttemptCallbackType = void (*)(int attempt_count, const char *reason);
using onReconnectFailedCallbackType  = void (*)(const char *reason);

// Auto-reconnect configuration structure
struct ProxyReconnectConfig {
    bool enabled = false;
    int interval_ms = 5000;          // Base interval between reconnect attempts
    int max_attempts = -1;           // Maximum attempts (-1 = unlimited)
    int backoff_multiplier = 1;      // Backoff multiplier (1 = no backoff, 2 = exponential)
    int max_interval_ms = 30000;     // Maximum interval with backoff
};


/**
 * @brief Initialize proxy resources, must call it at the beginning
 *
 * @return 0: on success, other on fail
 *
 */
PROXY_DLL_FUNCTION int el_proxy_init();


/**
 * @brief Start the Proxy with the specified address.
 *
 * @param address DAP host url address
 * @return 0: on success, other on fail
 */
PROXY_DLL_FUNCTION int el_proxy_start_with_address(char *address);


/**
 * @brief Force the Proxy to stop. This function can be used at any time.
 *
 */
PROXY_DLL_FUNCTION void el_proxy_stop();


/**
 * @brief Set the callback function to be used when the Proxy connection is successfully established.
 *        This function should be called before `el_proxy_start_with_address`
 *
 * @param callback
 */
PROXY_DLL_FUNCTION void el_proxy_set_on_connect_callback(onSocketConnectCallbackType callback);


/**
 * @brief Set the callback function to be used when the Proxy connection is disconnected.
 *
 * @param callback
 */
PROXY_DLL_FUNCTION void el_proxy_set_on_disconnect_callback(onSocketDisconnectCallbackType callback);


/**
 * @brief Enable auto-reconnect feature with specified configuration
 *
 * @param config Auto-reconnect configuration
 */
PROXY_DLL_FUNCTION void el_proxy_enable_auto_reconnect(ProxyReconnectConfig config);


/**
 * @brief Disable auto-reconnect feature
 *
 */
PROXY_DLL_FUNCTION void el_proxy_disable_auto_reconnect();


/**
 * @brief Set callback for reconnect attempts
 *
 * @param callback Called when a reconnect attempt is made
 */
PROXY_DLL_FUNCTION void el_proxy_set_on_reconnect_attempt_callback(onReconnectAttemptCallbackType callback);


/**
 * @brief Set callback for when reconnect fails permanently
 *
 * @param callback Called when max reconnect attempts reached
 */
PROXY_DLL_FUNCTION void el_proxy_set_on_reconnect_failed_callback(onReconnectFailedCallbackType callback);


/**
 * @brief Get current reconnect attempt count
 *
 * @return Current reconnect attempt count
 */
PROXY_DLL_FUNCTION int el_proxy_get_reconnect_attempt_count();


/**
 * @brief Check if auto-reconnect is currently enabled
 *
 * @return true if enabled, false otherwise
 */
PROXY_DLL_FUNCTION bool el_proxy_is_auto_reconnect_enabled();
