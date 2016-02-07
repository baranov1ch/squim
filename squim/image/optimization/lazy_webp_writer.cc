/*
 * Copyright 2016 Alexey Baranov <me@kotiki.cc>. All rights reserved.
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

#include "squim/image/optimization/lazy_webp_writer.h"

#include "squim/base/logging.h"
#include "squim/image/image_codec_factory.h"
#include "squim/image/image_encoder.h"
#include "squim/image/image_info.h"
#include "squim/image/multi_frame_writer.h"
#include "squim/image/single_frame_writer.h"
#include "squim/io/writer.h"

namespace image {

LazyWebPWriter::LazyWebPWriter(std::unique_ptr<io::VectorWriter> dest,
                               ImageCodecFactory* codec_factory,
                               const ImageInfo* image_info)
    : dest_(std::move(dest)),
      codec_factory_(codec_factory),
      image_info_(image_info) {}

Result LazyWebPWriter::Initialize(const ImageInfo* image_info) {
  DCHECK_EQ(image_info, image_info_);
  return Result::Ok();
}

void LazyWebPWriter::SetMetadata(const ImageMetadata* metadata) {
  image_metadata_ = metadata;
}

Result LazyWebPWriter::WriteFrame(ImageFrame* frame) {
  if (!inner_) {
    auto encoder =
        codec_factory_->CreateEncoder(ImageType::kWebP, std::move(dest_));
    if (!encoder)
      return Result::Error(Result::Code::kDunnoHowToEncode);

    if (image_info_->type == ImageType::kGif) {
      inner_.reset(new MultiFrameWriter(std::move(encoder)));
    } else {
      inner_.reset(new SingleFrameWriter(std::move(encoder)));
    }

    auto result = inner_->Initialize(image_info_);
    DCHECK(!result.pending());
    if (!result.ok())
      return result;

    if (image_metadata_)
      inner_->SetMetadata(image_metadata_);
  }

  return inner_->WriteFrame(frame);
}

Result LazyWebPWriter::FinishWrite(ImageOptimizationStats* stats) {
  if (inner_)
    return inner_->FinishWrite(stats);

  return Result::Ok();
}

}  // namespace image
