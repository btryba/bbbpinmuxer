/*
License: New BSD (found in root of github directory.)
Description: Uses MCP3008 and pontientiometer to determine current voltage.
				This setup is similiar to the tutorial on how to use MCP3008 with
				Rasperberry Pi found on Adafruit's website. However, Adafruit does not
				and is not promoting the use of this code.
*/
#include <fstream>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include "GPIOPinMuxer.cpp"
using namespace std;

int readADC(int, string, string, string, string, GPIOPinMuxer&);

int main()
{
	const string CHIPCLK = "60"; //p9_12
	const string DATAIN  = "50"; //p9_14 DATA in from chip
	const string DATAOUT  = "51"; //p9_16 DATA out to chip
	const string CHIPCS  = "4";  //p9_18
	const int availableChannels = 1;
	const int tolerance = 5;

	GPIOPinMuxer toChip("ADC");
	toChip.addPin(CHIPCLK, "output","low");
	toChip.addPin(DATAIN, "input","pullup");
	toChip.addPin(DATAOUT, "output","low");
	toChip.addPin(CHIPCS, "output","high");
	toChip.engage();

	int padding[availableChannels];

	for(int x = 0; x < 35; x++)
	{
		int ADCNum[availableChannels];

		for(int currentChannel = 0; currentChannel < availableChannels; currentChannel++)
		{
			ADCNum[currentChannel] = readADC(currentChannel, CHIPCLK, DATAIN, DATAOUT, CHIPCS, toChip);
			if(padding[currentChannel]-tolerance > ADCNum[currentChannel] || padding[currentChannel]+tolerance < ADCNum[currentChannel])
            {
                float vin = ADCNum[currentChannel]/304.0;
                float vtr = vin*4.079431489;
                float per = ADCNum[currentChannel]/7.05;
                if(currentChannel == 0)
                    system("clear");
                cout << "Decimal: " << ADCNum[currentChannel] << "/1024" << endl;
                cout << "Voltage Stepped Down: " << vin << " V" << endl;
                cout << "Voltage Actual: " << vtr << " V" << endl;
                cout << "Percent Open: " << per << "%" << endl << endl;
            }
            padding[currentChannel] = ADCNum[currentChannel];
        }
        usleep(500000);
	}

	return 0;
}

int readADC(int currentChannel, string CHIPCLK, string DATAIN, string DATAOUT, string CHIPCS, GPIOPinMuxer& toChip)
{
	if((currentChannel > 7) or (currentChannel < 0))
		return -1;
    toChip.setValue(CHIPCS,"1");
    toChip.setValue(CHIPCLK,"0");
    toChip.setValue(CHIPCS,"0");

	int bitsOut = currentChannel;
	bitsOut |= 24;
	bitsOut <<= 3;
	for (int temp=0; temp < 5; temp++)
	{
		if(bitsOut & 128)
            toChip.setValue(DATAOUT,"1");
		else
            toChip.setValue(DATAOUT,"0");
		bitsOut <<= 1;
		toChip.setValue(CHIPCLK,"1");
		toChip.setValue(CHIPCLK,"0");
	}

	int ADCOut = 0;
	for (int temp=0; temp < 12; temp++)
	{
		toChip.setValue(CHIPCLK,"1");
		toChip.setValue(CHIPCLK,"0");
		ADCOut <<= 1;
		if(toChip.getValue(DATAIN))
			ADCOut |= 1;
	}
	toChip.setValue(CHIPCS, "1");
	ADCOut >>= 1;

	return ADCOut;
}
