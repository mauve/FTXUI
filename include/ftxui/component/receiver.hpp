#ifndef FTXUI_COMPONENT_RECEIVER_HPP_
#define FTXUI_COMPONENT_RECEIVER_HPP_

#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>

namespace ftxui {

// Usage:
//
// Initialization:
// ---------------
//
// auto receiver = MakeReceiver<std:string>();
// auto sender_1= receiver.MakeSender();
// auto sender_2 = receiver.MakeSender();
//
// Then move the senders elsewhere, potentially in a different thread.
//
// On the producer side:
// ----------------------
// [thread 1] sender_1->Send("hello");
// [thread 2] sender_2->Send("world");
//
// On the consumer side:
// ---------------------
// char c;
// while(receiver->Receive(&c)) // Return true as long as there is a producer.
//   print(c)
//
// Receiver::Receive() returns true when there are no more senders.

// clang-format off
template<class T> class SenderImpl;
template<class T> class ReceiverImpl;
template<class T> using Sender = std::unique_ptr<SenderImpl<T>>;
template<class T> using Receiver = std::unique_ptr<ReceiverImpl<T>>;
template<class T> Receiver<T> MakeReceiver();
// clang-format on

// ---- Implementation part ----

template <class T>
class SenderImpl {
 public:
  void Send(T t) { sender_->Receive(std::move(t)); }
  ~SenderImpl() { sender_->senders_--; }

 private:
  friend class ReceiverImpl<T>;
  SenderImpl(ReceiverImpl<T>* consumer) : sender_(consumer) {}
  ReceiverImpl<T>* sender_;
};

template <class T>
class ReceiverImpl {
 public:
  Sender<T> MakeSender() {
    senders_++;
    return std::unique_ptr<SenderImpl<T>>(new SenderImpl<T>(this));
  }

  bool Receive(T* t) {
    while (senders_) {
      std::unique_lock<std::mutex> lock(mutex_);
      while (queue_.empty())
        notifier_.wait(lock);
      if (queue_.empty())
        continue;
      *t = std::move(queue_.front());
      queue_.pop();
      return true;
    }
    return false;
  }

 private:
  friend class SenderImpl<T>;

  void Receive(T t) {
    std::unique_lock<std::mutex> lock(mutex_);
    queue_.push(std::move(t));
    notifier_.notify_one();
  }

  std::mutex mutex_;
  std::queue<T> queue_;
  std::condition_variable notifier_;
  std::atomic<int> senders_ = 0;
};

template <class T>
Receiver<T> MakeReceiver() {
  return std::make_unique<ReceiverImpl<T>>();
}

}  // namespace ftxui

#endif  // FTXUI_COMPONENT_RECEIVER_HPP_
