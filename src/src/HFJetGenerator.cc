#include "HFJetGenerator.h"

HFJetGenerator::HFJetGenerator(Pythia8::Pythia *pythia, Pythia8::Pythia *pythiaPileup,
                               float pTJetMin, float pTJetMax, float pTTrackMax, std::string filename, float R,
                               float trackEff, bool pileup, bool smear):
        pythia(pythia), pythiaPileup(pythiaPileup), pTJetMin(pTJetMin), pTJetMax(pTJetMax), pTTrackMax(pTTrackMax), R(R),
        trackEff(trackEff), pileup(pileup), smear(smear) {
    tF = new TFile(Form("%s.root", filename.c_str()), "RECREATE");
    int BufSize = (int)pow(2., 16.);
    tT = new TTree("T", "T", BufSize);
    tT->SetAutoSave(500000); // autosave every 0.5 Mbytes

    // Initialized histograms that will hold feature distributions as well as different stats
    hStat = new TH1I("hStat", "", 3, 0, 2);
    hJetPt = new TH1F("hJetPt", ";p_{t}", 200, 0, 100);
    hJetPt_l = new TH1F("hJetPt_l", ";p_{t}", 200, 0, 100);
    hJetPt_c = new TH1F("hJetPt_c", ";p_{t}", 200, 0, 100);
    hJetPt_b = new TH1F("hJetPt_b", ";p_{t}", 200, 0, 100);
    hConstTrackPt = new TH1F("hConstTrackPt", "", 200, 0, 100);
    hConstTrackDCA_z = new TH1F("hConstTrackDCA_z", "", 4000, -100, 100);
    hConstTrackDCA_xy = new TH1F("hConstTrackDCA_xy", "", 4000, 0, 100);
    hConstTrackDCA_z_l = new TH1F("hConstTrackDCA_z_l", "", 4000, -100, 100);
    hConstTrackDCA_xy_l = new TH1F("hConstTrackDCA_xy_l", "", 4000, 0, 100);
    hConstTrackDCA_z_c = new TH1F("hConstTrackDCA_z_c", "", 4000, -100, 100);
    hConstTrackDCA_xy_c = new TH1F("hConstTrackDCA_xy_c", "", 4000, 0, 100);
    hConstTrackDCA_z_b = new TH1F("hConstTrackDCA_z_b", "", 4000, -100, 100);
    hConstTrackDCA_xy_b = new TH1F("hConstTrackDCA_xy_b", "", 4000, 0, 100);
    hDeltaR_l = new TH1F("hDeltaR_l", "", 1000, 0, 1);
    hDeltaR_c = new TH1F("hDeltaR_c", "", 1000, 0, 1);
    hDeltaR_b = new TH1F("hDeltaR_b", "", 1000, 0, 1);
    hZ_l = new TH1F("hZ_l", "", 1000, 0, 1);
    hZ_c = new TH1F("hZ_c", "", 1000, 0, 1);
    hZ_b = new TH1F("hZ_b", "", 1000, 0, 1);
    hSIP3D_l = new TH1F("hSIP3D_l", "", 4000, -100, 100);
    hSIP3D_c = new TH1F("hSIP3D_c", "", 4000, -100, 100);
    hSIP3D_b = new TH1F("hSIP3D_b", "", 4000, -100, 100);

    // Initialize branches of TTree
    tT->Branch("nTracks", &nTracks, "nTracks/I");
    tT->Branch("fPt", fPt, "fPt[nTracks]/F");
    tT->Branch("fEta", fEta, "fEta[nTracks]/F");
    tT->Branch("fPhi", fPhi, "fPhi[nTracks]/F");
    tT->Branch("fDCA_z", fDCA_z, "fDCA_z[nTracks]/F");
    tT->Branch("fDCA_xy", fDCA_xy, "fDCA_xy[nTracks]/F");
    tT->Branch("fDeltaR", fDeltaR, "fDeltaR[nTracks]/F");
    tT->Branch("fZ", fZ, "fZ[nTracks]/F");
    tT->Branch("fSIP3D",fSIP3D, "fSIP3D[nTracks]/F");
    tT->Branch("mTag", &mTag, "mTag/I");
    tT->Branch("fJetPt", &fJetPt, "fJetPt/F");
    tT->Branch("fJetPhi", &fJetPhi, "fJetPhi/F");
    tT->Branch("fJetEta", &fJetEta, "fJetEta/F");

    // Reset arrays that are going into the TTree
    clear();
}

