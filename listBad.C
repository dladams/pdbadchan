using Index = unsigned int;
using IndexSet = std::set<Index>;
using IndexVector= std::vector<Index>;
using RunsByChan = std::map<Index, IndexVector>;
using RmsByRun = std::map<Index, float>;
using RunRmsByChan = std::map<Index, RmsByRun>;
using RmsRange = std::pair<float, float>;
using RmsRangesByPlane = std::map<Index, RmsRange>;
using RmsRangesByPlaneByRun = std::map<Index, RmsRangesByPlane>;
using Name = std::string;
using NameByIndex = std::map<Index, Name>;
using NameVector = vector<Name>;
using FloatVector = vector<float>;

int chanColor(Index icha, Index& kcol) {
  const string myname = "chanColor: ";
  LineColors lc;
  // Use red for old bad, blue otherwise.
  if ( kcol == 99 ) {
    static auto psta = ptm->getPrivate<IndexMapTool>("channelStatusFromConfig");
    if ( psta ) {
      int cstat = psta->get(icha);
      //cout << myname << "Status for channel " << icha << " is " << cstat << endl;
      if ( cstat == 1 ) return lc.red();
    } else {
      cout << myname << "WARNING: Unable to load channel status tool" << endl;
    }
    return lc.blue();
  }
  // Return the color for kcol and increment.
  return lc.color(kcol++, 10);
}

// Pulser runs.
bool isPulser(Index run) {
  if ( run == 6068 ) return true;
  if ( run == 7604 ) return true;
  if ( run == 7605 ) return true;
  if ( run == 10489 ) return true;
  if ( run == 10491 ) return true;
  if ( run == 11526 ) return true;
  if ( run == 11527 ) return true;
  return false;
}

// Channels declared bad after March/April 2020
const IndexSet& badAfterSpring2020() {
  static IndexSet bads;
  if ( bads.size() == 0 ) {
    bads.insert(2382);
    bads.insert(2383);
    bads.insert(2858);
    bads.insert(3520);
    bads.insert(3521);
    bads.insert(3523);
    bads.insert(3528);
    bads.insert(4316);
    bads.insert(4317);
    bads.insert(4318);
    bads.insert(4319);
    bads.insert(5031);
    bads.insert(8340);
    bads.insert(8342);
    bads.insert(8344);
    bads.insert(8346);
    bads.insert(8348);
    bads.insert(9011);
    bads.insert(9013);
    bads.insert(9015);
    bads.insert(9017);
    bads.insert(9019);
    //bads.insert(10033);
    bads.insert(10072);
    bads.insert(10074);
    bads.insert(10075);
    bads.insert(10076);
    bads.insert(10078);
    bads.insert(10080);
    bads.insert(10082);
    bads.insert(11182);
    bads.insert(11194);
    bads.insert(11468);
    bads.insert(11926);
    bads.insert(12544);
    bads.insert(12546);
    bads.insert(12647);
    bads.insert(12648);
    bads.insert(12649);
    bads.insert(12650);
    bads.insert(12651);
    bads.insert(12652);
  }
  return bads;
}

