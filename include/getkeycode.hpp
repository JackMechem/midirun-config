#include <atomic>
class GetKeycode {
  public:
	static int getKey(std::atomic<bool> &keepGoing);
};
