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

#include "nebula/net/net_engine_manager.h"

#include <glog/logging.h>
#include <folly/MoveWrapper.h>

// #include "nebula/net/base/service_factory_manager.h"
#include "nebula/net/thread_local_conn_manager.h"

//#include "nebula/net/server/tcp_client_group.h"
//#include "nebula/net/server/tcp_server.h"
//
//#include "nebula/net/rpc_conn_manager.h"

namespace nebula {
  
folly::Singleton<NetEngineManager> g_service_manager;

std::shared_ptr<NetEngineManager> NetEngineManager::GetInstance() {
  return g_service_manager.try_get();
}

void NetEngineManager::set_thread_groups(const std::shared_ptr<ThreadGroupListManager>& thread_groups) {
  thread_groups_ = thread_groups;
  
  // 注册默认服务名"tcp_server"
//  auto acc = thread_groups_->GetIOThreadPoolExecutor(ThreadType::CONN_ACCEPT);
//  if (acc) {
//    ServiceFactoryManager::GetInstance()->RegisterServiceFactory(TcpServerFactory::GetDefaultFactory(acc));
//  }
//  
//  // 注册默认服务名"tcp_client"
//  auto conn = thread_groups_->GetIOThreadPoolExecutor(ThreadType::CONN_CLIENT);
//  if (conn) {
//    ServiceFactoryManager::GetInstance()->RegisterServiceFactory(TcpClientFactory::GetDefaultFactory(conn));
//  }
}


void NetEngineManager::Install(const std::string& name,
                               const std::string& type,
                               const std::string& proto) {
  
  auto service_config = LookupServiceConfig(name, type, proto);
  if (!service_config) {
    LOG(ERROR) << "Install - UNKNOWN service_type: name = " << name
                  << ", type = " << type
                  << ", proto = " << proto;
    return;
  }
  
  // CONN
  // 首先检查conn
  auto io_group = thread_groups_->GetIOThreadPoolExecutor(ThreadType::CONN);
  if (io_group) {
    if (type != "tcp_server" &&
        type != "tcp_client" &&
        type != "http_server" &&
        type != "rpc_server" &&
        type != "rpc_client") {
      LOG(ERROR) << "Install - thread_type is conn, UNKNOWN service_type: name = " << name
            << ", type = " << type
            << ", proto = " << proto;
      return;
    }
  } else {
    if (type == "tcp_server" ||
        type == "http_server" ||
        type == "rpc_server") {
      io_group = thread_groups_->GetIOThreadPoolExecutor(ThreadType::CONN_ACCEPT);
      if (!io_group) {
        LOG(ERROR) << "Install - not setup tcp_server's [accept] thread_type, service_type: name = " << name
              << ", type = " << type
              << ", proto = " << proto;
        return;
      }
    } else if (type == "tcp_client" || type == "rpc_client") {
      io_group = thread_groups_->GetIOThreadPoolExecutor(ThreadType::CONN_CLIENT);
      if (!io_group) {
        LOG(ERROR) << "Install - not setup tcp_client's [client] thread_type, service_type: name = " << name
              << ", type = " << type
              << ", proto = " << proto;
        return;
      }
    } else {
      LOG(ERROR) << "Install - UNKNOWN service_type: name = " << name << ", type = " << type
              << ", type = " << type
              << ", proto = " << proto;
      return;
    }
  }
  
  installeds_.push_back(std::make_pair(service_config, io_group));
}


void NetEngineManager::SetupService(const ServicesConfig& config) {
  service_configs_ = config.service_configs;
}

bool NetEngineManager::Start() {
  // 从配置文件里读取
  for (auto& service_config : service_configs_) {
    // 创建出一个service
    auto service = CreateService(*service_config);
    if (service) {
      // tcp_client需要特殊处理，在RegisterServiceBase
      if (!RegisterServiceBase(service)) {
        LOG(ERROR) << "Start - RegisterServiceBase error by " << service_config->ToString();
      }
    } else {
      LOG(ERROR) << "Start - CreateService error, type = " << service_config->ToString();
    }
  }
  
  for (auto it = services_.begin(); it!=services_.end(); ++it) {
    it->second->Start();
  }

  return true;
}

bool NetEngineManager::Pause() {
  return true;
}

bool NetEngineManager::Stop() {
  for (auto it = services_.begin(); it!=services_.end(); ++it) {
    it->second->Stop();
  }
  // services_.clear();
  return true;
}

//bool NetEngineManager::SendIOBuf(uint64_t conn_id, std::unique_ptr<folly::IOBuf> data) {
//  auto evb = thread_groups_->GetEventBaseByThreadID(conn_id >> 32);
//  if (!evb) {
//    LOG(ERROR) << "SendIOBufByConnID - thread_id not find, invalid conn_id: " << ToString(conn_id);
//    return false;
//  }
//  
//  // std::shared_ptr<folly::IOBuf> d = std::move(data);
//  auto d = folly::makeMoveWrapper(std::move(data));
//  
//  // TODO(@benqi):
//  //   C++14支持lambada捕获
//  //   线上gcc版本是否支持c++11
//  evb->runInEventBaseThread([conn_id, d]() mutable {
//    // GetConnManagerByThreadLocal().SendIOBufByConnID(conn_id, std::move((*d)->clone()));
//    GetConnManagerByThreadLocal().SendIOBufByConnID(conn_id, d.move());
//  });
//  return true;
//}
//
//bool NetEngineManager::SendIOBuf(const std::string& service_name,
//                               std::unique_ptr<folly::IOBuf> data,
//                               DispatchStrategy dispatch_strategy) {
//  bool rv = false;
//  // 首先找到tcp_client_group
//  auto service = Lookup(service_name);
//  if (service) {
//    if (service->GetModuleType() == ServiceModuleType::TCP_CLIENT_GROUP) {
//      rv = std::static_pointer_cast<TcpClientGroup>(service)->SendIOBuf(std::move(data), dispatch_strategy);
//    } else {
//      LOG(ERROR) << "SendIOBuf - service_name's module_type not TCP_CLIENT_GROUP, "
//      << service_name << " type is " << static_cast<int>(service->GetModuleType());
//    }
//  } else {
//    LOG(ERROR) << "SendIOBuf - not lookup: " << service_name;
//  }
//  return rv;
//}
//
//bool NetEngineManager::SendIOBuf(const std::string& server_name,
//                               uint32_t server_number,
//                               std::unique_ptr<folly::IOBuf> data) {
//  return RpcConnManager::GetInstance()->SendIOBufByRpc(server_name, server_number, std::move(data));
//}
//
//bool NetEngineManager::SendIOBuf(const std::string& service_name,
//               std::unique_ptr<folly::IOBuf> data,
//               const std::function<void()>& c) {
//  
//  bool rv = false;
//  // 首先找到tcp_client_group
//  auto service = Lookup(service_name);
//  if (service) {
//    if (service->GetModuleType() == ServiceModuleType::TCP_CLIENT_GROUP) {
//      rv = std::static_pointer_cast<TcpClientGroup>(service)->SendIOBuf(std::move(data), c);
//    } else {
//      LOG(ERROR) << "SendIOBuf - service_name's module_type not TCP_CLIENT_GROUP, "
//      << service_name << " type is " << static_cast<int>(service->GetModuleType());
//    }
//  } else {
//    LOG(ERROR) << "SendIOBuf - not lookup: " << service_name;
//  }
//  return rv;
//}

// 查找分组
std::shared_ptr<ServiceBase> NetEngineManager::Lookup(const std::string& service_name) {
  std::shared_ptr<ServiceBase> service;
  
  auto it = services_.find(service_name);
  if (it!=services_.end()) {
    service = it->second;
  }
  
  return service;
}

bool NetEngineManager::RegisterServiceBase(std::shared_ptr<ServiceBase> service) {
  bool rv = false;
  
  LOG(INFO) << "RegisterServiceBase - " << service->GetServiceName();
  
  auto it = services_.find(service->GetServiceName());
  
  // bool is_group = service->GetServiceType() == "tcp_client";
  if (service->GetServiceType() == "tcp_client" ||
      service->GetServiceType() == "rpc_client") {
    // tcp_client
    if (it != services_.end()) {
      // it找到，必须检查是否是组，而且类型相同
      if (it->second->GetModuleType() == ServiceModuleType::TCP_CLIENT_GROUP &&
          service->GetServiceType() == it->second->GetServiceType()) {
        
        LOG(INFO) << "RegisterServiceBase - client: register group's service " << service->GetServiceConfig().ToString();
        it->second->AddChild(service);
        rv = true;
      } else {
        // 非组或非相同服务类型，则配置有问题
        LOG(FATAL) << "RegisterServiceBase - client: not group, duplicated service name: " << service->GetServiceConfig().ToString();
      }
    } else {
      // 未找到，则加进
      LOG(INFO) << "RegisterServiceBase - client: register group's service " << service->GetServiceConfig().ToString();
      
      auto io_group = thread_groups_->GetIOThreadPoolExecutor(ThreadType::CONN);
      if (!io_group) {
        io_group = thread_groups_->GetIOThreadPoolExecutor(ThreadType::CONN_CLIENT);
      }
      
      auto k = std::make_pair("tcp_client_group", service->GetServiceProto());
      auto tcp_client_group = ServiceSelfRegisterFactoryManager::Execute3(k, service->GetServiceConfig(), io_group);
      tcp_client_group->AddChild(service);

      services_.insert(std::make_pair(service->GetServiceName(), tcp_client_group));
      
      rv = true;
    }
  } else if (service->GetServiceType() == "tcp_server" ||
             service->GetServiceType() == "http_server" ||
             service->GetServiceType() == "rpc_server") {
    // tcp_server
    if (it == services_.end()) {
      LOG(INFO) << "RegisterServiceBase - tcp_server: register tcp_server service " << service->GetServiceName();
      services_.insert(std::make_pair(service->GetServiceName(), service));
      rv = true;
    } else {
      LOG(FATAL) << "RegisterServiceBase - tcp_server: not group, duplicated service name: " << service->GetServiceName();
    }
  } else {
    // invalid service_type
    LOG(FATAL) << "RegisterServiceBase - invalid service type: " << service->GetServiceType();
  }
  
  return rv;
}

  
ServiceConfigPtr NetEngineManager::LookupServiceConfig(const std::string& name,
                                                     const std::string& type,
                                                     const std::string& proto) const {
  ServiceConfigPtr service_config;
  
  for (auto& v : service_configs_) {
    if (v->name == name &&
        v->type == type &&
        v->proto == proto) {
      service_config = v;
      break;
    }
  }
  
  return service_config;
}

ServiceBasePtr NetEngineManager::CreateService(const ServiceConfig& service_config) const {
  ServiceBasePtr service;
  
  for (auto& v : installeds_) {
    if (v.first->name == service_config.name &&
        v.first->type == service_config.type &&
        v.first->proto == service_config.proto) {
      auto k = std::make_pair(service_config.type, service_config.proto);
      service = ServiceSelfRegisterFactoryManager::Execute3(k, *v.first, v.second);
      break;
    }
  }
  
  return service;
}

}
