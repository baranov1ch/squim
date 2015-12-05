#ifndef IMAGE_IMAGE_SCANLINE_READER_H_
#define IMAGE_IMAGE_SCANLINE_READER_H_

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iterator>

#include "glog/logging.h"

namespace image {

namespace {
constexpr size_t kScanlineEnd = std::numeric_limits<size_t>::max();
}

class ImageFrame;

template <ColorScheme CS>
class Pixel {};

class ImageScanline {
 public:
  ImageScanline(ImageFrame* frame, size_t row) : frame_(frame), index_(row) {}
  ImageScanline(const ImageScanline& other)
      : frame_(other.frame_), index_(other.index_) {}

  ImageScanline& operator=(const ImageScanline& other) {
    if (this == &other)
      return *this;
    ImageScanline tmp(other);
    std::swap(*this, tmp);
    return *this;
  }

  uint8_t* ptr() { return frame_->GetData(frame_->stride() * index()); }

  const uint8_t* ptr() const {
    return frame_->GetData(frame_->stride() * index());
  }

  size_t width() const { return frame_->width(); }

  size_t index() const { return index_; }

  const ImageFrame* frame() const { return frame_; }

  void WritePixels(const uint8_t* data) {
    std::memcpy(ptr(), data,
                GetBytesPerPixel(frame_->color_scheme()) * width());
  }

 private:
  template <typename Scanline>
  friend class ScanlineIterator;

  void advance(int32_t where) const {
    if (where == 0) {
      return;
    } else if (where > 0) {
      increase(where);
    } else {
      decrease(-where);
    }
    CHECK(frame_->height() > index_ || index_ == kScanlineEnd);
  }

  void increase(size_t where) const {
    if (index_ == kScanlineEnd)
      return;

    // size_t boundaries check.
    auto rest = kScanlineEnd - index_;
    auto delta = std::min(where, rest);

    index_ += delta;
    // Image boundaries check
    if (index_ >= frame_->height())
      index_ = kScanlineEnd;
  }

  void decrease(size_t where) const {
    if (index_ == kScanlineEnd) {
      index_ = frame_->height() - 1;
      where--;
    }

    auto delta = std::min(index_, where);
    index_ -= delta;
  }

