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

/** makeAdu5PatFilesBothADUs.cxx
 *
 *   Cosmin Deaconu <cozzyd@kicp.uchicago.edu>  
 *
 *
 *   This program takes an input gpsTree containing position and attitude
 *   information from both adu5A and adu5B and combines them into a single,
 *   authorative tree.  All of this is meant for ANITA-III only! 
 *
 *   This is less trivial than it sounds because there are times when either
 *   adu5A or adu5B (and sometimes both)  do not report accurate attitude
 *   information. 
 *
 *   The following strategy is used, guided by the compile-time constants just below this comment. 
 *  
 *   0) y-values are unwrapped, with very short contiguous segments and segments that are low-scoring (see temperDiscontuities and scoreSegment functions)
 *
 *   1) Gaps in both trees are interpolated to an even 1 s grid using a sparse inversion of the Shannon-Whitaker Interpolation formula (this is sometimes called Yen's interpolator) 
 *      ith Tikhonov Regularization. 
 *
 *   2) Weights are assigned to each output based on a priori weights, mrms, brms and how far from a measured value the point lies. 
 *    
 *   3) A weighted average of the two is then taken for the attitude. 
 *
 *   4) For the position, the coordinates are converted to cartesian coordinates, averaged, then back to lat, long, alt
 *
 *   
 *   Judging from Ryan's comments and the code of GPSd.c, it seems like timeofday is the best time to use, as it appears to be something recorded from the GPS unit,
 *   as opposed to the other times which are generated upon read. 
 *
 *   The mrms and brms are just set to the heading error
 *  
 *
 *   There are some arrays filled with exceptions for misbehaving runs. Also, the timeofday -> time matching is special cased for run 434, which seems to have had a problem with its computer clock at the beginning. 
 *
 *
 */


/* A priori attitude weights due to different baselines for A and B . These will automatically be normalized later.*/
const double adu5A_attitude_a_priori_weight = 1.4; 
const double adu5B_attitude_a_priori_weight = 1.; 




/* Relative contributions of the mrms and brms to the weights. These will automatically be normalized */ 
const double mrms_weight = 1; 
const double brms_weight = 1; 


/* Maximum distance considered in interpolation, in seconds */ 
const int max_time = 120; 


/* Tikhonov parameters */ 
const double regularization_parameter = 0.002; 
const int regularization_order_heading = 2;  // use different regularization for heading and other stuff
const int regularization_order= 1; 

/* Filter used */ 
//const FFTtools::DigitalFilter * filter = 0; //new FFTtools::SavitzkyGolayFilter(3,3,3); 
const FFTtools::DigitalFilter * filter = new FFTtools::SavitzkyGolayFilter(4,20,20); 



/** Here's where it starts getting even sketchier. To remove discontinuous segments, we use a pretty shitty algorithm that kinda sorta works, but 
 * it was too hard to find good parameters for everything, so they can be overriden for indiviudal runs. Woohoo! */ 
 
const double default_min_score = 1;  // minimum score for the algorithm... just read the code
const bool enableStrictAttFlag = true;

//const int strict_attflag_exceptions[][2] = {{340,1}, {406,1}};    //add exceptions here if there need be any
//const double min_score_exceptions[][2] = { {142,5}, {293,4}, {431,5}};    //add exceptions here if there need be any

const double heading_slope_thresh = 1; 
//const double heading_slope_exceptions[][2] = { {142,0.2},{270,0.8}, {340,1.5},{431,0.5}};    //add exceptions here if there need be any

const int strict_attflag_exceptions[][2] = {{0,0}, {0,0}};
const double min_score_exceptions[][2] = {{0,0},{0,0}};
const double heading_slope_exceptions[][2] = {{0,0},{0,0}};

// Get rid of the ANITA-3 specific corrections

static bool debugFlag = false; // this is set from the command line by the second argument. If true, some stuff will be written to disk. 

static AnitaGeomTool * geom = AnitaGeomTool::Instance();  // eventually I'll make the relevant methods static

static double time_of_day_start(double t)
{
   UInt_t secs = (UInt_t) t; 
   UInt_t nsecs = (t - secs)*1e9; 

   TTimeStamp start_time(secs, nsecs); 
   UInt_t d,m,y; 
   start_time.GetDate(true, 0, &y, &m, &d); 
   TTimeStamp day_start(y, m, d, 0,0,0); 
   return day_start.AsDouble(); 
}

static int removeNaNs(TGraphErrors * g)
{
  int nremoved = 0; 

  
  for (int i = 0; i < g->GetN(); i++ ) 
  {
    if (std::isnan(g->GetY()[i]) || std::isnan(g->GetEY()[i])) 
    {
      g->SetPoint(i, g->GetX()[i], 0); 
      g->SetPointError(i, 0, TMath::Infinity()); 
      nremoved++; 
    }
  }

  return nremoved; 
}