struct TpsPlaneDesc {
  TpsPlaneDesc(Index itpsview) : m_itps(itpsview/4), m_iview(itpsview%4) { }
  TpsPlaneDesc(Index itps, Index iview) : m_itps(itps), m_iview(iview) { }
  static TpsPlaneDesc fromChannel(Index icha) {
    Index itps = icha/2560;
    Index iview = 99;
    bool isLeft = itps%2;
    Index ichr = icha%2560;
    if      ( ichr <  800 ) iview = 0;
    else if ( ichr < 1600 ) iview = 1;
    else if ( ichr < 2080 ) iview = (isLeft ? 2 : 3);
    else if ( ichr < 2560 ) iview = (isLeft ? 3 : 2);
    return TpsPlaneDesc(itps, iview);
  }
  Index tps() const { return m_itps; }
  Index viewIndex() const { return m_iview; }
  string viewName() const {
    if ( m_iview > 4 ) return "UnknownView";
    static string sviews[4] = {"u", "v", "z", "c"};
    return sviews[m_iview];
  }
  Index detIndex() const { return 4*m_itps + m_iview; }
  Index apaIndex() const {
    if ( m_itps > 5 ) return 99;
    static Index apaval[6] = {3, 5, 2, 6, 1, 4};
    return apaval[m_itps];
  }
  string planeName(bool upper =false) const {
    string spre = upper ? "TPS" : "tps";
    return spre + to_string(m_itps) + viewName();
  }
  string planeLabel() const {
    return "TPS " + to_string(m_itps) + viewName() +
           " (APA " + to_string(apaIndex()) + ")";
  }
private:
  Index m_itps;
  Index m_iview;
};

// Threshold for RMS to be bad depends on run, TPC set and view.
double rmsThreshold(Index run, Index itps, string sview) {
  double minrmsZ =  4.0;
  double minrmsI =  4.0;
  double minrmsC =  2.0;
  double minrms = sview == "u" ? minrmsI
                : sview == "v" ? minrmsI
                : sview == "z" ? minrmsZ
                : sview == "c" ? minrmsC
                : (abort(), -1.0);
  if ( run == 9060 ) minrms = 2.0;
  if ( run == 9070 ) minrms = 2.0;
  if ( run == 9070 && itps == 3 ) minrms = 1.9;
  if ( run == 9337 ) minrms = 2.0;
  //if ( run == 11142 ) minrms = 2.0;
  if ( run == 11296 && itps == 0 ) minrms = 2.0;
  return minrms;
}

TGraph* getGraph(int run, int itps) {
  string srun = to_string(run);
  while ( srun.size() < 6 ) srun = "0" + srun;
  string stps = to_string(itps);
  string sfin = "myrms/np04_run" + srun + "_evts000000-001000/event000000-000100/chmet_pedrawrms_tps" + stps + "_run" + srun + ".tpad";
  TPadManipulator* pp = TPadManipulator::read(sfin);
  if ( pp == nullptr ) {
    cout << "File not found: " << sfin << endl;
    return nullptr;
  }
  TGraph* pg = pp->graph();
  if ( pg == nullptr ) {
    cout << "Graph not found" << endl;
    return nullptr;
  }
  return pg;
}

