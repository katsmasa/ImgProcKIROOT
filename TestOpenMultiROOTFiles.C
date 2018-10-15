//
// TestOpenMultiROOTFiles.C  (by Katsumasa Ikematsu)
//
// Usage:
//   $ root -b -q 'TestOpenMultiROOTFiles.C("root/", "test", 2)'
//
#include <iostream>
#include <vector>
#include <string>
#include "TROOT.h"
#include "TFile.h"

void TestOpenMultiROOTFiles(const string directory, const string rootfilename, const int nfile) {

  vector<TFile*> rootfiles;
  for (int i = 0; i < nfile; ++i) {
    string path = directory + rootfilename + "." + to_string(i) + ".root";
    TFile* f = new TFile(path.c_str(), "READ");
    rootfiles.push_back(f);
  }

  for (int i = 0; i < nfile; ++i) {
    rootfiles.at(i)->cd();
    gROOT->ProcessLine(".ls");
    cout << endl;
  }
}
