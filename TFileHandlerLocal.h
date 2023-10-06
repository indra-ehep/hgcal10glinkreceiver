#ifndef TFileHandlerLocal_HH
#define TFileHandlerLocal_HH

#include <iostream>
#include <string>

#include "TFile.h"

class TFileHandlerLocal {
public:
 TFileHandlerLocal(const std::string &fileName, bool w=true) : _fileName(fileName), _doWrite(w) {
    if(_fileName.size()<5 ||
       _fileName.substr(fileName.size()-5,5)!=".root") _fileName+=".root";

    std::cout << "TFileHandlerLocal:: Opening ROOT file " << _fileName << std::endl;
    _rootFile=new TFile(_fileName.c_str(),"RECREATE");
  }

  ~TFileHandlerLocal() {
    if(_doWrite) {
      std::cout << "TFileHandlerLocal:: Writing ROOT file " << _fileName << std::endl;
      _rootFile->Write();
    }

    std::cout << "TFileHandlerLocal:: Closing ROOT file " << _fileName << std::endl;
    _rootFile->Close();
  
    delete _rootFile;
  }

  void cd() {
    _rootFile->cd();
  }

private:
  std::string _fileName;
  bool _doWrite;
  TFile *_rootFile;
};

#endif
