// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022-2025, The OpenROAD Authors

#pragma once

#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "absl/synchronization/mutex.h"

namespace odb {
class dbBlock;
class dbDatabase;
class dbMaster;
class dbMTerm;
class dbNet;
class dbInst;
class dbITerm;
}  // namespace odb

namespace utl {
class Logger;
}

// Timing modules (always included)
namespace cts {
class TritonCTS;
}

namespace rcx {
class Ext;
}

namespace rsz {
class Resizer;
}

namespace stt {
class SteinerTreeBuilder;
}

namespace sta {
class dbSta;
class LibertyCell;
}  // namespace sta

// Physical design modules (excluded in timing-only build)
#ifndef BUILD_TIMING_ONLY
namespace ifp {
class InitFloorplan;
}

namespace ant {
class AntennaChecker;
}

namespace dft {
class Dft;
}

namespace grt {
class GlobalRouter;
}

namespace gpl {
class Replace;
}

namespace dpl {
class Opendp;
}

namespace exa {
class Example;
}

namespace mpl {
class MacroPlacer;
}

namespace ppl {
class IOPlacer;
}

namespace tap {
class Tapcell;
}

namespace cgt {
class ClockGating;
}

namespace drt {
class TritonRoute;
}

namespace fin {
class Finale;
}

namespace par {
class PartitionMgr;
}

namespace rmp {
class Restructure;
}

namespace psm {
class PDNSim;
}

namespace pdn {
class PdnGen;
}

namespace pad {
class ICeWall;
}
#endif  // BUILD_TIMING_ONLY

namespace ord {

class OpenRoad;
class Tech;

class Design
{
 public:
  explicit Design(Tech* tech);

  void readVerilog(const std::string& file_name);
  void readDef(const std::string& file_name,
               bool continue_on_errors = false,
               bool floorplan_init = false,
               bool incremental = false);
  void link(const std::string& design_name);

  void readDb(std::istream& stream);
  void readDb(const std::string& file_name);
  void writeDb(std::ostream& stream);
  void writeDb(const std::string& file_name,
               std::optional<int> compression_level = std::nullopt);
  void writeDef(const std::string& file_name);

  odb::dbBlock* getBlock();
  utl::Logger* getLogger();

  int micronToDBU(double coord);

  // This is intended as a temporary back door to tcl from Python
  std::string evalTclString(const std::string& cmd);

  Tech* getTech();

  bool isSequential(odb::dbMaster* master);
  bool isBuffer(odb::dbMaster* master);
  bool isInverter(odb::dbMaster* master);
  bool isInSupply(odb::dbITerm* pin);
  std::string getITermName(odb::dbITerm* pin);
  bool isInClock(odb::dbInst* inst);
  bool isInClock(odb::dbITerm* iterm);
  std::uint64_t getNetRoutedLength(odb::dbNet* net);

  // Services - Timing modules (always included)
  cts::TritonCTS* getTritonCts();
  rcx::Ext* getOpenRCX();
  rsz::Resizer* getResizer();
  stt::SteinerTreeBuilder* getSteinerTreeBuilder();
  odb::dbDatabase* getDb();

#ifndef BUILD_TIMING_ONLY
  // Services - Physical design modules (excluded in timing-only build)
  ant::AntennaChecker* getAntennaChecker();
  cgt::ClockGating* getClockGating();
  dft::Dft* getDft();
  dpl::Opendp* getOpendp();
  exa::Example* getExample();
  drt::TritonRoute* getTritonRoute();
  fin::Finale* getFinale();
  gpl::Replace* getReplace();
  grt::GlobalRouter* getGlobalRouter();
  ifp::InitFloorplan getFloorplan();
  mpl::MacroPlacer* getMacroPlacer();
  pad::ICeWall* getICeWall();
  par::PartitionMgr* getPartitionMgr();
  pdn::PdnGen* getPdnGen();
  ppl::IOPlacer* getIOPlacer();
  psm::PDNSim* getPDNSim();
  rmp::Restructure* getRestructure();
  tap::Tapcell* getTapcell();
#endif  // BUILD_TIMING_ONLY

  // Needed by standalone startup, not for general use.
  ord::OpenRoad* getOpenRoad();

  // This returns a database that is not the one associated with
  // the rest of the application.  It is usable as a standalone
  // db but should not passed to any other Design or Tech APIs.
  //
  // This is useful if you need a second database for specialized
  // use cases and is not ordinarily required.
  static odb::dbDatabase* createDetachedDb();

 private:
  sta::dbSta* getSta();
  sta::LibertyCell* getLibertyCell(odb::dbMaster* master);

  Tech* tech_;

  // Single-thread access to the interpreter in evalTclString
  static absl::Mutex interp_mutex;
};

}  // namespace ord
