#pragma once
#include <array>
//
#include <iomanip>
#include <iostream>
//
#include <atomic>
#include <mutex>

namespace lyrahgames {

struct log {
  struct state {
    using czstring = const char*;
    enum counter {
      default_construct_calls = 0,
      construct_calls,
      destruct_calls,
      copy_construct_calls,
      copy_assign_calls,
      move_construct_calls,
      move_assign_calls,
      swap_calls,
      equal_calls,
      hash_calls,
      count
    };

    static constexpr czstring counter_names[] = {
        "Default Constructor Calls",  //
        "Constructor Calls",
        "Destructor Calls",
        "Copy Constructor Calls",
        "Copy Assignment Calls",
        "Move Constructor Calls",
        "Move Assignment Calls",
        "Swap Calls",
        "Equality Calls",
        "Hash Calls",
    };

    std::array<size_t, count> counters{};
  };

  log() = default;
  log(size_t i) : id{i} {}
  virtual ~log() noexcept = default;

  log(log&&)   = delete;
  log& operator=(log&&) = delete;

  // Copy operations are not allowed.
  log(const log&) = delete;
  log& operator=(const log&) = delete;

  // State
  mutable std::mutex access_mutex{};
  struct state       state {};
  size_t             id{};
};

inline void reset(struct log& log) {
  std::scoped_lock lock{log.access_mutex};
  log.state = {};
}

inline bool operator==(const struct log::state& s1,
                       const struct log::state& s2) noexcept {
  return s1.counters == s2.counters;
}

inline bool operator==(const struct log&        log,
                       const struct log::state& state) noexcept {
  std::scoped_lock lock{log.access_mutex};
  return log.state == state;
}

inline std::ostream& operator<<(std::ostream&            os,
                                const struct log::state& state) {
  using namespace std;
  os << '\n';
  for (size_t i = 0; i < state.counter::count; ++i)
    os << setw(30) << state.counter_names[i] << " = " << setw(7)
       << state.counters[i] << '\n';
  return os << '\n';
}

inline std::ostream& operator<<(std::ostream& os, const struct log& log) {
  using namespace std;
  std::scoped_lock lock{log.access_mutex};
  os << '\n'
     << setw(30) << "Log ID"
     << " = " << setw(7) << log.id;
  return os << log.state << '\n';
}

#define unique_log __COUNTER__

template <typename T, size_t I>
struct basic_log_value {
  using type                     = T;
  static constexpr size_t log_id = I;
  static struct log       log;

  basic_log_value() {
    std::scoped_lock lock{log.access_mutex};
    ++log.state.counters[log.state.default_construct_calls];
  }
  basic_log_value(int d) : value{d} {
    std::scoped_lock lock{log.access_mutex};
    ++log.state.counters[log.state.construct_calls];
  }
  virtual ~basic_log_value() noexcept {
    std::scoped_lock lock{log.access_mutex};
    ++log.state.counters[log.state.destruct_calls];
  }

  basic_log_value(const basic_log_value& v) : value{v.value} {
    std::scoped_lock lock{log.access_mutex};
    ++log.state.counters[log.state.copy_construct_calls];
  }
  basic_log_value& operator=(const basic_log_value& v) {
    value = v.value;
    std::scoped_lock lock{log.access_mutex};
    ++log.state.counters[log.state.copy_assign_calls];
    return *this;
  }

  basic_log_value(basic_log_value&& v) : value{v.value} {
    std::scoped_lock lock{log.access_mutex};
    ++log.state.counters[log.state.move_construct_calls];
  }
  basic_log_value& operator=(basic_log_value&& v) {
    value = v.value;
    std::scoped_lock lock{log.access_mutex};
    ++log.state.counters[log.state.move_assign_calls];
    return *this;
  }

  operator type() { return value; }

  // State
  type value;
};

template <typename T, size_t I>
log basic_log_value<T, I>::log{log_id};

// Take care with function arguments. By not using const references, log could
// show wrong or difficult-to-understand statistics because copy or move
// construction would take place.
template <typename T, size_t I>
inline bool operator==(const basic_log_value<T, I>& x,
                       const basic_log_value<T, I>& y) noexcept {
  std::scoped_lock lock{x.log.access_mutex};
  ++x.log.state.counters[x.log.state.equal_calls];
  return x.value == y.value;
}

}  // namespace lyrahgames

namespace std {
// Take care with function arguments. By not using const references, log could
// show wrong or difficult-to-understand statistics because copy or move
// construction would take place.
template <typename T, size_t I>
struct hash<lyrahgames::basic_log_value<T, I>> {
  size_t operator()(const lyrahgames::basic_log_value<T, I>& x) const noexcept {
    std::scoped_lock lock{x.log.access_mutex};
    ++x.log.state.counters[x.log.state.hash_calls];
    return hash<T>{}(x.value);
  }
};
}  // namespace std