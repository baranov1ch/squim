#ifndef IMAGE_OPTIMIZING_WRITER_H_
#define IMAGE_OPTIMIZING_WRITER_H_

#include <memory>

namespace image {

class OptimizingWriter {
 public:
  OptimizingWriter(std::unique_ptr<Params> params);
};

PixelIterator<RGBA> i(img);
for (const auto& p : i) {
  if (p.A() != Alpha::kOpaque) {
    opaque = false;
  }

  if (p.R() != p.G() && r.R() != p.B()) {
    grayscale = false;
  }

  if (!opaque && !grayscale)
    break;
}

class Buffer {
 public:
  virtual ReadResult ReadSome(const uint8** data, size_t* len) = 0;
};

enum class State {
  READ_REQUEST,
  READ_META,
  WRITE_RESPONSE,
  OPTIMIZE_LOOP,
  FINISH,
};

void OptimizeImage(ReaderWriterStream* stream) {
  Request req(stream);
  req.DoLoop();
  DoLoop(stream) OptimizeReq req;
  stream->Read(&req);
  if (!req.has_meta()) {
    stream->Write(ToProto(INVALID_MESSAGE));
    stream->Finish(ToGrpcCode(INVALID_MESSAGE));
    return;
  }

  OptimizationParams params(req.meta());
  Source in;
  Destination out;
  Optmizer optimizer;

  auto result = optimizer.Init(params in, out);
  if (result.Declined()) {
    stream->Write(ToProto(result.meta()));
    stream->Finish(ToGrpcCode(OK));
  }

  while (stream->Read(&req)) {
    if (!req.has_bytes()) {
      stream->Write(ToProto(INVALID_MESSAGE));
      return;
    }
    in.Add(res.bytes());

    auto result = optimizer.Process();
    if (result.has_something()) {
      if (result.meta())
        stream->Write(ToProto(result.meta()));
      while (!out.empty()) {
        res.set_bytes(ToProto(out.GetSome()));
        stream->Write(res);
      }
    }

    if (result.finished()) {
      stream->Finish(ToGrpcCode(result.code()));
      break;
    }

    CHECK(result.pending());
  }
}

Result Optimizer::Process() {
  switch (state_) {
    case CHOOSE_DECODER:
      rv = DoChooseDecoder();
      break;
    case READ_META:
      rv = DoReadImageMeta();
      break;
    case OPTIMIZE:
      rv = DoOptimize();
      break;
  }
}

Result Optimizer::DoChooseDecoder() {
  next_state_ = CHOOSE_DECODER_COMPLETE;
  return in_->PeekFull(signature_, kMaxSignatureSize);
}

Result Optimizer::DoChooseDecoderComplete(Result result) {
  if (!result.Ok())
    return result;

  next_state_ = READ_META;
  return GetDecoderBySignature();
}

Result Optimizer::DoReadImageMeta() {
  return decoder_->GetMetadata();
}

Result Optimizer::DoReadImageMetaComplete(Result result) {
  if (!result.Ok())
    return result;

  if (!decoder_->MetaCompleted()) {
    next_state_ = READ_META;
  } else {
    return DecideOnOptimization();
  }
}

Result Optimizer::DoOptimize() {
  CHECK(encoder_);
  while (decoder_->HasMoreFrames()) {
    ImageFrame* frame;
    auto result = decoder_->GetNextFrame(&frame);
    if (result.pending())
      return result;

    if (result.error())
      return result;

    CHECK(frame);
    result = encoder_->EncodeFrame(frame);
    if (!result.Ok())
      return result;
  }
}

}  // namespace image

#endif  // IMAGE_OPTIMIZING_WRITER_H_