struct segment
{
  double startx; 
  double starty; 
  double endx; 
  double endy; 
  double max; 
  double min; 
  int start_i; 
  int end_i; 
};
/* // Just force it to follow adu5B instead by making use of an empty tree
void removeTree()
{
  std::string file_name="gpsFile293.root";
  TFile *file=new TFile((file_name).c_str(),"update");
  std::string object_to_remove="adu5PatTree;1";
  gDirectory->Delete(object_to_remove.c_str());
  file->Close();
}
*/
static double scoreSegment(segment * seg, segment * last, segment * next, double slope_thresh = 1, double slope_penalty_factor = 5)
{
  int n = seg->end_i - seg->start_i +1; 


  double score = n; 


  //  penalize based on difference of slope from previous and next 
   double penalty = 0; 
   if (last) 
   {
      penalty= ( (seg->starty - last->endy) / (seg->startx - last->endx) ); 
   }
  printf("%d %f ", seg->start_i, penalty); 

  // make it even worse if the the next reverts to the previous
  if (next && fabs(penalty) > slope_thresh) 
  {
    double dpenalty =  (next->starty - seg->endy) / (next->startx - seg->endx) ; 
    printf(" (%f) ", dpenalty); 
    if ( dpenalty * penalty < 0)  // if opposite signs... 
    {
      penalty -= dpenalty; 
    }
  }


  printf("%f \n", penalty); 
  if (fabs(penalty) > slope_thresh) 
  {
    score -= slope_penalty_factor*penalty*penalty; 
  }


  /* not sure these are good ideas 
  // penalize if max is < min of both  or min > max of bot
  double the_min = (next && last) ? TMath::Min(last->min, next->min) 
                   : next ? next->min : last->min; 

  if (seg->max < the_min)
  {
    score -=  the_min - seg->max; 
  }


  double the_max = (next && last) ? TMath::Max(last->max, next->max) 
                   : next ? next->max : last->max; 

  if (seg->min > the_max)
  {
    score -=  seg->min - the_max  ; 
  }
  */

  return score; 

}

// would like to get rid of really bad discontinuities, so we get rid of the ones with the worst scores 
static int temperDiscontinuities(TGraph *g, double slope_thresh , double expected_dx , double min_score, double max_dy = 1) 
{
  // make a copy and unwrap it
  TGraph * gcopy = new TGraph(g->GetN(), g->GetX(), g->GetY()); 
  FFTtools::unwrap(gcopy->GetN(), gcopy->GetY()); 

  //we'll store all the segments here 
  std::vector<segment>  segments; 

  double xlast = gcopy->GetX()[0]; 
  double ylast =  gcopy->GetY()[0]; 

  segment seg; 
  seg.startx = xlast; 
  seg.start_i = 0; 
  seg.min = ylast;
  seg.max = ylast; 
  seg.starty= ylast; 

  for (int i = 1; i < gcopy->GetN(); i++) 
  {
    double x = gcopy->GetX()[i]; 
    double y = gcopy->GetY()[i]; 

    if (x - xlast > expected_dx || fabs(y - ylast) > max_dy) 
    {
      //finish this segment
      seg.endx = xlast; 
      seg.endy = ylast; 
      seg.end_i = i-1; 

      //push it 
      segments.push_back(seg); 

      //start a new one
      seg.startx = x; 
      seg.starty = y; 
      seg.start_i = i; 
      seg.min = y;
      seg.max = y; 
    }

    if ( y > seg.max) seg.max = y; 
    if ( y < seg.min) seg.min = y; 
    xlast = x; 
    ylast = y; 
  }

  int nremoved = 0; 

  while (true) 
  {
    int removed_this_iteration = 0; 

    int adj = 0; 
    std::vector<double> scores(segments.size()); 
    double worst_score = DBL_MAX; 
    for (size_t i = 0; i < segments.size(); i++) 
    {
      segment * seg = &segments[i]; 
      scores[i] = scoreSegment(seg, i == 0 ? 0 : &segments[i-1],  i == segments.size()-1 ? 0 : &segments[i+1], slope_thresh); 
      if (scores[i] < worst_score) worst_score = scores[i]; 

      printf("---\n"); 
    }


    int segment_to_remove = -1; 
    for (size_t i = 0; i < segments.size(); i++) 
    {
      segment * seg = &segments[i]; 
      seg->start_i -= adj; 
      seg->end_i -= adj; 
      printf("%f: (%d, %d), (%f %f) (%f %f) \n",scores[i], seg->start_i, seg->end_i, seg->startx, seg->endx, seg->starty, seg->endy); 
      if (scores[i] < min_score  && scores[i] == worst_score && segment_to_remove < 0)
      {
        segment_to_remove = i; 
        printf("Removing segment!\n"); 
        adj += seg->end_i - seg->start_i + 1;  // points afterwards must be shifted over
      }

    }

    if (segment_to_remove < 0) break; 

    
    segment * seg = &segments[segment_to_remove]; 

    for (int i = 0; i <= seg->end_i - seg->start_i; i++)
    {
      g->RemovePoint(seg->start_i); 
      nremoved++; 
      removed_this_iteration++; 
    }
    segments.erase(segments.begin() + segment_to_remove); 
    printf("\nthis iteration removed %d points\n", removed_this_iteration); 
  }


  return nremoved; 
}

