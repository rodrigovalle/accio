#include <thread>
#include <queue>

class ThreadPool {
public:
  ThreadPool(int n_threads);
  ~ThreadPool();

  template <class Fn, class... Args>
  schedule(Fn, Args);

private:
  std::queue<std::thread> pool;
  std::queue<std::thread> pool;
};
