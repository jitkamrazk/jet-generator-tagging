#include <string>
#include <vector>
#include <random>
#include <unistd.h>

#include "fastjet/ClusterSequence.hh"
#include "fastjet/PseudoJet.hh"
#include "fastjet/Selector.hh"
#include "fastjet/tools/Filter.hh"

#include "Pythia8/Pythia.h"

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TMath.h"

#include "parser.hh"
#include "HFJetGenerator.h"

using namespace std;

int get_seed(int seed) {
    if (seed > -1) {
        return seed;
    }
    int timeSeed = time(NULL);
    return abs(((timeSeed * 181) * ((time(NULL) - 83) * 359)) % 104729); //getpid()
}

int main(int argc, const char *argv[]) {
    // arguments
    string outName, configPath;
    float pTHatMin = 10;
    float pTHatMax = -1;
    float pTJetMin = 10;
    float pTJetMax = 200;
    float pTTrackMax = 30;
    float R = 0.4;
    float trackEff = 0.8;
    bool smear = false;
    bool pileup = false;

    optionparser::parser parser("Allowed options");

    parser.add_option("--out-file", "-o")
            .mode(optionparser::store_value)
            .default_value("test.root")
            .help("output file name");
    parser.add_option("--config", "-c")
            .mode(optionparser::store_value)
            .default_value("test.root")
            .help("output file name");
    parser.add_option("--r")
            .mode(optionparser::store_value)
            .default_value(0.4)
            .help("Jet radius");
    parser.add_option("--pt-hat-min")
            .mode(optionparser::store_value)
            .default_value(10)
            .help("minimum pT hat value of a process");
    parser.add_option("--pt-hat-max")
            .mode(optionparser::store_value)
            .default_value(-1)
            .help("minimum pT hat value of a process");
    parser.add_option("--pt-jet-min")
            .mode(optionparser::store_value)
            .default_value(10)
            .help("minimum jet pT");
    parser.add_option("--pt-jet-max")
            .mode(optionparser::store_value)
            .default_value(200)
            .help("maximum jet pT");
    parser.add_option("--pt-track-max")
            .mode(optionparser::store_value)
            .default_value(30)
            .help("Maximum pT of accepted tracks");
    parser.add_option("--tracking-efficiency")
            .mode(optionparser::store_value)
            .default_value(0.8)
            .help("Tracking efficiency");
    parser.add_option("--smear")
            .mode(optionparser::store_true)
            .help("Use fastsim in simulation");
    parser.add_option("--pileup")
            .mode(optionparser::store_true)
            .help("Use pileup in simulation");

    parser.eat_arguments(argc, argv);

    outName = parser.get_value<string>("outfile");
    configPath = parser.get_value<string>("config");
    pTHatMin = parser.get_value<float>("pthatmin");
    pTHatMax = parser.get_value<float>("pthatmax");
    pTJetMin = parser.get_value<float>("ptjetmin");
    pTJetMax = parser.get_value<float>("ptjetmax");
    pTTrackMax = parser.get_value<float>("pttrackmax");
    trackEff = parser.get_value<float>("trackingefficiency");
    smear = parser.get_value<bool>("smear");
    pileup = parser.get_value<bool>("pileup");

    R = parser.get_value<float>("r");

    Pythia8::Pythia *pythia = new Pythia8::Pythia();
    pythia->readFile(configPath);
    stringstream hatMin;
    stringstream hatMax;
    std::stringstream randomSeed;

    // This seems to give a good random seeds when running job arrays
    int seed = abs(((time(nullptr) * 181) * ((getpid() - 83) * 359)) % 104729);

    hatMin << "PhaseSpace:pTHatMin = " << pTHatMin;
    hatMax << "PhaseSpace:pTHatMax = " << pTHatMax;
    randomSeed << "Random:seed = " << seed;

    pythia->readString(hatMin.str());
    pythia->readString(hatMax.str());
    pythia->readString(randomSeed.str());
    pythia->init();

    Pythia8::Pythia *pythiaPileup = new Pythia8::Pythia();
    std::stringstream randomSeedPileup;
    int seedPileup = abs(((time(nullptr) * 624) * ((getpid() - 11) * 677)) % 104729);
    pythiaPileup->readString("SoftQCD:nonDiffractive = on");
    pythiaPileup->readString("HardQCD:all = off");
    pythiaPileup->readString("Random:setSeed = on");
    randomSeedPileup << "Random:seed = " << seedPileup;
    pythiaPileup->readString(randomSeedPileup.str());
    pythiaPileup->init();

    int nEvent = pythia->mode("Main:numberOfEvents");

    HFJetGenerator hfJetGenerator(pythia, pythiaPileup, pTJetMin, pTJetMax, pTTrackMax, outName, R, trackEff, pileup, smear);

    for (int evt = 0; evt < nEvent; evt++) {
        // We accept event only if it has required jets, i.e. 100 accepted events != 100 generated events
        while (true) {
            if (hfJetGenerator.generateEvent()) break;
        }

        if (evt % 1000 == 0) cout << "1000 events accepted" << endl;
    }

    hfJetGenerator.write();
    pythia->stat();

    return 0;
}