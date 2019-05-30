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

#include "he_ciphertext.hpp"
#include "seal/seal.h"

namespace ngraph {
namespace he {
struct SealPlaintextWrapper {
  SealPlaintextWrapper(const seal::Plaintext& plain,
                       bool complex_packing = false)
      : m_plaintext(plain), m_complex_packing(complex_packing) {}

  SealPlaintextWrapper(bool complex_packing = false)
      : m_complex_packing(complex_packing) {}

  bool m_complex_packing;
  seal::Plaintext m_plaintext;
};

inline std::unique_ptr<SealPlaintextWrapper> make_seal_plaintext_wrapper() {
  return std::make_unique<SealPlaintextWrapper>();
}

inline std::unique_ptr<SealPlaintextWrapper> make_seal_plaintext_wrapper(
    bool complex_packing) {
  return std::make_unique<SealPlaintextWrapper>(complex_packing);
}
}  // namespace he
}  // namespace ngraph
