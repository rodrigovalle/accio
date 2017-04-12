#include "threadpool.hpp"
#include <thread>

ThreadPool::ThreadPool(int n_threads) {
  if (n_threads <= 0) {
    throw runtime_error("ThreadPool: invalid number of threads specified");
  }
  for (int i = 0; i < n_threads; i++) {
    pool.push_back(std::thread());
  }
}

ThreadPool::~ThreadPool() {
}

template <class Fn, class... Args>
ThreadPool::schedule(Fn fn, Args args) {
  std::thread& pool.pop_back()
}

/* TODO: remove after testing */
int main() {
  ThreadPool tp();
  tp.schedule
}
