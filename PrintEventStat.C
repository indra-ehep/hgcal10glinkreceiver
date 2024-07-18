/**********************************************************************
 Created on : 17/07/2024
 Purpose    : Print the event statistics
 Author     : Indranil Das, Visiting Fellow
 Email      : indranil.das@cern.ch | indra.ehep@gmail.com
**********************************************************************/
int PrintEventStat(const char* infile="output/full/1_setzero/output_ele_BC9_Relay-1695829026_Link-1.root"){
  TFile *fin = TFile::Open(infile);
  TH1F *hEventCount = (TH1F *)fin->Get("diff_plots/hEventCount");
  hEventCount->Print();
  for(int ibin=2;ibin<=12;ibin++){
    if(ibin!=2 and ibin!=3 and ibin!=6 and ibin!=7 and ibin!=9) continue;
    if(ibin==2)
      std::cout<<" "<<TMath::Nint(hEventCount->GetBinContent(ibin));
    else if(ibin==12)
      std::cout<<" & "<<TMath::Nint(hEventCount->GetBinContent(ibin)) << "\\\\\\hline" <<endl;
    else
      std::cout<<" & "<<TMath::Nint(hEventCount->GetBinContent(ibin));
  }
  cout<< "\\\\\\hline" <<endl;
  delete hEventCount;
  fin->Close();
  delete fin;

  return true;
}
