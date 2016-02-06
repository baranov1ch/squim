/*
 * Copyright 2015 Alexey Baranov <me@kotiki.cc>. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef BASE_OPTIONAL_H_
#define BASE_OPTIONAL_H_

namespace base {

template <typename T>
class optional {
 public:
  constexpr optional() {}
  constexpr optional(const T& value) : has_value_(true), value_(value) {}
  constexpr optional(T&& value) : has_value_(true), value_(value) {}
  template <typename U>
  constexpr optional(U&& value)
      : has_value_(true), value_(value) {}
  optional(const optional<T>& other) {
    if (other.has_value_) {
      has_value_ = true;
      value_ = other.value_;
    }
  }

  optional(optional<T>&& other) {
    if (other.has_value_) {
      has_value_ = true;
      value_ = other.value_;
    }
  }

  ~optional() {}

  T& value() { return value_; }

  const T& value() const { return value_; }

  template <typename U>
  const T value_or(U&& value) const {
    return has_value_ ? value_ : value;
  }

  template <typename U>
  T value_or(U&& value) {
    return has_value_ ? value_ : value;
  }

  void reset() { has_value_ = false; }

  constexpr explicit operator bool() const { return has_value_; }

  const T* operator->() const { return &value_; }

  T* operator->() { return &value_; }

  const T& operator*() const { return value_; }

  T& operator*() { return value_; }

  optional<T>& operator=(const optional<T>& other) {
    has_value_ = other.has_value_;
    value_ = other.value_;
    return *this;
  }

  optional<T>& operator=(optional<T>&& other) {
    has_value_ = other.has_value_;
    value_ = other.value_;
    return *this;
  }

  template <typename U>
  optional<T>& operator=(U&& value) {
    has_value_ = true;
    value_ = value;
    return *this;
  }

 private:
  bool has_value_ = false;
  T value_;
};

template <typename T>
constexpr bool operator==(const optional<T>& lhs, const optional<T>& rhs) {
  return lhs ? rhs && *lhs == *rhs : !rhs;
}

template <typename T>
constexpr bool operator!=(const optional<T>& lhs, const optional<T>& rhs) {
  return lhs ? !rhs || *lhs != *rhs : rhs;
}

template <typename T>
constexpr bool operator==(const optional<T>& lhs, const T& rhs) {
  return lhs && *lhs == rhs;
}

template <typename T>
constexpr bool operator==(const T& lhs, const optional<T>& rhs) {
  return rhs && lhs == *rhs;
}

template <typename T>
constexpr bool operator!=(const optional<T>& lhs, const T& rhs) {
  return !lhs || *lhs != rhs;
}

template <typename T>
constexpr bool operator!=(const T& lhs, const optional<T>& rhs) {
  return !rhs || lhs != *rhs;
}

}  // namespace base

#endif  // BASE_OPTIONAL_H_
