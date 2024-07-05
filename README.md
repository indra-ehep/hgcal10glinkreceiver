# hgcal-tpg-fe

## Motivation
Emulation of the Trigger Primitive Generation (TPG) by HGCAL frontend ASICs.

## Download and Setup
Run the setup code to download additional dependencies and setup. This setup is prepared for lxplus machines.

```
./setup.sh
```

## Compile and run

### Test-beam data of September 2023
To process the test-beam binary files for the BC results
```
./compile.sh
./emul_test-beam_Sep23.exe 1695829026 1695829027 1
```
The arguments from left to right are relay number, run number and link number, respectively.
Similarly for the STC4 results
```
./emul_test-beam_Sep23.exe 1695733045 1695733046 1 
```

### Estimation of maximum energy output of ECONT
Modify the module setup inside the code,
```cpp
  //===============================================================================================================================
  //input paramaters for testing
  //===============================================================================================================================
  uint32_t zside = 0, sector = 0, link = 0, econt = 0;
  
  //A combinatin of following three will uniquiely identfy the module type in HGCAL
  //(outlook : some modification expected for the ECONTs connected to different types of modules, which are not considered in current tests)
  //det: (0,1) = (Si,Sci)
  //selTC4: (0,1) = (HD,LD);
  //module:
  //Si:==
  //LD: Full (F/0), Top (T/1), Bottom (B/2), Left (L/3), Right (R/4), Five (5/5)
  //HD: Full (F/0), Top (T/1) aka Half Minus, Bottom (B/2) aka Chop Two, Left (L/3), Right(R/4)
  //Sci:==
  //LD: A5A6(0), B11B12(1), C5(2), D8(3), E8(4), G3(5), G5(6), G7(7), G8(8)
  //HD: K6(0), K8(1), K11(2), K12(3), J12(4)
  uint32_t det = 0, selTC4 = 1, module = 0;
  
  uint32_t multfactor = 15;//TOT mult factor other values could be 31 or 8
  uint32_t inputLSB = 1; //lsb at the input TC from ROC
  uint32_t dropLSB = 1;  //lsb at the output during the packing
  uint32_t select = 1 ;  //0 = Threshold Sum (TS), 1 = Super Trigger Cell (STC), 2 = Best Choice (BC), 3 = Repeater, 4=Autoencoder (AE).
  uint32_t stc_type = 0; // 0 = STC4B(5E+4M), 1 = STC16(5E+4M), 3 = STC4A(4E+3M)
  uint32_t nelinks = 4; //1 = BC1, 2 = BC4, 3 = BC6, 4 = BC9, 5 = BC14..... (see details https://edms.cern.ch/ui/#!master/navigator/document?P:100053490:100430098:subDocs)
  uint32_t calibration = 0xFFF; //0x400 = 0.5, 0x800 = 1.0, 0xFFF = 1.99951 (max)
  
  uint32_t maxADC = 0x3FF ; //10 bit input in TPG path
  uint32_t maxTOT = 0xFFF ; //12 bit input in TPG path
  //===============================================================================================================================
```

then compile again and run as,
```
./compile.sh
./findEMax.exe
```
