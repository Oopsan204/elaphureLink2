/**
 * @file auto_reconnect_test.cpp
 * @author windowsair (msdn_01@sina.com)
 * @brief Auto-reconnect feature test
 *
 * @copyright BSD-2-Clause
 *
 */

#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>

#include "../../common/proxy_export.hpp"

// Test callbacks
std::atomic<int> connect_count(0);
std::atomic<int> disconnect_count(0);
std::atomic<int> reconnect_attempt_count(0);
std::atomic<int> reconnect_failed_count(0);

void on_connect(const char* msg) {
    connect_count++;
    std::cout << "[CONNECT] " << msg << " (count: " << connect_count << ")" << std::endl;
}

void on_disconnect(const char* msg) {
    disconnect_count++;
    std::cout << "[DISCONNECT] " << msg << " (count: " << disconnect_count << ")" << std::endl;
}

void on_reconnect_attempt(int attempt, const char* reason) {
    reconnect_attempt_count++;
    std::cout << "[RECONNECT ATTEMPT] #" << attempt << " - " << reason << std::endl;
}

void on_reconnect_failed(const char* reason) {
    reconnect_failed_count++;
    std::cout << "[RECONNECT FAILED] " << reason << std::endl;
}

int main() {
    std::cout << "=== Auto-Reconnect Feature Test ===" << std::endl;
    
    // Initialize proxy
    if (el_proxy_init() != 0) {
        std::cerr << "Failed to initialize proxy" << std::endl;
        return -1;
    }
    
    // Set callbacks
    el_proxy_set_on_connect_callback(on_connect);
    el_proxy_set_on_disconnect_callback(on_disconnect);
    el_proxy_set_on_reconnect_attempt_callback(on_reconnect_attempt);
    el_proxy_set_on_reconnect_failed_callback(on_reconnect_failed);
    
    // Configure auto-reconnect
    ProxyReconnectConfig config;
    config.enabled = true;
    config.interval_ms = 3000;         // 3 seconds between attempts
    config.max_attempts = 5;           // Maximum 5 attempts
    config.backoff_multiplier = 2;     // Exponential backoff
    config.max_interval_ms = 15000;    // Max 15 seconds interval
    
    el_proxy_enable_auto_reconnect(config);
    
    // Test 1: Connect to invalid address (should trigger reconnect attempts)
    std::cout << "\n--- Test 1: Invalid Address ---" << std::endl;
    char invalid_address[] = "192.168.1.99";  // Assuming this is unreachable
    
    int result = el_proxy_start_with_address(invalid_address);
    std::cout << "Start result: " << result << std::endl;
    
    // Wait for some reconnect attempts
    std::this_thread::sleep_for(std::chrono::seconds(20));
    
    std::cout << "Reconnect attempts so far: " << el_proxy_get_reconnect_attempt_count() << std::endl;
    
    // Test 2: Disable auto-reconnect and stop
    std::cout << "\n--- Test 2: Disable Auto-Reconnect ---" << std::endl;
    el_proxy_disable_auto_reconnect();
    
    std::cout << "Auto-reconnect enabled: " << (el_proxy_is_auto_reconnect_enabled() ? "Yes" : "No") << std::endl;
    
    el_proxy_stop();
    
    // Test 3: Valid address with auto-reconnect (optional - requires actual DAP server)
    std::cout << "\n--- Test 3: Valid Address (requires DAP server on localhost:3240) ---" << std::endl;
    std::cout << "Skipping valid address test - would need actual DAP server" << std::endl;
    
    // Final statistics
    std::cout << "\n=== Test Results ===" << std::endl;
    std::cout << "Connect events: " << connect_count << std::endl;
    std::cout << "Disconnect events: " << disconnect_count << std::endl;
    std::cout << "Reconnect attempts: " << reconnect_attempt_count << std::endl;
    std::cout << "Reconnect failures: " << reconnect_failed_count << std::endl;
    
    return 0;
}
