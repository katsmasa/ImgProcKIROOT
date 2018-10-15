//
// TestOpenRawImageByArray.C  (by Katsumasa Ikematsu)
//
// Usage:
//   $ root -l 'TestOpenRawImageByArray.C("raw/Earth16bit.raw", 16, 16, 20)'
//   $ root -l 'TestOpenRawImageByArray.C+("raw/test2_obj0.raw", 2560, 2160, 0.25)'
//
#include <iostream>
#include <fstream>
#include "TStyle.h"
#include "TCanvas.h"
#include "TH2.h"

using namespace std;

//#define DEBUG

int TestOpenRawImageByArray(const string filepath, const int xsize, const int ysize, const double magnifi) {

  unsigned short temp;
  double* imagedata;

  imagedata = new double[xsize * ysize];

  for (int j = 0; j < ysize; ++j) {
    for (int i = 0; i < xsize; ++i) {
      imagedata[i + j*xsize] = 0.0;
    }
  }

  memset(imagedata, 0x00, sizeof(imagedata[0]) * xsize * ysize);

#ifdef DEBUG
  cerr << endl;
  cerr << "sizeof(imagedata[0]) = " << sizeof(imagedata[0]) << endl;
  cerr << "sizeof(*imagedata)   = " << sizeof(*imagedata)   << endl;
  cerr << endl;
#endif

#if 0
  for (int j = 0; j < ysize; ++j) {
    for (int i = 0; i < xsize; ++i) {
      cerr << "(" << i << "," << j << ") = " << imagedata[i + j*xsize] << endl;
    }
  }
  cerr << endl;
#endif

  cout << "File path: " << filepath << endl;

  //ifstream ifs(filepath, ios::in | ios::binary);
  ifstream ifs(filepath, ios_base::in | ios_base::binary);
  if (!ifs) {
    cerr << "File open failed..." << endl;
    exit(1);
  }

  long data_length;
  data_length = (long)(xsize * ysize) * sizeof(unsigned short);

#ifdef DEBUG
  cerr << endl;
  cerr << "sizeof(unsigned short): " << sizeof(unsigned short) << endl;
  cerr << "data_length = " << data_length << endl;
  cerr << endl;
#endif

  //ifs.seekg(-1*data_length, ios::end);
  ifs.seekg(-1*data_length, ios_base::end);

  for (int j = 0; j < ysize; ++j) {
    for (int i = 0; i < xsize; ++i) {
      ifs.read((char*)&temp, sizeof(unsigned short));
      //cerr << "(" << i << "," << j << ") = " << temp << endl;
      imagedata[i + j*xsize] = (double)temp;
    }
  }
  ifs.close();

#ifdef DEBUG
  for (int j = 0; j < ysize; ++j) {
    for (int i = 0; i < xsize; ++i) {
      cerr << "(" << i << "," << j << ") = " << imagedata[i + j*xsize] << endl;
    }
  }
  cerr << endl;
#endif

  string title = "Image file: ";
  title.append(filepath);

  TCanvas* c = new TCanvas("canvas", title.data(), 0, 0, xsize * magnifi, ysize * magnifi);
//TH2S* h = new TH2S("hist", "; x; y", xsize, 0, xsize, ysize, 0, ysize);
  TH2D* h = new TH2D("hist", title.data(), xsize, 0, xsize, ysize, 0, ysize);

  for (int j = 0; j < ysize; ++j) {
    for (int i = 0; i < xsize; ++i) {
      h->SetBinContent(i + 1, ysize - j, imagedata[i + j*xsize]);
    }
  }

  gStyle->SetOptStat(0);
  gStyle->SetPalette(52);
  const Int_t NCont = 255;
  gStyle->SetNumberContours(NCont);
  c->cd();
  h->Draw("colz");
  c->Print("c.png");

  delete[] imagedata;
//delete h;
//delete c;

  return 0;
}
