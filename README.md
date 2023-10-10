# Hgcal10gLinkReceiver

## Download
```
git clone https://github.com/indra-ehep/hgcal10glinkreceiver
```

## Compile

```
cd hgcal10glinkreceiver
./compile.sh
```

## Execute

```
[daq@xxxxxxx hgcal10glinkreceiver]$ ./run.sh 
The relay numbers are : 
  { 1695548224, 1695548616, 1695548665, 1695550448, 1695552466, 1695553649, 1695558537, 1695558789, 1695559474, 1695559528, 1695560008, 1695562204, 1695562836, 1695563152, 1695563673, 1695564190, 1695564694, 1695565177, 1695565613, 1695566235, 1695568332, 1695573718, 1695573756, 1695587646, 1695587722, 1695592800, 1695593201, 1695656599, 1695657132, 1695658759, }
Select a relay number 
1695573756


 ============== Relay : 1695573756 Run : 1695573756 ================= 
FileReader::open()  opening file dat/Relay1695573756/Run1695573756_Link0_File0000000000.bin
Event : 352781 Event size do not match between trigger RO header and fourth 0xfecafe word
	Prev :: Event ID :  352780, OC[LSB] : 0, OC[MSB] : 0, BC[LSB] : 16, BC[MSB] : 2304
	Prev :: Event ID :  352779, OC[LSB] : 0, OC[MSB] : 0, BC[LSB] : 16, BC[MSB] : 2304
	Prev :: Event ID :  352778, OC[LSB] : 0, OC[MSB] : 0, BC[LSB] : 16, BC[MSB] : 2304
	Prev :: Event ID :  352777, OC[LSB] : 0, OC[MSB] : 0, BC[LSB] : 16, BC[MSB] : 2304
	Prev :: Event ID :  352776, OC[LSB] : 0, OC[MSB] : 0, BC[LSB] : 16, BC[MSB] : 2304
	Prev :: Event ID :  352775, OC[LSB] : 0, OC[MSB] : 0, BC[LSB] : 16, BC[MSB] : 2304
	Prev :: Event ID :  352774, OC[LSB] : 0, OC[MSB] : 0, BC[LSB] : 16, BC[MSB] : 2304
	Prev :: Event ID :  352773, OC[LSB] : 0, OC[MSB] : 0, BC[LSB] : 16, BC[MSB] : 2304
	Prev :: Event ID :  352772, OC[LSB] : 0, OC[MSB] : 0, BC[LSB] : 16, BC[MSB] : 2304
	Prev :: Event ID :  352771, OC[LSB] : 0, OC[MSB] : 0, BC[LSB] : 16, BC[MSB] : 2304
	.........................

```

Remeber to set the dat softlink path correctly according to your requirment.

## Output summary

The output summary of error will contain,


```
================================================================================
Summary of Relay 1695819718 and Run 1695819718

================================================================================
Relay	 Run	 NofEvts	 NofPhysT	 NofCalT	 NofCoinT	 NofRandT	 NofSoftT	 NofRegT	 RStrtE	 RStpE	 EvtIdE	 1stcafeE	 daqHE	 NbxE	 STCNumE	 STCLocE	 EngE	 EmptyTCs	
1695819718	1695819718	458638	0	0	0	458638	0	0	0	0	0	0	0	0	0	0	0	3	

================================================================================

```
where,

1. Relay : Relay number
2. Run : Run number
3. NofEvts : Number of events
4. PhysT : Number of physics triggers (L1a Trigger types)
5. CalT : Number of calibration triggers (L1a Trigger types)
6. CoinT : Number of coincident triggers of Phys and Cal (L1a Trigger types)
7. CalT : Number of random triggers (L1a Trigger types)
8. SoftT : Number of software triggers (L1a Trigger types)
9. RegT : Number of regular triggers (L1a Trigger types)
10. RStrE : Number of cases where FsmState::Starting validadity fails
11. RStpE : Number of cases where FsmState::Stopping validadity fails
12. EvtIdE : Number of cases where Event Id is corrupted 
13. xecafeE : Number of events where payload has six or more 0xfecafe
14. 1stcafeE : Number of cases where the location of first 0xfecafe word is wrong (This means either the event header has been expanded or the first is missed)
15. daqHE : Number of cases where the data volume mentioned in TRG LO header does not match with the last 8-bits of 0xfecafe word
16. NbxE : Number of cases where the bx mentioned in daq0 info of TRG LO does not match with the one mentioned for daq1
17. STCNE : Number of cases where STC number does not match with the one mentioned in the 4 MSB bits of unpacked STC address for LSB module
18. STCLE : Number of cases where STC location mentioned in the 2 LSB bits of unpacked STC address does not match with the one mentioned unpacker input location for LSB module
19. EngE : Number of cases where STC energy mentioned in the unpacked data does not match with the one in unpacker input for LSB module
20. BxE : Number of cases with Bx mismatching between unpacker input and output dat for LSB module
21. BxCE : Number of cases with Bx mismatching between central value stored unpacker input/output and those from 8 modulo bxId of Slink trailer for LSB module
22. ETCs : Number of empty TCs for LSB trigger data 

23.-28. : Repetition of 17 to 22 for MSB module

29. PV : Serenity software version 

### Acknowledgment
Main framework :: Paul : https://gitlab.cern.ch/pdauncey/hgcal10glinkreceiver

Additional guidelines :: Charlotte : https://gitlab.cern.ch/mknight/hgcal10glinkreceiver/-/tree/master 



