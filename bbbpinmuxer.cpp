/*
License: New BSD (found in root of github directory.)
Creation Date: 12-6-2013
Description: Library that sets up Muxer settings for Beagle Bone Black.
Also Library allows the ability to send bits to the GPIO ports.

Tested on: Beagle Bone Black revision A4 running default Angstrom OS

Will Only Compile following C++11 and later
*/

#pragma once
#include <fstream>
#include <string>
#include <stdlib.h>
#include <sstream>
#include <iostream>
#include <vector>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MEM_START 	            (0x44E10000)
#define MEM_END 		        (0x44E11FFF)
#define MEM_SIZE		        (MEM_END  - MEM_START)
#define GPIO_OE					(0x134)
#define GPIO_DATAOUT			(0x13C)
#define GPIO_CLEARDATAOUT		(0x190)
#define GPIO_SETDATAOUT			(0x194)

using namespace std;

class GPIOPinMuxer
{
    public:
        GPIOPinMuxer(string);
        ~GPIOPinMuxer();
        void addPin(string, string, string);
        void engage();
        int getValue(int);
        void setValue(int, int);
        int getValue(string);
        void setValue(string, string);

    private:
        const int offsets[65] = {0x18,0x1C,0x08,0x0C,0x90,0x94,0x9C,0x98,0x34,0x30,0x24,0x28,0x3C,0x38,0x2C,0x8C,0x20,0x84,0x80,0x14,0x10,0x04,0x00,0x7C,0xE0,0xE8,0xE4,0xEC,0xD8,0xDC,0xD4,0xCC,0xD0,0xC8,0xC0,0xC4,0xB8,0xBC,0xB0,0xB4,0xA8,0xAC,0xA0,0xA4,0x70,0x78,0x74,0x48,0x40,0x4C,0x15C,0x158,0x17C,0x178,0x154,0x150,0x44,0x184,0x1AC,0x180,0x1A4,0x19C,0x194,0x198,0x190};
        const int GPIONums[65] = {38,39,34,35,66,67,69,68,45,44,23,26,47,46,27,65,22,63,62,37,36,33,32,61,86,88,87,89,10,11,9,81,8,80,78,79,76,77,74,75,72,73,70,71,30,60,31,50,48,51,5,4,13,12,3,2,49,15,117,14,115,113,111,112,110};
        const string headerPin[65] = {"P8.3","P8.4","P8.5","P8.6","P8.7","P8.8","P8.9","P8.10","P8.11","P8.12","P8.13","P8.14","P8.15","P8.16","P8.17","P8.18","P8.19","P8.20","P8.21","P8.22","P8.23","P8.24","P8.25","P8.26","P8.27","P8.28","P8.29","P8.30","P8.31","P8.32","P8.33","P8.34","P8.35","P8.36","P8.37","P8.38","P8.39","P8.40","P8.41","P8.42","P8.43","P8.44","P8.45","P8.46","P9.11","P9.12","P9.13","P9.14","P9.15","P9.16","P9.17","P9.18","P9.19","P9.20","P9.21","P9.22","P9.23","P9.24","P9.25","P9.26","P9.27","P9.28","P9.29","P9.30","P9.31"};
        const int bigGPIO[65] = {1,1,1,1,2,2,2,2,1,1,0,0,1,1,0,2,0,1,1,1,1,1,1,1,2,2,2,2,0,0,0,2,0,2,2,2,2,2,2,2,2,2,2,2,0,1,0,1,1,1,0,0,0,0,0,0,1,0,3,0,3,3,3,3,3};
        const int smallGPIO[65] = {6,7,2,3,2,3,5,4,13,12,23,26,15,14,27,1,22,31,30,5,4,1,0,29,22,24,23,25,10,11,9,17,8,16,14,15,12,13,10,11,8,9,6,7,30,28,31,18,16,19,5,4,13,12,3,2,17,15,21,14,19,17,15,16,14};
        const uint32_t GPIOS[4] = {0x44E07000,0x4804C000,0x481AC000,0x481AE000};

        string capeNumber;
        string projectName;
        int intPreviousSlot;
        string currentSlot;
        int intCurrentSlot;

        int memcon;
        volatile uint32_t * memspan;
        volatile uint32_t * GPIOAddresses[4];


