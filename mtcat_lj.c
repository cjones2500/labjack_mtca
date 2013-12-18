#include "u3.h"
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>

const struct {
	unsigned char crate[20];   //human bit mask to front panel in !cable labels! cable 13 connects crate 12, and bit 12
	unsigned short reg[4];     //DS_A, DS_B, CLK, RST*
	unsigned char dev_number;  //labjack devNumber
} kMTCATcard[7]={
{{8, 11, 1, 19, 6, 17, 5, 12, 2, 9,
3, 14, 18, 7, 4, 10, 13, 20, 15, 16},
{1, 0, 2, 3}, 1}, //N100
{{5, 3, 18, 14, 20, 11, 17, 8, 6, 10,
12, 2, 7, 9, 19, 13, 4, 1, 16, 15},
{5, 4, 6, 7}, 1}, //N20
{{8, 11, 10, 19, 6, 17, 5, 12, 2, 9,
20, 3, 18, 7, 4, 1, 13, 14, 15, 16},
{9, 8, 14, 15}, 1}, //ESUM_LO
{{8, 11, 10, 19, 6, 17, 5, 12, 2, 9,
20, 3, 18, 14, 4, 7, 13, 1, 15, 16},
{11, 10, 12, 13}, 1}, //ESUM_HI
//for OWLs only crates 3, 13, 18 are connected
//crate 17 is Cherenkov calibration source
{{20, 20, 20, 18, 20, 19, 20, 20, 20, 20,
14, 20, 20, 20, 4, 20, 20, 20, 20, 20},
{17, 16, 18, 19}, 1}, //OWLE_LO
{{20, 20, 20, 18, 20, 19, 20, 20, 20, 20,
14, 20, 20, 20, 4, 20, 20, 20, 20, 20},
{1, 0, 2, 3}, 2}, //OWLE_HI
{{20, 18, 20, 19, 20, 20, 20, 14, 20, 20,
20, 20, 20, 20, 20, 20, 20, 20, 20, 4},
{5, 4, 6, 7}, 2}, //OWLE_N bit32 is the N16 cable
};

int reset_mtcat(unsigned char mtcat_id) //pull down to reset and enable all, pull up to enable clocking
{
	HANDLE hDevice = 0;
	long error = 0;

	hDevice = LJUSB_OpenDevice(kMTCATcard[mtcat_id].dev_number, 0, 3);
	if (hDevice == NULL) return -1;

	if ((error = eDO(hDevice, 1, kMTCATcard[mtcat_id].reg[3], 0)) != 0) goto out;
	usleep(10000);
	if ((error = eDO(hDevice, 0, kMTCATcard[mtcat_id].reg[3], 1)) != 0) goto out;

out:
	LJUSB_CloseDevice(hDevice);
	return (int)error;
}

int reset_all()
{
	HANDLE hDevice = 0;
	long error = 0;
	unsigned char dev_number = kMTCATcard[0].dev_number;
	hDevice = LJUSB_OpenDevice(dev_number, 0, 3);
	if (hDevice == NULL) return -1;

	unsigned char i = 0;
	for (i=0; i<7; i++) {
		if (kMTCATcard[i].dev_number != dev_number) {
			LJUSB_CloseDevice(hDevice);
			dev_number = kMTCATcard[i].dev_number;
			hDevice = LJUSB_OpenDevice(dev_number, 0, 3);
			if (hDevice == NULL) return -1;
		}

		if ((error = eDO(hDevice, 1, kMTCATcard[i].reg[3], 0)) != 0) goto out;
		usleep(1000);
		if ((error = eDO(hDevice, 0, kMTCATcard[i].reg[3], 1)) != 0) goto out;
	}

out:
	LJUSB_CloseDevice(hDevice);
	return (int)error;
return 0;
}

int load_crate_mask(unsigned int crate_mask, unsigned char mtcat_id) //crate_mask bit number == crate number, 1 == enable
{
	HANDLE hDevice = 0;
	long error = 0;

	hDevice = LJUSB_OpenDevice(kMTCATcard[mtcat_id].dev_number, 0, 3);
	if (hDevice == NULL) return -1;

	uint32_t cable_mask = 0;
	short i = 0;
	for (i = 0; i < 20; i++) {
		short j = 0;
		while (i + 1 != kMTCATcard[mtcat_id].crate[j] && j < 20) j++;
		cable_mask |= ((crate_mask >> i) & 0x1) << j;
	}

    //QUICK FIX MTCA OWL ESUM LO
    //ignore UI, always open all
    if (mtcat_id == 4) {
        cable_mask = 0xfffff;
    }

	cable_mask = ~cable_mask; //0 enables the crate

	//DS_A low, CLK low, DS_B high, 
	if ((error = eDO(hDevice, 1, kMTCATcard[mtcat_id].reg[0], 0)) != 0) goto out;
	if ((error = eDO(hDevice, 1, kMTCATcard[mtcat_id].reg[2], 0)) != 0) goto out;
	if ((error = eDO(hDevice, 1, kMTCATcard[mtcat_id].reg[1], 1)) != 0) goto out;

	for (i=19; i>=0; i--) { //DS_A mask, sleep, CLK high, sleep, CLK low
		if ((error = eDO(hDevice, 0, kMTCATcard[mtcat_id].reg[0], (cable_mask >> i) & 0x1)) != 0) goto out;
		usleep(1000);
		if ((error = eDO(hDevice, 0, kMTCATcard[mtcat_id].reg[2], 1)) != 0) goto out;
		usleep(1000);
		if ((error = eDO(hDevice, 0, kMTCATcard[mtcat_id].reg[2], 0)) != 0) goto out;
	}

	//both DS low
	if ((error = eDO(hDevice, 0, kMTCATcard[mtcat_id].reg[0], 0)) != 0) goto out;
	if ((error = eDO(hDevice, 0, kMTCATcard[mtcat_id].reg[1], 0)) != 0) goto out;

out:
	LJUSB_CloseDevice(hDevice);
	return (int)error;
}

