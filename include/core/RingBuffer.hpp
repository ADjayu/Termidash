#pragma once
#include <vector>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <streambuf>
#include <istream>
#include <ostream>
#include <chrono>
#include <string>

namespace termidash {

class CircularBuffer {
public:
    explicit CircularBuffer(size_t capacity = 1 << 20)
        : buf_(capacity), capacity_(capacity), head_(0), tail_(0), closed_(false) {}

    // write up to n bytes, block until at least one byte written or closed.
    size_t write(const char* data, size_t n) {
        size_t written = 0;
        while (written < n) {
            std::unique_lock<std::mutex> lk(mutex_);
            not_full_.wait(lk, [&]{ return closed_ || free_space() > 0; });
            if (closed_) break;
            size_t space = free_space();
            if (space == 0) continue;
            size_t toWrite = std::min(space, n - written);
            for (size_t i = 0; i < toWrite; ++i) {
                buf_[(tail_ + i) % capacity_] = data[written + i];
            }
            tail_ = (tail_ + toWrite) % capacity_;
            written += toWrite;
            lk.unlock();
            not_empty_.notify_one();
        }
        return written;
    }

    // read up to n bytes, block until at least one byte read or closed and empty.
    size_t read(char* out, size_t n) {
        size_t read = 0;
        while (read < n) {
            std::unique_lock<std::mutex> lk(mutex_);
            not_empty_.wait(lk, [&]{ return closed_ || available() > 0; });
            size_t avail = available();
            if (avail == 0) {
                if (closed_) break;
                else continue;
            }
            size_t toRead = std::min(avail, n - read);
            for (size_t i = 0; i < toRead; ++i) {
                out[read + i] = buf_[(head_ + i) % capacity_];
            }
            head_ = (head_ + toRead) % capacity_;
            read += toRead;
            lk.unlock();
            not_full_.notify_one();
        }
        return read;
    }

    void close() {
        {
            std::lock_guard<std::mutex> lk(mutex_);
            closed_ = true;
        }
        not_empty_.notify_all();
        not_full_.notify_all();
    }

    bool closed() const {
        return closed_;
    }

    size_t available() const {
        if (tail_ >= head_) return tail_ - head_;
        return capacity_ - (head_ - tail_);
    }

    size_t free_space() const {
        return capacity_ - available() - 1;
    }

    size_t capacity() const { return capacity_; }

private:
    std::vector<char> buf_;
    size_t capacity_;
    size_t head_;
    size_t tail_;
    bool closed_;
    mutable std::mutex mutex_;
    std::condition_variable not_empty_;
    std::condition_variable not_full_;
};

// streambuf writers/reader adapters
class CircularOutputBuf : public std::streambuf {
public:
    explicit CircularOutputBuf(std::shared_ptr<CircularBuffer> buf) : buf_(buf) {}
protected:
    int_type overflow(int_type ch) override {
        if (traits_type::eq_int_type(ch, traits_type::eof())) return traits_type::not_eof(ch);
        char c = traits_type::to_char_type(ch);
        buf_->write(&c, 1);
        return ch;
    }
    std::streamsize xsputn(const char* s, std::streamsize n) override {
        size_t wrote = buf_->write(s, static_cast<size_t>(n));
        return static_cast<std::streamsize>(wrote);
    }
    int sync() override { return 0; }
private:
    std::shared_ptr<CircularBuffer> buf_;
};

class CircularInputBuf : public std::streambuf {
public:
    explicit CircularInputBuf(std::shared_ptr<CircularBuffer> buf, size_t chunkSize = 8192)
        : buf_(buf), chunkSize_(chunkSize), tmp_(chunkSize_) {
        setg(tmp_.data(), tmp_.data(), tmp_.data());
    }
protected:
    int_type underflow() override {
        if (gptr() < egptr()) return traits_type::to_int_type(*gptr());
        // read into tmp_
        size_t n = buf_->read(tmp_.data(), chunkSize_);
        if (n == 0) {
            if (buf_->closed()) return traits_type::eof();
            // no data but not closed, wait briefly then try again
            // The CircularBuffer::read already blocks until data or closed, so we shouldn't get here
            return traits_type::eof();
        }
        setg(tmp_.data(), tmp_.data(), tmp_.data() + n);
        return traits_type::to_int_type(*gptr());
    }
private:
    std::shared_ptr<CircularBuffer> buf_;
    size_t chunkSize_;
    std::vector<char> tmp_;
};

class StreamBridge {
public:
    explicit StreamBridge(size_t capacity = 1 << 20) {
        buf_ = std::make_shared<CircularBuffer>(capacity);
        outbuf_ = std::make_unique<CircularOutputBuf>(buf_);
        inbuf_ = std::make_unique<CircularInputBuf>(buf_);
        out_stream_ = std::make_unique<std::ostream>(outbuf_.get());
        in_stream_ = std::make_unique<std::istream>(inbuf_.get());
    }

    std::ostream& out() { return *out_stream_; }
    std::istream& in() { return *in_stream_; }
    void closeWriter() { buf_->close(); }
    std::shared_ptr<CircularBuffer> buffer() const { return buf_; }

private:
    std::shared_ptr<CircularBuffer> buf_;
    std::unique_ptr<CircularOutputBuf> outbuf_;
    std::unique_ptr<CircularInputBuf> inbuf_;
    std::unique_ptr<std::ostream> out_stream_;
    std::unique_ptr<std::istream> in_stream_;
};

} // namespace termidash