        vector<string> GPIOPins;
        vector<string> direction;
        vector<string> dirModifier;
        vector<string> mux;
        vector<int> offset;
        vector<string> header;

        void setCapeNumber();
        string setSlot();
        void makeDTS();
        void enableDTS();
};

//Constructor/Destructor
GPIOPinMuxer::GPIOPinMuxer(string tName)
{
    projectName = tName;
    setCapeNumber();
    memcon = open("/dev/mem", O_RDWR | O_SYNC);
    memspan = (uint32_t*) mmap(NULL, MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memcon, MEM_START);
    GPIOAddresses[0] =(uint32_t*) mmap(NULL, 0xFFF, PROT_READ | PROT_WRITE, MAP_SHARED, memcon, GPIOS[0]);
    GPIOAddresses[1] =(uint32_t*) mmap(NULL, 0xFFF, PROT_READ | PROT_WRITE, MAP_SHARED, memcon, GPIOS[1]);
    GPIOAddresses[2] =(uint32_t*) mmap(NULL, 0xFFF, PROT_READ | PROT_WRITE, MAP_SHARED, memcon, GPIOS[2]);
    GPIOAddresses[3] =(uint32_t*) mmap(NULL, 0xFFF, PROT_READ | PROT_WRITE, MAP_SHARED, memcon, GPIOS[3]);
}

GPIOPinMuxer::~GPIOPinMuxer()
{
    string systemString = "rm /tmp/"+projectName+".dts";
    system(systemString.c_str());
    systemString = "rm /lib/firmware/"+projectName+"-00A0.dtbo";
    system(systemString.c_str());
    if(intCurrentSlot > intPreviousSlot)
    {
        systemString = "echo -" + currentSlot + " > /sys/devices/bone_capemgr." + capeNumber + "/slots";
        system(systemString.c_str());
    }
}

//Public Methods
void GPIOPinMuxer::addPin(string tGPIO, string tDirection, string tDirModifier)
{
    GPIOPins.push_back(tGPIO);
    direction.push_back(tDirection);
    dirModifier.push_back(tDirModifier);

    if(tDirection == "output")
        mux.push_back("0x07");
    else
        if(tDirModifier == "pulldown")
            mux.push_back("0x37");
        else
            mux.push_back("0x27");

    int indexOf = -1;
    for(int x = 0; x < 65; x++)
        if(GPIONums[x] == stoi(tGPIO))
            indexOf = x;
    if(indexOf != -1)
    {
        offset.push_back(offsets[indexOf]);
        header.push_back(headerPin[indexOf]);
    }
    else
        exit(1);
}

void GPIOPinMuxer::engage()
{
    string spreviousSlot = setSlot();
    intPreviousSlot = stoi(spreviousSlot);

    makeDTS();
    enableDTS();

    currentSlot = setSlot();
    intCurrentSlot = stoi(currentSlot);

    for(int x = 0; x < GPIOPins.size(); x++)
    {
        string gpioDirectionFile = "/sys/class/gpio/gpio"+GPIOPins.at(x)+"/direction";
        ofstream outputOut(gpioDirectionFile.c_str());
        if(direction.at(x) == "output")
        {
            outputOut << dirModifier.at(x);
            int indexOf = -1;
            for(int y = 0; y < 65; y++)
                if(GPIONums[y] == stoi(GPIOPins[x]))
                    indexOf = y;
            if(indexOf != -1)
            {
                GPIOAddresses[bigGPIO[indexOf]][0x134/4] &= ~(1 << smallGPIO[indexOf]);
                GPIOAddresses[bigGPIO[indexOf]][((0x190+0x4*(dirModifier.at(x) == "high"))/4)] = (1 << smallGPIO[indexOf]);
            }
        }
        outputOut.close();
    }
}

int GPIOPinMuxer::getValue(int identification)
{
    int GPIOBus = identification/32;
    int pinPosition = identification%32;
    return ((GPIOAddresses[GPIOBus][0x138/4] & (1 << pinPosition)) >> pinPosition);
}

void GPIOPinMuxer::setValue(int identification, int thisValue)
{
   int GPIOBus = identification/32;
   int pinPosition = identification%32;

   GPIOAddresses[GPIOBus][(0x190+0x4*(thisValue == 1))/4] = (1 << pinPosition);
}

