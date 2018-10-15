//
// TestOpenAndSaveRawImage.C  (by Katsumasa Ikematsu)
//
// Usage:
//   $ root -b -q 'TestOpenAndSaveRawImage.C+("root/test.root", "hist11", "raw/test2_obj0.raw", 2560, 2160, 1.)'
//
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "TROOT.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TFile.h"
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
    cerr << "Error: There is no file." << endl;
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

int TestOpenAndSaveRawImage(const string rootfilepath, const string histname, const string filepath, const int xsize, const int ysize, const double magnifi)
{
  TFile* fout = new TFile(rootfilepath.c_str(), "recreate");
  gROOT->ProcessLine(".ls");

  vector<uint16_t> pixel_values;

#ifdef DEBUG
  cerr << endl;
  if (check_is_little_endian()) {
    cerr << "Little endian" << endl;
  } else {
    cerr << "Big endian" << endl;
  }
#endif

  read_unsigned_16bit_int_binary_sequence(filepath, &pixel_values);

  string title = "Image file: ";
  title.append(filepath);

  TCanvas* c = new TCanvas("canvas", title.data(), 0, 0, xsize*magnifi, ysize*magnifi);
  TH2I* h = new TH2I(histname.c_str(), title.data(), xsize, 0, xsize, ysize, 0, ysize);

#ifdef DEBUG
  cerr << endl;
  cerr << "size of pixel_values vector: " << pixel_values.size() << endl;
  cerr << endl;
#endif

  if (pixel_values.size() != xsize*ysize) {
    cerr << "Error: input pixel sizes are inconsistent with the vector size" << endl;
    return -1;
  }

  for (int j = 0; j < ysize; ++j) {
    for (int i = 0; i < xsize; ++i) {
      h->SetBinContent(i + 1, ysize - j, pixel_values.at(i + j*xsize));
    }
  }

  fout->Write();
  gROOT->ProcessLine(".ls");

  return 0;
}