// use scores instead/
/*
static int removeLonePoints(TGraph* g, double expected_dx = 1) 
{
  double xlast = g->GetX()[0]; 
  double xnext = g->GetX()[1]; 
  std::vector<int> remove; 
  for (int i = 1; i < g->GetN()-1; i++) 
  {
    double x = xnext; 
    xnext  = g->GetX()[i+1]; 

    if (x - xlast > expected_dx && xnext - x > expected_dx)
    {
      remove.push_back(i); 
    }

    xlast = x; 
  }
  for (size_t i = 0; i < remove.size(); i++) 
  {
    g->RemovePoint(remove[i] - int(i));  
  }

  return int(remove.size()); 
} 
*/

//this didn't work well... scores worked better I think 
/*
static void penalizeLargeJumps(TGraphErrors *g, double jump_threshold_slope = 5,  double penalty_factor = 10, double penalty_decay = 20) 
{
  std::vector<double> penalty_x; 
  std::vector<double> penalty_val; 
  for (int i = 1; i < g->GetN(); i++) 
  {
    double slope = fabs( g->GetY()[i] - g->GetY()[i-1] / (g->GetX()[i] - g->GetX()[i-1])); 
    if (slope > jump_threshold_slope)
    {
      penalty_x.push_back(g->GetX()[i]); 
      penalty_val.push_back(slope * penalty_factor); 
    }

    for (size_t j = 0; j < penalty_x.size(); j++) 
    {

      g->GetEY()[i] += penalty_val[j] * exp(-(g->GetX()[i] - penalty_x[j]) / penalty_decay); 
    }
  }
}
*/


/* Make a graph from a chain
 *
 * In retrospect, it's not so intelligent to do each one individually because if a point is removed from one graph, it should really be removed from all. 
 * This may change in the future if I cared enough... 
 */ 
static TGraphErrors * makeInterpolatedAttitudeGraph(const char *var, 
                                                    const char * cut, 
                                                    TChain * c,
                                                    double a_priori_weight, 
                                                    double offset, 
                                                    double slope_thresh, 
                                                    double min_score) 

{

  /* Calculate weight normalization */ 
  const double weight_norml = 1./(mrms_weight + brms_weight); 

  /* weight string */ 
  TString wstr = TString::Format("%f*sqrt(%f*mrms*mrms + %f*brms*brms)*(1+attFlag)^3", a_priori_weight,mrms_weight/weight_norml , brms_weight/weight_norml); 

  /*Get var vs. timeof day and weight, subject to cut */ 
  int n = c->Draw(TString::Format("%s:timeOfDay/1000:%s",var,wstr.Data()), cut, "goff"); 

  if (n == 0)   //terribad
  {

    TGraphErrors * g = new TGraphErrors(2); 
    g->SetPoint(0, 0,0); 
    g->SetPoint(1, 1,0); 
    g->SetPointError(0, 0,TMath::Infinity()); 
    g->SetPointError(1, 1,TMath::Infinity()); 
    return g; 

  }


  //unwrap time of day and add offset 
  FFTtools::unwrap(n, c->GetV2(), 24*60*60); 
  for (int i = 0; i < n; i++) c->GetV2()[i] += offset; 

  /* We would like to pad to avoid edge effects from normalization */ 
  TGraphErrors g(n+2*max_time); 
  for (int i = 0; i < max_time; i++) 
  {
    g.SetPoint(i, c->GetV2()[0] - max_time + i, c->GetV1()[0]); 
    g.SetPointError(i, 0, c->GetV3()[0] * (1 + (max_time -i) * (max_time -i))); 
  }
  memcpy(g.GetY()+max_time, c->GetV1(), n * sizeof(double)); 
  memcpy(g.GetX()+max_time, c->GetV2(), n * sizeof(double)); 
  memcpy(g.GetEY()+max_time, c->GetV3(), n * sizeof(double)); 

  for (int i = max_time+n; i < n+2*max_time; i++) 
  {
    g.SetPoint(i, c->GetV2()[n-1] + i-max_time-n+1, c->GetV1()[n-1]); 
    g.SetPointError(i, 0, c->GetV3()[n-1] * (1 + (i - max_time -n) * (i - max_time - n))); 
  }

  // remove bad segments

  bool isHeading = !strcmp(var,"heading");  //yeah, yeah, yeah... 


  printf("%d points removed\n", temperDiscontinuities(&g, slope_thresh, 1, min_score ));

  
  //unwrap the values  
  if (isHeading)
  {
    FFTtools::unwrap(g.GetN(), g.GetY(), 360); 
  }


//  penalizeLargeJumps(&g);

  TGraphErrors * gj = FFTtools::getInterpolatedGraphSparseInvert(&g, 1, 0,max_time, 0, 0, regularization_parameter, isHeading ? regularization_order_heading : regularization_order); 

  //printf("%d lone points removed \n",removeLonePoints(gj, 1));
  
  
  printf("%d NaN's removed\n", removeNaNs(gj)); 

  if (filter) 
    filter->filterGraph(gj,false); 


  if (debugFlag)
  {
    gj->Write(TString::Format("%s_%s",c->GetName(), var)); 

  }

  return gj; 
}

