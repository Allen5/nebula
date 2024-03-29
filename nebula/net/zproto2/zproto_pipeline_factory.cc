/*
 *  Copyright (c) 2016, https://github.com/zhatalk
 *  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "nebula/net/zproto/zproto_pipeline_factory.h"

#include <wangle/channel/EventBaseHandler.h>

#include "nebula/net/thread_local_conn_manager.h"

#include "nebula/net/zproto/zproto_connection_handler.h"
#include "nebula/net/zproto/zproto_transport_handler.h"
#include "nebula/net/zproto/zproto_basic_sync_handler.h"
// #include "nebula/net/zproto/mtproto_codec.h"

// 设置readBufferSettings_
const uint64_t kDefaultMinAvailable = 8096;
const uint64_t kDefaultAllocationSize = 16192;

// void setReadBufferSettings(uint64_t minAvailable, uint64_t allocationSize);


///////////////////////////////////////////////////////////////////////////////////////////
NebulaPipeline::Ptr ZProtoPipelineFactory::newPipeline(std::shared_ptr<folly::AsyncTransportWrapper> sock) {
  auto pipeline = NebulaPipeline::create();
  pipeline->addBack(wangle::AsyncSocketHandler(sock));
  pipeline->addBack(wangle::EventBaseHandler()); // ensure we can write from any thread
  pipeline->addBack(TcpTransportDecoder());
  pipeline->addBack(TcpTransportHandler());
  pipeline->addBack(TransportHandler());
  pipeline->addBack(BasicSyncHandler());
  pipeline->addBack(ZProtoHandler(service_));
  
  pipeline->finalize();
  
  return pipeline;
}

///////////////////////////////////////////////////////////////////////////////////////////
NebulaPipeline::Ptr ZProtoClientPipelineFactory::newPipeline(std::shared_ptr<folly::AsyncTransportWrapper> sock) {
  auto pipeline = NebulaPipeline::create();
  pipeline->setReadBufferSettings(kDefaultMinAvailable, kDefaultAllocationSize);
  
  // Initialize TransportInfo and set it on the pipeline
  auto transportInfo = std::make_shared<wangle::TransportInfo>();
  folly::SocketAddress localAddr, peerAddr;
  sock->getLocalAddress(&localAddr);
  sock->getPeerAddress(&peerAddr);
  transportInfo->localAddr = std::make_shared<folly::SocketAddress>(localAddr);
  transportInfo->remoteAddr = std::make_shared<folly::SocketAddress>(peerAddr);
  pipeline->setTransportInfo(transportInfo);
  
  pipeline->addBack(wangle::AsyncSocketHandler(sock));
  pipeline->addBack(wangle::EventBaseHandler()); // ensure we can write from any thread
  pipeline->addBack(TcpTransportDecoder());
  pipeline->addBack(TcpTransportHandler());
  pipeline->addBack(TransportHandler());
  pipeline->addBack(BasicSyncHandler());
  pipeline->addBack(ZProtoHandler(service_));
  pipeline->finalize();
  
  return pipeline;
}

///////////////////////////////////////////////////////////////////////////////////////////
NebulaPipeline::Ptr ZProtoServerPipelineFactory::newPipeline(std::shared_ptr<folly::AsyncTransportWrapper> sock) {
  auto pipeline = NebulaPipeline::create();
  pipeline->setReadBufferSettings(kDefaultMinAvailable, kDefaultAllocationSize);
  pipeline->addBack(wangle::AsyncSocketHandler(sock));
  pipeline->addBack(TcpTransportDecoder());
  pipeline->addBack(TcpTransportHandler());
  pipeline->addBack(TransportHandler());
  pipeline->addBack(BasicSyncHandler());
  pipeline->addBack(ZProtoHandler(service_));
  pipeline->finalize();
  
  return pipeline;
}

