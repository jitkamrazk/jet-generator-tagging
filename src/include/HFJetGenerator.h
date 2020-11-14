#ifndef JETEVENTBUFFER_H
#define JETEVENTBUFFER_H

#include <string>
#include <vector>
#include <random>

#include "fastjet/ClusterSequence.hh"
#include "fastjet/PseudoJet.hh"
#include "fastjet/Selector.hh"
#include "fastjet/tools/Filter.hh"

#include "TFile.h"
#include "TTree.h"
#include "TH2F.h"
#include "TH2S.h"
#include "TMath.h"

#include "Pythia8/Pythia.h"

// Class for storing vertexing information in the FastJet.
class VertexInfo : public fastjet::PseudoJet::UserInfoBase {
public:
    float DCA_xy, DCA_x, DCA_y, DCA_z;

    VertexInfo(float DCA_xy, float DCA_x, float DCA_y, float DCA_z): DCA_xy(DCA_xy), DCA_x(DCA_x), DCA_y(DCA_y), DCA_z(DCA_z) {
    }
};



class HFJetGenerator {
public:
    HFJetGenerator(Pythia8::Pythia *pythia, Pythia8::Pythia *pythiaPileup, float pTJetMin, float pTJetMax, float pTTrackMax,
                   std::string filename, float R, float trackEff, bool pileup, bool smear);

    bool generateEvent();
    void write();

private:
    void clear();
    void processEvent(Pythia8::Pythia *pythia, std::vector<fastjet::PseudoJet> &event, bool pileup);
    float sigma_pt(float pt);
    float sigma_z(float z);
    float sigma_xy(float xy);
    float vertex_corrected_eta(float track_eta, float dca_z);

    Pythia8::Pythia *pythia, *pythiaPileup;
    float pTJetMin, pTJetMax, pTTrackMax, trackEff;
    bool pileup, smear;

    /*
     * Variables used for model training. Unfortunately std::vector seems to be having issues when reading
     * from Python environment, so we are using arrays here.
     */
    int   nTracks;
    float R;
    float fPt[100];
    float fEta[100];
    float fPhi[100];
    float fDCA_z[100];
    float fDCA_xy[100];
    float fDeltaR[100];
    float fZ[100];
    float fSIP3D[100];
    float fJetPt;
    float fJetEta;
    float fJetPhi;
    int   mTag; // 1 = light jet, 2 = c-jet, 3 = b-jet

    // Histograms that display feature distributions as well as stats.
    TH1I  *hStat;
    TH1F  *hJetPt;
    TH1F  *hJetPt_l;
    TH1F  *hJetPt_c;
    TH1F  *hJetPt_b;
    TH1F  *hConstTrackPt;
    TH1F  *hConstTrackDCA_xy;
    TH1F  *hConstTrackDCA_z;
    TH1F  *hConstTrackDCA_xy_l;
    TH1F  *hConstTrackDCA_z_l;
    TH1F  *hConstTrackDCA_xy_c;
    TH1F  *hConstTrackDCA_z_c;
    TH1F  *hConstTrackDCA_xy_b;
    TH1F  *hConstTrackDCA_z_b;
    TH1F  *hDeltaR_l;
    TH1F  *hDeltaR_c;
    TH1F  *hDeltaR_b;
    TH1F  *hZ_l;
    TH1F  *hZ_c;
    TH1F  *hZ_b;
    TH1F  *hSIP3D_l;
    TH1F  *hSIP3D_c;
    TH1F  *hSIP3D_b;

    TFile *tF;
    TTree *tT;
};

inline void HFJetGenerator::clear() {
    mTag = -999;
    nTracks = -999;
    fJetPt = -999;
    fJetEta = -999;
    fJetPhi = -999;

    for (int j = 0; j < 100; j++) {
        fPt[j] = 0;
        fEta[j] = 0;
        fPhi[j] = 0;
        fDCA_z[j] = 0;
        fDCA_xy[j] = 0;
        fDeltaR[j] = 0;
        fZ[j] = 0;
        fSIP3D[j] = 0;
    }
}

// Time Projection Chamber pt smearing function, taken from http://physics.fjfi.cvut.cz/publications/ejcf/DIS_Jan_Rusnak.pdf
inline float HFJetGenerator::sigma_pt(float pt) {
    return 0.003*pt*pt;
}

// Heavy-Flavor Tracker DCA_xy smearing, fitted from https://arxiv.org/pdf/1812.10224.pdf Fig. 2
inline float HFJetGenerator::sigma_xy(float xy) {
    if (xy > 2.5) {
        return 0.02;
    } else {
        return 0.028081/(-0.0364787 + xy) + 0.00564347;
    }
}

// Heavy-Flavor Tracker DCA_z smearing, fitted from https://arxiv.org/pdf/1812.10224.pdf Fig. 2
inline float HFJetGenerator::sigma_z(float z) {
    if (z > 2.5) {
        return 0.02;
    } else {
        return 0.0313573/(-0.0187011 + z) + 0.00782682;
    }
}

inline void HFJetGenerator::write() {
    hStat->Write();
    hJetPt->Write();
    hJetPt_l->Write();
    hJetPt_c->Write();
    hJetPt_b->Write();
    hConstTrackPt->Write();
    hConstTrackDCA_z->Write();
    hConstTrackDCA_xy->Write();
    hConstTrackDCA_z_l->Write();
    hConstTrackDCA_xy_l->Write();
    hConstTrackDCA_z_c->Write();
    hConstTrackDCA_xy_c->Write();
    hConstTrackDCA_z_b->Write();
    hConstTrackDCA_xy_b->Write();
    hDeltaR_l->Write();
    hDeltaR_c->Write();
    hDeltaR_b->Write();
    hZ_l->Write();
    hZ_c->Write();
    hZ_b->Write();
    tT->Write();
    tF->Close();
}

inline float HFJetGenerator::vertex_corrected_eta(float track_eta, float dca_z) {
    double track_theta = 2.0 * atan(exp(-track_eta));
    double z = 0.0;
    if (track_eta != 0.0) z = 2000 / tan(track_theta);
    double z_diff = z - dca_z;
    double theta_corr = atan2(2000, z_diff);
    double eta_corr = -log(tan(theta_corr / 2.0));
    return eta_corr;
}

#endif