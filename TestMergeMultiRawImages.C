//
// TestMergeMultiRawImages.C  (by Katsumasa Ikematsu)
//
// Usage:
//   $ root -l 'TestMergeMultiRawImages.C("raw/iconlist.txt", 16, 16, 40)'
//
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "TStyle.h"
#include "TCanvas.h"
#include "TH2.h"

using namespace std;

#define DEBUG

bool check_is_little_endian() {
  const uint16_t value = 1;
  return (*(char*)&value) ? true : false;
}

const size_t get_file_byte_size(ifstream& file) {
  file.seekg(0, ios::end);
  const size_t file_size = (size_t)file.tellg();
  file.seekg(0, ios::beg);

  return file_size;
}

void convert_unsigned_16bit_int_endian(vector<uint16_t>* target_vector) {
  for (auto& value : *target_vector) {
    value = (value << 8) | (value >> 8);  // different from the signed_16bit_int case
  }
}

bool read_unsigned_16bit_int_binary_sequence(const string& filepath, vector<uint16_t>* target_vector) {
  ifstream file(filepath, ios::in | ios::binary);
  if (!file) {
    cerr << "Error: There is no such file: ./" << filepath << endl;
    return false;
  }

#ifdef DEBUG
  cerr << endl;
  cerr << "file size: " << get_file_byte_size(file) << " (bytes)" << endl;;
  cerr << endl;
#endif

  const size_t file_size = get_file_byte_size(file);
  target_vector->clear();
  target_vector->resize(file_size / 2);  // uint16_t (16 bit) is 2 byte.

  file.read((char*)target_vector->data(), file_size);
  if (check_is_little_endian()) {
    //convert_unsigned_16bit_int_endian(target_vector);
  }
  file.close();

  return true;
}

int TestMergeMultiRawImages(const string filelist, const int xsize, const int ysize, const double magnifi)
{
  ifstream ifs(filelist);
  if ( ifs.fail() ) {
    cerr << "Error! Failed to open the file: ./" << filelist << endl;
    exit(1);
  }

  vector<string> filepaths;
  string temp_filepath;
  int nfile = 0;
  while (ifs >> temp_filepath) {
    filepaths.push_back(temp_filepath);
    nfile++;
  }
  ifs.close();
  cout << "the number of file in the image file list is " << nfile << endl;
//cout << "the number of file in the image file list is " << filepaths.size() << endl;

  vector<vector<uint16_t>> pixel_values;
  pixel_values.resize(nfile);

#ifdef DEBUG
  cerr << endl;
  if (check_is_little_endian()) {
    cerr << "Little endian" << endl;
  } else {
    cerr << "Big endian" << endl;
  }
#endif

  string canvastitle1 = "Display ";
  string canvastitle2 = to_string(nfile);
  string canvastitle3 = " image files";
  string canvastitle  = canvastitle1 + canvastitle2 + canvastitle3;
  vector<string> histotitles;

  for (int i = 0; i < nfile; ++i) {
    read_unsigned_16bit_int_binary_sequence(filepaths.at(i), &pixel_values.at(i));
    histotitles.push_back(filepaths.at(i));
  }

  TCanvas* c = new TCanvas("canvas", canvastitle.data(), 0, 0, nfile*xsize*magnifi, ysize*magnifi);

  vector<TH2I*> histos;
  for (int i = 0; i < nfile; ++i) {
    TH2I* h = new TH2I(Form("h%d", i), histotitles.at(i).data(), xsize, 0, xsize, ysize, 0, ysize);
    histos.push_back(h);
  }

#ifdef DEBUG
  cerr << endl;
  for (int i = 0; i < nfile; ++i) {
    cerr << "size of pixel_values vector for image file " << i << ": " << pixel_values.at(i).size() << endl;
  }
  cerr << endl;
#endif

  for (int i = 0; i < nfile; ++i) {
    if (pixel_values.at(i).size() != xsize*ysize) {
      cerr << "Error: input pixel sizes are inconsistent with the vector size" << endl;
      return -1;
    }
  }

  for (int k = 0; k < nfile; ++k) {
    for (int j = 0; j < ysize; ++j) {
      for (int i = 0; i < xsize; ++i) {
        histos.at(k)->SetBinContent(i + 1, ysize - j, pixel_values.at(k).at(i + j*xsize));
      }
    }
  }

  gStyle->SetOptStat(0);
  gStyle->SetPalette(52);
  const Int_t NCont = 255;
  gStyle->SetNumberContours(NCont);
  c->Divide(nfile);
  for (int i = 0; i < nfile; ++i) {
    c->cd(i+1);
  //histos.at(i)->Draw("colz");
  //histos.at(i)->Draw("text colz");
    histos.at(i)->Draw("text45 colz");
  }
  c->Update();

  //////////

  TCanvas* c_m = new TCanvas("c_m", "Merged image", 0, 0, xsize*magnifi, nfile*ysize*magnifi);
  c_m->cd();

  vector<vector<TH1D*>> projxhistos;
  projxhistos.resize(nfile);

  for (int k = 0; k < nfile; ++k) {
    for (int j = 0; j < ysize; ++j) {
      projxhistos.at(k).push_back(histos.at(k)->ProjectionX(Form("projxh%d", (k+1)*100000+(j+1)), j+1, j+1));
    }
  }

#if 0
  for (int k = 0; k < nfile; ++k) {
    for (int j = 0; j < ysize; ++j) {
      for (int i = 0; i < xsize; ++i) {
	cout << "(#img, #projx, #bin, bin-content) = (" << k << ", " << j+1 << ", " << i+1 << ", " << projxhistos.at(k).at(j)->GetBinContent(i+1) << ")" << endl;
      }
    }
  }
#endif

  TH2I* hmerge = new TH2I("hmerge", "Merged histo", xsize, 0, xsize, nfile*ysize, 0, nfile*ysize);
  for (int k = 0; k < nfile; ++k) {
    for (int j = 0; j < ysize; ++j) {
      int yindex = (nfile*j + 1) + k;
      //cout << "yindex = " << yindex << endl;
      for (int i = 0; i < xsize; ++i) {
        hmerge->SetBinContent(i+1, yindex, projxhistos.at(k).at(j)->GetBinContent(i+1));
      }
    }
  }
  hmerge->Draw("text45 colz");
//hmerge->Draw("colz");
  c_m->Update();
  c_m->Print("merged.png");

  return 0;
}
