#ifndef NFD_DAEMON_FW_FORWARDING_HINT_STRATEGY_HPP
#define NFD_DAEMON_FW_FORWARDING_HINT_STRATEGY_HPP

#include "fw/strategy.hpp"

namespace nfd {
namespace fw {

class ForwardingHintStrategy : public Strategy
{
public:
  explicit
  ForwardingHintStrategy(Forwarder& forwarder, const ndn::Name& name);

  static const ndn::Name&
  getStrategyName();

  void
  afterReceiveInterest(const Interest& interest,
                       const FaceEndpoint& ingress,
                       const shared_ptr<pit::Entry>& pitEntry) override;

private:
  void
  forwardFirstHop(const Interest& interest,
                  const FaceEndpoint& ingress,
                  const shared_ptr<pit::Entry>& pitEntry);

  void
  forwardWithHint(const Interest& interest,
                  const FaceEndpoint& ingress,
                  const shared_ptr<pit::Entry>& pitEntry);

  int
  extractHintValue(const Interest& interest) const;

  Interest
  makeUpdatedInterest(const Interest& original, int newCost) const;
};

} // namespace fw
} // namespace nfd

#endif // NFD_DAEMON_FW_FORWARDING_HINT_STRATEGY_HPP
