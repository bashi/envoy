#pragma once

#include <cstdint>
#include <functional>
#include <list>

#include "envoy/common/time.h"
#include "envoy/ssl/context_manager.h"
#include "envoy/ssl/private_key/private_key.h"
#include "envoy/stats/scope.h"

#include "source/common/tls/private_key/private_key_manager_impl.h"

namespace Envoy {
namespace Extensions {
namespace TransportSockets {
namespace Tls {

/**
 * The SSL context manager has the following threading model:
 * Contexts can be allocated via any thread (through in practice they are only allocated on the main
 * thread). They can be released from any thread (and in practice are since cluster information can
 * be released from any thread). Context allocation/free is a very uncommon thing so we just do a
 * global lock to protect it all.
 */
class ContextManagerImpl final : public Envoy::Ssl::ContextManager {
public:
  explicit ContextManagerImpl(TimeSource& time_source);
  ~ContextManagerImpl() override = default;

  // Ssl::ContextManager
  Ssl::ClientContextSharedPtr
  createSslClientContext(Stats::Scope& scope,
                         const Envoy::Ssl::ClientContextConfig& config) override;
  Ssl::ServerContextSharedPtr
  createSslServerContext(Stats::Scope& scope, const Envoy::Ssl::ServerContextConfig& config,
                         const std::vector<std::string>& server_names,
                         Ssl::ContextAdditionalInitFunc additional_init) override;
  absl::optional<uint32_t> daysUntilFirstCertExpires() const override;
  absl::optional<uint64_t> secondsUntilFirstOcspResponseExpires() const override;
  void iterateContexts(std::function<void(const Envoy::Ssl::Context&)> callback) override;
  Ssl::PrivateKeyMethodManager& privateKeyMethodManager() override {
    return private_key_method_manager_;
  };
  void removeContext(const Envoy::Ssl::ContextSharedPtr& old_context) override;

private:
  TimeSource& time_source_;
  absl::flat_hash_set<Envoy::Ssl::ContextSharedPtr> contexts_;
  PrivateKeyMethodManagerImpl private_key_method_manager_{};
};

} // namespace Tls
} // namespace TransportSockets
} // namespace Extensions
} // namespace Envoy
