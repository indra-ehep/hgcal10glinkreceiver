/**********************************************************************
 Created on : 20/12/2023
 Purpose    : ROC unpacker data
 Author     : Indranil Das, Visiting Fellow
 Email      : indranil.das@cern.ch | indra.ehep@gmail.com
**********************************************************************/
#include <bitset>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TCanvas.h"

typedef struct{
  int event,chip,half,channel,adc,adcm,toa,tot,totflag,trigtime,trigwidth,corruption,bxcounter,eventcounter,orbitcounter;
} rocdata;

typedef struct{
  int event,chip,trigtime,channelsumid,rawsum,validtp;
  float decompresssum;
} trigdata;

uint32_t compress(uint32_t val, bool isldm)
{
  
  uint32_t maxval = 0x3FFFF ;
  uint32_t maxval_ldm = 0x7FFFF ;
  uint32_t maxval_hdm = 0x1FFFFF ;
  val = (isldm)?val>>1:val>>3;
  if(isldm){
    if(val>maxval_ldm) val = maxval_ldm;
  }else{
    if(val>maxval_hdm) val = maxval_hdm;
  }
  if(val>maxval) val = maxval;

  //Ref:https://graphics.stanford.edu/%7Eseander/bithacks.html
  uint32_t r = 0; // r will be lg(v)
  uint32_t sub ;
  uint32_t shift ;
  uint32_t mant ;

  if(val>7){
    uint32_t v = val; 
    r = 0; 
    while (v >>= 1) r++;
    sub = r - 2;
    shift = r - 3;
    mant = (val>>shift) & 0x7;
  }else{
    r = 0;
    sub = 0;
    shift = 0;
    mant = val & 0x7;
  }
  
  bitset<4> expo = sub;
  bitset<3> mantissa = mant; 
  
  uint32_t packed = (sub<<3) | mant;
  
  return packed;
}

uint32_t decompress(uint32_t compressed, bool isldm)
{
  uint32_t mant = compressed & 0x7;
  uint32_t expo = (compressed>>3) & 0xF;

  if(expo==0) 
    return (isldm) ? (mant<<1) : (mant<<3) ;

  uint32_t shift = expo+2;
  uint32_t decomp = 1<<shift;
  uint32_t mpdeco = 1<<(shift-4);
  decomp = decomp | (mant<<(shift-3));
  decomp = decomp | mpdeco;
  decomp = (isldm) ? (decomp<<1) : (decomp<<3) ;
  
  return decomp;
}

