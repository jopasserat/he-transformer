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

#include <cstdlib>
#include <fstream>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <unordered_set>

#include "nlohmann/json.hpp"
#include "seal/seal.h"

namespace ngraph {
namespace he {
class HESealEncryptionParameters {
 public:
  HESealEncryptionParameters();
  HESealEncryptionParameters(const std::string& scheme_name,
                             std::uint64_t poly_modulus_degree,
                             std::uint64_t security_level,
                             std::uint64_t evaluation_decomposition_bit_count,
                             std::vector<std::uint64_t> coeff_modulus,
                             std::uint64_t plain_modulus = 0)
      : m_scheme_name(scheme_name),
        m_poly_modulus_degree(poly_modulus_degree),
        m_security_level(security_level),
        m_evaluation_decomposition_bit_count(
            evaluation_decomposition_bit_count),
        m_coeff_modulus(coeff_modulus),
        m_plain_modulus(plain_modulus) {
    NGRAPH_CHECK(scheme_name == "HE_SEAL", "Invalid scheme name ", scheme_name);
    m_seal_encryption_parameters =
        seal::EncryptionParameters(seal::scheme_type::CKKS);

    m_seal_encryption_parameters.set_poly_modulus_degree(poly_modulus_degree);

    std::vector<seal::SmallModulus> seal_coeff_modulus;
    for (const auto& value : coeff_modulus) {
      NGRAPH_INFO << "Setting coeff mod " << value;
      seal_coeff_modulus.emplace_back(seal::SmallModulus(value));
    }
    m_seal_encryption_parameters.set_coeff_modulus(seal_coeff_modulus);
  }

  void save(std::ostream& stream) const {
    seal::EncryptionParameters::Save(m_seal_encryption_parameters, stream);
  }

  seal::EncryptionParameters& seal_encryption_parameters() {
    return m_seal_encryption_parameters;
  }
  const seal::EncryptionParameters& seal_encryption_parameters() const {
    return m_seal_encryption_parameters;
  }

  inline const std::string& scheme_name() const { return m_scheme_name; }

  inline std::uint64_t poly_modulus_degree() { return m_poly_modulus_degree; }

  inline std::uint64_t security_level() { return m_security_level; }

  inline std::uint64_t evaluation_decomposition_bit_count() {
    return m_evaluation_decomposition_bit_count;
  }
  inline const std::vector<std::uint64_t>& coeff_modulus() {
    return m_coeff_modulus;
  }

