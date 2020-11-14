# Heavy-Flavor Jet Dataset Generator
This code is used to generate heavy-flavour jet dataset for [JetVLAD model](https://github.com/ponimatkin/NetVLAD-tagger-pytorch). The code is based on the [lukedeo/jet-generation](https://github.com/lukedeo/jet-generation) 
and [lukedeo/option-parser](https://github.com/lukedeo/option-parser/) repositories.

### Requirements
In order to compile this code the following packages are needed to be availabe in your `$PATH` environment variable:
1. `Pythia8` MC generator
2. `FastJet` code for jet clutering
3. `ROOT` library for exporting data

### Compilation
To compile the code just type `make`. If you would like to clean directory type `make clean` and to remove all stuff related to the compilation type `make purge`. 

 
### Running the code
After compilation you can find executable in the `/bin` folder. By typing `./bin/jet-generator --help` you can can see the list of all acceptable arguments. You can edit one of the `*.cmnd` files in `/configs` folder or write a 
new one in order to customize generator settings.

### Reproducing the dataset
The following table summarizes parameters used to generate the training, testing and validation datasets in the paper.

#### Cross-section weighted dataset
| jet pT [GeV/*c*]  | minimal ptHat [GeV/*c*]  | maximal ptHat [GeV/*c*] | maximum track pT [GeV/*c*] | jet Radius  | config | number of events with jets train/test/val |
|---|---|---|---|---|---|---|
| 5-10   | 3   | 12 | 60 | 0.4  | configs/hardqcd.cmnd  |  5х10<sup>6</sup>/5х10<sup>5</sup>/5х10<sup>5</sup> |
| 10-15   | 8   | 17 | 60 | 0.4  | configs/hardqcd.cmnd  |  5х10<sup>6</sup>/5х10<sup>5</sup>/5х10<sup>5</sup> |
| 15-20   | 13   | 22 | 60 | 0.4  | configs/hardqcd.cmnd  |  5х10<sup>6</sup>/5х10<sup>5</sup>/5х10<sup>5</sup> |
| 20-25   | 18   | 27 | 60 | 0.4  | configs/hardqcd.cmnd  |  5х10<sup>6</sup>/5х10<sup>5</sup>/5х10<sup>5</sup> |
| 25-40   | 23   | 42 | 60 | 0.4  | configs/hardqcd.cmnd  |  5х10<sup>6</sup>/5х10<sup>5</sup>/5х10<sup>5</sup> |

#### Balanced dataset
| jet pT [GeV/*c*]  | minimal ptHat [GeV/*c*]  | maximal ptHat [GeV/*c*] | maximum track pT [GeV/*c*] | jet Radius  | config | number of events with jets train/test/val |
|---|---|---|---|---|---|---|
| 5-10   | 3   | 12 | 60 | 0.4  | configs/charm.cmnd  |  1.25х10<sup>6</sup>/1.25х10<sup>5</sup>/1.25х10<sup>5</sup> |
| 5-10   | 3   | 12 | 60 | 0.4  | configs/bottom.cmnd  |  1.25х10<sup>6</sup>/1.25х10<sup>5</sup>/1.25х10<sup>5</sup> |
| 5-10   | 3   | 12 | 60 | 0.4  | configs/light.cmnd  |  2.5х10<sup>6</sup>/2.5х10<sup>5</sup>/2.5х10<sup>5</sup> |
| 10-15   | 8   | 17 | 60 | 0.4  | configs/charm.cmnd  |  1.25х10<sup>6</sup>/1.25х10<sup>5</sup>/1.25х10<sup>5</sup> |
| 10-15   | 8   | 17 | 60 | 0.4  | configs/bottom.cmnd  |  1.25х10<sup>6</sup>/1.25х10<sup>5</sup>/1.25х10<sup>5</sup> |
| 10-15   | 8   | 17 | 60 | 0.4  | configs/light.cmnd  |  2.5х10<sup>6</sup>/2.5х10<sup>5</sup>/2.5х10<sup>5</sup> |
| 15-20   | 13   | 22 | 60 | 0.4  | configs/charm.cmnd  |  1.25х10<sup>6</sup>/1.25х10<sup>5</sup>/1.25х10<sup>5</sup> |
| 15-20   | 13   | 22 | 60 | 0.4  | configs/bottom.cmnd  |  1.25х10<sup>6</sup>/1.25х10<sup>5</sup>/1.25х10<sup>5</sup> |
| 15-20   | 13   | 22 | 60 | 0.4  | configs/light.cmnd  |  2.5х10<sup>6</sup>/2.5х10<sup>5</sup>/2.5х10<sup>5</sup> |
| 20-25   | 18   | 27 | 60 | 0.4  | configs/charm.cmnd  |  1.25х10<sup>6</sup>/1.25х10<sup>5</sup>/1.25х10<sup>5</sup> |
| 20-25   | 18   | 27 | 60 | 0.4  | configs/bottom.cmnd  |  1.25х10<sup>6</sup>/1.25х10<sup>5</sup>/1.25х10<sup>5</sup> |
| 20-25   | 18   | 27 | 60 | 0.4  | configs/light.cmnd  |  2.5х10<sup>6</sup>/2.5х10<sup>5</sup>/2.5х10<sup>5</sup> |
| 25-40   | 23   | 42 | 60 | 0.4  | configs/charm.cmnd  |  1.25х10<sup>6</sup>/1.25х10<sup>5</sup>/1.25х10<sup>5</sup> |
| 25-40   | 23   | 42 | 60 | 0.4  | configs/bottom.cmnd  |  1.25х10<sup>6</sup>/1.25х10<sup>5</sup>/1.25х10<sup>5</sup> |
| 25-40   | 23   | 42 | 60 | 0.4  | configs/light.cmnd  |  2.5х10<sup>6</sup>/2.5х10<sup>5</sup>/2.5х10<sup>5</sup> |

Datasets in the same jet pT ranges are merged together.