/* Ignore this test, I can do it a simpler way
// Accounting for GPS outages, there is NO ADU5A information available, so we must rely on ADU5B
static void accountForGpsOutage(TChain * c2,  TGraph * glat, TGraph * glon,  TGraph * galt)
{
  int n2 = c2->GetEntries(); 
  Adu5Pat *pat2 = new Adu5Pat; 
  c2->SetBranchAddress("pat",&pat2); 

  int i2 = 0; 

  std::vector<double> time; 
  time.reserve(n2); 
  std::vector<double> lat; 
  lat.reserve(n2); 
  std::vector<double> lon; 
  lon.reserve(n2); 
  std::vector<double> alt; 
  alt.reserve(n2);
  
  UInt_t tod2 = 0; 
  bool wrap2 = false; 

  while (i2 < n2) 
  {
    c2->GetEntry(i2); 
    if (pat2->timeOfDay < tod2 && !wrap2) wrap2 = true; 

    tod2 = pat2->timeOfDay; 

    if (wrap2) tod2 += 24 * 60 * 60 * 1000; 

    // POSITION DATA: if lat is illogical (outside Antarctic circle) or altitude is illogical (i.e. when it goes to 0 on gps resets), don't use
    
    if(pat2->latitude > -66.5 ||pat2->altitude == 0)
      {
	i2++;
      }

    else
    {
      lat.push_back(pat2->latitude); 
      lon.push_back(pat2->longitude); 
      alt.push_back(pat2->altitude); 
      time.push_back(tod2/1000.);
      i2++;
    }
    
  }
  
  //assert (lon.size() < unsigned(n1 + n2));  // sanity check to make sure we don't loop infinitely and use all the memory on the cluster 




  // copy to graphs 
  glat->Set(time.size()); 
  memcpy(glat->GetX(), &time[0], sizeof(double) * time.size()); 
  memcpy(glat->GetY(), &lat[0], sizeof(double) * time.size());

  glon->Set(time.size()); 
  memcpy(glon->GetX(), &time[0], sizeof(double) * time.size()); 
  memcpy(glon->GetY(), &lon[0], sizeof(double) * time.size()); 

  galt->Set(time.size()); 
  memcpy(galt->GetX(), &time[0], sizeof(double) * time.size()); 
  memcpy(galt->GetY(), &alt[0], sizeof(double) * time.size()); 


  FFTtools::unwrap(glon->GetN(), glon->GetY(), 360); 
//  glon->Write("glon"); 
  if (filter && n2)
  {
    filter->filterGraph(glat); 
    filter->filterGraph(glon); 
    filter->filterGraph(galt); 
  }

  std::cout << "Accounted for one run involved in ADU5A outage" << std::endl;

}
*/

