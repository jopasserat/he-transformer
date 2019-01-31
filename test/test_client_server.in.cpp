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

#include "he_backend.hpp"
#include "ngraph/ngraph.hpp"
#include "seal/he_seal_backend.hpp"
#include "tcpip/tcpip_util.hpp"
#include "test_util.hpp"
#include "util/all_close.hpp"
#include "util/ndarray.hpp"
#include "util/test_control.hpp"
#include "util/test_tools.hpp"

using namespace std;
using namespace ngraph;

static string s_manifest = "${MANIFEST}";

NGRAPH_TEST(${BACKEND_NAME}, create_server_client) {
  auto client_backend = runtime::Backend::create("${BACKEND_NAME}");
  auto he_client_backend =
      static_cast<runtime::he::he_seal::HESealBackend*>(client_backend.get());

  auto context = he_client_backend->get_context();

  auto public_key = he_client_backend->get_public_key();

  NGRAPH_INFO << "Public key " << public_key;

  auto server_backend = runtime::Backend::create("${BACKEND_NAME}");
  auto he_server_backend =
      static_cast<runtime::he::he_seal::HESealBackend*>(server_backend.get());
  he_server_backend->set_role("SERVER");
  he_server_backend->set_public_key(public_key);
}

NGRAPH_TEST(${BACKEND_NAME}, server_send_receive) {
  NGRAPH_INFO << "Testing server send/receive";

  auto hostname = "localhost";
  auto port = 1025;

  // server_init(port);

  pid_t childPID = fork();
  if (childPID < 0) {
    throw ngraph_error("Fork failed");
  }

  if (childPID == 0)  // client
  {
    NGRAPH_INFO << "client pid";
    sleep(1);  // Wait until server opened
  } else {     // server
    NGRAPH_INFO << "server pid ";
    sleep(1);
  }

  /*tcp::socket connect_to_server(const std::string& hostname,
                                const int32_t port);

  void server_init(const int32_t port);

  void send_data(const tcp::socket& socket, const void* data, int bytes);

  void receive_data(const tcp::socket& socket, void* data, int bytes); */
}