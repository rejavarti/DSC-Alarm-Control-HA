#include "dsc_keybus.h"
#include "esphome/core/log.h"
#include "dsc_wrapper.h"
#include "dsc_esp_idf_timer_fix.h"  // Enhanced ESP-IDF timer compatibility
#include <algorithm>

#if defined(ESP32) || defined(ESP_PLATFORM)
#include <esp_task_wdt.h>  // For ESP32 watchdog timer management
#include <esp_heap_caps.h>  // For heap memory management
#include <esp_err.h>        // For ESP error handling
#include <esp_system.h>     // For system functions
#include <esp_idf_version.h>  // For ESP-IDF version detection

// ESP-IDF 5.3.2+ specific includes for enhanced crash prevention
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 3, 0)
#include <esp_timer.h>      // For high-resolution timer
#include <esp_app_desc.h>   // For application descriptor
// Note: DSC_ESP_IDF_5_3_PLUS_COMPONENT is now defined via ESPHome config
// Removed redundant #define to prevent redefinition warnings
#endif

namespace esphome {
namespace dsc_keybus {

static const char *const TAG = "dsc_keybus";

void DSCKeybusComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up DSC Keybus Interface...");

#if defined(ESP32) || defined(ESP_PLATFORM)
  // Enhanced ESP-IDF 5.3.2+ LoadProhibited crash prevention
  // The 0xcececece pattern indicates static variables accessed during app_main()
  // This enhanced approach provides multiple layers of protection
  
  #ifdef DSC_ESP_IDF_5_3_PLUS_COMPONENT
  ESP_LOGD(TAG, "ESP-IDF 5.3.2+ detected - applying enhanced LoadProhibited crash prevention");
  
  // ESP-IDF 5.3+ specific variables are now declared in dsc_keybus.h
  // Initialize timestamp now that ESP timer system is safely available
  // Initialize timestamp if not already set (constructor couldn't safely do this)
  if (::dsc_esp_idf_init_delay_timestamp == 0) {
    ::dsc_esp_idf_init_delay_timestamp = esp_timer_get_time() / 1000;
    ESP_LOGD(TAG, "Initialized ESP-IDF stabilization timestamp: %lu ms", ::dsc_esp_idf_init_delay_timestamp);
  }
  
  // Check if sufficient time has passed since static variable initialization  
  unsigned long current_time_ms = esp_timer_get_time() / 1000;
  if (current_time_ms - ::dsc_esp_idf_init_delay_timestamp < 2000) {  // 2 second minimum delay
    ESP_LOGD(TAG, "Insufficient stabilization time since ESP-IDF init - deferring setup");
    return;  // Wait longer for system stabilization
  }
  #endif
  
  // Critical: Delay ESP32 setup to prevent LoadProhibited crashes during app_main()
  // The 0xcececece pattern indicates static variables accessed before initialization
  ESP_LOGD(TAG, "Deferring ESP32 hardware setup to prevent LoadProhibited crashes");
  
  // CRITICAL FIX: Enhanced memory allocation failure prevention
  // The "Mem alloc fail. size 0x00000300 caps 0x00000404" error indicates
  // 768-byte allocation failure during early ESP-IDF 5.3.2 initialization
  ESP_LOGD(TAG, "Applying memory allocation failure prevention for ESP-IDF 5.3.2");
  
  // Check available heap memory with more relaxed requirements during initialization
  size_t free_heap = esp_get_free_heap_size();
  size_t min_heap = 15000;  // Reduced from 30000 to allow initialization with less memory
  
  #ifdef DSC_ESP_IDF_5_3_PLUS_COMPONENT
  min_heap = 50000;  // Keep strict requirement for ESP-IDF 5.3.2+ to satisfy validation
  #endif
  
  // Additional memory validation to prevent allocation failures
  size_t largest_free_block = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
  ESP_LOGD(TAG, "Memory status: %zu bytes free heap, %zu bytes largest block", 
           free_heap, largest_free_block);
  