// Get the RMS for each channel and return the following:
//   dbg > 0 ==> Display bad values
//   badchas ==> Bad channels are added to this list.
//   badrmss ==> Add RMS for each listed channel.
// Returns the number of bad channels.
Index getRMS(int run, int itps, int dbg, RunsByChan& badchas, RunRmsByChan& badrmss, RmsRangesByPlane& rans) {
  TGraph* pg = getGraph(run, itps);
  int npt = pg == nullptr ? 0 : pg->GetN();
  cout << "Run" << setw(6) << run << " TPS " << itps << " point count is" << setw(5) << npt << endl;
  if ( npt == 0 ) {
    cout << "ERROR: APA is missing." << endl;
    abort();
  }
  if ( npt != 2560 ) {
    cout << "WARNING: Run " << run << " TPC set " << itps << " has unexpected channel count: " << npt << endl;
  }
  if ( pg == nullptr ) return 0;
  // Fetch the RMS for each channel in the TPS. Use -1 if channel is missing.
  FloatVector rmss(2560, -1);
  Index icha1 = 2560*itps;
  Index icha2 = icha1 + 2560;
  for ( int ipt=0; ipt<npt; ++ipt ) {
    double xcha, rms;
    pg->GetPoint(ipt, xcha, rms);
    Index icha = int(xcha+0.01);
    if ( icha < icha1 || icha >= icha2 ) {
      cout << "ERROR: Unexpected channel number " << icha << " for TPS " << itps << endl;
      abort();
    }
    int ichr = icha%2560;
    rmss[ichr] = rms;
  }
  // Loop over channels and flag those tha are bad.
  bool isLeft = itps%2;
  Index nbad = 0;
  Index ipla = 9999;
  using RmsDist = std::multiset<float>;
  using RmsDistByChan = std::map<Index, RmsDist>;
  RmsDistByChan dists;
  for ( Index ichr=0; ichr<2560; ++ichr ) {
    Index icha = icha1 + ichr;
    float rms = rmss[ichr];
    TpsPlaneDesc tpd = TpsPlaneDesc::fromChannel(icha);
    string sview = tpd.viewName();
    Index itpv = tpd.detIndex();
    double minrms = rmsThreshold(run, itps, sview);
    double maxrms = 40.0;
    if ( rms >= 0.0 && (rms < minrms || rms > maxrms) ) {
      ++nbad;
      if ( dbg ) cout << sview << setw(7) << run << setw(6) << icha << ": " << rms << endl;
      badchas[icha].push_back(run);
    }
    if ( badrmss.count(icha) ) {
      badrmss[icha][run] = rms;
    }
    if ( rms >= 0.0 ) dists[itpv].insert(rms);
  }
  for ( auto idst : dists ) {
    Index itpv = idst.first;
    if ( rans.count(itpv) ) {
      cout << "ERROR: Duplicate range index: " << itpv << endl;
    } else {
      const RmsDist& dist = idst.second;
      Index nrms = dist.size();
      RmsDist::const_iterator irms = dist.begin();
      advance(irms, 0.05*nrms);
      rans[itpv].first = *irms;
      advance(irms, 0.90*nrms);
      rans[itpv].second = *irms;
    }
  }
  return nbad;
}

void listBadForRun(int run, RunsByChan& badchas, RunRmsByChan& badrmss, RmsRangesByPlane& rans, int dbg) {
  Index nbad = 0;
  for ( int itps : { 0, 1, 2, 3, 4, 5} ) {
    nbad += getRMS(run, itps, dbg, badchas, badrmss, rans);
  }
  cout << "Run " << run << " bad channel count: " << setw(3) << nbad
       << ". Total: " << setw(3) << badchas.size() << endl;
}

Index
getBadCategory(const IndexVector& runs, const NameByIndex& runtypes, Index npulAll) {
  Index nrunAll = runtypes.size();
  Index nbcoAll = nrunAll - npulAll;
  Index nrun = 0;
  Index nbco = 0;
  Index npul = 0;
  for ( Index run : runs ) {
    ++nrun;
    if ( runtypes.at(run) == "pulser" ) ++npul;
    else ++nbco;
  }
  if ( nrun == nrunAll ) return 1;
  if ( nbco == nbcoAll ) return 2;
  if ( nbco == 0 ) {
    if ( npul == npulAll ) return 3;
    return 4;
  }
  return 0;
}