int GPIOPinMuxer::getValue(string identification)
{
    return getValue(stoi(identification));
}

void GPIOPinMuxer::setValue(string identification, string thisValue)
{
    setValue(stoi(identification), stoi(thisValue));
}

//Private Methods
void GPIOPinMuxer::setCapeNumber()
{
    system("ls /sys/devices/ > /tmp/devicesdir");
    string capeStream = "/tmp/devicesdir";
    string capeLine;
    ifstream capeFile(capeStream.c_str());
    while(getline(capeFile,capeLine))
            if(capeLine.substr(0,4) == "bone")
                capeNumber = capeLine.substr(13,1);
    capeFile.close();
    string systemString = "rm /tmp/devicesdir";
    system(systemString.c_str());
}

string GPIOPinMuxer::setSlot()
{
    string lastline;
    string slot = "";
    string preline = "0";
    string slotstream = "/sys/devices/bone_capemgr." + capeNumber + "/slots";
    ifstream slotFile(slotstream.c_str());
    while(getline(slotFile,lastline))
            if(!slotFile.eof())
                preline = lastline;
    slotFile.close();
    string currentChar = "";
    currentSlot = "";
    for(int x = 0; x < preline.length() && currentChar != ":"; x++)
    {
        slot += currentChar;
        currentChar = preline.substr(x,1);
    }
    int intSlot = stoi(slot);
    slot = to_string(intSlot);
    return slot;
}

void GPIOPinMuxer::makeDTS()
{
    string filename = "/tmp/" + projectName + ".dts";
    ofstream codeOut(filename.c_str());
    codeOut << "/dts-v1/;\r\n";
    codeOut << "/plugin/;\r\n\r\n";
    codeOut << "/{\r\n";
    codeOut << "       compatible = \"ti,beaglebone\", \"ti,beaglebone-black\";\r\n";
    codeOut << "       part-number = \"" << projectName << "\";\r\n";
    codeOut << "       version = \"00A0\";\r\n\r\n";
    codeOut << "       exclusive-use =\r\n";
    for(int x = 0; x < header.size(); x++)
    {
        codeOut << "             \"" << header.at(x) << "\"";
        if(x == header.size()-1)
            codeOut << ";\r\n";
        else
            codeOut << ",\r\n";
    }
    codeOut << "       fragment@0 {\r\n";
    codeOut << "             target = <&am33xx_pinmux>;\r\n\r\n";
    codeOut << "             __overlay__ {\r\n";
    codeOut << "                  " << projectName << ": " << projectName << "_Pins {\r\n";
    codeOut << "                        pinctrl-single,pins = <\r\n";
    for(int x = 0; x < offset.size(); x++)
    {
        codeOut << "                                0x" << hex << offset.at(x) << " " << hex << mux.at(x) << "\r\n";
    }
    codeOut << "                        >;\r\n";
    codeOut << "                  };\r\n";
    codeOut << "             };\r\n";
    codeOut << "       };\r\n\r\n";
    codeOut << "       fragment@1 {\r\n";
    codeOut << "                target = <&ocp>;\r\n";
    codeOut << "                __overlay__ {\r\n";
	codeOut << "                        test_helper: helper {\r\n";
	codeOut << "                                compatible = \"bone-pinmux-helper\";\r\n";
    codeOut << "                                pinctrl-names = \"default\";\r\n";
    codeOut << "                                pinctrl-0 = <&" << projectName << ">;\r\n";
    codeOut << "                                status = \"okay\";\r\n";
	codeOut << "                        };\r\n";
    codeOut << "                };\r\n";
    codeOut << "        };\r\n";
    codeOut << "    };\r\n";
    codeOut.close();
}

void GPIOPinMuxer::enableDTS()
{
    string compileDTS = "dtc -O dtb -o /tmp/"+projectName+"-00A0.dtbo -b 0 -@ /tmp/"+projectName+".dts";
    system(compileDTS.c_str());

    string moveString = "mv /tmp/"+projectName+"-00A0.dtbo /lib/firmware/";
    system(moveString.c_str());

    string enableString = "echo " + projectName + " > /sys/devices/bone_capemgr." + capeNumber + "/slots";
    system(enableString.c_str());
}
