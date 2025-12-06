#pragma once

#include <stdint.h>
#include <stddef.h>
#include <atomic>

namespace uvos
{
/** Capacity-independent base class for FIFO. Use FIFO instead. */
template <typename T> class FIFOBase
{
  protected:
    FIFOBase(T* buffer, size_t size) : buf_(buffer), end_(size)
    {
        head_.store(0, std::memory_order_relaxed);
        tail_.store(0, std::memory_order_relaxed);
    }

  public:
    /** Destructor */
    ~FIFOBase() {}

    /** Copies all elements from another FIFO */
    FIFOBase<T>& operator=(const FIFOBase<T>& other)
    {
        if (this == &other) return *this; // Self-assignment check

        head_.store(0, std::memory_order_relaxed);
        tail_.store(0, std::memory_order_relaxed);
        if (!other.IsEmpty())
        {
            size_t readPtr = other.tail_.load(std::memory_order_acquire);
            while ((readPtr != other.head_.load(std::memory_order_acquire)) &&
                   (head_.load(std::memory_order_relaxed) < end_))
            {
                buf_[head_.load(std::memory_order_relaxed)] = other.buf_[readPtr++];
                if (readPtr >= other.end_) readPtr -= other.end_;
                head_.fetch_add(1, std::memory_order_relaxed);
            }
        }
        return *this;
    }

    /** Type alias for a handler function that processes elements */
    using RingBufHandler = void (*)(T const el);

    /** Removes all elements from the FIFO */
    void Clear()
    {
        head_.store(0, std::memory_order_relaxed);
        tail_.store(0, std::memory_order_relaxed);
    }

    /* Adds an element to the buffer if not already full */
    bool PutIfNotFull(T const el) {
        size_t h = head_.load(std::memory_order_relaxed);
        size_t next_head = h + 1;
        if (next_head == end_) {
            next_head = 0;
        }

        size_t t = tail_.load(std::memory_order_acquire);

        if (next_head != t) { // Buffer not full?
            buf_[h] = el; // Write element
            head_.store(next_head, std::memory_order_release);
            return true;
        }
        else {
            return false; // Buffer full
        }
    }

    /* Adds an element to the buffer, overwriting the oldest element if full */
    bool PutWithOverwrite(T const el)
    {
        size_t h = head_.load(std::memory_order_relaxed);
        size_t next_head = h + 1;
        if (next_head == end_)
        {
            next_head = 0; // Wrap around
        }

        size_t t = tail_.load(std::memory_order_acquire);

        // FIX: Write data FIRST, before advancing tail
        // This prevents consumer from reading stale data in the race window
        buf_[h] = el;

        // Check if buffer is full
        if (next_head == t)
        {
            // Buffer is full, advance tail to overwrite oldest element
            size_t next_tail = t + 1;
            if (next_tail == end_)
            {
                next_tail = 0; // Wrap around
            }
            tail_.store(next_tail, std::memory_order_release);
        }

        head_.store(next_head, std::memory_order_release); // Advance the head
        return true;
    }

    /* Retrieves an element from the buffer */
    bool Get(T& el) {
        size_t t = tail_.load(std::memory_order_relaxed);
        size_t h = head_.load(std::memory_order_acquire);

        if (h != t) { // Buffer not empty?
            el = buf_[t];
            size_t next_tail = t + 1;
            if (next_tail == end_) {
                next_tail = 0;
            }
            tail_.store(next_tail, std::memory_order_release);
            return true;
        }
        else {
            return false; // Buffer empty
        }
    }

    /** Retrieves next element without removing it */
    bool Peek(T &el) const {
        size_t t = tail_.load(std::memory_order_relaxed);
        size_t h = head_.load(std::memory_order_acquire);
        if (t == h) {
            return false; // empty
        }
        el = buf_[t];
        return true;
    }

    /** Returns true if the buffer is empty */
    bool IsEmpty() const {
        return head_.load(std::memory_order_acquire) == tail_.load(std::memory_order_relaxed);
    }

    /** Returns true if the buffer is full */
    bool IsFull() const {
        size_t nextIn = (head_.load(std::memory_order_relaxed) + 1) % end_;
        return nextIn == tail_.load(std::memory_order_acquire);
    }

    size_t GetNumElements() const {
        size_t in = head_.load(std::memory_order_acquire);  // Ensure visibility of the latest head_ update
        size_t out = tail_.load(std::memory_order_acquire); // Ensure visibility of the latest tail_ update
        return (in >= out) ? (in - out) : (end_ - out + in);
    }

    /** Returns the total capacity */
    size_t GetCapacity() const {
        return end_ - 1;
    }

  private:
    T* buf_;
    const size_t end_;
    std::atomic<size_t> head_;
    std::atomic<size_t> tail_;
};

/** A simple FIFO ring buffer with a fixed size. */
template <typename T, size_t capacity> class FIFO : public FIFOBase<T>
{
  public:
    /** Creates an empty FIFO */
    FIFO() : FIFOBase<T>(buffer_, capacity + 1) {}

    /** Creates a FIFO and copies all values from another FIFO */
    // FIX: Properly initialize base class before calling operator=
    // Original bug: called operator= on uninitialized base class (undefined behavior)
    template <size_t otherCapacity>
    FIFO(const FIFO<T, otherCapacity>& other)
        : FIFOBase<T>(buffer_, capacity + 1)
    {
        FIFOBase<T>::operator=(other);
    }

    /** Copies all values from another FIFO */
    template <size_t otherCapacity>
    FIFO<T, capacity>& operator=(const FIFO<T, otherCapacity>& other)
    {
        FIFOBase<T>::operator=(other);
        return *this;
    }

  private:
    T buffer_[capacity + 1]; // Array for storing elements
};

} // namespace uvos
