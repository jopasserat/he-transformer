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

#include <signal.h>
#include <sys/types.h>

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
  auto hostname = "localhost";
  auto port = 33000;

  seal::PublicKey pk_server;
  seal::PublicKey pk_client;

  auto server_fun = [hostname, port, &pk_server]() {
    auto server_backend = runtime::Backend::create("${BACKEND_NAME}");
    runtime::he::he_seal::HESealBackend* he_server_backend;

    he_server_backend =
        static_cast<runtime::he::he_seal::HESealBackend*>(server_backend.get());

    seal::PublicKey pk_server_init = *he_server_backend->get_public_key();
    auto cnt_init = pk_server_init.data().uint64_count();
    NGRAPH_INFO << "pk size_init " << cnt_init;
    NGRAPH_INFO << "pk_server_init data[0] = "
                << pk_server_init.data().data()[0];

    he_server_backend->set_role("SERVER");
    // he_server_backend->set_public_key(public_key);
    he_server_backend->server_init(port, 3);

    NGRAPH_INFO << "Server closed";
    NGRAPH_INFO << "Getting server pk...";
    // auto tmp = he_server_backend->get_secret_key();
    pk_server = *he_server_backend->get_public_key();
    NGRAPH_INFO << "Got server pk!!";
    auto cnt = pk_server.data().uint64_count();
    NGRAPH_INFO << "pk size " << cnt;
    NGRAPH_INFO << "pk_server data[0] = " << pk_server.data().data()[0];
  };

  auto client_fun = [hostname, port, &pk_client]() {
    auto client_backend = runtime::Backend::create("${BACKEND_NAME}");
    auto he_client_backend =
        static_cast<runtime::he::he_seal::HESealBackend*>(client_backend.get());
    he_client_backend->set_role("CLIENT");

    pk_client = *he_client_backend->get_public_key();

    auto context = he_client_backend->get_context();

    std::stringstream public_key_stream;
    he_client_backend->get_public_key()->save(public_key_stream);
    std::string public_key_str = public_key_stream.str();

    NGRAPH_INFO << "pk size " << public_key_str.size();

    // sleep(3);  // Wait until server initialized.

    auto socket = runtime::he::network::connect_to_server(hostname, port);

    runtime::he::network::send_data(socket, public_key_str);

    //  sleep(1);

    runtime::he::network::send_data(socket, "DONE");
  };

  // Run server and client
  std::thread t1(server_fun);
  std::thread t2(client_fun);

  // Wait until threads are done
  t1.join();
  t2.join();

  // Check keys are qual
  NGRAPH_INFO << "asserting eq";
  auto cnt = pk_client.data().uint64_count();
  NGRAPH_INFO << "cnt " << cnt;
  ASSERT_EQ(pk_client.data().uint64_count(), pk_server.data().uint64_count());
  NGRAPH_INFO << "asserteed eq";

  for (size_t i = 0; i < pk_client.data().uint64_count(); i++) {
    ASSERT_EQ(pk_client.data().data()[i], pk_server.data().data()[i]);
  }
  ASSERT_TRUE(pk_client.parms_id() == pk_server.parms_id());
}

NGRAPH_TEST(${BACKEND_NAME}, copy_public_key_string) {
  auto backend = runtime::Backend::create("${BACKEND_NAME}");
  auto he_backend =
      static_cast<runtime::he::he_seal::HESealBackend*>(backend.get());

  stringstream stream;
  he_backend->get_public_key()->save(stream);

  std::string public_key_str = stream.str();
  ASSERT_EQ(public_key_str.size(), 65609);
  stringstream stream2(public_key_str);

  auto backend2 = runtime::Backend::create("${BACKEND_NAME}");
  auto he_backend2 =
      static_cast<runtime::he::he_seal::HESealBackend*>(backend2.get());
  he_backend2->get_public_key()->load(he_backend2->get_context(), stream2);

  auto pk = *he_backend->get_public_key();
  auto pk2 = *he_backend2->get_public_key();

  ASSERT_EQ(pk.data().uint64_count(), pk2.data().uint64_count());

  for (size_t i = 0; i < pk.data().uint64_count(); i++) {
    ASSERT_EQ(pk.data().data()[i], pk2.data().data()[i]);
  }
  ASSERT_TRUE(pk.parms_id() == pk2.parms_id());
}

NGRAPH_TEST(${BACKEND_NAME}, copy_public_key) {
  auto backend = runtime::Backend::create("${BACKEND_NAME}");
  auto he_backend =
      static_cast<runtime::he::he_seal::HESealBackend*>(backend.get());

  stringstream stream;
  he_backend->get_public_key()->save(stream);

  auto backend2 = runtime::Backend::create("${BACKEND_NAME}");
  auto he_backend2 =
      static_cast<runtime::he::he_seal::HESealBackend*>(backend2.get());
  he_backend2->get_public_key()->load(he_backend2->get_context(), stream);

  auto pk = *he_backend->get_public_key();
  auto pk2 = *he_backend2->get_public_key();

  ASSERT_EQ(pk.data().uint64_count(), pk2.data().uint64_count());

  for (size_t i = 0; i < pk.data().uint64_count(); i++) {
    ASSERT_EQ(pk.data().data()[i], pk2.data().data()[i]);
  }
  ASSERT_TRUE(pk.parms_id() == pk2.parms_id());
}

NGRAPH_TEST(${BACKEND_NAME}, server_and_client_test) {
  auto hostname = "localhost";
  auto port = 33000;

  pid_t childPID = fork();
  if (childPID < 0) {
    throw ngraph_error("Fork failed");
  }

  if (childPID > 0)  // server
  {
    runtime::he::network::server_init(port, 2);
    sleep(2);  // Wait until client finished its connections

  } else {     // client
    sleep(1);  // Wait until server opened

    auto socket = runtime::he::network::connect_to_server(hostname, port);
    socket = runtime::he::network::connect_to_server(hostname, port);

    runtime::he::network::send_data(socket, "Message");

    void* data;
    runtime::he::network::receive_data(socket, data);

    NGRAPH_INFO << "Closing socket";
    socket.close();

    return;
  }
  NGRAPH_INFO << "Reached end of test";
}