

void plotNeoThingies() {

  char crap[180];
  
  Double_t runNum[220];
  Double_t numHeadMtron[220];
  Double_t numHeadNeo[220];
  Double_t firstNeoHead[220];
  Double_t firstMtronHead[220];
  Double_t lastNeoHead[220];
  Double_t lastMtronHead[220];
  Double_t numEventMtron[220];
  Double_t numEventNeo[220];
  Double_t firstNeoEvent[220];
  Double_t firstMtronEvent[220];
  Double_t lastNeoEvent[220];
  Double_t lastMtronEvent[220];
  

  ifstream MtronFile("allRunSummaryMtron.txt");
  MtronFile.getline(crap,179);
  ifstream NeoFile("allRunSummaryNeobrick.txt");
  NeoFile.getline(crap,179);

  Int_t tempRun,match;

  while(MtronFile >> tempRun) {
    if(tempRun>201) break;
    MtronFile >> firstMtronHead[tempRun] >> lastMtronHead[tempRun] >> numHeadMtron[tempRun] >>  firstMtronEvent[tempRun] >> lastMtronEvent[tempRun] >> numEventMtron[tempRun] >> match;
  }

  Double_t diffHeadFirst[220]={0};
  Double_t diffHeadLast[220]={0};
  Double_t diffHeadNumber[220]={0};

  Double_t diffEventFirst[220]={0};
  Double_t diffEventLast[220]={0};
  Double_t diffEventNumber[220]={0};


  Int_t countMissingHeaders=0;
  Int_t countMissingEvents=0;
  Int_t mtronEvents=0;

  while(NeoFile >> tempRun) {
    if(tempRun>201) break;
    NeoFile >> firstNeoHead[tempRun] >> lastNeoHead[tempRun] >> numHeadNeo[tempRun] >>  firstNeoEvent[tempRun] >> lastNeoEvent[tempRun] >> numEventNeo[tempRun] >> match;
    runNum[tempRun]=tempRun;
    if(tempRun>11 && firstMtronHead[tempRun]>0 && firstNeoHead[tempRun]>0) {
      diffHeadFirst[tempRun]=firstMtronHead[tempRun]-firstNeoHead[tempRun];
      diffHeadLast[tempRun]=lastMtronHead[tempRun]-lastNeoHead[tempRun];
      diffHeadNumber[tempRun]=numHeadMtron[tempRun]-numHeadNeo[tempRun];
      std::cout << tempRun << "\t" << diffHeadFirst[tempRun] << "\t"
		<< diffHeadLast[tempRun] << "\t" << diffHeadNumber[tempRun]
		<< "\n";
      if(diffHeadNumber[tempRun]>0)
	countMissingHeaders+=diffHeadNumber[tempRun];
    }

    runNum[tempRun]=tempRun;
    if(tempRun>11 && firstMtronEvent[tempRun]>0 && firstNeoEvent[tempRun]>0) {
      diffEventFirst[tempRun]=firstMtronEvent[tempRun]-firstNeoEvent[tempRun];
      diffEventLast[tempRun]=lastMtronEvent[tempRun]-lastNeoEvent[tempRun];
      diffEventNumber[tempRun]=numEventMtron[tempRun]-numEventNeo[tempRun];
      std::cout << tempRun << "\t" << diffEventFirst[tempRun] << "\t"
		<< diffEventLast[tempRun] << "\t" << diffEventNumber[tempRun]
		<< "\n";
      if(diffEventNumber[tempRun]>0)
	countMissingEvents+=diffEventNumber[tempRun];
      mtronEvents+=numEventMtron[tempRun];
    }
    else {
      std::cout << "Broken " << tempRun << "\n";
    }

  }
  std::cout <<countMissingHeaders << "\t" << 100*Double_t(countMissingHeaders)/mtronEvents
	    << "\t" << countMissingEvents << "\t" <<  100*Double_t(countMissingEvents)/mtronEvents
	    << "\t" << mtronEvents << "\n";
  TCanvas *can = new TCanvas("can","can");  
  can->Divide(1,2);
  can->cd(1);
  {
    TMultiGraph *mg = new TMultiGraph();
    TGraph *grFirstHead=new TGraph(186,&runNum[12],&diffHeadFirst[12]);
    mg->Add(grFirstHead,"p");
    TGraph *grLastHead=new TGraph(186,&runNum[12],&diffHeadLast[12]);
    grLastHead->SetMarkerColor(getNiceColour(2));
    grLastHead->SetMarkerStyle(22);
    mg->Add(grLastHead,"p");
    TGraph *grNumberHead=new TGraph(186,&runNum[12],&diffHeadNumber[12]);
    grNumberHead->SetMarkerColor(getNiceColour(3));
    grNumberHead->SetMarkerStyle(23);
    mg->Add(grNumberHead,"p");
    mg->SetTitle("Comparison of Headers");
    mg->Draw("ap");
    mg->GetXaxis()->SetTitle("Run Number");
    mg->GetYaxis()->SetTitle("#mtron - #neobrick");

    TLegend *leggy = new TLegend(0.7,0.7,0.9,0.9);
    leggy->SetFillStyle(0);
    leggy->SetFillColor(0);
    leggy->SetBorderSize(1);
    leggy->AddEntry(grFirstHead,"#Delta First","p");
    leggy->AddEntry(grLastHead,"#Delta Last","p");
    leggy->AddEntry(grNumberHead,"#Delta Number","p");
    leggy->Draw("same");

  }
  can->cd(2);
  {
    TMultiGraph *mg = new TMultiGraph();
    TGraph *grFirstEvent=new TGraph(186,&runNum[12],&diffEventFirst[12]);
    mg->Add(grFirstEvent,"p");
    TGraph *grLastEvent=new TGraph(186,&runNum[12],&diffEventLast[12]);
    grLastEvent->SetMarkerColor(getNiceColour(2));
    grLastEvent->SetMarkerStyle(22);
    mg->Add(grLastEvent,"p");
    TGraph *grNumberEvent=new TGraph(186,&runNum[12],&diffEventNumber[12]);
    grNumberEvent->SetMarkerColor(getNiceColour(3));
    grNumberEvent->SetMarkerStyle(23);
    mg->Add(grNumberEvent,"p");
    mg->SetTitle("Comparison of Events");
    mg->Draw("ap");
    mg->GetXaxis()->SetTitle("Run Number");
    mg->GetYaxis()->SetTitle("#mtron - #neobrick");
  }
  
  

  



}

