/**
 * EventHive-C | Advanced Event Loop Extensions
 * ============================================================
 * Specialized handlers for cross-platform event orchestration.
 * 
 * Features:
 * - Signal handling for graceful shutdown
 * - High-precision interval timers
 * - Async task scheduling for background work
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "include/event_loop.h"

static uv_loop_t* global_loop = NULL;
static uv_signal_t sig_int_handle;
static uv_signal_t sig_term_handle;

/**
 * Signal callback for graceful shutdown
 */
static void on_signal(uv_signal_t* handle, int signum) {
    printf("\n[Signal] Received signal %d. Initiating graceful shutdown...\n", signum);
    
    // Stop the loop
    uv_stop(handle->loop);
    
    // Cleanup signal handles
    uv_signal_stop(&sig_int_handle);
    uv_signal_stop(&sig_term_handle);
}

/**
 * Initializes the event loop with signal handling
 */
int event_loop_init(uv_loop_t* loop) {
    global_loop = loop;
    
    int r;
    
    // Setup SIGINT (Ctrl+C)
    r = uv_signal_init(loop, &sig_int_handle);
    if (r < 0) return r;
    r = uv_signal_start(&sig_int_handle, on_signal, SIGINT);
    if (r < 0) return r;
    
    // Setup SIGTERM (Kill)
    r = uv_signal_init(loop, &sig_term_handle);
    if (r < 0) return r;
    r = uv_signal_start(&sig_term_handle, on_signal, SIGTERM);
    if (r < 0) return r;
    
    printf("[Event Loop] Signal handlers initialized successfully.\n");
    return 0;
}

/**
 * Schedules an asynchronous task
 */
typedef struct {
    uv_async_t handle;
    void (*callback)(void*);
    void* arg;
} AsyncTask;

static void async_cb(uv_async_t* handle) {
    AsyncTask* task = (AsyncTask*)handle->data;
    if (task && task->callback) {
        task->callback(task->arg);
    }
    
    // Clean up
    uv_close((uv_handle_t*)handle, (uv_close_cb)free);
}

void event_loop_schedule_async(uv_loop_t* loop, void (*cb)(void*), void* arg) {
    AsyncTask* task = (AsyncTask*)malloc(sizeof(AsyncTask));
    if (!task) return;
    
    task->callback = cb;
    task->arg = arg;
    task->handle.data = task;
    
    uv_async_init(loop, &task->handle, async_cb);
    uv_async_send(&task->handle);
}

/**
 * High-precision timer implementation
 */
typedef struct {
    uv_timer_t timer;
    void (*callback)(void*);
    void* arg;
    int repeat;
} TimerTask;

static void timer_cb(uv_timer_t* handle) {
    TimerTask* task = (TimerTask*)handle->data;
    if (task && task->callback) {
        task->callback(task->arg);
    }
    
    if (!task->repeat) {
        uv_timer_stop(handle);
        uv_close((uv_handle_t*)handle, (uv_close_cb)free);
    }
}

void event_loop_set_timer(uv_loop_t* loop, uint64_t timeout_ms, uint64_t repeat_ms, void (*cb)(void*), void* arg) {
    TimerTask* task = (TimerTask*)malloc(sizeof(TimerTask));
    if (!task) return;
    
    task->callback = cb;
    task->arg = arg;
    task->repeat = (repeat_ms > 0);
    task->timer.data = task;
    
    uv_timer_init(loop, &task->timer);
    uv_timer_start(&task->timer, timer_cb, timeout_ms, repeat_ms);
}
