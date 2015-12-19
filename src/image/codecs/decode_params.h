#ifndef IMAGE_CODECS_DECODE_PARAMS_H_
#define IMAGE_CODECS_DECODE_PARAMS_H_

#include <set>

#include "image/image_constants.h"

namespace image {

struct DecodeParams {
  bool color_scheme_allowed(ColorScheme scheme) const {
    return allowed_color_schemes.find(scheme) != allowed_color_schemes.end();
  }
  std::set<ColorScheme> allowed_color_schemes;
};

}  // namespace image

#endif  // IMAGE_CODECS_DECODE_PARAMS_H_
