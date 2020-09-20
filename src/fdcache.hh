#pragma once

#include <unordered_map>
#include <type_traits>
#include <stdexcept>

#include <fdcap.hh>

// FDCache class to associate multiple user defined types with FDCap objects,
// but allows modifying the cache with types convertible to the key type.

template <class T>
class FDCache {
  static_assert(std::is_reference_v<T> == false, "Type T cannot be a reference.");

  std::unordered_map<T, fdcap::FDCap> assoc_map;

  [[nodiscard]] int get_new_fd()
  {
    int fd = -1;
    return fd;
  }

  template <class U>
  using het_key_t = std::enable_if_t<std::is_convertible_v<U, T>>;
public:
  FDCache() = default;
  FDCache(const FDCache&) = delete;
  FDCache& operator=(const FDCache&) = delete;
  FDCache(FDCache&&) = delete;
  FDCache& operator=(FDCache&&) = delete;
  ~FDCache() = default;

  // Returns false if key doesn't exist
  template <class U, typename = het_key_t<U>>
  void revoke(U&& t)
  {
    assoc_map.erase(std::forward<U>(t));
  }
  // Takes ownership of the inserted FDCap object
  template <class U, typename = het_key_t<U>>
  [[nodiscard]] bool insert(U&& t, fdcap::FDCap&& fdc)
  {
    auto [_, b] = assoc_map.emplace(std::forward<U>(t), std::move(fdc));
    return b;
  }
  // Returns false if key doesn't exist
  template <class U, typename = het_key_t<U>>
  [[nodiscard]] bool replace(U&& t, fdcap::FDCap&& fdc)
  {
    try {
      auto& cur_fdc = assoc_map.at(std::forward<U>(t));
      cur_fdc = std::move(fdc);
      return true;
    } catch (std::out_of_range&) {
      return false;
    }
  }
};
