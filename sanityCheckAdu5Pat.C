static TCanvas * static_canvas = 0;  
static int current_run = 0; 

void makePlot(TTree * tree, const char * draw, const char * cut, bool same )
{
  int n = tree->Draw(draw,cut,"goff"); 
  TGraph *g = new TGraph(n, tree->GetV2(), tree->GetV1()); 
  g->SetTitle(draw); 
  g->SetMarkerStyle(tree->GetMarkerStyle()); 
  g->SetMarkerColor(tree->GetMarkerColor()); 
  FFTtools::unwrap(g->GetN(), g->GetX(), 24); 
  g->Draw( same ? "psame" : "ap"); 
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

  c->Divide(3,2); 

  c->cd(1); 
  const char * heading_cutstr = "(heading >0)  && (heading < 360) && (abs(pitch) < 3) && (abs(roll) < 3 ) &&  !(heading == 0 && pitch == 0 && roll == 0)"; 
  const char * other_cutstr = "(heading >0)  && (heading < 360) && (abs(pitch) < 3) && (abs(roll) < 3 ) &&  !(pitch == 0 && roll == 0)"; 

  makePlot(adu5I, "heading:timeOfDay/3600000","", false) ; 
  makePlot(adu5A, "heading:timeOfDay/3600000","attFlag==0",true); 
  makePlot(adu5B, "heading:timeOfDay/3600000","attFlag==0",true); 
  adu5A->SetMarkerStyle(1); 
  adu5B->SetMarkerStyle(1); 
  makePlot(adu5A, "heading:timeOfDay/3600000",heading_cutstr,true); 
  makePlot(adu5B, "heading:timeOfDay/3600000",heading_cutstr,true); 
  makePlot(adu5I, "heading:timeOfDay/3600000","",true); 

  adu5A->SetMarkerStyle(7); 
  adu5B->SetMarkerStyle(7); 

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
