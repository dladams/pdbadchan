{
  gROOT->ProcessLine("ArtServiceHelper::load(\"load.fcl\")");
  gROOT->ProcessLine("DuneToolManager* ptm = DuneToolManager::instance(\"load.fcl\")");
  //gROOT->ProcessLine(".L drawGainDist.C");
}