void listBad(Index drawStat =999, float plotmax = 40.0, int dbg =0) {
  static auto pchs = ptm->getPrivate<IndexMapTool>("myChannelStatus");
  if ( ! pchs ) {
    cout << "ERROR: Unable to find channel status tool." << endl;
    abort();
  }
  NameVector badCats = {
    "Good",
    "All bad",
    "All bad except pulser",
    "Intermittent bad ASIC",
    "Intermittent bad channel",
    "Intermittent bad pulser",
    "Bad after Dec 2018",
    "Bad after Sep 2019",
    "Bad after Jan 2020",
    "Bad after Mar 2020",
    "Bad after Apr 2020",
    "All noisy",
    "Intermittent noisy",
    "Bad in Nov 2019 survey",
    "Noisy in Dec 2018 survey",
    "????"
  };
  RunsByChan badchas;
  RunRmsByChan badrmss;
  RmsRangesByPlaneByRun ranges;
  // Read the list of runs to process.
  ifstream fin("runs.txt");
  IndexSet runs;
  using IndexMap = map<Index, Index>;
  IndexMap runidxs;
  Index nrun = 0;
  NameByIndex runlabs;
  NameByIndex runtypes;
  Index npulser = 0;
  Index nbeam   = 0;
  Index ncosmic = 0;
  Name spatAll;
  while ( fin && !fin.eof() ) {
    string slin;
    getline(fin, slin);
    if ( slin.size() == 0 || slin[0] == '#' ) continue;
    istringstream ssin(slin);
    Index run;
    Name type;
    Name lab;
    ssin >> run >> type >> lab;
    if ( fin.eof() ) break;
    runs.insert(run);
    runidxs[run] = nrun++;
    runlabs[run] = lab;
    if ( type == "beam" ) {
      ++nbeam;
      spatAll += "_";
    } else if ( type == "cosmic" ) {
      ++ncosmic;
      spatAll += "_";
    } else if ( type == "pulser" ) {
      ++npulser;
      spatAll += "-";
    } else {
      cout << "Run " << run << " has invalid type " << type << endl;
      abort();
    }
    runtypes[run] = type;
  }
  // Find the bad channels.
  for ( Index run : runs ) {
    listBadForRun(run, badchas, badrmss, ranges[run], dbg);
  }
  // Fetch the RMS values for the bad channels.
  for ( const auto& irbc : badchas ) badrmss[irbc.first];
  badchas.clear();
  ranges.clear();
  for ( Index run : runs ) {
    listBadForRun(run, badchas, badrmss, ranges[run], dbg);
  }
  // Create bin labels.
  vector<string> binlabs;
  for ( const auto& iran : ranges ) {
    Index run = iran.first;
    binlabs.push_back(runlabs[run]);
  }
  // Create plot for each APA and plane.
  Index ntps = 6;
  vector<TPadManipulator*> topmans(ntps);
  vector<string> fnamtops(ntps);
  map<string, TPadManipulator*> pltmans;
  for ( Index itps=0; itps<ntps; ++itps ) {
    TPadManipulator* pmantop = new TPadManipulator(1400, 1000);
    pmantop->split(2,2);
    topmans[itps] = pmantop;
    fnamtops[itps] = "badchan-tps" + to_string(itps) + ".png";
    Index ipad = 0;
    for ( Index iview : {2, 0, 3, 1} ) {
      TpsPlaneDesc tpd(itps, iview);
      string spla = tpd.planeName();
      Index itpv = tpd.detIndex();
      string sview = tpd.viewName();
      TPadManipulator* pman = pmantop->man(ipad++);
      pltmans[spla] = pman;
      // Create histogram for axis and labels.
      Index nrun = ranges.size();
      string hnam = "pha" + spla;
      string sttl = tpd.planeLabel();
      string httl = sttl + ";Run date;RMS [ADC count]";
      TH1* pha = new TH1F(hnam.c_str(), httl.c_str(), nrun, 0, nrun);
      pha->SetStats(0);
      pha->GetXaxis()->SetLabelSize(0.04);   // ROOT ignores this for string labels :(.
      pman->add(pha,"h");
      // Add graphs showing range limits.
      TGraph* pg1 = new TGraph();
      TGraph* pg2 = new TGraph();
      TGraph* pgt = new TGraph();
      pg1->SetLineWidth(3);
      pg2->SetLineWidth(3);
      pgt->SetLineWidth(2);
      pg1->SetLineColor(3);
      pg2->SetLineColor(3);
      pgt->SetLineColor(1);
      pgt->SetLineStyle(3);
      for ( TGraph* pg : {pg1, pg2, pgt} ) {
        pg->SetTitle(sttl.c_str());
        pg->GetXaxis()->SetTitle("Run index");
        pg->GetYaxis()->SetTitle("RMS [ADC count]");
      }
      // Loop over runs.
      for ( const auto& irbp : ranges ) {
        Index run = irbp.first;
        float xrun = runidxs[run] + 0.5;
        const auto& rangesByPlane = irbp.second;
        RmsRangesByPlane::const_iterator iran = rangesByPlane.find(itpv);
        if ( iran != rangesByPlane.end() ) {
          float y1 = iran->second.first;
          float y2 = iran->second.second;
          float yt = rmsThreshold(run, itps, sview);
          if ( y2 >= 0.0 ) {
            pg1->SetPoint(pg1->GetN(), xrun, y1);
            pg2->SetPoint(pg2->GetN(), xrun, y2);
            pgt->SetPoint(pgt->GetN(), xrun, yt);
          }
          //cout << "Central: " << xrun << ": " << y1 << ", " << y2 << endl;
        }
      }
      if ( pg1->GetN() ) {
        pman->add(pg1, "L");
        pman->add(pg2, "L");
        pman->add(pgt, "L");
      } else {
        cout << "ERROR: No ranges found for plane " << tpd.planeName() << endl;
      }
      pman->setRangeY(0, plotmax);
      pman->addAxis();
      pman->setBinLabelsX(binlabs);
      // Add legend.
      string stxt = "Channels found bad";
      if ( drawStat < 999 ) {
        stxt += " flagged \"" + badCats[drawStat] + "\"";
      }
      TLatex* ptxt = new TLatex(0.12, 0.87, stxt.c_str());
      ptxt->SetNDC();
      ptxt->SetTextFont(42);
      ptxt->SetTextSize(0.040);
      pman->add(ptxt);
    }
  }

  // Add graph for each bad channel.
  string oldview;
  IndexVector kcols(24, 0);     // 0 to rotate colors, 99 for red/blue
  IndexVector kmrks(24, 0);     // 0 to rotate colors, 99 for red/blue
  Index kcol = 0;     // 0 to alternate, 99 for red/bue
  Index kmrk = 0;
  Index markers[] = {2, 5, 4, 26, 32, 28, 46, 30, 27, 44};
  vector<string> fnams;
  for ( auto chamap : badrmss ) {
    Index icha = chamap.first;
    if ( drawStat < 999 ) {
      Index icat = pchs->get(icha);
      if ( icat != drawStat ) continue;
    }
    TpsPlaneDesc tpd = TpsPlaneDesc::fromChannel(icha);
    Index itpv = tpd.detIndex();
    string spla = tpd.planeName();
    string sttl = tpd.planeLabel() + "_chan" + to_string(icha);
    int msty = markers[kmrks[itpv]];
    kmrks[itpv] = (kmrks[itpv]+1)%10;
    Index icol = chanColor(icha, kcols[itpv]);
    TPadManipulator* pman = pltmans[spla];
    TGraph* pga = new TGraph();
    TGraph* pgp = new TGraph();
    pga->SetMarkerColor(icol);
    pgp->SetMarkerColor(icol);
    pga->SetMarkerStyle(msty);
    pga->SetLineColor(icol);
    pgp->SetMarkerStyle(25);
    pga->SetTitle(sttl.c_str());
    pga->GetXaxis()->SetTitle("Run index");
    pga->GetYaxis()->SetTitle("RMS [ADC count]");
    if ( pchs->get(icha) == 0 ) {
      pga->SetLineStyle(3);
    }
    for ( auto runrms : chamap.second ) {
      Index run = runrms.first;
      float rms = runrms.second;
      //cout << "ZZZZZZZZZZZ  " << run << ": " << rms << endl;
      if ( rms >= 0.0 ) {
        float x = runidxs[run] + 0.5;
        pga->SetPoint(pga->GetN(), x, rms);
        if ( isPulser(run) ) pgp->SetPoint(pgp->GetN(), x, rms);
      }
    }
    pman->add(pga, "PL");
    pman->add(pgp, "P");
  }

  for ( Index iman=0; iman<topmans.size(); ++iman ) {
    TPadManipulator* pman = topmans[iman];
    string fnam = fnamtops[iman];
    cout << "Printing " << fnam << endl;
    pman->print(fnam);
  }
  Index count = 0;
  for ( const auto& irbp : ranges ) {
    Index run = irbp.first;
    cout << count++ << " " << run << endl;
  }
  count = 0;
  for ( auto krun : runidxs ) {
    cout << krun.first << ": " << krun.second << endl;
  }
  // Text report.
  auto psta = ptm->getPrivate<IndexMapTool>("channelStatusFromConfig");
  cout << "Bad channels (* denotes those in Nov 2019 bad channels list)" << endl;
  cout << "  Plane Channel (chp)  ASICch  Runs" << endl;
  string splaold;
  ProtoduneChannelHelper chh(true);
  Index nbad = 0;
  Index nbadOld = 0;
  Index nbadNew = 0;
  IndexVector badCounts(badCats.size(), 0);
  for ( const auto& irbc : badchas ) {
    Index icha = irbc.first;
    Index ichr = icha%2560;
    Index ichp = ichr;
    if ( ichp >= 800 ) {
      ichp -=800;
      if ( ichp >= 800 ) {
        ichp -= 800;
        if ( ichp >= 480 ) {
          ichp -= 480;
          if ( ichp >= 480 ) abort();
        }
      }
    }
    const IndexVector& runs = irbc.second;
    //Index icat = getBadCategory(runs, runtypes, npulser);
    Index icat = pchs->get(icha);
    ++badCounts[icat];
    if ( drawStat < 999 ) {
      Index icat = pchs->get(icha);
      if ( icat != drawStat ) continue;
    }
    string scat = badCats[icat];
    int cstat = psta->get(icha);
    bool wasBad = cstat == 1;
    bool wasNoisy = cstat == 2;
    string sstat = wasBad ? "*" : wasNoisy ? "N" : " ";
    wasBad ? ++nbadOld : ++nbadNew;
    ++nbad;
    // Build and show the the pattern.
    string spat = spatAll;
    for ( Index run : runs ) spat[runidxs[run]] = (runtypes[run] == "pulser" ? 'P' : '*');
    const RmsByRun& rmss = badrmss[icha];
    for ( auto irms : rmss ) {
      Index run = irms.first;
      float rms = irms.second;
      if ( rms < 0 ) spat[runidxs[run]] = ' ';
    }
    TpsPlaneDesc tpd = TpsPlaneDesc::fromChannel(icha);
    string spla = tpd.planeName();
    if ( spla != splaold ) cout << endl;
    //if ( spla == splaold ) spla = "     ";
    splaold = spla;
    string sach = chh.asicChannelName(icha);
    cout << sstat << setw(6) << spla << setw(8) << icha << " (" << setw(3) << ichp << ") " << sach << " ";
    cout << setw(30) << scat << ": ";
    cout << spat << " ";
    // List the bad runs
    // If there are too many, list the good runs instead.
    if ( runs.size() < 0.6*runidxs.size() ) {
      for ( Index run : runs ) {
        cout << setw(6) << run;
      }
    } else {
      for ( auto krun : runidxs ) {
        Index run = krun.first;
        if ( std::count(runs.begin(), runs.end(), run) == 0 ) cout << setw(7) << -int(run);
      }
    }
    cout << endl;
  }
  cout << "Nbad old:" << setw(4) << nbadOld << endl;
  cout << "Nbad new:" << setw(4) << nbadNew << endl;
  cout << "Nbad tot:" << setw(4) << nbad    << endl;
  Index icnt = 0;
  for ( Index icat=0; icat<badCats.size(); ++icat ) {
    Name scat = badCats[icat];
    Index ncatBad = badCounts[icat];
    cout << setw(3) << icnt++ << setw(30) << scat << ":" << setw(4) << ncatBad << endl;
  }
}

