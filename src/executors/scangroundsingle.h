// This may look like C code, but it is really -*- C++ -*-

#ifndef _SCAN_GROUND_SINGLE_H
#define _SCAN_GROUND_SINGLE_H

#include "executor.h"
#include "geoconvert.h"

#include <string>

namespace Exec {

  class ScanGroundSingle : public virtual Executor {
  private:
    std::vector<std::string> uavs;
    std::vector< std::map<std::string, double> > sensors_params;
    GeoConvert geoconv;

  public:
    ScanGroundSingle (std::string ns, int id) : Executor (ns, id) {
      // Set to true if the executor should expand the node during delegation
      set_delegation_expandable(false);
    };

    virtual ~ScanGroundSingle () {};

    virtual int expand (int free_id);
    virtual bool check ();

    bool prepare ();
    void start ();
    bool abort ();

    virtual bool get_constraints (std::vector<std::string> & cons);
    
  };

};

#endif