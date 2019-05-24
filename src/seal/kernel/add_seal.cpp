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

#include "seal/kernel/add_seal.hpp"
#include "seal/bfv/kernel/add_seal_bfv.hpp"
#include "seal/ckks/he_seal_ckks_backend.hpp"
#include "seal/ckks/kernel/add_seal_ckks.hpp"

using namespace std;
using namespace ngraph::runtime::he;

void he_seal::kernel::scalar_add(
    shared_ptr<he_seal::SealCiphertextWrapper>& arg0,
    shared_ptr<he_seal::SealCiphertextWrapper>& arg1,
    shared_ptr<he_seal::SealCiphertextWrapper>& out,
    const element::Type& element_type,
    const he_seal::HESealBackend* he_seal_backend,
    const seal::MemoryPoolHandle& pool) {
  if (arg0->is_zero()) {
    NGRAPH_INFO << "Arg0 is 0 in add C+C";
    out = make_shared<he_seal::SealCiphertextWrapper>(*arg1);
  } else if (arg1->is_zero()) {
    NGRAPH_INFO << "Arg1 is 0 in add C+C";
    out = make_shared<he_seal::SealCiphertextWrapper>(*arg0);
  } else {
    NGRAPH_INFO << "regular add C+C";
    if (auto he_seal_ckks_backend =
            dynamic_cast<const he_seal::HESealCKKSBackend*>(he_seal_backend)) {
      he_seal::ckks::kernel::scalar_add_ckks(arg0, arg1, out, element_type,
                                             he_seal_ckks_backend, pool);
    } else if (auto he_seal_bfv_backend =
                   dynamic_cast<const he_seal::HESealBFVBackend*>(
                       he_seal_backend)) {
      he_seal::bfv::kernel::scalar_add_bfv(arg0, arg1, out, element_type,
                                           he_seal_bfv_backend);
    } else {
      throw ngraph_error("HESealBackend is neither BFV nor CKKS");
    }
  }
}

void he_seal::kernel::scalar_add(
    shared_ptr<he_seal::SealCiphertextWrapper>& arg0,
    shared_ptr<he_seal::SealPlaintextWrapper>& arg1,
    shared_ptr<he_seal::SealCiphertextWrapper>& out,
    const element::Type& element_type,
    const he_seal::HESealBackend* he_seal_backend,
    const seal::MemoryPoolHandle& pool) {
  NGRAPH_ASSERT(element_type == element::f32);

  if (arg0->is_zero()) {
    NGRAPH_INFO << "arg0 is 0 in C+P";
    auto arg1_hetext = dynamic_pointer_cast<runtime::he::HEPlaintext>(arg1);
    auto out_hetext = dynamic_pointer_cast<runtime::he::HECiphertext>(out);
    he_seal_backend->encrypt(out_hetext, arg1_hetext);
    out = runtime::he::he_seal::cast_to_seal_hetext(out_hetext);
    return;
  }

  // TODO: handle case where arg1 = {0, 0, 0, 0, ...}
  bool add_zero = arg1->is_single_value() && (arg1->get_values()[0] == 0.0f);

  if (add_zero) {
    NGRAPH_INFO << "arg1 is zero in C+P";
    out = make_shared<he_seal::SealCiphertextWrapper>(*arg0);
  } else {
    if (auto he_seal_ckks_backend =
            dynamic_cast<const he_seal::HESealCKKSBackend*>(he_seal_backend)) {
      he_seal::ckks::kernel::scalar_add_ckks(arg0, arg1, out, element_type,
                                             he_seal_ckks_backend, pool);
    } else if (auto he_seal_bfv_backend =
                   dynamic_cast<const he_seal::HESealBFVBackend*>(
                       he_seal_backend)) {
      he_seal::bfv::kernel::scalar_add_bfv(arg0, arg1, out, element_type,
                                           he_seal_bfv_backend);
    } else {
      throw ngraph_error("HESealBackend is neither BFV nor CKKS");
    }
  }
}

void he_seal::kernel::scalar_add(
    shared_ptr<he_seal::SealPlaintextWrapper>& arg0,
    shared_ptr<he_seal::SealCiphertextWrapper>& arg1,
    shared_ptr<he_seal::SealCiphertextWrapper>& out,
    const element::Type& element_type,
    const he_seal::HESealBackend* he_seal_backend,
    const seal::MemoryPoolHandle& pool) {
  he_seal::kernel::scalar_add(arg1, arg0, out, element_type, he_seal_backend);
}

void he_seal::kernel::scalar_add(
    shared_ptr<he_seal::SealPlaintextWrapper>& arg0,
    shared_ptr<he_seal::SealPlaintextWrapper>& arg1,
    shared_ptr<he_seal::SealPlaintextWrapper>& out,
    const element::Type& element_type,
    const he_seal::HESealBackend* he_seal_backend,
    const seal::MemoryPoolHandle& pool) {
  NGRAPH_ASSERT(element_type == element::f32);

  const std::vector<float>& arg0_vals = arg0->get_values();
  const std::vector<float>& arg1_vals = arg1->get_values();
  std::vector<float> out_vals(arg0->num_values());

  std::transform(arg0_vals.begin(), arg0_vals.end(), arg1_vals.begin(),
                 out_vals.begin(), std::plus<float>());
  out->set_values(out_vals);
}