static void makePositionGraphsPostOutage(TChain *c1, TChain * c2,  TGraph * glat, TGraph * glon,  TGraph * galt,  double offset)
{

  int n1 = c1->GetEntries(); 
  int n2 = c2->GetEntries(); 
  Adu5Pat *pat1 = new Adu5Pat; 
  Adu5Pat *pat2 = new Adu5Pat; 
  c1->SetBranchAddress("pat",&pat1); 
  c2->SetBranchAddress("pat",&pat2); 


  int i1 = 0; 
  int i2 = 0; 

  std::vector<double> time; 
  time.reserve(n1); 
  std::vector<double> lat; 
  lat.reserve(n1); 
  std::vector<double> lon; 
  lon.reserve(n1); 
  std::vector<double> alt; 
  alt.reserve(n1); 

  UInt_t tod1 = 0; 
  UInt_t tod2 = 0; 
  bool wrap1 = false; 
  bool wrap2 = false; 

  while (i1 < n1 || i2 < n2) 
  {
    c1->GetEntry(i1); 
    c2->GetEntry(i2); 

//      printf("%d %d %u %u\n",i1,i2,pat1->timeOfDay,pat2->timeOfDay);   // debugging shit 
//    c1->Show(i1); 
//    c2->Show(i2); 
//
    //need to handle wrapping properly here in case not synced and don't reset to 0 at same time. we assume you can only wrap once (i.e. runs shorter than one day) 
    if (pat1->timeOfDay < tod1 && !wrap1) wrap1 = true; 
    if (pat2->timeOfDay < tod2 && !wrap2) wrap2 = true; 

    tod1 = pat1->timeOfDay; 
    tod2 = pat2->timeOfDay; 

    if (wrap1) tod1 += 24 * 60 * 60 * 1000; 
    if (wrap2) tod2 += 24 * 60 * 60 * 1000; 

    // POSITION DATA: if lat is illogical (outside Antarctic circle) or altitude is illogical (i.e. when it goes to 0 on gps resets), don't use
    
    if(pat2->latitude > -66.5 ||pat2->altitude == 0 || pat1->latitude == 0 || pat1->altitude == 0)
      {
	i1++;
	i2++;
      }

    else
    {
	//account for cases when times are not synced
	if (tod1 < tod2 && i1 < n1) 
	  {
	    lat.push_back(pat1->latitude); 
	    lon.push_back(pat1->longitude); 
	    alt.push_back(pat1->altitude); 
	    time.push_back(tod1/1000. + offset); 
	    i1++; 
	  }
	else if (tod2 < tod1 && i2 < n2) 
	  {
	    lat.push_back(pat2->latitude); 
	    lon.push_back(pat2->longitude); 
	    alt.push_back(pat2->altitude); 
	    time.push_back(tod2/1000. + offset); 
	    i2++; 
	  }
	else  // if they are synced, take the average of the two positions 
	  {
	    time.push_back(tod1/1000. + offset); 

	    double p1[3], p2[3]; 
	    geom->getCartesianCoords(pat1->latitude,pat1->longitude,pat1->altitude, p1); 
	    geom->getCartesianCoords(pat2->latitude,pat2->longitude,pat2->altitude, p2);

	    for (int i = 0; i < 3; i++) 
	      {
		p1[i]+=p2[i]; 
		p1[i]/=2; 
	      }

	    double dlat, dlon, dalt; 
	    geom->getLatLonAltFromCartesian(p1, dlat,dlon,dalt); 
	    //      printf("%f %f %f\n", dlat,dlon,dalt); 
	    lat.push_back(dlat); 
	    lon.push_back(dlon); 
	    alt.push_back(dalt); 
	    i1++; 
	    i2++; 
	  }

	}
  
	//assert (lon.size() < unsigned(n1 + n2));  // sanity check to make sure we don't loop infinitely and use all the memory on the cluster 
  }



  // copy to graphs 
  glat->Set(time.size()); 
  memcpy(glat->GetX(), &time[0], sizeof(double) * time.size()); 
  memcpy(glat->GetY(), &lat[0], sizeof(double) * time.size());

  glon->Set(time.size()); 
  memcpy(glon->GetX(), &time[0], sizeof(double) * time.size()); 
  memcpy(glon->GetY(), &lon[0], sizeof(double) * time.size()); 

  galt->Set(time.size()); 
  memcpy(galt->GetX(), &time[0], sizeof(double) * time.size()); 
  memcpy(galt->GetY(), &alt[0], sizeof(double) * time.size()); 


  FFTtools::unwrap(glon->GetN(), glon->GetY(), 360); 
//  glon->Write("glon"); 
  if (filter && (n1 || n2))
  {
    filter->filterGraph(glat); 
    filter->filterGraph(glon); 
    filter->filterGraph(galt); 
  }

}