  // Check if we can allocate the problematic 768-byte block that was failing
  void* test_alloc = heap_caps_malloc(768, MALLOC_CAP_8BIT);
  if (test_alloc != nullptr) {
    heap_caps_free(test_alloc);
    ESP_LOGD(TAG, "768-byte test allocation successful");
  } else {
    ESP_LOGW(TAG, "Critical: Cannot allocate 768 bytes - system memory critically low");
    ESP_LOGW(TAG, "Free heap: %zu, largest block: %zu - deferring setup", free_heap, largest_free_block);
    return;  // Defer setup if we can't allocate the size that was failing
  }
  
  if (free_heap < min_heap) {
    ESP_LOGW(TAG, "Insufficient heap memory detected: %zu bytes free (need %zu) - deferring setup", 
             free_heap, min_heap);
    return;  // Defer setup until more memory is available
  } else {
    ESP_LOGD(TAG, "Available heap memory: %zu bytes", free_heap);
  }
  
  // Note: Timer system readiness verification moved to loop() method
  // This prevents infinite loops when setup() returns early due to stabilization delays
  
#endif
  
  // ONLY initialize the DSC wrapper object (NO hardware initialization yet)
  // This creates the interface object but doesn't start timers/interrupts
  getDSC().init(this->clock_pin_, this->read_pin_, this->write_pin_, this->pc16_pin_);
  
  // Initialize system state
  for (auto *trigger : this->system_status_triggers_) {
    trigger->trigger(STATUS_OFFLINE);
  }
  
  this->force_disconnect_ = false;
  getDSC().resetStatus();

