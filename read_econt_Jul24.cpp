/**********************************************************************
 Created on : 14/07/2024
 Purpose    : Read the econt data for test-beam of July 2024
 Author     : Indranil Das, Visiting Fellow
 Email      : indranil.das@cern.ch | indra.ehep@gmail.com
**********************************************************************/
#include <iostream>

#include "TH1D.h"
#include "TH2D.h"
#include "TMath.h"
#include "TProfile.h"
#include "TSystem.h"
#include "TCanvas.h"
#include "TFile.h"

#include "TFileHandlerLocal.h"
#include "common/inc/FileReader.h"

#include "yaml-cpp/yaml.h"
#include <deque>

#include <string>
#include <fstream>
#include <cassert>

#include "TPGFEDataformat.hh"
#include "TPGFEConfiguration.hh"
#include "TPGFEReader2024.hh"


int main(int argc, char** argv)
{
  //===============================================================================================================================
  // ./compile.sh
  // ./read_econt_Jul24.exe $Relay $rname $link_number
  //===============================================================================================================================  
  if(argc < 3){
    std::cerr << argv[0] << ": no relay and/or run numbers specified" << std::endl;
    return false;
  }
  if(argc < 4){
    std::cerr << argv[1] << ": no link number (1 or 2) is specified " << std::endl;
    return false;
  }
  //===============================================================================================================================
  
  //===============================================================================================================================
  //Assign relay,run and link numbers from input
  //===============================================================================================================================
  uint32_t relayNumber(0);
  uint32_t runNumber(0);
  uint32_t linkNumber(0);
  std::istringstream issRelay(argv[1]);
  issRelay >> relayNumber;
  std::istringstream issRun(argv[2]);
  issRun >> runNumber;
  std::istringstream issLink(argv[3]);
  issLink >> linkNumber;
  if(linkNumber!=1 and linkNumber!=2){
    std::cerr << "Link number "<< argv[3] <<"is out of bound (use: 1 or 2)" << std::endl;
    return false;
  }
  uint32_t trig_linkNumber = TMath::FloorNint((linkNumber-1)/2);
  int isMSB = 1;
  if(linkNumber==1) isMSB = 0;
  //===============================================================================================================================


  //===============================================================================================================================
  //Read channel mapping
  //===============================================================================================================================
  TPGFEConfiguration::Configuration cfgs;
  cfgs.setSiChMapFile("cfgmap/WaferCellMapTraces.txt");
  cfgs.setSciChMapFile("cfgmap/channels_sipmontile_HDtypes.hgcal.txt");
  cfgs.initId();
  cfgs.readSiChMapping();
  cfgs.readSciChMapping();
  cfgs.loadModIdxToNameMapping();
  //===============================================================================================================================

  
  //===============================================================================================================================
  //Read ECON-T setting
  //===============================================================================================================================
  
  cfgs.setEconTFile(Form("cfgmap/init_econt.yaml",relayNumber));
  cfgs.readEconTConfigYaml();
  
  //===============================================================================================================================

  //===============================================================================================================================
  //Set ECON-T parameters manually (as it was for September, 2023 beam-test)
  //===============================================================================================================================
  
  std::map<uint32_t,TPGFEConfiguration::ConfigEconT>& econTPar =  cfgs.getEconTPara();
  for(const auto& it : econTPar){
    econTPar[it.first].setDensity(1);
    econTPar[it.first].setDropLSB(1);
    if(relayNumber==1695733045){
      //following for STC4A
      econTPar[it.first].setSelect(1);
      econTPar[it.first].setSTCType(3);
    }else if(relayNumber==1695829026 or relayNumber==1695829376) {
      //following for BC9
      econTPar[it.first].setSelect(2);
      econTPar[it.first].setNElinks(4);
    }
    econTPar[it.first].setCalibration(0x800);
  }
  //===============================================================================================================================

  //===============================================================================================================================
  //Set and Initialize the ECONT reader
  //===============================================================================================================================
  TPGFEReader::ECONTReader econTReader(cfgs);
  econTReader.init(relayNumber,runNumber,linkNumber);
  //output array
  std::map<uint64_t,std::vector<std::pair<uint32_t,std::vector<TPGFEDataformat::TcRawData>>>> econtarray; //event,moduleId (from link)
  //===============================================================================================================================

  
  //===============================================================================================================================
  //Read one event
  //===============================================================================================================================
  uint64_t event = 10;
  econTReader.getEvents(event, econtarray);
  //===============================================================================================================================
  
  
  //===============================================================================================================================
  //Print the read value
  //===============================================================================================================================
  const std::vector<std::pair<uint32_t,std::vector<TPGFEDataformat::TcRawData>>>& vecont = econtarray.at(event);
  for(const auto& modpair : vecont){
    const uint32_t& modnum = modpair.first;
    const std::vector<TPGFEDataformat::TcRawData>& econtlist = modpair.second;
    std::cout <<"Event: "<< event << ", Module: " << modnum << ", size: " << econtlist.size() << std::endl;
    for(size_t itc=0 ; itc < econtlist.size() ; itc++){
      const TPGFEDataformat::TcRawData& tcedata = econtlist.at(itc);
      tcedata.print();
    }//loop to print the TC/STC results
  }//modules
  //===============================================================================================================================

  //delete the file readers
  econTReader.terminate();
  
  return true;
}