static void makePositionGraphs(TChain *c1, TChain * c2,  TGraph * glat, TGraph * glon,  TGraph * galt,  double offset)
{

  int n1 = c1->GetEntries(); 
  int n2 = c2->GetEntries(); 
  Adu5Pat *pat1 = new Adu5Pat; 
  Adu5Pat *pat2 = new Adu5Pat; 
  c1->SetBranchAddress("pat",&pat1); 
  c2->SetBranchAddress("pat",&pat2); 


  int i1 = 0; 
  int i2 = 0; 

  std::vector<double> time; 
  time.reserve(n1); 
  std::vector<double> lat; 
  lat.reserve(n1); 
  std::vector<double> lon; 
  lon.reserve(n1); 
  std::vector<double> alt; 
  alt.reserve(n1); 

  UInt_t tod1 = 0; 
  UInt_t tod2 = 0; 
  bool wrap1 = false; 
  bool wrap2 = false; 

  while (i1 < n1 || i2 < n2) 
  {
    c1->GetEntry(i1); 
    c2->GetEntry(i2); 

//      printf("%d %d %u %u\n",i1,i2,pat1->timeOfDay,pat2->timeOfDay);   // debugging shit 
//    c1->Show(i1); 
//    c2->Show(i2); 
//
    //need to handle wrapping properly here in case not synced and don't reset to 0 at same time. we assume you can only wrap once (i.e. runs shorter than one day) 
    if (pat1->timeOfDay < tod1 && !wrap1) wrap1 = true; 
    if (pat2->timeOfDay < tod2 && !wrap2) wrap2 = true; 

    tod1 = pat1->timeOfDay; 
    tod2 = pat2->timeOfDay; 

    if (wrap1) tod1 += 24 * 60 * 60 * 1000; 
    if (wrap2) tod2 += 24 * 60 * 60 * 1000; 

    // POSITION DATA: if lat is illogical (outside Antarctic circle) or altitude is illogical (i.e. when it goes to 0 on gps resets), don't use
    
    if(pat2->latitude > -66.5 ||pat2->altitude == 0)
      {
	i1++;
	i2++;
      }

    else
    {
	//account for cases when times are not synced
	if (tod1 < tod2 && i1 < n1) 
	  {
	    lat.push_back(pat1->latitude); 
	    lon.push_back(pat1->longitude); 
	    alt.push_back(pat1->altitude); 
	    time.push_back(tod1/1000. + offset); 
	    i1++; 
	  }
	else if (tod2 < tod1 && i2 < n2) 
	  {
	    lat.push_back(pat2->latitude); 
	    lon.push_back(pat2->longitude); 
	    alt.push_back(pat2->altitude); 
	    time.push_back(tod2/1000. + offset); 
	    i2++; 
	  }
	else  // if they are synced, take the average of the two positions 
	  {
	    time.push_back(tod1/1000. + offset); 

	    double p1[3], p2[3]; 
	    geom->getCartesianCoords(pat1->latitude,pat1->longitude,pat1->altitude, p1); 
	    geom->getCartesianCoords(pat2->latitude,pat2->longitude,pat2->altitude, p2);

	    for (int i = 0; i < 3; i++) 
	      {
		p1[i]+=p2[i]; 
		p1[i]/=2; 
	      }

	    double dlat, dlon, dalt; 
	    geom->getLatLonAltFromCartesian(p1, dlat,dlon,dalt); 
	    //      printf("%f %f %f\n", dlat,dlon,dalt); 
	    lat.push_back(dlat); 
	    lon.push_back(dlon); 
	    alt.push_back(dalt); 
	    i1++; 
	    i2++; 
	  }

	}
  
	//assert (lon.size() < unsigned(n1 + n2));  // sanity check to make sure we don't loop infinitely and use all the memory on the cluster 
  }



  // copy to graphs 
  glat->Set(time.size()); 
  memcpy(glat->GetX(), &time[0], sizeof(double) * time.size()); 
  memcpy(glat->GetY(), &lat[0], sizeof(double) * time.size());

  glon->Set(time.size()); 
  memcpy(glon->GetX(), &time[0], sizeof(double) * time.size()); 
  memcpy(glon->GetY(), &lon[0], sizeof(double) * time.size()); 

  galt->Set(time.size()); 
  memcpy(galt->GetX(), &time[0], sizeof(double) * time.size()); 
  memcpy(galt->GetY(), &alt[0], sizeof(double) * time.size()); 


  FFTtools::unwrap(glon->GetN(), glon->GetY(), 360); 
//  glon->Write("glon"); 
  if (filter && (n1 || n2))
  {
    filter->filterGraph(glat); 
    filter->filterGraph(glon); 
    filter->filterGraph(galt); 
  }

}


