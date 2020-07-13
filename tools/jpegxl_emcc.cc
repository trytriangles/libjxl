// Copyright (c) the JPEG XL Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <jxl/base/span.h>
#include <jxl/codec_in_out.h>
#include <jxl/dec_file.h>
#include <jxl/enc_cache.h>
#include <jxl/enc_file.h>
#include <jxl/extras/codec.h>

#include <cstring>

using namespace jxl;

extern "C" {

/* NOTA BENE: see file history to uncover how to decode HDR JPEGs to pixels. */

/** Result: uint32_t 'size' followed by compressed image (JXL). */
uint8_t* compress(const uint8_t* data, size_t size) {
  PaddedBytes compressed;
  CodecInOut io;
  Codec input_codec;
  if (!SetFromBytes(Span<const uint8_t>(data, size), &io, nullptr,
                    &input_codec)) {
    return nullptr;
  }
  CompressParams params;
  PassesEncoderState passes_encoder_state;
  if (!EncodeFile(params, &io, &passes_encoder_state, &compressed, nullptr,
                  nullptr)) {
    return nullptr;
  }
  size_t compressed_size = compressed.size();
  uint8_t* result = reinterpret_cast<uint8_t*>(malloc(compressed_size + 4));
  uint32_t* meta = reinterpret_cast<uint32_t*>(result);
  meta[0] = compressed_size;
  memcpy(result + 4, compressed.data(), compressed_size);
  return result;
  return nullptr;
}

/** Result: uint32_t 'size' followed by decompressed image (JPG). */
uint8_t* decompress(const uint8_t* data, size_t size) {
  PaddedBytes decompressed;
  CodecInOut io;
  DecompressParams params;
  if (!DecodeFile(params, Span<const uint8_t>(data, size), &io, nullptr,
                  nullptr)) {
    return nullptr;
  }
  io.use_sjpeg = false;
  io.jpeg_quality = 100;
  if (!Encode(io, Codec::kJPG, io.Main().c_current(), 8, &decompressed,
              nullptr)) {
    return nullptr;
  }
  size_t decompressed_size = decompressed.size();
  uint8_t* result = reinterpret_cast<uint8_t*>(malloc(decompressed_size + 4));
  uint32_t* meta = reinterpret_cast<uint32_t*>(result);
  meta[0] = decompressed_size;
  memcpy(result + 4, decompressed.data(), decompressed_size);
  return result;
}

}  // extern "C"