 private:
  seal::EncryptionParameters m_seal_encryption_parameters{
      seal::scheme_type::CKKS};
  std::string m_scheme_name;
  std::uint64_t m_poly_modulus_degree;
  std::uint64_t m_security_level;
  std::uint64_t m_evaluation_decomposition_bit_count;
  std::vector<std::uint64_t> m_coeff_modulus;
  std::uint64_t m_plain_modulus;
};

inline ngraph::he::HESealEncryptionParameters default_ckks_parameters() {
  std::vector<std::uint64_t> coeff_modulus;
  auto small_mods = seal::util::global_variables::default_small_mods_30bit;

  for (size_t i = 0; i < 4; ++i) {
    const auto small_mod = small_mods[i];
    coeff_modulus.emplace_back(small_mod.value());
  }

  auto params = ngraph::he::HESealEncryptionParameters(
      "HE_SEAL",
      1024,  // poly_modulus_degree
      128,   // security_level
      60,    // evaluation_decomposition_bit_count
      coeff_modulus);
  return params;
}

inline ngraph::he::HESealEncryptionParameters parse_config_or_use_default(
    const std::string& scheme_name) {
  std::unordered_set<std::string> valid_scheme_names{"HE_SEAL"};
  if (valid_scheme_names.find(scheme_name) == valid_scheme_names.end()) {
    throw ngraph_error("Invalid scheme name " + scheme_name);
  }

  const char* config_path = getenv("NGRAPH_HE_SEAL_CONFIG");
  if (config_path == nullptr) {
    NGRAPH_INFO << "Using default SEAL CKKS parameters" << config_path;
    return default_ckks_parameters();
  }

  try {
    // Read file to string
    std::ifstream f(config_path);
    std::stringstream ss;
    ss << f.rdbuf();
    std::string s = ss.str();

    // Parse json
    nlohmann::json js = nlohmann::json::parse(s);
    std::string parsed_scheme_name = js["scheme_name"];
    if (parsed_scheme_name != scheme_name) {
      throw ngraph_error("Parsed scheme name " + parsed_scheme_name +
                         " doesn't match scheme name " + scheme_name);
    }

    uint64_t poly_modulus_degree = js["poly_modulus_degree"];
    uint64_t security_level = js["security_level"];
    uint64_t evaluation_decomposition_bit_count =
        js["evaluation_decomposition_bit_count"];
    std::unordered_set<uint64_t> valid_poly_modulus{1024, 2048,  4096,
                                                    8192, 16384, 32768};
    if (valid_poly_modulus.count(poly_modulus_degree) == 0) {
      throw ngraph_error(
          "poly_modulus_degree must be 1024, 2048, 4096, 8192, 16384, "
          "32768");
    }

    std::unordered_set<uint64_t> valid_security_level{128, 192, 256};
    if (valid_security_level.count(security_level) == 0) {
      throw ngraph_error("security_level must be 128, 192, 256");
    }

    if (evaluation_decomposition_bit_count > 60 ||
        evaluation_decomposition_bit_count < 1) {
      throw ngraph_error(
          "evaluation_decomposition_bit_count must be between 1 and "
          "60");
    }

    std::vector<uint64_t> coeff_modulus;
    std::vector<seal::SmallModulus> small_mods;
    uint64_t coeff_count;
    auto coeff_mod = js.find("coeff_modulus");
    if (coeff_mod != js.end()) {
      // Use given coeff mods
      std::string coeff_mod_name = coeff_mod->begin().key();

      std::unordered_set<std::string> valid_coeff_mods{
          "custom_mods",      "small_mods_20bit", "small_mods_30bit",
          "small_mods_40bit", "small_mods_50bit", "small_mods_60bit"};

      auto valid_coeff_mod = valid_coeff_mods.find(coeff_mod_name);
      if (valid_coeff_mod == valid_coeff_mods.end()) {
        throw ngraph_error("Coeff modulus " + coeff_mod_name + " not valid");
      }

      if (coeff_mod_name == "custom_mods") {
        auto custom_mods =
            coeff_mod->begin()
                .value()
                .get<std::vector<uint64_t>>();  //->begin().value();

        for (const auto custom_mod : custom_mods) {
          small_mods.emplace_back(custom_mod);
        }
        coeff_count = small_mods.size();
      } else {
        uint64_t bit_count = stoi(coeff_mod_name.substr(11, 2));
        coeff_count = coeff_mod->begin().value();

        NGRAPH_INFO << "Using SEAL CKKS config with " << coeff_count << " "
                    << bit_count << "-bit coefficients";

        if (bit_count == 30) {
          small_mods = seal::util::global_variables::default_small_mods_30bit;
        } else if (bit_count == 40) {
          small_mods = seal::util::global_variables::default_small_mods_40bit;
        } else if (bit_count == 50) {
          small_mods = seal::util::global_variables::default_small_mods_50bit;
        } else if (bit_count == 60) {
          small_mods = seal::util::global_variables::default_small_mods_60bit;
        }
        if (coeff_count > small_mods.size()) {
          std::stringstream ss;
          ss << "Coefficient modulus count " << coeff_count << " too large";
          throw ngraph_error(ss.str());
        }
      }
    } else {  // Use default coefficient modulus
      if (security_level == 128) {
        small_mods =
            seal::DefaultParams::coeff_modulus_128(poly_modulus_degree);
      } else if (security_level == 192) {
        small_mods =
            seal::DefaultParams::coeff_modulus_192(poly_modulus_degree);
      } else if (security_level == 256) {
        small_mods =
            seal::DefaultParams::coeff_modulus_256(poly_modulus_degree);
      }
      coeff_count = small_mods.size();
    }
    for (size_t i = 0; i < coeff_count; ++i) {
      coeff_modulus.emplace_back(small_mods[i].value());
    }
    auto params = ngraph::he::HESealEncryptionParameters(
        scheme_name, poly_modulus_degree, security_level,
        evaluation_decomposition_bit_count, coeff_modulus);

    return params;

  } catch (const std::exception& e) {
    std::stringstream ss;
    ss << "Error parsing NGRAPH_HE_SEAL_CONFIG: " << e.what();
    throw ngraph_error(ss.str());
  }
}
}  // namespace he
}  // namespace ngraph