  ImageFrame* frame_ = nullptr;
  mutable size_t index_ = kScanlineEnd;
};

template <typename Scanline>
class ScanlineIterator
    : public std::iterator<std::random_access_iterator_tag, Scanline> {
 public:
  typedef std::random_access_iterator_tag iterator_category;
  typedef typename std::iterator<std::random_access_iterator_tag,
                                 Scanline>::value_type value_type;
  typedef typename std::iterator<std::random_access_iterator_tag,
                                 Scanline>::difference_type difference_type;
  typedef typename std::iterator<std::random_access_iterator_tag,
                                 Scanline>::reference reference;
  typedef typename std::iterator<std::random_access_iterator_tag,
                                 Scanline>::pointer pointer;

  explicit ScanlineIterator(Scanline scanline) : scanline_(scanline) {}
  ScanlineIterator(const ScanlineIterator& other)
      : scanline_(other.scanline_) {}

  ScanlineIterator& operator=(const ScanlineIterator& other) {
    if (this == &other)
      return *this;
    ScanlineIterator tmp(other);
    std::swap(*this, tmp);
    return *this;
  }

  ScanlineIterator& operator++() {
    scanline_.advance(1);
    return *this;
  }

  ScanlineIterator& operator--() {
    scanline_.advance(-1);
    return *this;
  }

  ScanlineIterator operator++(int) {
    Scanline scanline(scanline_);
    scanline_.advance(1);
    return ScanlineIterator(scanline);
  }

  ScanlineIterator operator--(int) {
    Scanline scanline(scanline_);
    scanline_.advance(-1);
    return ScanlineIterator(scanline);
  }

  ScanlineIterator operator+(const difference_type& n) const {
    Scanline scanline(scanline_);
    scanline_.advance(n);
    return ScanlineIterator(scanline);
  }

  ScanlineIterator& operator+=(const difference_type& n) {
    scanline_.advance(n);
    return *this;
  }
  ScanlineIterator operator-(const difference_type& n) const {
    Scanline scanline(scanline_);
    scanline_.advance(-n);
    return ScanlineIterator(scanline);
  }

  ScanlineIterator& operator-=(const difference_type& n) {
    scanline_.advance(-n);
    return *this;
  }

  reference operator*() { return scanline_; }
  pointer operator->() { return &scanline_; }

 private:
  template <typename Scanline1>
  friend bool operator==(const ScanlineIterator<Scanline1>& sc1,
                         const ScanlineIterator<Scanline1>& sc2);

  template <typename Scanline1>
  friend bool operator!=(const ScanlineIterator<Scanline1>& sc1,
                         const ScanlineIterator<Scanline1>& sc2);

  template <typename Scanline1>
  friend bool operator<(const ScanlineIterator<Scanline1>& sc1,
                        const ScanlineIterator<Scanline1>& sc2);

  template <typename Scanline1>
  friend bool operator>(const ScanlineIterator<Scanline1>& sc1,
                        const ScanlineIterator<Scanline1>& sc2);

  template <typename Scanline1>
  friend bool operator<=(const ScanlineIterator<Scanline1>& sc1,
                         const ScanlineIterator<Scanline1>& sc2);

  template <typename Scanline1>
  friend bool operator>=(const ScanlineIterator<Scanline1>& sc1,
                         const ScanlineIterator<Scanline1>& sc2);

  Scanline scanline_;
};

template <typename Scanline>
bool operator==(const ScanlineIterator<Scanline>& sc1,
                const ScanlineIterator<Scanline>& sc2) {
  CHECK_EQ(sc1.scanline_.frame(), sc2.scanline_.frame());
  return sc1.scanline_.index() == sc2.scanline_.index();
}

template <typename Scanline>
bool operator!=(const ScanlineIterator<Scanline>& sc1,
                const ScanlineIterator<Scanline>& sc2) {
  CHECK_EQ(sc1.scanline_.frame(), sc2.scanline_.frame());
  return sc1.scanline_.index() != sc2.scanline_.index();
}

template <typename Scanline>
bool operator<(const ScanlineIterator<Scanline>& sc1,
               const ScanlineIterator<Scanline>& sc2) {
  CHECK_EQ(sc1.scanline_.frame(), sc2.scanline_.frame());
  return sc1.scanline_.index() < sc2.scanline_.index();
}

template <typename Scanline>
bool operator>(const ScanlineIterator<Scanline>& sc1,
               const ScanlineIterator<Scanline>& sc2) {
  CHECK_EQ(sc1.scanline_.frame(), sc2.scanline_.frame());
  return sc1.scanline_.index() > sc2.scanline_.index();
}

template <typename Scanline>
bool operator<=(const ScanlineIterator<Scanline>& sc1,
                const ScanlineIterator<Scanline>& sc2) {
  CHECK_EQ(sc1.scanline_.frame(), sc2.scanline_.frame());
  return sc1.scanline_.index() <= sc2.scanline_.index();
}

template <typename Scanline>
bool operator>=(const ScanlineIterator<Scanline>& sc1,
                const ScanlineIterator<Scanline>& sc2) {
  CHECK_EQ(sc1.scanline_.frame(), sc2.scanline_.frame());
  return sc1.scanline_.index() >= sc2.scanline_.index();
}

template <typename Scanline>
typename ScanlineIterator<Scanline>::difference_type operator+(
    const ScanlineIterator<Scanline>& sc1,
    const ScanlineIterator<Scanline>& sc2) {
  CHECK_EQ(sc1.scanline_.frame(), sc2.scanline_.frame());
  Scanline scanline(sc1.scanline_);
  scanline.advance(sc2.scanline_.index());
  return ScanlineIterator<Scanline>(scanline);
}

template <typename Scanline>
typename ScanlineIterator<Scanline>::difference_type operator-(
    const ScanlineIterator<Scanline>& sc1,
    const ScanlineIterator<Scanline>& sc2) {
  CHECK_EQ(sc1.scanline_.frame(), sc2.scanline_.frame());
  Scanline scanline(sc1.scanline_);
  scanline.advance(-sc2.scanline_.index());
  return ScanlineIterator<Scanline>(scanline);
}

class ScanlineReader {
 public:
  using iterator = ScanlineIterator<ImageScanline>;
  using const_iterator = ScanlineIterator<const ImageScanline>;

  explicit ScanlineReader(ImageFrame* frame) : image_frame_(frame) {}

  iterator begin() {
    ImageScanline scanline(image_frame_, 0);
    return iterator(scanline);
  }

  iterator end() { return iterator(ImageScanline(image_frame_, kScanlineEnd)); }

  const_iterator begin() const {
    return const_iterator(ImageScanline(image_frame_, 0));
  }

  const_iterator end() const {
    return const_iterator(ImageScanline(image_frame_, kScanlineEnd));
  }

  size_t size() const { return image_frame_->height(); }

  ImageScanline at(size_t index) {
    CHECK_GT(image_frame_->height(), index);
    return ImageScanline(image_frame_, index);
  }

 private:
  ImageFrame* image_frame_;
};

}  // namespace image

#endif  // IMAGE_IMAGE_SCANLINE_READER_H_
