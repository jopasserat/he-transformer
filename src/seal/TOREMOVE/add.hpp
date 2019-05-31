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

#include "he_ciphertext.hpp"
#include "he_plaintext.hpp"
#include "he_seal_backend.hpp"
#include "ngraph/type/element_type.hpp"

namespace ngraph {
namespace he {
void scalar_add(
    std::shared_ptr<ngraph::he::SealCiphertextWrapper>& arg0,
    std::shared_ptr<ngraph::he::SealCiphertextWrapper>& arg1,
    std::shared_ptr<ngraph::he::SealCiphertextWrapper>& out,
    const element::Type& element_type, const HESealBackend* he_seal_backend,
    const ngraph::he::HESealBackend* he_seal_backend,
    const seal::MemoryPoolHandle& pool = seal::MemoryManager::GetPool());

void scalar_add(std::shared_ptr<ngraph::he::SealCiphertextWrapper>& arg0,
                const HEPlaintext& arg1,
                std::shared_ptr<ngraph::he::SealCiphertextWrapper>& out,
                const element::Type& element_type,
                const ngraph::he::HESealBasckend* he_seal_backend);

void scalar_add(const HEPlaintext& arg0,
                std::shared_ptr<SealCiphertextWrapper>& arg1,
                std::shared_ptr<ngraph::he::SealCiphertextWrapper>& out,
                const element::Type& element_type,
                const ngraph::he::HESealBasckend* he_seal_backend);

void scalar_add(const HEPlaintext& arg0, const HEPlaintext& arg1,
                HEPlaintext& out, const element::Type& element_type,
                const ngraph::he::HESealBasckend* he_seal_backend);

inline void add(std::vector<std::shared_ptr<SealCiphertextWrapper>>& arg0,
                std::vector<std::shared_ptr<SealCiphertextWrapper>>& arg1,
                std::vector<std::shared_ptr<SealCiphertextWrapper>>& out,
                const element::Type& element_type,
                const ngraph::he::HESealBasckend* he_seal_backend,
                size_t count) {
#pragma omp parallel for
  for (size_t i = 0; i < count; ++i) {
    scalar_add(arg0[i], arg1[i], out[i], element_type, he_seal_backend);
  }
}

inline void add(std::vector<std::shared_ptr<SealCiphertextWrapper>>& arg0,
                const std::vector<std::unique_ptr<HEPlaintext>>& arg1,
                std::vector<std::shared_ptr<SealCiphertextWrapper>>& out,
                const element::Type& element_type,
                const ngraph::he::HESealBasckend* he_seal_backend,
                size_t count) {
#pragma omp parallel for
  for (size_t i = 0; i < count; ++i) {
    scalar_add(arg0[i], *arg1[i], out[i], element_type, he_seal_backend);
  }
}

inline void add(const std::vector<std::unique_ptr<HEPlaintext>>& arg0,
                std::vector<std::shared_ptr<SealCiphertextWrapper>>& arg1,
                std::vector<std::shared_ptr<SealCiphertextWrapper>>& out,
                const element::Type& element_type,
                const ngraph::he::HESealBasckend* he_seal_backend,
                size_t count) {
  add(arg1, arg0, out, element_type, he_seal_backend, count);
}

inline void add(std::vector<std::unique_ptr<HEPlaintext>>& arg0,
                std::vector<std::unique_ptr<HEPlaintext>>& arg1,
                std::vector<std::unique_ptr<HEPlaintext>>& out,
                const element::Type& element_type,
                const ngraph::he::HESealBasckend* he_seal_backend,
                size_t count) {
#pragma omp parallel for
  for (size_t i = 0; i < count; ++i) {
    scalar_add(*arg0[i], *arg1[i], *out[i], element_type, he_seal_backend);
  }
}
}  // namespace he
}  // namespace ngraph