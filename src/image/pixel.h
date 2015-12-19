#ifndef IMAGE_PIXEL_H_
#define IMAGE_PIXEL_H_

#include <cstdint>

namespace image {

class RGBAPixel {
 public:
  RGBAPixel(uint8_t* data) : data_(data) {}
  uint8_t r() const { return data_[0]; }
  uint8_t g() const { return data_[1]; }
  uint8_t b() const { return data_[2]; }
  uint8_t a() const { return data_[3]; }

  void set(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    data_[0] = r;
    data_[1] = g;
    data_[2] = b;
    data_[3] = a;
  }

  static size_t size() { return 4; }

 private:
  uint8_t* data_;
};

class RGBPixel {
 public:
  RGBPixel(uint8_t* data) : data_(data) {}
  uint8_t r() const { return data_[0]; }
  uint8_t g() const { return data_[1]; }
  uint8_t b() const { return data_[2]; }

  void set(uint8_t r, uint8_t g, uint8_t b) {
    data_[0] = r;
    data_[1] = g;
    data_[2] = b;
  }

  static size_t size() { return 3; }

 private:
  uint8_t* data_;
};

class GrayScaleAlphaPixel {
 public:
  GrayScaleAlphaPixel(uint8_t* data) : data_(data) {}
  uint8_t g() const { return data_[0]; }
  uint8_t a() const { return data_[1]; }

  void set(uint8_t g, uint8_t a) {
    data_[0] = g;
    data_[1] = a;
  }

  static size_t size() { return 2; }

 private:
  uint8_t* data_;
};

class GrayScalePixel {
 public:
  GrayScalePixel(uint8_t* data) : data_(data) {}
  uint8_t g() const { return data_[0]; }

  void set(uint8_t g) { data_[0] = g; }

  static size_t size() { return 1; }

 private:
  uint8_t* data_;
};

}  // namespace image

#endif  // IMAGE_PIXEL_H_
