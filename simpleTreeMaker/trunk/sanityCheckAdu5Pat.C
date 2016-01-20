static TCanvas * static_canvas = 0;  
static int current_run = 0; 

TGraph * makePlot(TTree * tree, const char * draw, const char * cut, bool same )
{
  int n = tree->Draw(draw,cut,"goff"); 
  TGraph *g = new TGraph(n, tree->GetV2(), tree->GetV1()); 
  g->SetTitle(draw); 
  g->SetMarkerStyle(tree->GetMarkerStyle()); 
  g->SetMarkerColor(tree->GetMarkerColor()); 
  FFTtools::unwrap(g->GetN(), g->GetX(), 24); 
  g->Draw( same ? "psame" : "ap"); 
  return g; 
}

TCanvas * sanityCheckAdu5Pat(int run = 342, const char * outputdir = 0, int w=2000, int h = 1000, TCanvas * c = 0, const char * format = "png" )
{
  const char * datadir = getenv("ANITA_ROOT_DATA"); 

  TFile old_file(TString::Format("%s/run%d/gpsFile%d.root",datadir,run,run)); 

  TTree * adu5A = (TTree*) old_file.Get("adu5PatTree"); 

  if (adu5A->GetEntries() == 0) 
  {
    printf("no attitude!\n"); 
  }
  adu5A->SetMarkerColor(2); 
  adu5A->SetMarkerStyle(7); 
  TTree * adu5B = (TTree*) old_file.Get("adu5bPatTree"); 
  adu5B->SetMarkerColor(3); 
  adu5B->SetMarkerStyle(7); 
  
  TFile new_file(TString::Format("%s/run%d/gpsEvent%d.root",datadir,run,run)); 

  TTree * adu5I = (TTree*) new_file.Get("adu5PatTree"); 
  if (!adu5I) return 0; 
  adu5I->SetMarkerStyle(7); 


  if (c) 
  {
    c->Clear(); 
    c->SetName(TString::Format("c%d", run));
    c->SetTitle(TString::Format("Run %d", run)); 
  }
  else
  {
   c= new TCanvas(TString::Format("c%d", run), TString::Format("Run %d", run), w, h); 
  }

  last_canvas = c; 
  current_run = run; 
  TCanvas * c2 = new TCanvas("dhead","dhead", 1000,500); 

  c->Divide(3,2); 
  c2->Divide(2,1); 

  c->cd(1); 
  const char * heading_cutstr = "(heading >0)  && (heading < 360) && (abs(pitch) < 3) && (abs(roll) < 3 ) &&  !(heading == 0 && pitch == 0 && roll == 0)"; 
  const char * other_cutstr = "(heading >0)  && (heading < 360) && (abs(pitch) < 3) && (abs(roll) < 3 ) &&  !(pitch == 0 && roll == 0)"; 

  TGraph * gI = makePlot(adu5I, "heading:timeOfDay/3600000","", false) ; 
  TGraph * gA = makePlot(adu5A, "heading:timeOfDay/3600000","attFlag==0",true); 
  TGraph * gB = makePlot(adu5B, "heading:timeOfDay/3600000","attFlag==0",true); 
  adu5A->SetMarkerStyle(1); 
  adu5B->SetMarkerStyle(1); 
  makePlot(adu5A, "heading:timeOfDay/3600000",heading_cutstr,true); 
  makePlot(adu5B, "heading:timeOfDay/3600000",heading_cutstr,true); 
  makePlot(adu5I, "heading:timeOfDay/3600000","",true); 

  adu5A->SetMarkerStyle(7); 
  adu5B->SetMarkerStyle(7); 

  TGraph * dA = new TGraph(gA->GetN(), gA->GetX(), gA->GetY()); 
  TGraph * dB = new TGraph(gB->GetN(), gB->GetX(), gB->GetY()); 

  double x0 = gI->GetX()[0]; 
  double x1 = gI->GetX()[gI->GetN()-1]; 
  for (int i = 0; i < dA->GetN(); i++) 
  {
    double Ax = dA->GetX()[i]; 
    if (Ax <= x1) 
    {
      dA->GetY()[i] -= gI->Eval(Ax); 
      FFTtools::wrap(dA->GetY()[i]); 
    }
    else
    {

      dA->Set(i); 
      break; 

    }

  }
  for (int i = 0; i < dB->GetN(); i++) 
  {
    double Bx = dB->GetX()[i]; 
    if (Bx <= x1) 
    {
      dB->GetY()[i] -= gI->Eval(Bx); 
      FFTtools::wrap(dB->GetY()[i]); 
    }
    else 
    {
      dB->Set(i); 
      break; 
    }
    
  }
  c2->cd(1); 
  dA->SetLineColor(2); 
  dA->SetMarkerColor(2); 
  dA->Draw("alp"); 
  c2->cd(2); 
  dB->SetLineColor(3); 
  dB->SetMarkerColor(3); 
  dB->Draw("alp"); 
  


  c->cd(2); 
  makePlot(adu5I,"pitch:timeOfDay/3600000","",false); 
  makePlot(adu5A,"pitch:timeOfDay/3600000",other_cutstr,true); 
  makePlot(adu5B,"pitch:timeOfDay/3600000",other_cutstr,true); 
  makePlot(adu5I,"pitch:timeOfDay/3600000","",true); 

  c->cd(3); 
  makePlot(adu5I,"roll:timeOfDay/3600000","",false); 
  makePlot(adu5A,"roll:timeOfDay/3600000",other_cutstr,true); 
  makePlot(adu5B,"roll:timeOfDay/3600000",other_cutstr,true); 
  makePlot(adu5I,"roll:timeOfDay/3600000","",true); 

  c->cd(4); 
  makePlot(adu5I,"latitude:timeOfDay/3600000","",false); 
  makePlot(adu5A,"latitude:timeOfDay/3600000","",true); 
  makePlot(adu5B,"latitude:timeOfDay/3600000","",true); 
  makePlot(adu5I,"latitude:timeOfDay/3600000","",true); 
  
  c->cd(5); 
  makePlot(adu5I,"longitude:timeOfDay/3600000","",false); 
  makePlot(adu5A,"longitude:timeOfDay/3600000","",true); 
  makePlot(adu5B,"longitude:timeOfDay/3600000","",true); 
  makePlot(adu5I,"longitude:timeOfDay/3600000","",true); 

  c->cd(6); 
  makePlot(adu5I,"altitude:timeOfDay/3600000","",false); 
  makePlot(adu5A,"altitude:timeOfDay/3600000","",true); 
  makePlot(adu5B,"altitude:timeOfDay/3600000","",true); 
  makePlot(adu5I,"altitude:timeOfDay/3600000","",true); 


  c->Draw(); 


  if (outputdir)
  {
    c->SaveAs(TString::Format("%s/%d.%s",outputdir,run,format)); 
  }


  return c; 

}

void make(int run, const char * outputdir = "sanity")
{
  static_canvas = sanityCheckAdu5Pat(run, outputdir,1920, 1000, static_canvas); 
}
void next(const char * outputdir = "sanity")
{
  make (current_run+1); 
}

void previous()
{
  make(current_run-1); 
}

void doAll(int first, int last, int w = 2000, int h = 1000,  const char * outputdir = "sanity", const char * format = "png") 
{

  for (int i = first; i <=last; i++) 
  {
    delete sanityCheckAdu5Pat(i, outputdir, w, h); 
  }
}