static double best(TGraphErrors* A, TGraphErrors *B, double time, double * err, double wrap_center = 180) 
{

  double Aerr,Berr; 
  double Aval  = FFTtools::linearInterpolateValueAndError(time, A, &Aerr); 
  double Bval  = FFTtools::linearInterpolateValueAndError(time, B, &Berr); 
  double delta = Aval - Bval; 
  delta = FFTtools::wrap(delta, 360,0); 
  Bval  = Aval - delta; 


  double wA = 1./(Aerr*Aerr); 
  double wB = 1./(Berr*Berr); 

  //std::cout << "wA = " << wA << ",wB = " << wB <<  std::endl;
      
  if (wA ==0 && wB == 0)  
  {
    // this is hopeless, let's play russian roulette!  
    //free((void*)0xdeadbeef); // These lines shoot me in the head for a seg fault
    //*err = TMath::Infinity(); // So does this, i.e. for run3 of the ANITA-4 data set
    //*err =std::numeric_limits<double>::infinity(); // doesn't work either
    *err = TMath::Infinity();
    return 0; 
  }
  if (wA == 0) 
  {
    if (err) 
      *err = Berr; 
    return FFTtools::wrap(Bval, 360, wrap_center); 
  }
  if (wB == 0) 
  {
    if (err) 
      *err = Aerr; 
    return FFTtools::wrap(Aval, 360, wrap_center); 
  }

  if (err) *err = sqrt(Aerr*Aerr + Berr*Berr); 
//  printf(" %f %f %f %f\n",  Aval, Bval, wA, wB); 
  return FFTtools::wrap((wA * Aval + wB*Bval) / (wA + wB),360, wrap_center); 
}