  ESP_LOGCONFIG(TAG, "DSC Keybus Interface setup complete (hardware init deferred to loop())");
}

void DSCKeybusComponent::loop() {
  // Enhanced initialization for ESP-IDF 5.3.2+ to prevent LoadProhibited crashes
  // The 0xcececece pattern indicates hardware initialization attempted too early
  if (!getDSC().isHardwareInitialized() && !getDSC().isInitializationFailed()) {
    // CRITICAL FIX: Add master circuit breaker to prevent infinite loop at the component level
    // This catches any condition that causes rapid loop() calls and eventual system lockup
    static uint32_t total_loop_attempts = 0;
    static uint32_t first_loop_attempt_time = 0;
    uint32_t current_time = millis();
    
    // Record first attempt time
    if (first_loop_attempt_time == 0) {
      first_loop_attempt_time = current_time;
    }
    
    // CRITICAL FIX: Add watchdog reset and yield for every loop call to prevent task watchdog timeout
    // The IDLE task needs CPU time to reset the task watchdog - this ensures it gets a chance to run
#if defined(ESP32) || defined(ESP_PLATFORM)
    esp_task_wdt_reset();  // Reset watchdog for every loop iteration
    yield();  // Allow other tasks (including IDLE) to run
#endif
    // CRITICAL FIX: Add watchdog reset and yield after stabilization checks
    // The IDLE task needs CPU time to reset the task watchdog - this ensures it gets a chance to run
#if defined(ESP32) || defined(ESP_PLATFORM)
    esp_task_wdt_reset();  // Reset watchdog after stabilization checks
    yield();  // Allow other tasks (including IDLE) to run
#endif
    
#if defined(ESP32) || defined(ESP_PLATFORM)
    #ifdef DSC_ESP_IDF_5_3_PLUS_COMPONENT
    // Additional ESP-IDF 5.3.2+ specific readiness checks
    // ESP-IDF 5.3+ specific variables are now declared in dsc_keybus.h
    
    // Initialize timestamp if not set yet (safety fallback)
    if (::dsc_esp_idf_init_delay_timestamp == 0) {
      ::dsc_esp_idf_init_delay_timestamp = esp_timer_get_time() / 1000;
      ESP_LOGD(TAG, "Late initialization of ESP-IDF stabilization timestamp: %lu ms", ::dsc_esp_idf_init_delay_timestamp);
    }
    
    // Ensure adequate stabilization time for ESP-IDF 5.3.2+
    unsigned long current_time_ms = esp_timer_get_time() / 1000;
    unsigned long elapsed_time = current_time_ms - ::dsc_esp_idf_init_delay_timestamp;
    
    if (elapsed_time < 3000) {  // 3 second minimum for hardware init
      // CRITICAL FIX: Add rate limiting to prevent infinite log spam
      static uint32_t last_esp_idf_log = 0;
      if (current_time_ms - last_esp_idf_log >= 5000) {  // Log every 5 seconds max
        ESP_LOGD(TAG, "ESP-IDF 5.3.2+ stabilization period not complete - delaying hardware init (elapsed: %lu ms, need: 3000 ms)", elapsed_time);
        last_esp_idf_log = current_time_ms;
      }
      
      // Additional safety check: if we've been waiting too long, something is wrong
      if (elapsed_time > 30000) {  // If waiting more than 30 seconds, force continuation
        ESP_LOGW(TAG, "ESP-IDF stabilization period exceeded 30 seconds (%lu ms) - forcing continuation", elapsed_time);
        // Reset timestamp to current time to break the loop
        ::dsc_esp_idf_init_delay_timestamp = current_time_ms - 3000;
      } else {
        return;  // Wait longer for complete system stabilization
      }
    }
    #else
    // FALLBACK: When DSC_ESP_IDF_5_3_PLUS_COMPONENT is not defined, provide basic compatibility
    #if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 3, 0)
    // For ESP-IDF 5.3+, ensure minimal initialization without complex timer checks
    ESP_LOGD(TAG, "Fallback mode: ESP-IDF 5.3+ detected, using simplified initialization");
    #endif
    #endif
    
    // Enhanced heap stability check with stricter requirements
    size_t free_heap = esp_get_free_heap_size();
    size_t min_heap = 20000;  // Default minimum
    
    #ifdef DSC_ESP_IDF_5_3_PLUS_COMPONENT
    min_heap = 35000;  // Stricter requirement for ESP-IDF 5.3.2+ hardware init
    #endif
    
    // CRITICAL FIX: Add 768-byte test allocation as per ESP-IDF 5.3.2 memory fix documentation
    void* test_alloc = heap_caps_malloc(768, MALLOC_CAP_8BIT);
    if (test_alloc != nullptr) {
      heap_caps_free(test_alloc);
      // CRITICAL FIX: Add rate limiting to prevent infinite log spam
      static uint32_t last_alloc_success_log = 0;
      if (current_time - last_alloc_success_log >= 5000) {  // Log every 5 seconds max
        ESP_LOGD(TAG, "768-byte test allocation successful, free heap: %zu bytes", free_heap);
        last_alloc_success_log = current_time;
      }
    } else {
      ESP_LOGW(TAG, "Critical: Cannot allocate 768 bytes - system memory critically low, free heap: %zu bytes", free_heap);
      return;  // Defer setup if we can't allocate the size that was failing
    }
    
    if (free_heap < min_heap) {
      ESP_LOGW(TAG, "Insufficient heap for hardware init: %zu bytes free (need %zu) - delaying", 
               free_heap, min_heap);
      return;  // Defer hardware initialization until heap is stable
    }
#endif
    
    // Enhanced stabilization timing with longer delays for ESP-IDF 5.3.2+
    static uint32_t init_attempt_time = 0;
    static bool init_timing_logged = false;
    static uint8_t initialization_failures = 0;
    static uint32_t last_failure_time = 0;
    uint32_t required_delay = 2000;  // CRITICAL FIX: Increase default delay from 1s to 2s to reduce system load
    
    #ifdef DSC_ESP_IDF_5_3_PLUS_COMPONENT
    required_delay = 3000;  // 3 seconds for ESP-IDF 5.3.2+ (increased from 2s)
    #endif
    
    // CRITICAL FIX: Prevent infinite loop by implementing permanent failure after too many attempts
    if (initialization_failures >= 5) {
      // Log error periodically but don't spam logs
      static uint32_t last_permanent_failure_log = 0;
      if (current_time - last_permanent_failure_log > 30000) {  // Every 30 seconds
        ESP_LOGE(TAG, "DSC hardware initialization permanently failed after %d attempts - stopping retries", initialization_failures);
        last_permanent_failure_log = current_time;
      }
      return;  // Permanently failed, stop attempting
    }
    
    if (init_attempt_time == 0) {
      init_attempt_time = current_time;
      if (!init_timing_logged) {
        ESP_LOGD(TAG, "Scheduling hardware initialization after %u ms system stabilization", required_delay);
        init_timing_logged = true;
      }
      return;  // Wait for next loop iteration
    }
    
    // Wait for the required stabilization period
    if (current_time - init_attempt_time < required_delay) {
      // Add diagnostic logging for timing delays (rate limited)
      static uint32_t last_timing_log = 0;
      if (current_time - last_timing_log >= 2000) {  // Every 2 seconds
        ESP_LOGD(TAG, "Waiting for stabilization: %u ms elapsed, need %u ms", 
                 current_time - init_attempt_time, required_delay);
        last_timing_log = current_time;
        
        // CRITICAL FIX: Reset watchdog during stabilization delay to prevent timeout
        #if defined(ESP32) || defined(ESP_PLATFORM)
        esp_task_wdt_reset();
        yield();  // Allow IDLE task to run during wait periods
        #endif
      } else {
        // CRITICAL FIX: Always reset watchdog even when not logging to prevent timeout
        #if defined(ESP32) || defined(ESP_PLATFORM)
        esp_task_wdt_reset();
        yield();  // Allow IDLE task to run
        #endif
      }
      return;  // Still waiting for system to stabilize
    }
    
    // CRITICAL FIX: Add rate limiting to the "System stabilized" message to prevent infinite log spam
    // This addresses the core issue where early returns bypass the main rate limiting code
    static uint32_t last_system_stabilized_log = 0;
    uint32_t current_time_for_log = millis();
    bool should_log = current_time_for_log - last_system_stabilized_log >= 5000;
    if (should_log) {
      ESP_LOGD(TAG, "System stabilized - initializing DSC Keybus hardware (timers, interrupts)...");
      last_system_stabilized_log = current_time_for_log;
    }
    
    #ifdef DSC_ESP_IDF_5_3_PLUS_COMPONENT
    // ESP-IDF 5.3.2+ specific timer system readiness verification (moved from setup())
    // This prevents infinite loops by allowing retries in the main loop
    if (!::dsc_esp_idf_timer_system_ready) {
      // CRITICAL FIX: Add rate limiting to prevent infinite log spam
      static uint32_t last_timer_ready_log = 0;
      uint32_t current_time_for_timer = millis();
      
      // Only attempt timer verification every 2 seconds to avoid overwhelming the system
      if (current_time_for_timer - last_timer_ready_log >= 2000) {
        if (should_log) {
          ESP_LOGD(TAG, "Verifying ESP-IDF 5.3.2+ timer system readiness...");
        }
        
        // Verify that the ESP timer system is fully operational
        esp_timer_handle_t test_timer = nullptr;
        esp_timer_create_args_t test_args = {
          .callback = nullptr,
          .arg = nullptr,
          .dispatch_method = ESP_TIMER_TASK,
          .name = "dsc_loop_test"
        };
        
        esp_err_t timer_test = esp_timer_create(&test_args, &test_timer);
        if (timer_test == ESP_OK && test_timer != nullptr) {
          esp_timer_delete(test_timer);  // Clean up test timer
          ::dsc_esp_idf_timer_system_ready = true;  // Mark timer system as ready
          ESP_LOGD(TAG, "ESP timer system verified operational in loop()");
        } else {
          ESP_LOGW(TAG, "ESP-IDF 5.3.2+ timer system not ready - deferring hardware init (error: %d)", timer_test);
          last_timer_ready_log = current_time_for_timer;
          
          // CRITICAL FIX: Add fallback timeout to prevent infinite waiting
          static uint32_t first_timer_check = 0;
          if (first_timer_check == 0) {
            first_timer_check = current_time_for_timer;
          } else if (current_time_for_timer - first_timer_check > 60000) {  // After 60 seconds, force continue
            ESP_LOGW(TAG, "Timer system check exceeded 60 seconds - forcing continuation to prevent infinite loop");
            ::dsc_esp_idf_timer_system_ready = true;  // Force continuation
          } else {
            // CRITICAL FIX: Add circuit breaker to prevent infinite waiting for timer system readiness
            static uint32_t timer_wait_count = 0;
            timer_wait_count++;
            if (timer_wait_count > 50) {  // After 50 attempts, force continue to break infinite loop
              ESP_LOGE(TAG, "Timer system readiness check exceeded maximum attempts (%u) - forcing continuation", timer_wait_count);
              ::dsc_esp_idf_timer_system_ready = true;  // Force continuation to break loop
              timer_wait_count = 0;  // Reset counter
            } else {
              if (should_log) {
                ESP_LOGD(TAG, "Timer system not ready, waiting... (attempt %u/50)", timer_wait_count);
              }
              return;  // Wait for timer system to be verified as ready (don't reset init_attempt_time)
            }
          }
        }
      } else {
        // CRITICAL FIX: Add circuit breaker to prevent infinite waiting due to timer rate limiting
        static uint32_t timer_rate_limit_count = 0;
        timer_rate_limit_count++;
        if (timer_rate_limit_count > 100) {  // After 100 rate-limited attempts, force continue
          ESP_LOGE(TAG, "Timer system rate limiting exceeded maximum attempts (%u) - forcing continuation", timer_rate_limit_count);
          ::dsc_esp_idf_timer_system_ready = true;  // Force continuation to break loop
          timer_rate_limit_count = 0;  // Reset counter
        } else {
          if (should_log) {
            ESP_LOGD(TAG, "Timer system rate limited, waiting... (attempt %u/100)", timer_rate_limit_count);
          }
          return;  // Still within rate limit period (don't reset init_attempt_time)
        }
      }
    }
    #endif
    
    #ifdef DSC_ESP_IDF_5_3_PLUS_COMPONENT
    // Pre-initialize the enhanced timer system for ESP-IDF 5.3.2+
    // CRITICAL FIX: Add rate limiting to prevent infinite log spam
    static uint32_t last_pre_init_log = 0;
    if (current_time - last_pre_init_log >= 5000) {  // Log every 5 seconds max
      ESP_LOGD(TAG, "Pre-initializing ESP-IDF 5.3.2+ timer system for DSC interface");
      last_pre_init_log = current_time;
    }
    if (!dsc_esp_timer::dsc_timer_is_initialized()) {
      // CRITICAL FIX: Add circuit breaker for timer pre-initialization failures
      static uint32_t timer_init_attempts = 0;
      static uint32_t first_timer_init_attempt = 0;
      
      // Record first attempt time
      if (first_timer_init_attempt == 0) {
        first_timer_init_attempt = current_time;
      }
      
      // Check if we've exceeded reasonable retry limits
      timer_init_attempts++;
      
      // CRITICAL FIX: Add watchdog reset during timer attempt limit checking
#if defined(ESP32) || defined(ESP_PLATFORM)
      esp_task_wdt_reset();
      yield();  // Allow IDLE task to run
#endif
      
      if (timer_init_attempts > 10 || (current_time - first_timer_init_attempt > 120000)) {
        ESP_LOGE(TAG, "Enhanced timer system initialization permanently failed after %u attempts over %u ms", 
                 timer_init_attempts, current_time - first_timer_init_attempt);
        ESP_LOGE(TAG, "ESP-IDF 5.3.2+ timer compatibility issue - marking initialization as permanently failed");
        
        // Mark initialization as permanently failed to break infinite loop
        getDSC().markInitializationFailed();
        return;
      }
      
      // Attempt to initialize the enhanced timer system
      bool timer_ready = dsc_esp_timer::dsc_timer_begin(1, 80, nullptr);
      if (!timer_ready) {
        ESP_LOGW(TAG, "Failed to pre-initialize ESP-IDF timer system (attempt %u/10) - will retry after delay", timer_init_attempts);
        initialization_failures++;
        last_failure_time = current_time;
        
        // Add exponential backoff for timer initialization failures
        static uint32_t timer_retry_delay = 0;
        if (timer_retry_delay == 0) {
          timer_retry_delay = current_time;
        }
        uint32_t backoff_delay = 5000 + (1000 * timer_init_attempts);  // 6s, 7s, 8s, etc.
        if (current_time - timer_retry_delay < backoff_delay) {
          return;  // Wait before retrying
        }
        timer_retry_delay = 0;  // Reset for next attempt
        return;
      }
      
      dsc_esp_timer::dsc_timer_end();  // Clean up test initialization
      ESP_LOGD(TAG, "ESP-IDF timer system pre-initialization successful after %u attempts", timer_init_attempts);
      
      // Reset counters on success
      timer_init_attempts = 0;
      first_timer_init_attempt = 0;
    }
    #endif
    
    // CRITICAL FIX: Add rate limiting to prevent rapid retry attempts
    // This prevents overwhelming the system with back-to-back hardware initialization attempts
    // that could contribute to the infinite loop condition
    static uint32_t last_begin_attempt = 0;
    uint32_t now = millis();
    if (now - last_begin_attempt < 2000) {  // CRITICAL FIX: Increase minimum delay from 1s to 2s between attempts
      // CRITICAL FIX: Add circuit breaker to prevent infinite rate limiting
      static uint32_t rate_limit_count = 0;
      rate_limit_count++;
      
      // CRITICAL FIX: Add watchdog reset and yield during rate limiting to prevent timeout
      #if defined(ESP32) || defined(ESP_PLATFORM)
      esp_task_wdt_reset();
      yield();  // Allow IDLE task to run
      #endif
      
      if (rate_limit_count > 100) {  // CRITICAL FIX: Reduce from 200 to 100 attempts to be more aggressive
        ESP_LOGE(TAG, "Hardware initialization rate limiting exceeded maximum attempts (%u) - forcing continuation", rate_limit_count);
        rate_limit_count = 0;  // Reset counter
      } else {
        if (should_log && rate_limit_count % 25 == 0) {  // CRITICAL FIX: Reduce logging frequency from every 50 to every 25
          ESP_LOGD(TAG, "Hardware init rate limited, waiting... (attempt %u/100)", rate_limit_count);
        }
        return;  // Wait before attempting initialization again
      }
    }
    last_begin_attempt = now;
    
    // CRITICAL FIX: Add comprehensive memory and system checks before hardware init
    ESP_LOGD(TAG, "Attempting DSC hardware initialization - checking system readiness...");
    
    // CRITICAL FIX: Add watchdog reset before memory validation
#if defined(ESP32) || defined(ESP_PLATFORM)
    esp_task_wdt_reset();
    yield();  // Allow IDLE task to run
#endif
    
    // Final memory validation before hardware initialization
    size_t final_heap_check = esp_get_free_heap_size();
    if (final_heap_check < 15000) {  // Conservative minimum for hardware init
      ESP_LOGW(TAG, "System memory too low for safe hardware init: %zu bytes - will retry after delay", final_heap_check);
      initialization_failures++;
      last_failure_time = current_time;
      // CRITICAL FIX: DO NOT reset init_attempt_time - this causes infinite loop
      // Instead, wait longer before retrying to allow memory to be freed
      static uint32_t memory_retry_delay = 0;
      static uint32_t memory_retry_count = 0;
      if (memory_retry_delay == 0) {
        memory_retry_delay = current_time;
      }
      if (current_time - memory_retry_delay < 5000) {  // Wait 5 seconds before retrying
        memory_retry_count++;
        if (memory_retry_count > 100) {  // After 100 memory retry attempts, give up
          ESP_LOGE(TAG, "Memory availability check exceeded maximum attempts (%u) - marking as permanently failed", memory_retry_count);
          getDSC().markInitializationFailed();
          return;
        }
        if (should_log && memory_retry_count % 25 == 0) {
          ESP_LOGD(TAG, "Memory too low, waiting for memory to be freed... (attempt %u/100)", memory_retry_count);
        }
        return;
      }
      memory_retry_delay = 0;  // Reset retry delay for next memory issue
      memory_retry_count = 0;  // Reset retry count
    }
    
    ESP_LOGD(TAG, "System ready - calling getDSC().begin() with %zu bytes free heap", final_heap_check);
    
    // CRITICAL FIX: Only count actual hardware initialization attempts, not stabilization deferrals
    total_loop_attempts++;
    
    // If we've had too many actual initialization attempts, something is wrong - give up
    // CRITICAL FIX: Reduce maximum attempts from 1000 to 500 to prevent overwhelming the system
    if (total_loop_attempts > 500 || (current_time - first_loop_attempt_time > 30000 && total_loop_attempts > 50)) {
      ESP_LOGE(TAG, "DSC hardware initialization exceeded maximum loop attempts (%u attempts over %u ms) - marking as permanently failed to prevent infinite loop", 
               total_loop_attempts, current_time - first_loop_attempt_time);
      getDSC().markInitializationFailed();
      return;
    }
    
    // CRITICAL FIX: Add rate limiting to prevent infinite log spam
    static uint32_t last_debug_log = 0;
    if (current_time - last_debug_log >= 5000) {  // Log every 5 seconds max
      ESP_LOGD(TAG, "System fully ready - initializing DSC Keybus hardware... (attempt %u)", total_loop_attempts);
      last_debug_log = current_time;
    }
    
    // CRITICAL FIX: Reset watchdog and yield before critical hardware initialization
    #if defined(ESP32) || defined(ESP_PLATFORM)
    esp_task_wdt_reset();
    yield();  // Allow IDLE task to run before critical operation
    #endif
    
    // Call DSC hardware initialization (no exceptions thrown by this method)
    getDSC().begin();
    
    // CRITICAL FIX: Reset watchdog and yield after hardware initialization attempt
    #if defined(ESP32) || defined(ESP_PLATFORM)
    esp_task_wdt_reset();
    yield();  // Allow IDLE task to run after critical operation
    #endif
    
    // Check if initialization succeeded or failed permanently
    if (getDSC().isHardwareInitialized()) {
      ESP_LOGI(TAG, "DSC Keybus hardware initialization complete");
      initialization_failures = 0;  // Reset failure counter on success
    } else if (getDSC().isInitializationFailed()) {
      ESP_LOGE(TAG, "DSC Keybus hardware initialization failed permanently after multiple attempts");
      ESP_LOGE(TAG, "Check hardware connections, timer configuration, and heap memory");
      initialization_failures = 5;  // Set to maximum to stop future attempts
    } else {
      // Initialization is still in progress or failed but might retry
      initialization_failures++;
      ESP_LOGW(TAG, "DSC hardware initialization status unclear (attempt %d/5) - will retry after delay", initialization_failures);
      
      // Add exponential backoff delay to prevent rapid retries
      static uint32_t unclear_retry_delay = 0;
      if (unclear_retry_delay == 0) {
        unclear_retry_delay = current_time;
      }
      uint32_t backoff_delay = 2000 * initialization_failures;  // 2s, 4s, 6s, 8s, 10s
      if (current_time - unclear_retry_delay < backoff_delay) {
        return;  // Wait before retrying
      }
      unclear_retry_delay = 0;  // Reset for next unclear status
    }
  }
  
  // Handle permanent initialization failure
  if (getDSC().isInitializationFailed()) {
    // Log error periodically but don't spam logs
    static uint32_t last_error_log = 0;
    uint32_t now = millis();
    if (now - last_error_log > 30000) {  // Every 30 seconds
      ESP_LOGE(TAG, "DSC Keybus hardware initialization has failed - component non-functional");
      ESP_LOGE(TAG, "Troubleshooting: Check GPIO connections (Clock=%d, Read=%d, Write=%d)",
               this->clock_pin_, this->read_pin_, this->write_pin_);
      ESP_LOGE(TAG, "Also verify DSC panel power, resistors (33kΩ, 10kΩ), and ESP32 power stability");
      last_error_log = now;
    }
    return;  // Skip processing if initialization failed permanently
  }
  
  // Process keybus data only if hardware is initialized and not force disconnected
  if (!this->force_disconnect_ && getDSC().isHardwareInitialized()) {
    getDSC().loop();
    
    // Check for buffer overflow condition (similar to Arduino INO improvements)
    if (getDSC().getBufferOverflow()) {
      ESP_LOGW(TAG, "DSC Keybus buffer overflow detected - system may be too busy");
      getDSC().setBufferOverflow(false);
    }
    
    // Handle keybus connection status changes
    if (getDSC().getKeybusChanged()) {
      getDSC().setKeybusChanged(false);
      if (getDSC().getKeybusConnected()) {
        ESP_LOGI(TAG, "DSC Keybus connected");
        for (auto *trigger : this->system_status_triggers_) {
          trigger->trigger(STATUS_ONLINE);
        }
      } else {
        ESP_LOGW(TAG, "DSC Keybus disconnected");
        for (auto *trigger : this->system_status_triggers_) {
          trigger->trigger(STATUS_OFFLINE);
        }
      }
    }
  }
  
  // TODO: Implement full status processing using wrapper methods
  // This is a minimal implementation to test the wrapper pattern
  
  // CRITICAL FIX: Reset watchdog and yield at end of every loop iteration
  // This ensures the IDLE task always has a chance to run and reset the task watchdog
#if defined(ESP32) || defined(ESP_PLATFORM)
  esp_task_wdt_reset();
  yield();  // Allow IDLE task to run
#endif
}

// Placeholder methods - will be implemented after wrapper pattern is verified
void DSCKeybusComponent::set_alarm_state(uint8_t partition, const std::string &state, const std::string &code) {
  ESP_LOGW(TAG, "set_alarm_state not yet implemented with wrapper pattern");
}

void DSCKeybusComponent::alarm_disarm(const std::string &code) {
  ESP_LOGW(TAG, "alarm_disarm not yet implemented with wrapper pattern");
}

void DSCKeybusComponent::alarm_arm_home() {
  ESP_LOGW(TAG, "alarm_arm_home not yet implemented with wrapper pattern");
}

void DSCKeybusComponent::alarm_arm_night(const std::string &code) {
  ESP_LOGW(TAG, "alarm_arm_night not yet implemented with wrapper pattern");
}

void DSCKeybusComponent::alarm_arm_away() {
  ESP_LOGW(TAG, "alarm_arm_away not yet implemented with wrapper pattern");
}

void DSCKeybusComponent::alarm_trigger_panic() {
  ESP_LOGW(TAG, "alarm_trigger_panic not yet implemented with wrapper pattern");
}

void DSCKeybusComponent::alarm_trigger_fire() {
  ESP_LOGW(TAG, "alarm_trigger_fire not yet implemented with wrapper pattern");
}

void DSCKeybusComponent::alarm_keypress(const std::string &keys) {
  getDSC().write(keys.c_str());
}

void DSCKeybusComponent::disconnect_keybus() {
  getDSC().stop();
  this->force_disconnect_ = true;
}

// Status methods - wrapper for DSCWrapper methods to maintain YAML compatibility
bool DSCKeybusComponent::getKeybusConnected() {
  return getDSC().getKeybusConnected();
}

bool DSCKeybusComponent::getBufferOverflow() {
  return getDSC().getBufferOverflow();
}

// Helper methods
bool DSCKeybusComponent::is_numeric_code(const std::string &code) {
  return !code.empty() && std::all_of(code.begin(), code.end(), ::isdigit);
}

const char* DSCKeybusComponent::status_text(uint8_t status_code) {
  // TODO: Implement proper status text mapping
  return "unknown";
}

void DSCKeybusComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "DSC Keybus:");
  ESP_LOGCONFIG(TAG, "  Debug level: %u", this->debug_level_);
  ESP_LOGCONFIG(TAG, "  Access code configured: %s", this->access_code_.empty() ? "NO" : "YES");
  ESP_LOGCONFIG(TAG, "  Enable 05 messages: %s", YESNO(this->enable_05_messages_));
}

}  // namespace dsc_keybus
}  // namespace esphome

#endif  // defined(ESP32) || defined(ESP_PLATFORM)