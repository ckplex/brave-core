/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ledger/internal/publisher/v4_publisher.h"

#include "base/json/json_reader.h"
#include "bat/ledger/internal/ledger_impl.h"
#include "base/logging.h"
#include "third_party/brotli/include/brotli/decode.h"

namespace {

class BrotliStreamDecoder {
 public:
  BrotliStreamDecoder() {
    brotli_state_ = BrotliDecoderCreateInstance(&Allocate, &Free, this);
    out_vector_.reserve(1024);
  }

  BrotliStreamDecoder(const BrotliStreamDecoder&) = delete;
  BrotliStreamDecoder& operator=(const BrotliStreamDecoder&) = delete;

  ~BrotliStreamDecoder() {
    BrotliDecoderDestroyInstance(brotli_state_);
    brotli_state_ = nullptr;
  }

  enum class Result {
    Done = 0,
    InputRequired,
    Error
  };

  template<typename F>
  Result DecodeString(const std::string& input, F callback) {
    static_assert(sizeof(char) == sizeof(uint8_t), "safe reinterpret_cast");
    if (input.length() == 0) {
      return Result::Error;
    }
    return Decode(
        reinterpret_cast<const uint8_t*>(input.data()),
        input.length(),
        [&callback](const uint8_t* data, size_t length) {
          callback(reinterpret_cast<const char*>(data), length);
        });
  }

  template<typename F>
  Result Decode(const uint8_t* input_buffer, size_t input_length, F callback) {
    if (!input_buffer || input_length == 0) {
      return Result::Error;
    }

    uint8_t* output_buffer = out_vector_.data();
    size_t output_length = out_vector_.capacity();

    for (;;) {
      auto brotli_result = BrotliDecoderDecompressStream(
        brotli_state_,
        &input_length,
        &input_buffer,
        &output_length,
        &output_buffer,
        nullptr);

      switch (brotli_result) {
        case BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT: {
          callback(out_vector_.data(), out_vector_.capacity() - output_length);
          output_buffer = out_vector_.data();
          output_length = out_vector_.capacity();
          break;
        }
        case BROTLI_DECODER_RESULT_SUCCESS: {
          callback(out_vector_.data(), out_vector_.capacity() - output_length);
          return Result::Done;
        }
        case BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT: {
          return Result::InputRequired;
        }
        default: {
          return Result::Error;
        }
      }
    }
  }

 private:
  static void* Allocate(void* opaque, size_t size) {
    size_t* array = reinterpret_cast<size_t*>(malloc(size + sizeof(size_t)));
    if (!array) {
      return nullptr;
    }
    array[0] = size;
    return &array[1];
  }

  static void Free(void* opaque, void* address) {
    if (!address) {
      return;
    }
    size_t* array = reinterpret_cast<size_t*>(address);
    free(&array[-1]);
  }

  BrotliDecoderState* brotli_state_;
  std::vector<uint8_t> out_vector_;
};

}  // namespace

namespace braveledger_v4_publisher {

using bat_ledger::LedgerImpl;

void FetchPrefixList(LedgerImpl* ledger) {
  std::vector<std::string> headers;
  std::string url = "https://rewards-stg.bravesoftware.com/all_channels";

  ledger->LoadURL(
      url,
      headers,
      "",
      "",
      ledger::UrlMethod::GET,
      [](
          const int status,
          const std::string& body,
          const std::map<std::string, std::string>& headers) {
        BrotliStreamDecoder decoder;
        std::string data;
        decoder.DecodeString(body, [&data](const char* buffer, size_t length) {
          data.append(buffer, length);
        });

        base::Optional<base::Value> value = base::JSONReader::Read(data);
        if (value) {
          for (auto& item : value->GetList()) {
            LOG(INFO) << "[[zenparsing]] " << item.GetString();
          }
        } else {
          LOG(INFO) << "[[zenparsing]] " << data;
        }
      });
}

}  // namespace braveledger_v4_publisher
