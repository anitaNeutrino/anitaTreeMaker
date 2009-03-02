

void plotEventNumberThingies() {

  char crap[180];
  
  Double_t runNum[270];
  Double_t numHead[270];
  Double_t firstHead[270];
  Double_t lastHead[270];
  Double_t numEvent[270];
  Double_t firstEvent[270];
  Double_t lastEvent[270];
  Double_t diffHeads[270]={0};
  Double_t firstMinusLast[270]={0};
  Double_t cumulativeEvents[270]={0};
  Double_t totalMissingEvents[270]={0};

  ifstream AllFile("allRunSummaryAll.txt");
  AllFile.getline(crap,179);
  Int_t tempRun,match;

  while(AllFile >> tempRun) {
    AllFile >> firstHead[tempRun] >> lastHead[tempRun] >> numHead[tempRun] >>  firstEvent[tempRun] >> lastEvent[tempRun] >> numEvent[tempRun] >> match;
    runNum[tempRun]=tempRun;
    if(firstHead[tempRun]>0) 
      diffHeads[tempRun]=(1+lastHead[tempRun]-firstHead[tempRun])-numHead[tempRun];
    if(tempRun>1) {
      if(firstHead[tempRun]>0) {
	if(lastHead[tempRun-1]>0) {
	  firstMinusLast[tempRun]=firstHead[tempRun]-lastHead[tempRun-1];
	}
	else if(lastHead[tempRun-2]>0) {
	  firstMinusLast[tempRun]=firstHead[tempRun]-lastHead[tempRun-2];
	}
      }
    }
    if(tempRun>11) {
      cumulativeEvents[tempRun]=cumulativeEvents[tempRun-1]+numHead[tempRun];
      totalMissingEvents[tempRun]=totalMissingEvents[tempRun-1];
      totalMissingEvents[tempRun]+=firstMinusLast[tempRun];
      totalMissingEvents[tempRun]+=diffHeads[tempRun];
    }
  }

  TCanvas *can = new TCanvas("can","can");
  can->Divide(1,2);
  can->cd(1);
  TGraph *gr = new TGraph(251,&runNum[12],&diffHeads[12]);
  gr->SetTitle("Missing Event Numbers (within run)");
  gr->Draw("ap");
  gr->GetXaxis()->SetTitle("Run");
  gr->GetYaxis()->SetTitle("# Events");

  can->cd(2);
  TGraph *grEnd = new TGraph(251,&runNum[12],&firstMinusLast[12]);
  grEnd->Draw("ap");
  grEnd->SetMarkerColor(getNiceColour(3));
  grEnd->SetTitle("Missing Event Numbers (between runs)");
  grEnd->GetXaxis()->SetTitle("Run");
  grEnd->GetYaxis()->SetTitle("# Events");


  for(int run=12;run<263;run++) {
    std::cout << run << "\t" << (Int_t)cumulativeEvents[run] << "\t" << totalMissingEvents[run] << "\t" << 100*totalMissingEvents[run]/cumulativeEvents[run] << "\n";
    numHead[run]/=1000;
    cumulativeEvents[run]/=1e6;

  }


  TCanvas *canEvents = new TCanvas("canEvents","canEvents");
  canEvents->Divide(1,2);
  canEvents->cd(1);
  TGraph *gr = new TGraph(251,&runNum[12],&numHead[12]);
  gr->SetTitle("Events per Run");
  gr->Draw("ap");
  gr->GetXaxis()->SetTitle("Run");
  gr->GetYaxis()->SetTitle("# Thousand Events");
  sortOutTitle();

  canEvents->cd(2);
  TGraph *grEnd = new TGraph(251,&runNum[12],&cumulativeEvents[12]);
  grEnd->Draw("ap");
  grEnd->SetMarkerColor(getNiceColour(3));
  grEnd->SetTitle("Cumulative Events");
  grEnd->GetXaxis()->SetTitle("Run");
  grEnd->GetYaxis()->SetTitle("# Million Events");
  sortOutTitle();
  



}

