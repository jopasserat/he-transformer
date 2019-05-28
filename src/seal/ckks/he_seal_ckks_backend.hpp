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
#include <mutex>
#include <vector>

#include "he_cipher_tensor.hpp"
#include "he_encryption_parameters.hpp"
#include "he_plain_tensor.hpp"
#include "he_tensor.hpp"
#include "ngraph/runtime/backend.hpp"
#include "seal/he_seal_backend.hpp"
#include "seal/he_seal_encryption_parameters.hpp"
#include "seal/seal.h"
#include "seal/seal_ciphertext_wrapper.hpp"
#include "seal/seal_plaintext_wrapper.hpp"

namespace ngraph {
namespace runtime {
namespace he {
namespace he_seal {
class HESealCKKSBackend : public HESealBackend {
 public:
  HESealCKKSBackend();
  HESealCKKSBackend(
      const std::shared_ptr<runtime::he::HEEncryptionParameters>& sp);
  HESealCKKSBackend(HESealCKKSBackend& he_backend) = default;
  ~HESealCKKSBackend(){};

  std::shared_ptr<runtime::Tensor> create_batched_cipher_tensor(
      const element::Type& element_type, const Shape& shape) override;

  std::shared_ptr<runtime::Tensor> create_batched_plain_tensor(
      const element::Type& element_type, const Shape& shape) override;

  std::shared_ptr<seal::SEALContext> make_seal_context(
      const std::shared_ptr<runtime::he::HEEncryptionParameters> sp) override;

  void encode(
      std::shared_ptr<runtime::he::he_seal::SealPlaintextWrapper>& plaintext,
      seal::parms_id_type parms_id, double scale, bool complex) const override;

  void encode(
      std::shared_ptr<ngraph::runtime::he::he_seal::SealPlaintextWrapper>&
          plaintext,
      bool complex) const override {
    encode(plaintext, m_context->first_parms_id(), m_scale, complex);
  }

  void encode(std::shared_ptr<runtime::he::HEPlaintext>& output,
              const void* input, const element::Type& element_type,
              bool complex, size_t count = 1) const override;
  void decode(void* output, std::shared_ptr<runtime::he::HEPlaintext>& input,
              const element::Type& element_type,
              size_t count = 1) const override;

  void decode(std::shared_ptr<runtime::he::HEPlaintext>& input) const override;

  const inline std::shared_ptr<seal::CKKSEncoder> get_ckks_encoder() const {
    return m_ckks_encoder;
  }

 private:
  std::shared_ptr<seal::CKKSEncoder> m_ckks_encoder;
  // Scale with which to encode new ciphertexts
  double m_scale;

  // mutable std::mutex m_encode_mutex;
};

inline const runtime::he::he_seal::HESealCKKSBackend* cast_to_seal_ckks_backend(
    const runtime::he::HEBackend* he_seal_backend) {
  auto he_seal_ckks_backend =
      dynamic_cast<const runtime::he::he_seal::HESealCKKSBackend*>(
          he_seal_backend);
  NGRAPH_ASSERT(he_seal_ckks_backend != nullptr);
  return he_seal_ckks_backend;
};
}  // namespace he_seal
}  // namespace he
}  // namespace runtime
}  // namespace ngraph
