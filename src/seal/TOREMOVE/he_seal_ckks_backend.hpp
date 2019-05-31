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
namespace he {
class HESealCKKSBackend : public HESealBackend {
 public:
  HESealCKKSBackend();
  HESealCKKSBackend(
      const std::shared_ptr<ngraph::he::HEEncryptionParameters>& sp);
  HESealCKKSBackend(HESealCKKSBackend& he_backend) = default;
  ~HESealCKKSBackend(){};

  std::shared_ptr<runtime::Tensor> create_batched_cipher_tensor(
      const element::Type& element_type, const Shape& shape) override;

  std::shared_ptr<runtime::Tensor> create_batched_plain_tensor(
      const element::Type& element_type, const Shape& shape) override;

  std::shared_ptr<seal::SEALContext> make_seal_context(
      const std::shared_ptr<ngraph::he::HEEncryptionParameters> sp) override;

  void encode(ngraph::he::SealPlaintextWrapper& destination,
              const ngraph::he::HEPlaintext& plaintext,
              seal::parms_id_type parms_id, double scale) const override;

  void encode(ngraph::he::SealPlaintextWrapper& destination,
              const ngraph::he::HEPlaintext& plaintext) const override;

  void encode(ngraph::he::HEPlaintext& output, const void* input,
              const element::Type& type, bool complex,
              size_t count = 1) const override;

  void decode(void* output, const ngraph::he::HEPlaintext& input,
              const element::Type& element_type,
              size_t count = 1) const override;

  void decode(ngraph::he::HEPlaintext& output,
              const ngraph::he::SealPlaintextWrapper& input) const override;

  const inline std::shared_ptr<seal::CKKSEncoder> get_ckks_encoder() const {
    return m_ckks_encoder;
  }

 private:
  std::shared_ptr<seal::CKKSEncoder> m_ckks_encoder;
  // Scale with which to encode new ciphertexts
  double m_scale;
};

inline const ngraph::he::HESealCKKSBackend* cast_to_seal_ckks_backend(
    const ngraph::he::HEBackend* he_seal_backend) {
  auto he_seal_ckks_backend =
      dynamic_cast<const ngraph::he::HESealCKKSBackend*>(he_seal_backend);
  NGRAPH_CHECK(he_seal_ckks_backend != nullptr);
  return he_seal_ckks_backend;
};
}  // namespace he
}  // namespace ngraph