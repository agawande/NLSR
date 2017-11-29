NLSR version 0.4.0
++++++++++++++++++

**New features**:

- **breaking change** Discover Available Faces (:issue:`2954`)

- **breaking change** Accommodate new HR coordinates (:issue:`3751`)

**Changes**:

- Use latest ChronoSync as dependency (:issue:`2400`)

- **breaking change** Use separate name prefix and sequence number
  for each LSA type in ChronoSync (:issue:`3964`)

- **breaking change** Refactor validator to v2 (:issue:`4119`)

- **breaking change** Use security\:\:v2 instead of removed security\:\:v1  (:issue:`3964`)

- Use UNIX timestamps as default in logging (:issue:`4187`)

**Bug Fixes**:

- Readvertise causes withdrawal of site prefix (:issue:`4177`)

- Error when face-dataset-fetch-interval is set correctly (:issue:`4211`)

- NLSR advertise functionality is broken (:issue:`4215`)

- NLSR heap buffer overflow error (:issue:`4217`)

- Fix SyncLogicHandler's tests (:issue:`4254`)

**Code Changes**:

- Construct LSA data strings using std\:\:ostringstream (:issue:`2346`)

- Send RIB registration commands within random interval of expiration time (:issue:`2648`)

- Remove unnecessary routing table calculations in hyperbolic routing (:issue:`2776`)

- FIB entries should be refreshed independently of routing table calculations (:issue:`2778`)

- Nlsr class should create and maintain Scheduler and Face instances (:issue:`2803`)

- NLSR's configuration parameters should be logged at INFO level (:issue:`2850`)

- A RoutingTable calculation should only update NamePrefixTableEntries with an
  affected RoutingTableEntry (:issue:`2864`)

- Design statistics collection mechanism (:issue:`2955`)

- Implement statistics collector/recorder (:issue:`2956`)

- Implement a NameLsa\:\:isEqualContent() method (:issue:`2962`)

- Remove uses of aliases defined in ndn-cxx/common.hpp (:issue:`3406`)

- Refactor Fib removeOldNextHopsFromFibEntryAndNfd() for simplicity, readability. (:issue:`3820`)

- Add more logging messages to NLSR (:issue:`3934`)

- Move top-level NLSR prefix registration  (:issue:`3938`)

- Get rid of aliases that are imported in ndn-cxx/common.hpp (:issue:`3983`)

- Canonize neighbor Face URIs on configuration file load  (:issue:`4063`)

- Refactor AdjacencyList to provide iterators instead of bare pointers (:issue:`4068`)

- Merge Fib\:\:update and Fib\:\:processUpdate (:issue:`4105`)

- NLSR Doxygen (:issue:`4118`)

- Update NLSR Developer's Guide to reflect Face discovery mechanism (:issue:`4121`)

- Add a finally() to canonizeUris (:issue:`4128`)

- Replace NamePrefixList\:\:writeLog with NamePrefixList\:\:operator<< (:issue:`4131`)

- Check for self in ChronoSync updates (:issue:`4134`)

- Write unit test to ensure selection of cheapest next-hop (:issue:`4169`)

- Install nlsr.conf as sample in the system (:issue:`4197`)

- Turn on address sanitizer builds for NLSR (:issue:`4206`)

- Resolve all ASan errors in unit tests (:issue:`4230`)

- Simplify LSDB Dataset Interest Handler and LSA Publisher (:issue:`4235`)

- Agnosticize Map (:issue:`4239`)

- Replace Map's data structure (:issue:`4240`)

- Fix LSDB's LsdbSync test (:issue:`4257`)

- Re-enable validation for remote routers for dataset requests (:issue:`4263`)

- Increase independence of SyncLogicHandler (:issue:`4264`)

- Use unique_ptr for signals (:issue:`4268`)

- Move sequence number file setting to the constructor (:issue:`4288`)

- Use of removed ndn-cxx identifiers (:issue:`4088`)

- Use network name in sync prefix (:issue:`4101`)