bool HFJetGenerator::generateEvent() {
    if (!pythia->next()) return false;
    if (pileup && !pythiaPileup->next()) return false;

    std::vector<fastjet::PseudoJet> event;

    processEvent(pythia, event, false);
    if (pileup) {
        processEvent(pythiaPileup, event, true);
    }

    // Jet clustering
    fastjet::JetDefinition jet_def(fastjet::antikt_algorithm, R);
    fastjet::ClusterSequence cs(event, jet_def);
    fastjet::Selector selector = fastjet::SelectorAbsEtaMax(1 - R)*fastjet::SelectorPtRange(pTJetMin, pTJetMax);
    std::vector<fastjet::PseudoJet> jets = selector(cs.inclusive_jets(5.0));

    if (jets.size() == 0) return false;

    for (int i = 0; i < jets.size(); i++) {
        fastjet::PseudoJet jet = jets[i];
        fJetPt = jet.pt();
        fJetEta = jet.eta();
        fJetPhi = jet.phi();
        hJetPt->Fill(fJetPt);

        if (jet.constituents().size() == 1) continue; // We accept jets with > 1 particles

        /*
         * Matching strategy:
         * 1. Go over every parton in the event
         * 2. Verify that parton has status code 23 (i.e. outgoing of the hard scattering)
         * 3. Make sure the parton lies within |eta| < 1.0 for detector constraints
         * 4. Match to jet within jet radius, the jet flavor is assumed to be of the heavies status code 23 parton within radius
         */
        std::vector<int> matches;
        for (int i = 0; i < pythia->event.size(); i++) {
            if (!pythia->event[i].isParton()) continue;
            if (abs(pythia->event[i].status()) != 23) continue;
            fastjet::PseudoJet tagCandidate(pythia->event[i].px(), pythia->event[i].py(), pythia->event[i].pz(), pythia->event[i].e());
            if (abs(tagCandidate.eta()) > 1.0) continue;

            int pid = abs(pythia->event[i].id());
            if (jet.delta_R(tagCandidate) > R) continue;
            if (pid == 5) {
                matches.push_back(3);
            } else if (pid == 4) {
                matches.push_back(2);
            } else if (pid == 1 || pid == 2 || pid == 3 || pid == 21) {
                matches.push_back(1);
            }
        }

        if (!matches.empty()) {
            int final_tag = *std::max_element(std::begin(matches), std::end(matches)); // Choose highest tag (i.e. heaviest tag parton)
            mTag = final_tag;

            if (mTag == 3) {
                hJetPt_b->Fill(jet.pt());
            } else if (mTag == 2) {
                hJetPt_c->Fill(jet.pt());
            } else if (mTag == 1) {
                hJetPt_l->Fill(jet.pt());
            }

            hStat->Fill(1);

            // Fill features used by the model to train
            std::vector<fastjet::PseudoJet> constituents = jet.constituents();
            nTracks = constituents.size();
            for (int j = 0; j < nTracks; j++) {
                float dca_x = constituents[j].user_info<VertexInfo>().DCA_x;
                float dca_y = constituents[j].user_info<VertexInfo>().DCA_y;
                float dca_z = constituents[j].user_info<VertexInfo>().DCA_z;
                float dca_xy = constituents[j].user_info<VertexInfo>().DCA_xy;
                float deltaR = constituents[j].delta_R(jet);
                float sip3d = TMath::Sign(1, dca_x*jet.pt() + dca_y*jet.eta() + dca_z*jet.phi())*sqrt(dca_x*dca_x + dca_y*dca_y + dca_z*dca_z)/sigma_z(constituents[j].modp());

                if (mTag == 1) {
                    hConstTrackDCA_z_l->Fill(abs(dca_z));
                    hConstTrackDCA_xy_l->Fill(dca_xy);
                    hDeltaR_l->Fill(deltaR);
                    hZ_l->Fill(constituents[j].pt() / jet.pt());
                    hSIP3D_l->Fill(sip3d);
                } else if (mTag == 2) {
                    hConstTrackDCA_z_c->Fill(abs(dca_z));
                    hConstTrackDCA_xy_c->Fill(dca_xy);
                    hDeltaR_c->Fill(deltaR);
                    hZ_c->Fill(constituents[j].pt() / jet.pt());
                    hSIP3D_c->Fill(sip3d);
                } else if (mTag == 3) {
                    hConstTrackDCA_z_b->Fill(abs(dca_z));
                    hConstTrackDCA_xy_b->Fill(dca_xy);
                    hDeltaR_b->Fill(deltaR);
                    hZ_b->Fill(constituents[j].pt() / jet.pt());
                    hSIP3D_b->Fill(sip3d);
                }

                fPt[j] = constituents[j].pt();
                fEta[j] = constituents[j].eta();
                fPhi[j] = constituents[j].phi();
                fZ[j] = constituents[j].pt()/jet.pt();
                fDeltaR[j] = deltaR;
                fSIP3D[j] = sip3d;
                fDCA_xy[j] = dca_xy;
                fDCA_z[j] = dca_z;

            }
            tT->Fill();
        } else {
            clear();
            return false;
        }
        clear();
    }

    return true;
}

