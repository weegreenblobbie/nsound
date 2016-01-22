//-----------------------------------------------------------------------------
//
//  $Id: ns_vocoder.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//-----------------------------------------------------------------------------

#include <Nsound/NsoundAll.h>

#include <cstdio>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

using namespace Nsound;

using std::cout;
using std::cerr;
using std::endl;

//-----------------------------------------------------------------------------
boolean
file_exists(const std::string filename)
{
    std::FILE * f_in;

    f_in = std::fopen(filename.c_str(), "r");

    if(f_in)
    {
        std::fclose(f_in);
        return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
void
printUsage()
{
    cout << endl
         << "usage: ns_vocoder OPTIONS voice_wave carrier_wave output_wave" << endl
         << endl
         << "Options are:" << endl
         << endl
         << "    -h|--help        Prints this message" << endl
         << "    -v|--verbose     Verbose" << endl
         << "    -n|--bands N     The number of frequency bands to use" << endl
         << "    -w|--window S    The window duration in seconds for calculating the envelope" << endl
         << "    -m|--fmax F      The maximum frequency in the filter bank" << endl
         << endl;
}

//-----------------------------------------------------------------------------
int
main(int argc, char ** argv)
{
    if(argc < 3)
    {
        printUsage();
        return 1;
    }

    std::vector<std::string> args;

    for(int32 i = 1; i < argc; ++i)
    {
        args.push_back(argv[i]);
    }

    float64 specific_window = 0;
    uint32  specific_n_bands = 0;
    float64 specific_fmax = 0;

    boolean verbose = false;

    std::vector<std::string>::iterator itor;
    std::vector<std::string>::iterator end;
    std::vector<std::string> files;

    for(itor = args.begin(), end = args.end();
        itor != args.end();
        ++itor)
    {
        if(*itor == "-h" || *itor == "--help")
        {
            printUsage();
        }
        else
        if(*itor == "-m" || *itor == "--fmax")
        {
            std::stringstream ss(*(itor + 1));
            ss >> specific_fmax;
            ++itor;
        }
        else
        if(*itor == "-n" || *itor == "--bands")
        {
            std::stringstream ss(*(itor + 1));
            ss >> specific_n_bands;
            ++itor;
        }
        else
        if(*itor == "-w" || *itor == "--window")
        {
            std::stringstream ss(*(itor + 1));
            ss >> specific_window;
            ++itor;
        }
        else
        if(*itor == "-v" || *itor == "--verbose")
        {
            verbose = true;
        }
        else
        {
            files.push_back(*itor);
        }
    }

    if(files.size() != 3)
    {
        cerr << "Wrong number of arguments!\n";
        return 1;
    }

    std::string voice_file   = files[0];
    std::string carrier_file = files[1];
    std::string output_file  = files[2];

    if(!file_exists(voice_file))
    {
        cerr << "ns_vocoder: can't find the file \"" << voice_file
             << "\"" << endl;

        return 1;
    }

    if(!file_exists(carrier_file))
    {
        cerr << "ns_vocoder: can't find the file \"" << voice_file
             << "\"" << endl;

        return 1;
    }

    AudioStream voice(voice_file);
    voice.mono();
    AudioStream carrier(carrier_file);

    float64 sr1 = voice.getSampleRate();
    float64 sr2 = carrier.getSampleRate();
    float64 sr = sr1;

    // Resample one of the signals if necessary.
    if(sr1 != sr2)
    {
        if(sr1 > sr2)
        {
            carrier.resample2(sr1);
            sr = sr1;
        }
        else
        {
            voice.resample2(sr2);
            sr = sr2;
        }
    }

    // Setup defaults
    float64 window = 0.020;
    uint32  n_bands = 16;
    float64 fmax = 4000.0;

    if(specific_window > 0)
    {
        window = specific_window;
    }

    if(specific_n_bands > 0)
    {
        n_bands = specific_n_bands;
    }

    if(specific_fmax > 0)
    {
        fmax = specific_fmax;
    }

    if(verbose)
    {
        cout << "voice wav   = " << voice_file << endl
             << "carrier wav = " << carrier_file << endl
             << "output wav  = " << output_file << endl
             << "n bands     = " << n_bands << endl
             << "window sec  = " << window << endl
             << "fmax        = " << fmax << endl;
    }

    Vocoder vocoder(sr, window, n_bands, fmax);

    AudioStream output(sr, carrier.getNChannels());

    for(uint32 i = 0; i < carrier.getNChannels(); ++i)
    {
        Buffer::iterator v = voice[0].begin();
        Buffer::iterator end = voice[0].end();

		// Using a circulator iterator incase the carrier isn't the same length as the
        // voice signal.
        Buffer::circular_iterator c = carrier[i].cbegin();

        Buffer temp;

        while(v != end)
        {
            temp << vocoder.filter(*v, *c);
            ++v;
            ++c;
        }

        output[i] = temp;

        vocoder.reset();
    }

    output.normalize();

    output *= 0.666;

    output >> output_file.c_str();

    return 0;
}
