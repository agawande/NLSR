#include <cstdlib>
#include <string>
#include <sstream>
#include <cstdio>

#include "nlsr.hpp"
#include "adjacent.hpp"
#include "logger.hpp"


namespace nlsr {

INIT_LOGGER("nlsr");

using namespace ndn;
using namespace std;

void
Nlsr::registrationFailed(const ndn::Name& name)
{
  std::cerr << "ERROR: Failed to register prefix in local hub's daemon" << endl;
  throw Error("Error: Prefix registration failed");
}

void
Nlsr::onRegistrationSuccess(const ndn::Name& name)
{
}

void
Nlsr::setInfoInterestFilter()
{
  ndn::Name name(m_confParam.getRouterPrefix());
  _LOG_DEBUG("Setting interest filter for name: " << name);
  getNlsrFace().setInterestFilter(name,
                                  ndn::bind(&HelloProtocol::processInterest,
                                            &m_helloProtocol, _1, _2),
                                  ndn::bind(&Nlsr::onRegistrationSuccess, this, _1),
                                  ndn::bind(&Nlsr::registrationFailed, this, _1));
}

void
Nlsr::setLsaInterestFilter()
{
  ndn::Name name = m_confParam.getLsaPrefix();
  name.append(m_confParam.getRouterPrefix());
  _LOG_DEBUG("Setting interest filter for name: " << name);
  getNlsrFace().setInterestFilter(name,
                                  ndn::bind(&Lsdb::processInterest,
                                            &m_nlsrLsdb, _1, _2),
                                  ndn::bind(&Nlsr::onRegistrationSuccess, this, _1),
                                  ndn::bind(&Nlsr::registrationFailed, this, _1));
}

void
Nlsr::registerPrefixes()
{
  std::string strategy("ndn:/localhost/nfd/strategy/broadcast");
  std::list<Adjacent>& adjacents = m_adjacencyList.getAdjList();
  for (std::list<Adjacent>::iterator it = adjacents.begin();
       it != adjacents.end(); it++) {
    m_fib.registerPrefix((*it).getName(), (*it).getConnectingFaceUri(),
                         (*it).getLinkCost(), 31536000); /* One Year in seconds */
    m_fib.registerPrefix(m_confParam.getChronosyncPrefix(),
                         (*it).getConnectingFaceUri(), (*it).getLinkCost(), 31536000);
    m_fib.registerPrefix(m_confParam.getLsaPrefix(),
                         (*it).getConnectingFaceUri(), (*it).getLinkCost(), 31536000);
     m_fib.setStrategy((*it).getName(), strategy);
  }
  
  m_fib.setStrategy(m_confParam.getChronosyncPrefix(), strategy);
  m_fib.setStrategy(m_confParam.getLsaPrefix(), strategy);
}

void
Nlsr::initialize()
{
  _LOG_DEBUG("Initializing Nlsr");
  m_confParam.buildRouterPrefix();
  m_nlsrLsdb.setLsaRefreshTime(m_confParam.getLsaRefreshTime());
  m_nlsrLsdb.setThisRouterPrefix(m_confParam.getRouterPrefix().toUri());
  m_fib.setEntryRefreshTime(2 * m_confParam.getLsaRefreshTime());
  m_sequencingManager.setSeqFileName(m_confParam.getSeqFileDir());
  m_sequencingManager.initiateSeqNoFromFile();
  /* debugging purpose start */
  cout << m_confParam;
  m_adjacencyList.print();
  m_namePrefixList.print();
  /* debugging purpose end */
  /* Logging start */
  m_confParam.writeLog();
  m_adjacencyList.writeLog();
  m_namePrefixList.writeLog();
  /* Logging end */
  registerPrefixes();
  setInfoInterestFilter();
  setLsaInterestFilter();
  m_nlsrLsdb.buildAndInstallOwnNameLsa();
  m_nlsrLsdb.buildAndInstallOwnCoordinateLsa();
  m_syncLogicHandler.setSyncPrefix(m_confParam.getChronosyncPrefix().toUri());
  m_syncLogicHandler.createSyncSocket(boost::ref(*this));
  //m_interestManager.scheduleInfoInterest(10);
  m_helloProtocol.scheduleInterest(10);
}

void
Nlsr::startEventLoop()
{
  m_nlsrFace.processEvents();
}

void
Nlsr::usage(const string& progname)
{
  cout << "Usage: " << progname << " [OPTIONS...]" << endl;
  cout << "   NDN routing...." << endl;
  cout << "       -d, --daemon        Run in daemon mode" << endl;
  cout << "       -f, --config_file   Specify configuration file name" << endl;
  cout << "       -p, --api_port      port where api client will connect" << endl;
  cout << "       -h, --help          Display this help message" << endl;
}


} // namespace nlsr
