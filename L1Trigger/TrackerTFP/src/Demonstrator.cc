#include "L1Trigger/TrackerTFP/interface/Demonstrator.h"

#include <vector>
#include <string>
#include <sstream>
#include <fstream>

using namespace std;
using namespace edm;
using namespace tt;

namespace trackerTFP {

  Demonstrator::Demonstrator(const ParameterSet& iConfig, const Setup* setup)
      : dirIPBB_(iConfig.getParameter<string>("DirIPBB")),
        runTime_(iConfig.getParameter<double>("RunTime")),
        dirIn_(dirIPBB_ + "in.txt"),
        dirOut_(dirIPBB_ + "out.txt"),
        dirPre_(dirIPBB_ + "pre.txt"),
        dirDiff_(dirIPBB_ + "diff.txt"),
        numFrames_(setup->numFramesIO()),
        numFramesInfra_(setup->numFramesInfra()),
        numRegions_(setup->numRegions()),
        ievt_(0) {}

  // plays input through modelsim and compares result with output
  bool Demonstrator::analyze(int ievt, const vector<vector<Frame>>& input, const vector<vector<Frame>>& output) const {
    stringstream ss;
    vector<int> InLinkMapping{0,1,2,3,4,5,6,7,
                              8,9,10,11,12,13,14,15,
                              16,17,18,19,20,21,22,23,
                              24,25,26,27,28,29,30,31,
                              32,33,34,35,36,37,38,39,
                              40,41,42,43,44,45,46,47,
                              48,49,50,51,52,53,54,55,
                              56,57,58,59,60,61,62,63,
                              64,65,66,67,68,69,70,71
                              };
    vector<int> OutLinkMapping{40,41,42,43,44,45,46,47,48,49,50,51,54,55,56,57,58,59,60,61};
    // converts input into stringstream
    //convertNine(input, ss, InLinkMapping);
    convert(input, ss);
    // play input through modelsim
    sim(ievt, ss, "in");
    // converts output into stringstream
    convert(output, ss);
    //convertNine(output, ss, OutLinkMapping);
    // compares output with modelsim output
    sim(ievt, ss, "out");
    return compare(ievt,ss);
  }

  // converts streams of bv into stringstream
  void Demonstrator::convert(const vector<vector<Frame>>& bits, stringstream& ss) const {
    // reset ss
    ss.str("");
    ss.clear();
    // number of tranceiver in a quad
    static constexpr int quad = 4;
    const int numChannel = bits.size() / numRegions_;
    const int voidChannel = numChannel % quad == 0 ? 0 : quad - numChannel % quad;
    // start with header
    ss << header(numChannel + voidChannel);
    int nFrame(0);
    // create one packet per region
    for (int region = 0; region < numRegions_; region++) {
      const int offset = region * numChannel;
      // start with emp 6 frame gap
      ss << infraGap(nFrame, numChannel + voidChannel);
      for (int frame = 0; frame < numFrames_; frame++) {
        // write one frame for all channel
        ss << this->frame(nFrame);
        for (int channel = 0; channel < numChannel; channel++) {
          const vector<Frame>& bvs = bits[offset + channel];
          ss << (frame < (int)bvs.size() ? hex(bvs[frame]) : hex(Frame()));
        }
        for (int channel = 0; channel < voidChannel; channel++)
          ss << " 0v" << string(TTBV::S_ / 4, '0');
        ss << endl;
      }
    }
  }

  void Demonstrator::convertNineInterleave(const vector<vector<Frame>>& bits, stringstream& ss) const {
    // reset ss
    ss.str("");
    ss.clear();
    vector<int> EvenLinkMapping{0,1,2,3,4,5,6,7,
                                16,17,18,19,20,21,22,23,
                                32,33,34,35,36,37,38,39,
                                48,49,50,51,52,53,54,55,
                                64,65,66,67,68,69,70,71};
    vector<int> OddLinkMapping{ 8,9,10,11,12,13,14,15,
                                24,25,26,27,28,29,30,31,
                                40,41,42,43,44,45,46,47,
                                56,57,58,59,60,61,62,63,
                                0, 1, 2, 3, 4, 5, 6, 7};

    // number of tranceiver in a quad
    static constexpr int quad = 4;
    const int numChannel = 40; //bits.size();

    const int voidChannel = numChannel % quad == 0 ? 0 : quad - numChannel % quad;

    // start with header
    ss << header(numChannel + voidChannel);
    int nFrame(0);
    // create one packet per region
      const int offset = 0;
      // start with emp 6 frame gap
      ss << infraGap(nFrame, numChannel + voidChannel);
      for (int frame = 0; frame < numFrames_; frame++) {
        // write one frame for all channel
        ss << this->frame(nFrame);

        if (frame < 78) {
          for (int channel = 0; channel < numChannel; channel++) {
            const vector<Frame>& bvs = bits[EvenLinkMapping[channel]];
            ss << (frame < (int)bvs.size() ? hex(bvs[frame]) : hex(Frame()));
            }
          }else {
          for (int channel = 0; channel < numChannel; channel++) {
            const vector<Frame>& bvs = bits[OddLinkMapping[channel]];
            ss << ((frame-78) < (int)bvs.size() ? hex(bvs[(frame-78)]) : hex(Frame()));
            }
          }

        for (int channel = 0; channel < voidChannel; channel++)
          ss << " 0v" << string(TTBV::S_ / 4, '0');
        ss << endl;
      }
  }

