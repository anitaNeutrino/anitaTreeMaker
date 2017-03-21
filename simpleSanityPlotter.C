/**

   Simple program to plot a full canvas with all pat (position + altitude) information
   The upper most two plots to each indiviual canvas are the raw ADU5A/B data with no cuts
   The final plot shows the interpolated line (black) and ADU5A(red)/B(green) with cuts, to see how the interpolated line behaves
   Visualises GPS data in an easy way for data checking.

 **/

#include <cmath>
#include "TTree.h" 
#include <assert.h>
#include "TChain.h" 
#include "DigitalFilter.h" 
#include "TLeaf.h" 
#include "TGraphErrors.h" 
#include "TTimeStamp.h" 
#include "TFile.h" 
#include "RFInterpolate.h" 
#include "FFTtools.h"
#include "AnitaGeomTool.h"
#include "RawAnitaHeader.h" 
#include "Adu5Pat.h"
#include <string>

Int_t run = 107;

TCanvas* plotPatVar(string patVar)
{

  char const * datadir = "/home/berg/Dropbox/LinuxSync/PhD/ANITA/2017Work/gpsEvent";

  TFile old_file(TString::Format("%s/run%d/gpsFile%d.root",datadir,run,run)); 

  TTree * adu5A = (TTree*) old_file.Get("adu5PatTree");
  
  adu5A->SetMarkerColor(2); //red
  adu5A->SetMarkerStyle(7);
  adu5A->SetMarkerSize(.5);
  TTree * adu5B = (TTree*) old_file.Get("adu5bPatTree"); 
  adu5B->SetMarkerColor(3); //green
  adu5B->SetMarkerStyle(7); 
  
  TFile new_file(TString::Format("%s/run%d/timedGpsEvent%d.root",datadir,run,run)); 

  TTree * adu5I = (TTree*) new_file.Get("adu5PatTree"); 
  if (!adu5I) return 0; 
  adu5I->SetMarkerStyle(7);

    if (adu5A->GetEntries() == 0 || adu5B->GetEntries() == 0 || adu5I->GetEntries() == 0) 
      {
	printf("no attitude!\n"); 
      }
  
  TCanvas *c = new TCanvas();
  c->SetFillColor(0);
  
  TPad *c1_1= new TPad("c1_1", "c1_1",0,0.5,0.5,1);
  TPad *c1_2= new TPad("c1_2", "c1_2",0.5,0.5,1,1); 
  TPad *c1_3= new TPad("c1_3", "c1_3",0,0,1,0.5);
 
  c1_1->Draw();
  c1_2->Draw();
  c1_3->Draw();

  c1_1->cd();
  // raw adu5A
  int n = adu5A->Draw(TString::Format("%s:realTime",patVar.c_str()),"","goff");
  TGraph *g = new TGraph(n,adu5A->GetV2(),adu5A->GetV1());
  g->SetMarkerColor(2);
  g->SetMarkerStyle(20);
  g->SetTitle("raw ADU5A");
  g->SetTitle(TString::Format("Run %d: ADU5A %s", run, patVar.c_str()));
  g->GetXaxis()->SetTitle("realTime");
  g->GetXaxis()->SetTimeDisplay(1);
  g->GetXaxis()->SetNdivisions(5);
  g->GetYaxis()->SetTitle(patVar.c_str());
  g->Draw("ap");

  c1_2->cd();
  // raw adu5B
  int n1 = adu5B->Draw(TString::Format("%s:realTime",patVar.c_str()),"","goff");
  TGraph *g1 = new TGraph(n1,adu5B->GetV2(),adu5B->GetV1());
  g1->SetMarkerColor(3);
  g1->SetMarkerStyle(20);
  g1->SetTitle("raw ADU5B");
  g1->SetTitle(TString::Format("Run %d: ADU5B %s", run, patVar.c_str()));
  g1->GetXaxis()->SetTitle("realTime");
  g1->GetXaxis()->SetTimeDisplay(1);
  g1->GetXaxis()->SetNdivisions(5);
  g1->GetYaxis()->SetTitle(patVar.c_str());
  g1->Draw("ap ");

  if(adu5A->GetEntries()  - adu5B->GetEntries() > 10 || adu5A->GetEntries()  - adu5B->GetEntries() < -10)
    {
      cout << "adu5A entries = " << adu5A->GetEntries() << endl;
      cout << "adu5B entries = " << adu5B->GetEntries() << endl;
      cerr << "May be an error with amount of entries used" << endl;
    }

  c1_3->cd();

    // Interpolated
  int n2 = adu5I->Draw(TString::Format("%s:realTime",patVar.c_str()),"","goff");
  TGraph *g2 = new TGraph(n2,adu5I->GetV2(),adu5I->GetV1());
  g2->SetMarkerStyle(20);
  g2->SetMarkerSize(0.8);
  g2->SetTitle(TString::Format("Run %d: %s", run, patVar.c_str()));
  g2->GetXaxis()->SetTitle("realTime");
  g2->GetXaxis()->SetTimeDisplay(1);
  g2->GetXaxis()->SetNdivisions(5);
  g2->GetYaxis()->SetTitle(patVar.c_str());
  g2->Draw("ap");
  
  int n3 = adu5A->Draw(TString::Format("%s:realTime",patVar.c_str()),"","goff");
  TGraph *g3 = new TGraph(n3,adu5A->GetV2(),adu5A->GetV1());
  g3->SetMarkerColor(2);
  g3->SetMarkerStyle(20);
  g3->SetMarkerSize(1.8); // make slightly thicker than ADU5B, so both are always in sight (ADU5A overlaps ADU5B)
  Int_t adu5APoints = g3->GetN(); // Important. If the graph has an illegal number of points, do NOT draw it, or the whole canvas will be empty!
  if(adu5APoints > 0)
    {
      g3->Draw("p");
    }
  
  // adu5B with cuts for plotting
  int n4 = adu5B->Draw(TString::Format("%s:realTime",patVar.c_str()),"","goff");
  TGraph *g4 = new TGraph(n4,adu5B->GetV2(),adu5B->GetV1());
  g4->SetMarkerColor(3);
  g4->SetMarkerStyle(20);
  g4->SetMarkerSize(1.3);
  Int_t adu5BPoints = g4->GetN(); // Important. If the graph has an illegal number of points, do NOT draw it, or the whole canvas will be empty!
  if(adu5BPoints > 0)
    {
      g4->Draw("p");
    }

  g2->Draw("p"); // Axis and points are drawn earlier, the points are then redrawn to overlap the individual ADU5A/B points
  
  return c;
  
}


void simpleSanityPlotter()
{

  TCanvas *totCanv = new TCanvas("totCanv","totCanv",3840,2160);
   totCanv->Divide(3,2);
   
  TCanvas *blob1 = plotPatVar("heading");
  totCanv->cd(1);
  blob1->DrawClonePad();
   
  TCanvas *blob2 = plotPatVar("pitch");
  totCanv->cd(2);
  blob2->DrawClonePad();
  
  TCanvas *blob3 = plotPatVar("roll");
  totCanv->cd(3);
  blob3->DrawClonePad();
  
  TCanvas *blob4 = plotPatVar("latitude");
  totCanv->cd(4);
  blob4->DrawClonePad();
  
  TCanvas *blob5 = plotPatVar("longitude");
  totCanv->cd(5);
  blob5->DrawClonePad();
  
  TCanvas *blob6 = plotPatVar("altitude");
  totCanv->cd(6);
  blob6->DrawClonePad();

  totCanv->Draw();
  totCanv->SaveAs(TString::Format("allPatInfoRun%d.png",run)); 

  return;

}