int main (int nargs, char const ** args) 
{
  //set up input 
  //char * datadir = getenv("ANITA_ROOT_GPS_DATA");

  // Put your working ANITA data directory below
  char const * datadir = "/home/berg/Dropbox/LinuxSync/PhD/ANITA/2017Work/gpsEvent";
  
  TChain headers("headTree");

   RawAnitaHeader * hdr = new RawAnitaHeader; 
   headers.SetBranchAddress("header",&hdr); 

   TChain adu5A("adu5PatTree"); 
   TChain adu5B("adu5bPatTree"); 

   int run = atoi(args[1]);
   std::cout << "run is " << run << std::endl;

   headers.Add(TString::Format("%s/run%d/timedHeadFile%d.root", datadir, run, run)); 
   
   if(run == 293)
     {
       adu5A.Add(TString::Format("%s/run294/gpsFile294.root", datadir)); // add an empty tree. This will make the interpolation follow ADU5B and ignore this set. I could use any empty tree, but this is convenient...
       adu5B.Add(TString::Format("%s/run%d/gpsFile%d.root", datadir, run, run)); 
     }
   
   else
     {
   
       adu5A.Add(TString::Format("%s/run%d/gpsFile%d.root", datadir, run, run)); 
       adu5B.Add(TString::Format("%s/run%d/gpsFile%d.root", datadir, run, run)); 
     }
   
   if (nargs > 2 && atoi(args[2])) 
   {
     debugFlag = true; 
   }

   //setup output 
   TFile out(TString::Format("%s/run%d/timedGpsEvent%d.root",datadir,run,run),"RECREATE"); 
   TTree * tree = new TTree("adu5PatTree","Timed tree of Interpolated ADU5 Positions and Attitude"); 

   Adu5Pat * pat = new Adu5Pat(); 
   tree->Branch("pat",&pat); 
   tree->Branch("eventNumber",&hdr->eventNumber); 
   tree->Branch("run",&run); 

   //figure out timeoffset for timeofday
   TLeaf * timeleaf = adu5B.GetLeaf("realTime");
   // changing this from adu5A -> adu5B to aid in accounting for adu5A outages

   //timeleaf->GetBranch()->GetEntry(run == 434 ? 960 : 0); /////Special case run 434 which has a problem 
   timeleaf->GetBranch()->GetEntry(0); /////nevermind, I had an old gpsFile 


   double time0 = timeleaf->GetValue(); 

   double offset = time_of_day_start(time0); 


   // generate the attitude graphs
   // Don't cut on attflag... sometimes the heading is ok but the rest isn't
   
   bool cut_attflag = enableStrictAttFlag; 
   for (size_t i = 0; i < sizeof(strict_attflag_exceptions) / sizeof(strict_attflag_exceptions[0]); i++) 
   {
      if (strict_attflag_exceptions[i][0] == run)
      {
        cut_attflag = strict_attflag_exceptions[i][1]; 
        printf("Overwrote attflag\n"); 
        break; 
      }
   }



   // sue me
   const char * heading_cutstr =  cut_attflag ? "attFlag == 0  && (heading >0)  && (heading < 360) && (abs(pitch) < 3) && (abs(roll) < 3 ) &&  !(heading == 0 && pitch == 0 && roll == 0)" : "(heading >0)  && (heading < 360) && (abs(pitch) < 3) && (abs(roll) < 3 ) &&  !(heading == 0 && pitch == 0 && roll == 0)"; 
   const char * other_cutstr =  cut_attflag ? "attFlag == 0 &&   (heading >0)  && (heading < 360) && (abs(pitch) < 3) && (abs(roll) < 3 ) &&  !(pitch == 0 && roll == 0)": "(heading >0)  && (heading < 360) && (abs(pitch) < 3) && (abs(roll) < 3 ) &&  !(pitch == 0 && roll == 0)"; 

   const double wA = adu5A_attitude_a_priori_weight / ( adu5A_attitude_a_priori_weight + adu5B_attitude_a_priori_weight); 
   const double wB = adu5B_attitude_a_priori_weight / ( adu5A_attitude_a_priori_weight + adu5B_attitude_a_priori_weight); 

   //check for exception
   double this_min_score = default_min_score; 
   for (size_t i = 0; i < sizeof(min_score_exceptions) / sizeof(min_score_exceptions[0]); i++) 
   {
      if (min_score_exceptions[i][0] == run)
      {
        this_min_score = min_score_exceptions[i][1]; 
        printf("Overwrote minscore to %f\n", this_min_score); 
        break; 
      }
   }



   double hthresh = heading_slope_thresh; 
   for (size_t i = 0; i < sizeof(heading_slope_exceptions) / sizeof(heading_slope_exceptions[0]); i++) 
   {
      if (heading_slope_exceptions[i][0] == run)
      {
        hthresh = heading_slope_exceptions[i][1]; 
        printf("Overwrote hthresh to %f\n", hthresh); 
        break; 
      }
   }

       TGraphErrors * headingA = makeInterpolatedAttitudeGraph("heading",heading_cutstr, &adu5A, wA, offset,hthresh, this_min_score); 
       TGraphErrors * headingB = makeInterpolatedAttitudeGraph("heading",heading_cutstr, &adu5B, wB, offset,hthresh, this_min_score); 
       TGraphErrors * pitchA = makeInterpolatedAttitudeGraph("pitch",other_cutstr, &adu5A, wA, offset,0.1, this_min_score); 
       TGraphErrors * pitchB = makeInterpolatedAttitudeGraph("pitch",other_cutstr, &adu5B, wB, offset,0.1, this_min_score); 
       TGraphErrors * rollA = makeInterpolatedAttitudeGraph("roll",other_cutstr, &adu5A, wA, offset,0.1, this_min_score); 
       TGraphErrors * rollB = makeInterpolatedAttitudeGraph("roll",other_cutstr, &adu5B, wB, offset,0.1, this_min_score); 
   
   // generate the position graphs

       TGraph alt, lon, lat;


       if(run == 299)
	 {
	   makePositionGraphsPostOutage(&adu5A, &adu5B, &lat, &lon, &alt, offset);
	 }

       else
	 {
	   makePositionGraphs(&adu5A, &adu5B, &lat, &lon, &alt, offset); // <- currently crashing here
	 }
   
       //make output 
       for (int i = 0; i < headers.GetEntries(); i++) 
   {
     headers.GetEntry(i); 

     out.cd(); 
     double t = hdr->triggerTime+ hdr->triggerTimeNs*1e-9; 
     pat->latitude = lat.Eval(t); 
     pat->longitude = FFTtools::wrap(lon.Eval(t),360,0); 
     pat->altitude = alt.Eval(t);

     double headingError; 
     pat->heading = best(headingA, headingB, t, &headingError); 
     pat->pitch = best(pitchA, pitchB, t, 0,0); 
     pat->roll = best(rollA, rollB, t, 0,0); 

     

     /* too lazy to estimate mrms and brms right now, set them both to the weight on the heading  */
     pat->mrms = headingError; 
     pat->brms = headingError; 

     pat->attFlag = headingError < 0 ? 0 : 1; 

     pat->run = run; 
     pat->realTime = hdr->triggerTime; 
     pat->readTime = hdr->triggerTime;   // I don't think this is meaningful here? 
     pat->payloadTime = hdr->payloadTime; 
     pat->payloadTimeUs = hdr->payloadTimeUs; 
     pat->timeOfDay =1000* (t - time_of_day_start(t)); 

     printf("Event: %u\t heading: %f pitch: %f roll: %f lat: %f lon:%f alt: %f\n", hdr->eventNumber, pat->heading, pat->pitch, pat->roll, pat->latitude, pat->longitude, pat->altitude);  

     pat->intFlag =  hdr->triggerTimeNs; // Since I think the ADU5's take data on the second... this might be the right thing? 
     if (pat->intFlag == 0) pat->intFlag = 1;  // avoid confustion with raw data 
     tree->Fill(); 

     //std::cout << "blob" << std::endl;


   }

   tree->Write();

   std::cout << "Done" << std::endl;

   return 0; 
}
