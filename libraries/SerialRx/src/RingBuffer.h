/*
 * SerialRx - Arduino library for serial RC protocols
 *
 * Simple ring buffer (FIFO) template
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <Arduino.h>  // For noInterrupts()/interrupts()

/**
 * @brief Simple ring buffer template for message queue
 * @tparam T Type of elements to store
 * @tparam capacity Maximum number of elements
 */
template <typename T, size_t capacity>
class RingBuffer {
public:
    RingBuffer() : head_(0), tail_(0), count_(0) {}

    /**
     * @brief Check if buffer is empty
     */
    inline bool IsEmpty() const {
        return count_ == 0;
    }

    /**
     * @brief Check if buffer is full
     */
    inline bool IsFull() const {
        return count_ == capacity;
    }

    /**
     * @brief Get number of elements in buffer
     */
    inline size_t Count() const {
        return count_;
    }

    /**
     * @brief Put element in buffer (with overwrite if full)
     * @param item Element to store
     * @return true if successful
     */
    bool PutWithOverwrite(const T& item) {
        noInterrupts();  // Critical section

        buffer_[head_] = item;
        head_ = (head_ + 1) % capacity;

        if (count_ < capacity) {
            count_++;
        } else {
            // Buffer full - overwrite oldest element
            tail_ = (tail_ + 1) % capacity;
        }

        interrupts();
        return true;
    }

    /**
     * @brief Get element from buffer
     * @param item Reference to store retrieved element
     * @return true if element retrieved, false if buffer empty
     */
    bool Get(T& item) {
        if (IsEmpty()) {
            return false;
        }

        noInterrupts();  // Critical section

        item = buffer_[tail_];
        tail_ = (tail_ + 1) % capacity;
        count_--;

        interrupts();
        return true;
    }

    /**
     * @brief Clear all elements from buffer
     */
    void Clear() {
        noInterrupts();
        head_ = 0;
        tail_ = 0;
        count_ = 0;
        interrupts();
    }

private:
    T buffer_[capacity];
    volatile size_t head_;   // Write index
    volatile size_t tail_;   // Read index
    volatile size_t count_;  // Number of elements
};
