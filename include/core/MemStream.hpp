#pragma once
#include <streambuf>
#include <istream>
#include <ostream>
#include <string>
#include <memory>

namespace termidash {

// MemoryOutputBuf: collects written characters into a std::string.
class MemoryOutputBuf : public std::streambuf {
public:
    MemoryOutputBuf() {
        // reserve small default capacity
        buffer_.reserve(1024);
    }
    std::string str() const { return buffer_; }
protected:
    // single character
    int_type overflow(int_type ch) override {
        if (traits_type::eq_int_type(ch, traits_type::eof())) return traits_type::not_eof(ch);
        buffer_.push_back(traits_type::to_char_type(ch));
        return ch;
    }
    // block write
    std::streamsize xsputn(const char* s, std::streamsize n) override {
        buffer_.append(s, static_cast<size_t>(n));
        return n;
    }
private:
    std::string buffer_;
};

// MemoryOutputStream: ostream wrapper
class MemoryOutputStream : public std::ostream {
public:
    MemoryOutputStream() : std::ostream(&buf_), buf_() {}
    std::string str() const { return buf_.str(); }
    MemoryOutputBuf* rdbuf_ptr() { return &buf_; }
private:
    MemoryOutputBuf buf_;
};

// MemoryInputBuf: provides read-only view into an existing string buffer.
// It keeps a shared_ptr to the owning string to ensure lifetime.
class MemoryInputBuf : public std::streambuf {
public:
    MemoryInputBuf() : data_(nullptr) {}
    explicit MemoryInputBuf(std::shared_ptr<std::string> data) {
        setBuffer(data);
    }
    void setBuffer(std::shared_ptr<std::string> data) {
        data_ = data;
        if (data_ && !data_->empty()) {
            char* begin = const_cast<char*>(data_->data());
            setg(begin, begin, begin + data_->size());
        } else {
            setg(nullptr, nullptr, nullptr);
        }
    }
protected:
    // handle underflow
    int_type underflow() override {
        if (!data_ || gptr() >= egptr()) return traits_type::eof();
        return traits_type::to_int_type(*gptr());
    }
private:
    std::shared_ptr<std::string> data_;
};

// MemoryInputStream: istream wrapper that references underlying shared string
class MemoryInputStream : public std::istream {
public:
    MemoryInputStream() : std::istream(nullptr), ibuf_() { rdbuf(&ibuf_); }
    explicit MemoryInputStream(std::shared_ptr<std::string> data) : std::istream(nullptr), ibuf_(data) {
        rdbuf(&ibuf_);
    }
    explicit MemoryInputStream(const std::string &s) : std::istream(nullptr), owned_(std::make_shared<std::string>(s)), ibuf_(owned_) {
        rdbuf(&ibuf_);
    }
    // construct from rvalue string (take ownership)
    explicit MemoryInputStream(std::string &&s) : std::istream(nullptr), owned_(std::make_shared<std::string>(std::move(s))), ibuf_(owned_) {
        rdbuf(&ibuf_);
    }
private:
    std::shared_ptr<std::string> owned_;
    MemoryInputBuf ibuf_;
};

} // namespace termidash