  void Demonstrator::convertNine(const vector<vector<Frame>>& bits, stringstream& ss, std::vector<int> linkmap) const {
    // reset ss

    ss.str("");
    ss.clear();
    // number of tranceiver in a quad
    static constexpr int quad = 4;
    const int numChannel = bits.size();
    std::cout << numChannel << std::endl;
    const int voidChannel = numChannel % quad == 0 ? 0 : quad - numChannel % quad;
    // start with header
    ss << header(numChannel + voidChannel,linkmap);
    int nFrame(0);
    // create one packet per region
      const int offset = 0;
      // start with emp 6 frame gap
      ss << infraGap(nFrame, numChannel + voidChannel);
      for (int frame = 0; frame < numFrames_; frame++) {
        // write one frame for all channel
        ss << this->frame(nFrame);
        for (int channel = 0; channel < numChannel; channel++) {
          const vector<Frame>& bvs = bits[channel];
          ss << (frame < (int)bvs.size() ? hex(bvs[frame]) : hex(Frame()));
        }
        for (int channel = 0; channel < voidChannel; channel++)
          ss << " 0v" << string(TTBV::S_ / 4, '0');
        ss << endl;
      }
  }

  // plays stringstream through modelsim
  void Demonstrator::sim(int ievt, const stringstream& ss, std::string fname) const {
    // write ss to disk
    fstream fs;
    std::string NewdirIn_(dirIPBB_ + fname + std::to_string(ievt) + ".txt");
    fs.open(NewdirIn_.c_str(), fstream::out);
    fs << ss.rdbuf();
    fs.close();
    // run modelsim
    stringstream cmd;
    cmd << "cd " << dirIPBB_ ;
    //cmd << "cd " << dirIPBB_ << " && ./run_sim -quiet -c work.top -do 'run " << runTime_
    //    << "us' -do 'quit' &> /dev/null";
    system(cmd.str().c_str());
  }

  // compares stringstream with modelsim output
  bool Demonstrator::compare(int ievt, stringstream& ss) const {
    // write ss to disk
    fstream fs;
    fs.open(dirPre_.c_str(), fstream::out);
    fs << ss.rdbuf();
    fs.close();
    // use linux diff on disk
    const string c = "diff " + dirPre_ + " " + dirOut_ + " &> " + dirDiff_;
    system(c.c_str());
    ss.str("");
    ss.clear();
    // read diff output
    fs.open(dirDiff_.c_str(), fstream::in);
    ss << fs.rdbuf();
    fs.close();
    // count lines, 4 are expected
    int n(0);
    string token;
    while (getline(ss, token))
      n++;
    return n == 4;
  }

  // creates emp file header
  string Demonstrator::header(int numLinks,std::vector<int> linkmap) const {
    stringstream ss;
    // file header
    ss << "Board CMSSW" << endl << " Quad/Chan :";
    // quad header
    for (int link = 0; link < numLinks; link++)
      ss << "        q" << setfill('0') << setw(2) << linkmap[link] / 4 << "c" << linkmap[link] % 4 << "      ";
    ss << endl;
    // link header
    ss << "      Link :";
    for (int link = 0; link < numLinks; link++)
      ss << "         " << setfill('0') << setw(3) << linkmap[link] << "       ";
    ss << endl;
    return ss.str();
  }

  string Demonstrator::header(int numLinks) const {
    stringstream ss;
    // file header
    ss << "Board CMSSW" << endl << " Quad/Chan :";
    // quad header
    for (int link = 0; link < numLinks; link++)
      ss << "        q" << setfill('0') << setw(2) << link / 4 << "c" << link % 4 << "      ";
    ss << endl;
    // link header
    ss << "      Link :";
    for (int link = 0; link < numLinks; link++)
      ss << "         " << setfill('0') << setw(3) << link << "       ";
    ss << endl;
    return ss.str();
  }

  // creates 6 frame gap between packets
  string Demonstrator::infraGap(int& nFrame, int numLinks) const {
    stringstream ss;
    for (int gap = 0; gap < numFramesInfra_; gap++) {
      ss << frame(nFrame);
      for (int link = 0; link < numLinks; link++)
        ss << " 0v" << string(TTBV::S_ / 4, '0');
      ss << endl;
    }
    return ss.str();
  }

  // creates frame number
  string Demonstrator::frame(int& nFrame) const {
    stringstream ss;
    ss << "Frame " << setfill('0') << setw(4) << nFrame++ << " :";
    return ss.str();
  }

  // converts bv into hex
  string Demonstrator::hex(const Frame& bv) const {
    stringstream ss;
    ss << " 1v" << setfill('0') << setw(TTBV::S_ / 4) << std::hex << bv.to_ullong();
    return ss.str();
  }

}  // namespace trackerTFP