//int ROCRead(const char *infile="/Data/hgcaltestbeam/register/SingleModuleTest/MLFL00041/energy_scan/electron_100GeV/beam_run/run_20230914_121656/beam_run99.root")
//int ROCRead(const char *infile="/home/indra/Data/BeamTest_Sept23/eos_dpg_hgcal/tb_hgcal/2023/BeamTestSep/SingleModuleTest/MLFL00041/energy_scan/electron_100GeV/beam_run/run_20230914_121656/beam_run99.root")
//int ROCRead(const char *infile="/eos/cms/store/group/dpg_hgcal/tb_hgcal/2023/BeamTestSep/SingleModuleTest/MLFL00041/energy_scan/electron_200GeV/beam_run/run_20230914_115411/beam_run1.root")
//int ROCRead(const char *infile="/eos/cms/store/group/dpg_hgcal/tb_hgcal/2023/BeamTestSep/SingleModuleTest/MLFL00041/position_scan/electron_200GeV_chip1_chip2_trg_fifo_latency_8/beam_run/run_20230914_170604/beam_run0.root")
int ROCRead(const char *infile="/Data/hgcaltestbeam/register/SingleModuleTest/MLFL00041/position_scan/electron_200GeV_chip1_chip2_trg_fifo_latency_8/beam_run/run_20230914_170604/beam_run0.root")
//int ROCRead(const char *infile="/vols/cms/idas/BeamTest_Sept23/eos_dpg_hgcal/tb_hgcal/2023/BeamTestSep/SingleModuleTest/MLFL00041/position_scan/electron_200GeV_chip1_chip2_trg_fifo_latency_8/beam_run/run_20230914_170604/beam_run0.root")
{
  TFile *fin = TFile::Open(infile);
  TTree *roctr = (TTree *)fin->Get("unpacker_data/hgcroc");
  TTree *trigtr = (TTree *)fin->Get("unpacker_data/triggerhgcroc");

  trigdata tdata;
  trigtr->SetBranchAddress("event",&(tdata.event));
  trigtr->SetBranchAddress("chip",&(tdata.chip));
  trigtr->SetBranchAddress("trigtime",&(tdata.trigtime));
  trigtr->SetBranchAddress("channelsumid",&(tdata.channelsumid));
  trigtr->SetBranchAddress("rawsum",&(tdata.rawsum));
  trigtr->SetBranchAddress("decompresssum",&(tdata.decompresssum));
  trigtr->SetBranchAddress("validtp",&(tdata.validtp));

  rocdata rdata;
  roctr->SetBranchAddress("event",&(rdata.event));
  roctr->SetBranchAddress("chip",&(rdata.chip));
  roctr->SetBranchAddress("half",&(rdata.half));
  roctr->SetBranchAddress("channel",&(rdata.channel));
  roctr->SetBranchAddress("adc",&(rdata.adc));
  roctr->SetBranchAddress("adcm",&(rdata.adcm));
  roctr->SetBranchAddress("toa",&(rdata.toa));
  roctr->SetBranchAddress("tot",&(rdata.tot));
  roctr->SetBranchAddress("totflag",&(rdata.totflag));
  roctr->SetBranchAddress("trigtime",&(rdata.trigtime));
  roctr->SetBranchAddress("trigwidth",&(rdata.trigwidth));
  roctr->SetBranchAddress("corruption",&(rdata.corruption));
  roctr->SetBranchAddress("bxcounter",&(rdata.bxcounter));
  roctr->SetBranchAddress("eventcounter",&(rdata.eventcounter));
  roctr->SetBranchAddress("orbitcounter",&(rdata.orbitcounter));
  
  cout<<" DAQ Entries : "<<roctr->GetEntries() << endl;;
  cout<<" TRIG Entries : "<<trigtr->GetEntries() << endl;
  
  // roctr->Show(10);
  // trigtr->Show(10);
  // roctr->Scan("chip:channel:adc:tot:totflag");
  // trigtr->Scan("chip:channelsumid:rawsum:decompresssum:validtp");  
  // roctr->Draw("chip");
  // trigtr->Draw("event");
  // roctr->Draw("corruption");
  //trigtr->Draw("trigtime");
  
  vector<trigdata> trarray;
  trarray.clear();
  vector<rocdata> rocarray;
  rocarray.clear();

  //trigtr->Show(1);
  for(int ievent=0;ievent<trigtr->GetEntries();ievent++){
    trigtr->GetEvent(ievent);
    //printf("tdata : %05d,\t%02d,\t%7d,\t%02d,\t%3d,\t%7.0f,\t%d\n", tdata.event, tdata.chip, tdata.trigtime, tdata.channelsumid, tdata.rawsum, tdata.decompresssum, tdata.validtp);
    if(tdata.validtp)
      trarray.push_back(tdata);
  }
  
  //roctr->Show(1);
  for(int ievent=0;ievent<roctr->GetEntries();ievent++){
    roctr->GetEvent(ievent);
    //printf("rdata : %05d,\t%02d,\t%02d,\t%02d,\t%5d,\t%5d,\t%5d,\t%5d,\t%d,\t%d,\t%d,\t%d,\t%d,\t%d,\t%d\n", rdata.event, rdata.chip, rdata.half, rdata.channel, rdata.adc, rdata.adcm, rdata.toa, rdata.tot, rdata.totflag, rdata.trigtime, rdata.trigwidth, rdata.corruption, rdata.bxcounter, rdata.eventcounter, rdata.orbitcounter);
    if(!rdata.corruption)
      rocarray.push_back(rdata);
  }
  
  printf("trigger array-size : %zu\n",trarray.size());
  printf("daq array-size : %zu\n",rocarray.size());

  fin->Close();
  delete fin;

  string Dens;
  unsigned Wtype, ROC, HalfROC, Seq;
  string ROCpin;
  unsigned ROCCH;
  int SiCell, TrLink, TrCell, iu, iv;
  float trace;
  int t;
  //Dens   Wtype     ROC HalfROC     Seq  ROCpin  SiCell  TrLink  TrCell      iu      iv   trace       t
  ifstream inwafermap("/home/indra/Downloads/WaferCellMapTraces.txt");
  //ifstream inwafermap("/afs/cern.ch/user/i/idas/Downloads/WaferCellMapTraces.txt");
  //ifstream inwafermap("/home/hep/idas/codes/WaferCellMapTraces.txt");
  stringstream ss;
  string s;
  
  map<int,pair<int,int>> rocchtoiuiv;
  map<int,tuple<int,int,int,int>> tctorocch;
  rocchtoiuiv.clear();
  tctorocch.clear();
  
  int prevTrigCell = -1;
  int itc = 0;
  unsigned rocpin[4];
  while(getline(inwafermap,s)){
    //cout << s.size() << endl;
    if(s.find(" LD ")!=string::npos or s.find(" HD ")!=string::npos){
      //cout << s << endl;
      ss << s.data() << endl;
      ss >> Dens >> Wtype >> ROC >> HalfROC >> Seq >> ROCpin >> SiCell  >> TrLink  >> TrCell  >> iu  >> iv  >> trace  >> t ;
      if(ROCpin.find("CALIB")==string::npos){
	ROCCH = stoi(ROCpin);
	if(Dens.find("LD")!=string::npos){
	  //if(Wtype==0 and ROC==0 and TrLink!=-1 and TrCell!=-1){
	  if(Wtype==0 and TrLink!=-1 and TrCell!=-1){
	    //int absTC = TrLink*4 + TrCell;
	    int absTC = ROC*16 + TrLink*4 + TrCell;
	    //cout <<"\t"<< Dens <<"\t"<< Wtype <<"\t"<< ROC <<"\t"<< HalfROC <<"\t"<< Seq <<"\t"<< ROCpin <<"\t"<< SiCell  <<"\t"<< TrLink  <<"\t"<< TrCell  <<"\t new TC : "<< absTC <<"\t"<< iu  <<"\t"<< iv  <<"\t"<< trace  <<"\t"<< t << endl ;
	    if(itc==3) prevTrigCell = absTC;	    
	    //rocpin[itc++] = ROCCH;
	    rocpin[itc++] = ROC*72 + ROCCH;
	    rocchtoiuiv[ROCCH] = make_pair(iu, iv);
	  }
	  if(itc==4){
	    tctorocch[prevTrigCell] = make_tuple(rocpin[0],rocpin[1],rocpin[2],rocpin[3]);
	    itc = 0;
	  }
	}
      }
    }else{
      ;//cout << s << endl;
    }
  }
  inwafermap.close();
  for(const auto& tcmap : tctorocch){
    if(tcmap.first < 8 )
      cout <<"TC " <<tcmap.first<<", pins :  ("<< get<0>(tcmap.second) << ", "<< get<1>(tcmap.second) << ", "<< get<2>(tcmap.second) << ", "<< get<3>(tcmap.second) << ") "<<endl;
    else
      cout <<"TC " <<tcmap.first<<", pins :  ("<< get<0>(tcmap.second) << ", "<< get<1>(tcmap.second) << ", "<< get<2>(tcmap.second) << ", "<< get<3>(tcmap.second) << ") "<<endl;
  }
  
  TH1F *hCompressDiffADC = new TH1F("hCompressDiffADC","Difference in (Emulator - ROC) compression ADC (totflag=0)", 100, -50, 50);
  TH1F *hCompressDiffTOT = new TH1F("hCompressDiffTOT","Difference in (Emulator - ROC) compression TOT (totflag=3)", 100, -50, 50);
  TH1F *hCompressDiffTOTUP = new TH1F("hCompressDiffTOTUP","Difference in (Emulator - ROC) compression for TOT>=512 (totflag=3)", 100, -50, 50);
  TH1F *hCompressDiffTOT9Bit = new TH1F("hCompressDiffTOT9Bit","Difference in (Emulator - ROC) compression for TOT<512 (totflag=3)", 100, -50, 50);
  TH1F *hCompressDiffTOTUPSel = new TH1F("hCompressDiffTOTUPSel","Difference in (Emulator - ROC) compression for TOTUPSel (totflag=3)", 100, -50, 50);
  TH1F *hDecompressDiffROC = new TH1F("hDecompressDiffROC","Difference for (decompressed - totadc) in ROC code", 1000, -500, 500);
  TH1F *hDecompressDiffECONT = new TH1F("hDecompressDiffECONT","Difference for (decompressed - totadc) in ECONT", 1000, -500, 500);
  TH1F *hDecompressDiffROCP = new TH1F("hDecompressDiffROCP","Difference for (decompressed - totadc)/totadc in ROC code in \%", 100, -50, 50);
  TH1F *hDecompressDiffECONTP = new TH1F("hDecompressDiffECONTP","Difference for (decompressed - totadc)/totadc in ECONT in \%", 100, -50, 50);
  
  int choffset = get<0>(tctorocch[8]) ; //since there are 16 TC per chip 
  for(const auto& trig : trarray){
    //if(trig.event>100) continue;
    //if(trig.chip!=0) continue;
    uint32_t totadc = 0;
    uint32_t totadcup = 0;
    int noftot3 = 0;
    bool istot1 = false;
    bool istot2 = false;
    bool issattot = false;
    int channelsumid = 16*trig.chip + trig.channelsumid;
    for(const auto& roc : rocarray){
      if(roc.event!=trig.event or roc.chip!=trig.chip) continue;
      if(roc.channel>=36) continue;
      int ch = (roc.half==1)?(roc.channel+choffset):roc.channel;
      ch += 72*roc.chip ; 
      uint32_t adc = uint32_t(roc.adc) & 0x3FF;
      uint32_t tot = uint32_t(roc.tot) ; //shift by 2 bits to recover 12-bit information
      uint32_t totup = tot + 7;
      uint32_t totlin = tot*25;
      uint32_t totlinup = totup*25;
      if(ch==get<0>(tctorocch[channelsumid]) or ch==get<1>(tctorocch[channelsumid]) or ch==get<2>(tctorocch[channelsumid]) or ch==get<3>(tctorocch[channelsumid])){
	totadc += (roc.totflag==3) ? totlin : adc ;
	totadcup += (roc.totflag==3) ? totlinup : adc ;
	if(roc.totflag==3) noftot3++;
	if(roc.totflag==1) istot1 = true;
	if(roc.totflag==2) istot2 = true;
	if(tot>=512 and roc.totflag==3) issattot = true;
	//cout<<"ievent : " << roc.event <<", chip : " << roc.chip << ", half : "<<roc.half<< ", channel : " << roc.channel<<", ch : "<<ch<<", adc : "<<adc<<", totflag : "<<roc.totflag <<", totadc : "<<totadc<<endl;
      }
    }//roc for loop
    
    if(!istot1 and !istot2) {
      uint32_t compressed = compress(totadc, 1);
      uint32_t compressedup = compress(totadcup, 1);
      uint32_t decompressed = decompress(compressed, 1);
      float diff = float(compressed) - float(trig.rawsum);
      float diffup = float(compressedup) - float(trig.rawsum);
      float decomp_diff_roc = float(trig.decompresssum) - float(totadc);
      float decomp_diff_econt = float(decompressed) - float(totadc);
      
      if(noftot3==0){
	hCompressDiffADC->Fill(diff);
	hDecompressDiffROC->Fill(decomp_diff_roc);
	hDecompressDiffECONT->Fill(decomp_diff_econt);
	hDecompressDiffROCP->Fill(100.*decomp_diff_roc/totadc);
	hDecompressDiffECONTP->Fill(100.*decomp_diff_econt/totadc);
      }
      
      if(noftot3>0){
	hCompressDiffTOT->Fill(diff);
	if(!issattot)
	  hCompressDiffTOT9Bit->Fill(diff);
	else
	  hCompressDiffTOTUP->Fill(diff);	
	if(!issattot)
	  hCompressDiffTOTUPSel->Fill(diff);
	else
	  hCompressDiffTOTUPSel->Fill(diffup);	
	hDecompressDiffROC->Fill(decomp_diff_roc);
	hDecompressDiffECONT->Fill(decomp_diff_econt);
	hDecompressDiffROCP->Fill(100.*decomp_diff_roc/totadc);
	hDecompressDiffECONTP->Fill(100.*decomp_diff_econt/totadc);
      }
      
      if(TMath::Abs(trig.rawsum - int(compressed))>0){
	printf("trig : Event : %05d, Chip : %02d, Trigtime : %4d, Channelsumid : %02d(%d), Sensorsum : %u, Rawsum : %3d, Compressed : %u, CompressedUp : %u, Decompresssum : %7.0f, ValidTP : %d\n", trig.event, trig.chip, trig.trigtime, trig.channelsumid, channelsumid, totadc, trig.rawsum, compressed, compressedup, trig.decompresssum, trig.validtp);
	// for(const auto& roc : rocarray){
	//   if(roc.event!=trig.event or roc.chip!=trig.chip) continue;
	//   if(trig.channelsumid>=8 and roc.half==0) continue;
	//   int ch = (roc.half==1)?(roc.channel+choffset):roc.channel;
	//   if(ch==get<0>(tctorocch[trig.channelsumid]) or ch==get<1>(tctorocch[trig.channelsumid]) or ch==get<2>(tctorocch[trig.channelsumid]) or ch==get<3>(tctorocch[trig.channelsumid]))
	//     printf("\troc Event : : %05d, Chip : %02d, Half : %02d, Channel : %02d, ADC : %5d, ADCM : %5d, TOA : %5d, TOT : %5d, TOTflag : %2d, Trigtime : %3d, Trigwidth : %d, Corruption : %d, BXCounter : %d, EventCounter : %d, OrbitCounter : %d\n", roc.event, roc.chip, roc.half, roc.channel, roc.adc, roc.adcm, roc.toa, roc.tot, roc.totflag, roc.trigtime, roc.trigwidth, roc.corruption, roc.bxcounter, roc.eventcounter, roc.orbitcounter);
	// }
      }//rawsum mismatch
    }//has a second incomplete tot
  }//trig for loop
  
  hCompressDiffADC->SetMinimum(1.e-1);
  hCompressDiffADC->GetXaxis()->SetTitle("Difference in compressed value : Emulator - ROC");
  hCompressDiffADC->SetLineColor(kRed);
  hCompressDiffADC->SetLineWidth(2);
  
  hCompressDiffTOT->SetMinimum(1.e-1);
  hCompressDiffTOT->GetXaxis()->SetTitle("Difference in compressed value : Emulator - ROC");
  hCompressDiffTOT->SetLineColor(kBlue);
  hCompressDiffTOT->SetLineWidth(2);
  
  hCompressDiffTOTUPSel->SetMinimum(1.e-1);
  hCompressDiffTOTUPSel->GetXaxis()->SetTitle("Difference in compressed value : Emulator - ROC");
  hCompressDiffTOTUPSel->SetLineColor(kGreen+2);
  hCompressDiffTOTUPSel->SetLineWidth(2);

  hCompressDiffTOTUP->SetMinimum(1.e-1);
  hCompressDiffTOTUP->GetXaxis()->SetTitle("Difference in compressed value : Emulator - ROC");
  hCompressDiffTOTUP->SetLineColor(kBlack);
  hCompressDiffTOTUP->SetLineWidth(2);

  hCompressDiffTOT9Bit->SetMinimum(1.e-1);
  hCompressDiffTOT9Bit->GetXaxis()->SetTitle("Difference in compressed value : Emulator - ROC");
  hCompressDiffTOT9Bit->SetLineColor(kMagenta);
  hCompressDiffTOT9Bit->SetLineWidth(2);

  hDecompressDiffECONT->SetMinimum(1.e-1);
  hDecompressDiffECONT->GetXaxis()->SetTitle("Difference in decompressed value : (Emulator/ROC - totadc)");
  hDecompressDiffECONT->SetLineColor(kBlue);
  hDecompressDiffROC->SetLineColor(kRed);

  hDecompressDiffECONTP->SetMinimum(1.e-1);
  hDecompressDiffECONTP->GetXaxis()->SetTitle("Difference in decompressed value : (Emulator/ROC - totadc) in \%");
  hDecompressDiffECONTP->SetLineColor(kBlue);
  hDecompressDiffROCP->SetLineColor(kRed);

  TCanvas *c1 = new TCanvas("c1","c1",800,600);
  c1->Divide(2,1);
  c1->cd(1)->SetLogy();
  hCompressDiffADC->Draw();
  c1->cd(2)->SetLogy();
  hCompressDiffTOT->Draw();

  TCanvas *c2 = new TCanvas("c2","c2",800,600);
  c2->Divide(3,1);
  c2->cd(1)->SetLogy();
  hCompressDiffTOT->Draw();
  c2->cd(2)->SetLogy();
  hCompressDiffTOT9Bit->Draw();
  c2->cd(3)->SetLogy();
  hCompressDiffTOTUP->Draw();

  TCanvas *c3 = new TCanvas("c3","c3",800,600);
  c3->Divide(2,1);
  c3->cd(1)->SetLogy();
  hCompressDiffTOT->Draw();
  c3->cd(2)->SetLogy();
  hCompressDiffTOTUPSel->Draw();

  TCanvas *c4 = new TCanvas("c4","c4",800,600);
  c4->SetLogy();
  hDecompressDiffECONT->Draw();
  hDecompressDiffROC->Draw("sames");

  TCanvas *c5 = new TCanvas("c5","c5",800,600);
  c5->SetLogy();
  hDecompressDiffECONTP->Draw();
  hDecompressDiffROCP->Draw("sames");
  
  TFile *fout = new TFile("log/CompressDiff.root","recreate");
  c1->Write();
  c2->Write();
  c3->Write();
  c4->Write();
  c5->Write();
  hCompressDiffADC->Write();
  hCompressDiffTOT->Write();
  hCompressDiffTOT9Bit->Write();
  hCompressDiffTOTUP->Write();
  hCompressDiffTOTUPSel->Write();
  hDecompressDiffECONT->Write();
  hDecompressDiffROC->Write();
  hDecompressDiffECONTP->Write();
  hDecompressDiffROCP->Write();
  fout->Close();

  return true;
}
