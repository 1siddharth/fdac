#pragma once

#include <fdcap.hh>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>

// FDCache class to associate multiple user defined types with FDCap objects,
// but allows modifying the cache with types convertible to the key type.

template <class T>
class FDCache {
  static_assert(std::is_reference_v<T> == false,
                "Type T cannot be a reference.");

  std::unordered_map<T, fdcap::FDCap> assoc_map;

  template <class U>
  using het_key_t = std::enable_if_t<std::is_convertible_v<U, T>>;

 public:
  FDCache() = default;
  FDCache(const FDCache&) = delete;
  FDCache& operator=(const FDCache&) = delete;
  FDCache(FDCache&&) = delete;
  FDCache& operator=(FDCache&&) = delete;
  ~FDCache() = default;

  [[nodiscard]] static int get_new_fd() {
    int fd = -1;
    return fd;
  }

  // Returns false if key doesn't exist
  template <class U, typename = het_key_t<U>>
  void revoke(U&& t) {
    assoc_map.erase(std::forward<U>(t));
  }
  // Takes ownership of the inserted FDCap object
  template <class U, typename = het_key_t<U>>
  [[nodiscard]] bool insert(U&& t, fdcap::FDCap&& fdc) {
    auto [_, b] = assoc_map.emplace(std::forward<U>(t), std::move(fdc));
    return b;
  }
  // Returns false if key doesn't exist
  template <class U, typename = het_key_t<U>>
  [[nodiscard]] bool replace(U&& t, fdcap::FDCap&& fdc) {
    if (auto it = assoc_map.find(std::forward<U>(t)); it != assoc_map.end()) {
      it->second = std::move(fdc);
      return true;
    } else {
      return false;
    }
  }
  template <class U, typename = het_key_t<U>>
  [[nodiscard]] fdcap::FDCap* get(U&& t) {
    if (auto it = assoc_map.find(std::forward<U>(t)); it != assoc_map.end()) {
      return &(it->second);
    } else {
      return nullptr;
    }
  }
};
