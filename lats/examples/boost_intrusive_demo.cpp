#include <atomic>
#include <boost/intrusive/list.hpp>
#include <boost/intrusive/list_hook.hpp>
#include <chrono>
#include <iostream>
#include <list>
#include <memory>
#include <vector>

namespace bi = boost::intrusive;

struct MyObject : public bi::list_base_hook<> {
  int value;

  explicit MyObject(int v) : value(v) {
    std::cout << "  MyObject(" << value << ") constructed\n";
  }

  ~MyObject() { std::cout << "  MyObject(" << value << ") desctruted\n"; }
};

using MyList = bi::list<MyObject>;

void demo_basic_intrusive_list() {
  std::cout << "\n=== Demo 1: Basic Intrusive List ===\n";

  MyObject obj1(10);
  MyObject obj2(20);
  MyObject obj3(30);

  MyList list;
  std::cout << "\nAdding objects to list...\n";
  list.push_back(obj1);
  list.push_back(obj2);
  list.push_back(obj3);

  std::cout << "\nIterating through list:\n";
  for (const auto &obj : list) {
    std::cout << " Value: " << obj.value << "\n";
  }

  std::cout << "\nRemoveing obj2...\n";
  list.erase(list.iterator_to(obj2));

  std::cout << "\nList after removal:\n";
  for (const auto &obj : list) {
    std::cout << "  Value: " << obj.value << "\n";
  }

  std::cout << "\nClearing list (but objects still exist)...\n";
  list.clear();

  std::cout << "\nObjects still accessible:\n";
  std::cout << "  obj1.value = " << obj1.value << "\n";
  std::cout << "  obj2.value = " << obj2.value << "\n";
  std::cout << "  obj3.value = " << obj3.value << "\n";

  std::cout << "\nExiting scope (objects will be destructed)...\n";
}

struct NonIntrusiveData {
  int value;
  explicit NonIntrusiveData(int v) : value(v) {}
};

struct IntrusiveData : public bi::list_base_hook<> {
  int value;
  explicit IntrusiveData(int v) : value(v) {}
};

void demo_memory_comparison() {
  std::cout << "\n=== Demo 2: Memory Layout Comparison ===\n";

  std::cout << "\nNon-Intrusive (std::list<NonIntrusiveData*>):\n";
  std::cout << "  Size of NonIntrusiveData: " << sizeof(NonIntrusiveData)
            << " bytes\n";
  std::cout << "  Each list node needs: ~" << sizeof(NonIntrusiveData)
            << " + 16 bytes (prev/next pointers)\n";
  std::cout << "  Total allocations per element: 2 (object + node)\n";

  std::cout << "\nIntrusive (boost::intrusive::list<IntrusiveData>):\n";
  std::cout << "  Size of IntrusiveData: " << sizeof(IntrusiveData)
            << " bytes\n";
  std::cout << "  Hook embedded in object (prev/next pointers inside)\n";
  std::cout << "  Total allocations per element: 1 (just the object)\n";

  std::cout
      << "\nMemory savings: ~16 bytes per element + better cache locality!\n";
}

void demo_performance_comparison() {
  std::cout << "\n=== Demo 3: Performance Comparison ===\n";

  constexpr size_t N = 100'000;

  // Non-Intrusive: std::list
  {
    auto start = std::chrono::high_resolution_clock::now();

    std::list<NonIntrusiveData *> non_intrusive_list;
    std::vector<std::unique_ptr<NonIntrusiveData>> storage;

    for (size_t i = 0; i < N; ++i) {
      auto ptr = std::make_unique<NonIntrusiveData>(i);
      non_intrusive_list.push_back(ptr.get());
      storage.push_back(std::move(ptr));
    }

    // Iterate
    size_t sum = 0;
    for (auto *ptr : non_intrusive_list) {
      sum += ptr->value;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << "\nNon-Intrusive (std::list):\n";
    std::cout << "  Time: " << duration.count() << " μs\n";
    std::cout << "  Sum: " << sum << "\n";
  }

  // Intrusive: boost::intrusive::list
  {
    auto start = std::chrono::high_resolution_clock::now();

    std::vector<IntrusiveData> storage;
    bi::list<IntrusiveData> intrusive_list;
    storage.reserve(N);

    for (size_t i = 0; i < N; ++i) {
      storage.emplace_back(i);
      intrusive_list.push_back(storage.back());
    }

    // Iterate
    size_t sum = 0;
    for (const auto &obj : intrusive_list) {
      sum += obj.value;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << "\nIntrusive (boost::intrusive::list):\n";
    std::cout << "  Time: " << duration.count() << " μs\n";
    std::cout << "  Sum: " << sum << "\n";
  }
}

int main() {
  demo_basic_intrusive_list();
  demo_memory_comparison();
  demo_performance_comparison();

  return 0;
}
