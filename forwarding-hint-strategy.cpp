#include "forwarding-hint-strategy.hpp"

#include "fw/forwarder.hpp"
#include "table/fib.hpp"
#include "core/logger.hpp"

namespace nfd {
namespace fw {

NFD_LOG_INIT(ForwardingHintStrategy);

//const ndn::Name STRATEGY_NAME("/localhost/nfd/strategy/forwarding-hint");

NFD_REGISTER_STRATEGY(ForwardingHintStrategy);

//const ndn::Name&
//ForwardingHintStrategy::getStrategyName()
//{
//  return STRATEGY_NAME;
//}
const ndn::Name&
ForwardingHintStrategy::getStrategyName()
{
  static const auto strategyName = Name("/localhost/nfd/strategy/forwarding-hint").appendVersion(1);
  return strategyName;
}



ForwardingHintStrategy::ForwardingHintStrategy(Forwarder& forwarder,
                                               const ndn::Name& name)
  : Strategy(forwarder)
{
  this->setInstanceName(name);
}


void
ForwardingHintStrategy::afterReceiveInterest(const Interest& interest,
                                             const FaceEndpoint& ingress,
                                             const shared_ptr<pit::Entry>& pitEntry)
{
  // Primeiro salto: nenhum OutRecord ainda
  if (pitEntry->getOutRecords().empty()) {
    forwardFirstHop(interest, ingress, pitEntry);
    return;
  }

  // Saltos subsequentes
  forwardWithHint(interest, ingress, pitEntry);
}

void
ForwardingHintStrategy::forwardFirstHop(const Interest& interest,
                                        const FaceEndpoint& ingress,
                                        const shared_ptr<pit::Entry>& pitEntry)
{
  const fib::Entry& fibEntry = this->lookupFib(*pitEntry);

  for (const auto& nh : fibEntry.getNextHops()) {
    if (nh.getFace().getId() == ingress.face.getId())
      continue;

    int newCost = static_cast<int>(nh.getCost()) - 1;

    Interest outInterest = makeUpdatedInterest(interest, newCost);
    this->sendInterest(outInterest, nh.getFace(), pitEntry);
  }
}

void
ForwardingHintStrategy::forwardWithHint(const Interest& interest,
                                        const FaceEndpoint& ingress,
                                        const shared_ptr<pit::Entry>& pitEntry)
{
  int cost = extractHintValue(interest);
  if (cost < 0) {
    forwardFirstHop(interest, ingress, pitEntry);
    return;
  }

  const fib::Entry& fibEntry = this->lookupFib(*pitEntry);

  for (const auto& nh : fibEntry.getNextHops()) {
    if (static_cast<int>(nh.getCost()) != cost)
      continue;

    if (nh.getFace().getId() == ingress.face.getId())
      continue;

    Interest outInterest = makeUpdatedInterest(interest, cost - 1);
    this->sendInterest(outInterest, nh.getFace(), pitEntry);
    return;
  }

  // fallback
  forwardFirstHop(interest, ingress, pitEntry);
}

int
ForwardingHintStrategy::extractHintValue(const Interest& interest) const
{
  auto fh = interest.getForwardingHint();
  if (fh.empty())
    return -1;

  try {
    return static_cast<int>(fh.front().at(0).toNumber());
  }
  catch (...) {
    return -1;
  }
}

Interest
ForwardingHintStrategy::makeUpdatedInterest(const Interest& original,
                                            int newCost) const
{
  Interest interest(original);

  if (newCost < 0) {
    interest.setForwardingHint({});
    return interest;
  }

  ndn::Name hint;
  hint.appendNumber(newCost);

  interest.setForwardingHint({hint});
  return interest;
}

} // namespace fw
} // namespace nfd
