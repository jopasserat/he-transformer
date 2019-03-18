//*****************************************************************************
// Copyright 2018-2019 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//*****************************************************************************

#pragma once

#include "he_encoder.hpp"
#include "seal/seal.h"

namespace ngraph {
namespace runtime {
namespace he {
namespace he_seal {
struct SealCKKSEncoderWrapper : public HEEncoder {
  SealCKKSEncoderWrapper(){};
  SealCKKSEncoderWrapper(std::shared_ptr<seal::CKKSEncoder> encoder)
      : m_ckks_encoder(encoder){};

  std::shared_ptr<seal::CKKSEncoder> get_encoder() noexcept {
    return m_ckks_encoder;
  }

  std::shared_ptr<seal::CKKSEncoder> m_ckks_encoder;
};
}  // namespace he_seal
}  // namespace he
}  // namespace runtime
}  // namespace ngraph
