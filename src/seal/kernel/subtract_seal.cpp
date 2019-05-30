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

#include "seal/kernel/subtract_seal.hpp"

void ngraph::he::scalar_subtract(
    std::shared_ptr<ngraph::he::SealCiphertextWrapper>& arg0,
    std::shared_ptr<ngraph::he::SealCiphertextWrapper>& arg1,
    std::shared_ptr<ngraph::he::SealCiphertextWrapper>& out,
    const element::Type& element_type, const HESealBackend* he_seal_backend) {
  if (arg0->is_zero()) {
    NGRAPH_INFO << "Arg0 is 0 in sub C-C";
    he_seal_backend->get_evaluator()->negate(arg1->m_ciphertext,
                                             out->m_ciphertext);
  } else if (arg1->is_zero()) {
    NGRAPH_INFO << "Arg1 is 0 in sub C-C";
    out = std::make_shared<ngraph::he::SealCiphertextWrapper>(*arg0);
  } else {
    he_seal_backend->get_evaluator()->sub(
        arg0->m_ciphertext, arg1->m_ciphertext, out->m_ciphertext);
  }
}

void ngraph::he::scalar_subtract(
    std::shared_ptr<ngraph::he::SealCiphertextWrapper>& arg0,
    const HEPlaintext& arg1,
    std::shared_ptr<ngraph::he::SealCiphertextWrapper>& out,
    const element::Type& element_type, const HESealBackend* he_seal_backend) {
  auto p = make_seal_plaintext_wrapper(arg0->complex_packing());
  he_seal_backend->encode(*p, arg1, arg0->m_ciphertext.parms_id(),
                          arg0->m_ciphertext.scale());
  he_seal_backend->get_evaluator()->sub_plain(
      arg0->m_ciphertext, p->m_plaintext, out->m_ciphertext);
}

void ngraph::he::scalar_subtract(const HEPlaintext& arg0,
                                 const HEPlaintext& arg1, HEPlaintext& out,
                                 const element::Type& element_type,
                                 const HESealBackend* he_seal_backend) {
  NGRAPH_CHECK(element_type == element::f32);

  const std::vector<float>& arg0_vals = arg0.get_values();
  const std::vector<float>& arg1_vals = arg1.get_values();
  std::vector<float> out_vals(arg0.num_values());

  std::transform(arg0_vals.begin(), arg0_vals.end(), arg1_vals.begin(),
                 out_vals.begin(), std::minus<float>());
  out.set_values(out_vals);
}
