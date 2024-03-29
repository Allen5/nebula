#  Copyright (c) 2016, https://github.com/nebula-im/nebula
#  All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

set (SRC_LIST
  base/attach_data_helper.h
  base/nebula_pipeline.cc
  base/nebula_pipeline.h
  base/tcp_conn_event_callback.h
  thread_local_conn_manager.cc
  thread_local_conn_manager.h
  thread_group_list_manager.cc
  thread_group_list_manager.h
  base_server.cc
  base_server.h
  net_engine_manager.cc
  net_engine_manager.h
  base/client_bootstrap2.h
  base/service_base.h
  base/service_plugin.h
  base/service_config.cc
  base/service_config.h

  engine/cluster_manager.cc
  engine/cluster_manager.h
  engine/tcp_service_base.cc
  engine/tcp_service_base.h
  engine/tcp_server.h
  engine/tcp_client_group.h
  engine/tcp_client_pool.h
  engine/tcp_client.h
  engine/http_client.cc
  engine/http_client.h
  engine/http_server.cc
  engine/http_server.h
  engine/simple_tcp_client.cc
  engine/simple_tcp_client.h
  engine/simple_conn_handler.cc
  engine/simple_conn_handler.h
  engine/rpc_server.h
  engine/rpc_client.h


  http/http_client_lib.cc
  http/http_client_lib.h
  http/http_server_lib.cc
  http/http_server_lib.h
  http/http_server_lib_acceptor.cc
  http/http_server_lib_acceptor.h

  handler/nebula_event_callback.h
  handler/zproto/zproto_event_callback.cc
  handler/zproto/zproto_event_callback.h
  handler/zproto/zproto_handler.cc
  handler/zproto/zproto_handler.h
  handler/zproto/zproto_pipeline_factory.cc
  handler/zproto/zproto_pipeline_factory.h
  handler/zproto/zproto_frame_handler.cc
  handler/zproto/zproto_frame_handler.h
  handler/zproto/zproto_package_handler.cc
  handler/zproto/zproto_package_handler.h

  handler/module_install.cc
  handler/module_install.h
  handler/nebula_base_handler.cc
  handler/nebula_base_handler.h
  handler/nebula_handler_util.cc
  handler/nebula_handler_util.h
  handler/echo/echo_handler.cc
  handler/echo/echo_handler.h
  handler/http/http_request_handler.cc
  handler/http/http_request_handler.h

  zproto/zproto_frame_data.cc
  zproto/zproto_frame_data.h
  zproto/zproto_package_data.cc
  zproto/zproto_package_data.h

  rpc/zrpc_client_handler.cc
  rpc/zrpc_client_handler.h
  rpc/zrpc_pipeline_factory.cc
  rpc/zrpc_pipeline_factory.h
  rpc/zrpc_server_handler.cc
  rpc/zrpc_server_handler.h
  rpc/zrpc_client_dispatcher.cc
  rpc/zrpc_client_dispatcher.h
  rpc/zrpc_service.cc
  rpc/zrpc_service.h
  rpc/zrpc_module.cc
  rpc/zrpc_service_util.cc
  rpc/zrpc_service_util.h

  zproto/api/api_message_box.cc
  zproto/api/api_message_box.h
  zproto/api/proto/auth.proto
)

add_library(nebula-net STATIC ${SRC_LIST})
target_link_libraries(nebula-net)

add_subdirectory(handler/zproto/test)
add_subdirectory(test)
#add_subdirectory(zproto/api/cc)
