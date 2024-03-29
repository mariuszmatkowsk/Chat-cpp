#pragma once

#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <optional>
#include <tuple>

template <typename T> struct Inner {
    std::mutex mutex;
    std::condition_variable cv;
    std::deque<T> queue;
    unsigned senders{0};
};

template <typename T> class Sender {
public:
    Sender(const std::shared_ptr<Inner<T>> inner) : inner_{inner} {
        std::unique_lock lock{inner_->mutex};
        inner_->senders = 1;
    }
    Sender(const Sender& other) {
        inner_ = other.inner_;
        std::scoped_lock lock{inner_->mutex};
        inner_->senders++;
    }

    Sender& operator=(const Sender& other) {
        inner_ = other.inner_;
        std::scoped_lock lock{inner_->mutex};
        inner_->senders++;
        return *this;
    }

    Sender(Sender&&) noexcept = default;

    Sender& operator=(Sender&&) noexcept = default;

    ~Sender() {
        if (inner_) {
            std::scoped_lock lock{inner_->mutex};

            inner_->senders--;
            if (inner_->senders == 0) {
                inner_->cv.notify_one();
            }
        }
    }

    void send(T data) {
        std::unique_lock lock{inner_->mutex};
        inner_->queue.push_back(std::move(data));
        lock.unlock();
        inner_->cv.notify_one();
    }

private:
    std::shared_ptr<Inner<T>> inner_;
};

template <typename T> class Receiver {
public:
    Receiver(const std::shared_ptr<Inner<T>> inner) : inner_{inner} {
    }

    Receiver(const Receiver&) = delete;
    Receiver& operator=(const Receiver&) = delete;

    Receiver(Receiver&&) = default;
    Receiver& operator=(Receiver&&) = default;

    std::optional<T> recv() {
        std::unique_lock lock{inner_->mutex};

        auto& queue = inner_->queue;

        while (true) {
            if (!queue.empty()) {
                auto data = queue.front();
                queue.pop_front();
                return std::optional<T>{data};
            } else if (inner_->senders == 0) {
                return std::nullopt;
            } else {
                inner_->cv.wait(lock);
            }
        }
    }

private:
    const std::shared_ptr<Inner<T>> inner_;
};

template <typename T> std::tuple<Sender<T>, Receiver<T>> make_channel() {
    auto inner = std::make_shared<Inner<T>>();
    return {Sender{inner}, Receiver{inner}};
}
