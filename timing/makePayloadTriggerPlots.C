void makePayloadTriggerPlots(int start_run, int end_run = -1, bool timed = false) 
{
  gStyle->SetOptStat(0); 
  if (end_run <= 0) end_run =start_run; 

  for (int i = start_run; i <= end_run; i++) 
  {

    TFile f(TString::Format("%s/run%d/%s%d.root", getenv("ANITA_ROOT_DATA"), i, timed ? "timedHeadFile" : "headFile", i)); 
    TString title;

    TTree * tree = (TTree*) f.Get("headTree"); 

    double min = tree->GetMinimum("payloadTime"); 
    double max = tree->GetMaximum("payloadTime"); 

    TCanvas * c = new TCanvas("c","c",1920,1000); 
    c->Divide(1,2); 
    c->cd(1); 

    if (timed) 
    {
      title.Form("Run %d goodTimeFlag",i); 
      TH2I axis0("flags",title, 10,min,max,10,-3.1,1.1); 
      axis0.GetXaxis()->SetTitle("Payload Time");
      axis0.GetYaxis()->SetTitle("goodTimeFlag");
      axis0.DrawCopy(); 
      tree->Draw("goodTimeFlag:payloadTime","","lsame"); 
    }
    else
    {
      tree->Draw("triggerTime+ 1e-9 * triggerTimeNs - payloadTime - payloadTimeUs*1e-6:payloadTime","","l"); 
    }


    c->cd(2); 

    title.Form("Run %d Diagnostics",i); 

    TH2I axis("diagnostics",title, 10,min,max,10,-3,3); 
    axis.GetXaxis()->SetTitle("Payload Time"); 
    if (timed) 
    {
      axis.GetYaxis()->SetTitle("B: tr-pay, R: c3poNum/250e6-1, G: ppsNum/100, P: tttFrac" ); 
    }
    else
    {
      axis.GetYaxis()->SetTitle("B: tr-pay, R: c3poNum/250e6-1, G: ppsNum/100"); 
    }
    axis.GetXaxis()->SetTimeDisplay(1); 
    axis.DrawCopy(); 
    tree->Draw("triggerTime+ 1e-9 * triggerTimeNs - payloadTime - payloadTimeUs*1e-6:payloadTime","","lsame"); 
    tree->SetLineColor(2); 
    tree->Draw("rawc3poNum/250e6-1:payloadTime","","lsame"); 
    tree->SetLineColor(3); 
    tree->Draw("ppsNum/1000:payloadTime","","lsame"); 
    if (timed) 
    {
      tree->SetLineColor(6); 
      tree->Draw("tttAlignmentFraction:payloadTime","","lsame"); 
    }
    
    tree->SetLineColor(7); 
      

    c->SaveAs(TString::Format("payloadTrigger/%s_%d.png", timed ?  "timed_run": "run", i)); 
  }
}