void HFJetGenerator::processEvent(Pythia8::Pythia *pythia, std::vector<fastjet::PseudoJet> &event, bool is_pileup) {
    // Random generators needed for the efficiency and pileup simulations
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_real_distribution<float> track_eff(0.0, 1.0);
    std::uniform_real_distribution<float> pileup_vtx(-60.0, 60.0);

    float pileup_event_vtx = pileup_vtx(generator);

    for (int i = 0; i < pythia->event.size(); i++) {
        if (!(pythia->event[i].isFinal() && pythia->event[i].isCharged())) continue;
        if (smear && (track_eff(generator) > trackEff)) continue; // Reject tracks with given probability

        float pt, p, eta, phi, dca_x, dca_y, dca_z, dca_xy;
        dca_x = pythia->event[i].xProd();
        dca_y = pythia->event[i].yProd();
        if (is_pileup) {
            dca_z = pythia->event[i].zProd() + pileup_event_vtx;
        } else {
            dca_z = pythia->event[i].zProd();
        }

        pt = pythia->event[i].pT();
        p = pythia->event[i].pAbs();
        phi = pythia->event[i].phi();

        if (is_pileup) {
            eta = vertex_corrected_eta(pythia->event[i].eta(), dca_z);
        } else {
            eta = pythia->event[i].eta();
        }

        dca_xy = sqrt(pythia->event[i].xProd()*pythia->event[i].xProd() + pythia->event[i].yProd()*pythia->event[i].yProd());

        if (smear) {
            // Observable smearings
            std::normal_distribution<float> pt_smearing(pt, sigma_pt(pt));
            std::normal_distribution<float> x_smearing(dca_x, sigma_z(p));
            std::normal_distribution<float> y_smearing(dca_y, sigma_z(p));
            std::normal_distribution<float> z_smearing(dca_z, sigma_z(p));
            std::normal_distribution<float> xy_smearing(dca_xy, sigma_xy(p));

            pt = pt_smearing(generator);
            dca_x = x_smearing(generator);
            dca_y = y_smearing(generator);
            dca_z = z_smearing(generator);
            dca_xy = xy_smearing(generator);
        }

        // Make sure that observables are accepted
        if (pt > pTTrackMax || pt < 0.2 || abs(eta) > 1.0 || abs(dca_z) > 60 || abs(dca_xy) > 20) continue;

        hConstTrackPt->Fill(pt);
        hConstTrackDCA_z->Fill(dca_z);
        hConstTrackDCA_xy->Fill(dca_xy);

        fastjet::PseudoJet track;
        track.reset_momentum_PtYPhiM(pt, eta, phi, 0.139570); //assume that track has pion mass
        track.set_user_info(new VertexInfo(abs(dca_xy), dca_x, dca_y, dca_z));
        event.push_back(track);
    }
}