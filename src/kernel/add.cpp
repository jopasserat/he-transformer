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

#include "kernel/add.hpp"
#include "seal/he_seal_backend.hpp"
#include "seal/kernel/add_seal.hpp"

using namespace std;
using namespace ngraph::runtime::he;

void kernel::scalar_add(shared_ptr<HECiphertext>& arg0,
                        shared_ptr<HECiphertext>& arg1,
                        shared_ptr<HECiphertext>& out,
                        const element::Type& element_type,
                        const HEBackend* he_backend) {
  auto he_seal_backend =
      dynamic_cast<const he_seal::HESealBackend*>(he_backend);
  NGRAPH_ASSERT(he_seal_backend != nullptr);
  auto arg0_seal = he_seal::cast_to_seal_hetext(arg0);
  auto arg1_seal = he_seal::cast_to_seal_hetext(arg1);
  auto out_seal = he_seal::cast_to_seal_hetext(out);

  he_seal::kernel::scalar_add(arg0_seal, arg1_seal, out_seal, element_type,
                              he_seal_backend);
}

void kernel::scalar_add(shared_ptr<HEPlaintext>& arg0,
                        shared_ptr<HEPlaintext>& arg1,
                        shared_ptr<HEPlaintext>& out,
                        const element::Type& element_type,
                        const HEBackend* he_backend) {
  std::vector<float> arg0_vals = arg0->get_values();
  std::vector<float> arg1_vals = arg1->get_values();
  std::vector<float> out_vals(arg0->num_values());

  std::transform(arg0_vals.begin(), arg0_vals.end(), arg1_vals.begin(),
                 out_vals.begin(), std::plus<float>());

  out->set_values(out_vals);
}

void kernel::scalar_add(shared_ptr<HECiphertext>& arg0,
                        shared_ptr<HEPlaintext>& arg1,
                        shared_ptr<HECiphertext>& out,
                        const element::Type& element_type,
                        const HEBackend* he_backend) {
  auto he_seal_backend =
      dynamic_cast<const he_seal::HESealBackend*>(he_backend);
  NGRAPH_ASSERT(he_seal_backend != nullptr);
  auto arg0_seal = he_seal::cast_to_seal_hetext(arg0);
  auto arg1_seal = he_seal::cast_to_seal_hetext(arg1);
  auto out_seal = he_seal::cast_to_seal_hetext(out);

  he_seal::kernel::scalar_add(arg0_seal, arg1_seal, out_seal, element_type,
                              he_seal_backend);
}

void kernel::scalar_add(shared_ptr<HEPlaintext>& arg0,
                        shared_ptr<HECiphertext>& arg1,
                        shared_ptr<HECiphertext>& out,
                        const element::Type& element_type,
                        const HEBackend* he_backend) {
  scalar_add(arg1, arg0, out, element_type, he_backend);
}
