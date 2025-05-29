# Auto-Reconnect Feature Documentation

## Overview

The elaphureLink Proxy now supports automatic reconnection when the connection to the DAP host is lost. This feature helps maintain stable debugging sessions even when network connectivity is intermittent.

## Configuration

### Auto-Reconnect Parameters

```cpp
struct ProxyReconnectConfig {
    bool enabled = false;               // Enable/disable auto-reconnect
    int interval_ms = 5000;            // Base interval between reconnect attempts (ms)
    int max_attempts = -1;             // Maximum attempts (-1 = unlimited)
    int backoff_multiplier = 1;        // Backoff multiplier (1 = no backoff, 2 = exponential)
    int max_interval_ms = 30000;       // Maximum interval with backoff (ms)
};
```

### Configuration Examples

#### Basic Auto-Reconnect
```cpp
ProxyReconnectConfig config;
config.enabled = true;
config.interval_ms = 5000;     // Retry every 5 seconds
config.max_attempts = 10;      // Try up to 10 times
el_proxy_enable_auto_reconnect(config);
```

#### Exponential Backoff
```cpp
ProxyReconnectConfig config;
config.enabled = true;
config.interval_ms = 2000;        // Start with 2 seconds
config.max_attempts = -1;         // Unlimited attempts
config.backoff_multiplier = 2;    // Double interval each time
config.max_interval_ms = 30000;   // Cap at 30 seconds
el_proxy_enable_auto_reconnect(config);
```

## API Reference

### Core Functions

#### `el_proxy_enable_auto_reconnect(ProxyReconnectConfig config)`
Enables auto-reconnect with the specified configuration.

#### `el_proxy_disable_auto_reconnect()`
Disables the auto-reconnect feature.

#### `el_proxy_is_auto_reconnect_enabled()`
Returns whether auto-reconnect is currently enabled.

#### `el_proxy_get_reconnect_attempt_count()`
Returns the current number of reconnect attempts for the active session.

### Callback Functions

#### `el_proxy_set_on_reconnect_attempt_callback(onReconnectAttemptCallbackType callback)`
Sets a callback to be called when a reconnect attempt is made.

**Callback signature:**
```cpp
void callback(int attempt_count, const char* reason);
```

#### `el_proxy_set_on_reconnect_failed_callback(onReconnectFailedCallbackType callback)`
Sets a callback to be called when reconnection fails permanently (max attempts reached).

**Callback signature:**
```cpp
void callback(const char* reason);
```

## Behavior Details

### When Auto-Reconnect Triggers

Auto-reconnect is triggered when:
- Initial connection fails
- Connection is lost during handshake phase
- Connection is lost during device info retrieval
- Connection is lost during data transfer
- Socket errors occur during communication

### Reconnect Process

1. **Connection Loss Detection**: When a network error occurs, the system determines if auto-reconnect should be attempted
2. **Attempt Scheduling**: A timer is set based on the configured interval and backoff settings
3. **Reconnection Attempt**: A new socket is created and connection is attempted
4. **Success/Failure Handling**: 
   - On success: Reset attempt counter and resume normal operation
   - On failure: Schedule next attempt or give up if max attempts reached

### Backoff Calculation

When `backoff_multiplier > 1`, the reconnect interval increases with each failed attempt:

```
actual_interval = base_interval * (backoff_multiplier ^ attempt_count)
```

The interval is capped at `max_interval_ms`.

### Example Timeline with Exponential Backoff
```
Attempt 1: After 2 seconds
Attempt 2: After 4 seconds  
Attempt 3: After 8 seconds
Attempt 4: After 16 seconds
Attempt 5: After 30 seconds (capped)
Attempt 6: After 30 seconds (capped)
...
```

## Integration Examples

### Console Application
```cpp
#include "proxy_export.hpp"

void on_reconnect_attempt(int attempt, const char* reason) {
    printf("Reconnect attempt #%d: %s\\n", attempt, reason);
}

int main() {
    el_proxy_init();
    
    ProxyReconnectConfig config;
    config.enabled = true;
    config.interval_ms = 3000;
    config.max_attempts = 5;
    
    el_proxy_enable_auto_reconnect(config);
    el_proxy_set_on_reconnect_attempt_callback(on_reconnect_attempt);
    
    el_proxy_start_with_address("192.168.1.100");
    
    // Your application logic...
    
    return 0;
}
```

### WPF Application Integration
```csharp
// In your ViewModel or service class
public void EnableAutoReconnect(bool enabled, int intervalMs = 5000, int maxAttempts = -1) {
    var config = new ProxyReconnectConfig {
        enabled = enabled,
        interval_ms = intervalMs,
        max_attempts = maxAttempts,
        backoff_multiplier = 2,
        max_interval_ms = 30000
    };
    
    NativeMethods.el_proxy_enable_auto_reconnect(config);
    NativeMethods.el_proxy_set_on_reconnect_attempt_callback(OnReconnectAttempt);
}

private void OnReconnectAttempt(int attempt, string reason) {
    Dispatcher.Invoke(() => {
        StatusMessage = $"Reconnecting... Attempt {attempt}: {reason}";
        IsReconnecting = true;
    });
}
```

## Best Practices

### Recommended Settings
- **Development**: Short intervals (2-5 seconds), limited attempts (5-10)
- **Production**: Longer intervals (5-10 seconds), exponential backoff, unlimited or high attempt count
- **Battery-powered devices**: Longer intervals to conserve power

### Error Handling
- Always set reconnect attempt and failure callbacks to provide user feedback
- Consider implementing circuit breaker pattern for persistent failures
- Log reconnection events for debugging and monitoring

### Performance Considerations
- Auto-reconnect attempts use background timers and don't block the main thread
- Network operations are asynchronous and won't freeze the UI
- Consider the impact of frequent reconnection attempts on network resources

## Troubleshooting

### Common Issues

1. **Reconnect not triggering**: Ensure `enabled = true` in configuration
2. **Too frequent attempts**: Increase `interval_ms` or implement backoff
3. **Callbacks not called**: Verify callbacks are set before starting connection
4. **Memory leaks**: Ensure proper cleanup when disabling auto-reconnect

### Debug Information
Use the following to monitor auto-reconnect behavior:
- `el_proxy_is_auto_reconnect_enabled()` - Check if feature is active
- `el_proxy_get_reconnect_attempt_count()` - Monitor attempt count
- Reconnect attempt callback - Log all reconnection events
- Reconnect failed callback - Detect when max attempts reached
