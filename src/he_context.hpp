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

#include <memory>
#include "he_encoder.hpp"
#include "he_public_key.hpp"
#include "he_relin_key.hpp"

namespace ngraph {
namespace runtime {
namespace he {
class HEContext {
 public:
  HEContext(std::shared_ptr<HEEncoder>& he_encoder,
            std::shared_ptr<HEPublicKey>& he_public_key,
            std::shared_ptr<HERelinKey>& he_relin_key)
      : m_he_encoder(he_encoder),
        m_he_public_key(he_public_key),
        m_he_relin_key(he_relin_key){};
  virtual ~HEContext(){};

 private:
  std::shared_ptr<HEEncoder> m_he_encoder;
  std::shared_ptr<HEPublicKey> m_he_public_key;
  std::shared_ptr<HERelinKey> m_he_relin_key;
};
}  // namespace he
}  // namespace runtime
}  // namespace ngraph
