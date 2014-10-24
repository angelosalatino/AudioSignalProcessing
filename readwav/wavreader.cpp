/*

  Created by Angelo Antonio Salatino on 22/10/2013

  Version 1 (22/10/2013): file created, main code implemented, exception handled
  Version 2 (20/10/2014): translated in english


This cpp file is based on K. L. Srinivas' work:
http://www.ee.iitb.ac.in/daplab/resources/wav_read_write.cpp

Algorithm:
1. opens the wav input file, open also the file_output to write all the samples
2. reads the header (it reads all the information available such as frequency, number of channels, bits per sample and so on)
3. it allocates all buffers to store data
4. reads samples
5. saves samples on file_output


SOME NOTES

1) The code below is able to read only little-endian format with either 8 or 16 bits per sample.
Other cases are unhandled!!

*/

/*
 * This code has been tested on Linux machine using g++ compiler.
 */

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>

using namespace std;

#define printERR(x) cerr<<" ------ Error found: "<<x<<endl;

//#define DEBUG 0 //Uncomment in case of debug

// WAVE PCM audio file format (other information are available here: https://ccrma.stanford.edu/courses/422/projects/WaveFormat/ )
typedef struct header_file
{
    char chunk_id[4]; // RIFF = little-endian (default), RIFX = big-endian
    int chunk_size;
    char format[4];
    char subchunk1_id[4];
    int subchunk1_size;
    short int audio_format;
    short int num_channels;     //  <------ number of channels
    int sample_rate;			//  <------ sampling frequency
    int byte_rate;
    short int block_align;
    short int bits_per_sample;  //  <------ bits per sample
    char subchunk2_id[4];
    int subchunk2_size;			// subchunk2_size indicates how many bytes are needed to store all the samples.
} header;

typedef struct header_file* header_p; //defining a header pointer (header_file structure)




int main(int argc, char** argv)
{
    try{

        if(argc!=3) throw "Uncorrect number of inputs. (Use ./wavreader input_file output_file)";

        header_p meta = new header;
        short int* pU = NULL;
        unsigned char* pC = NULL;
        double * gWavDataIn = NULL;
        char* wBuffer = NULL;

        ifstream infile;
        ofstream outfile;

        try{
            infile.exceptions (ifstream::eofbit | ifstream::failbit | ifstream::badbit);
            infile.open(argv[1], ios::in|ios::binary);
        }catch(std::ifstream::failure e){
            printERR("file input not found");
        }

        try{
            outfile.exceptions (std::ios::failbit | std::ios::badbit);
            outfile.open(argv[2]);
        }catch(std::ios_base::failure &e){
            printERR("file output not opened");
        }



        infile.read ((char*)meta, sizeof(header));
        long numOfSample = (meta->subchunk2_size/meta->num_channels)/(meta->bits_per_sample/8);

        cout << " Format : "; for(int i = 0; i < 4; i++) cout << meta->chunk_id[i]; cout << endl;
        cout << " Header size: "<<sizeof(*meta)<<" bytes" << endl;
        cout << " Sample Rate "<< meta->sample_rate <<" Hz" << endl;
        cout << " Chunk2 size (byte): " << meta->subchunk2_size << endl;
        cout << " Bits per samples: " << meta->bits_per_sample << " bit" <<endl;
        cout << " Number of channels: " << meta->num_channels << endl;
        cout << " Number of samples: " << numOfSample << endl;

        if (meta->chunk_id[3] == 'X') throw "big-endian case unhandled";
        else cout << " Byte order: Little-Endian " << endl;

        try{
            gWavDataIn = new double[numOfSample]; //data structure storing the samples
            wBuffer = new char[meta->subchunk2_size]; //data structure storing the bytes
        }catch ( bad_alloc& e ) {
            printERR("unallocated memory");
        }

        try{
            /* reading the samples (as bytes) from file */
            infile.read (wBuffer, meta->subchunk2_size*sizeof(char));
        }catch(std::ios_base::failure &e){
            printERR("error in reading the samples");
        }

        /* data conversion: from byte to samples  */
        if(meta->bits_per_sample == 16)
        {
            pU = (short*) wBuffer;
            for( int i = 0; i < numOfSample; i++)
            {
                gWavDataIn[i] = (double) (pU[i]);
            }
        }
        else if(meta->bits_per_sample == 8)
        {
            pC = (unsigned char*) wBuffer;
            for( int i = 0; i < numOfSample; i++)
            {
                gWavDataIn[i] = (double) (pC[i]);
            }
        }
        else
        {
            throw "Unhandled case";
        }


        delete[] wBuffer;

        /*
         * At this stage al the samples are available in
         * gWavDataIn
         */


#ifdef  DEBUG
        for( int i = 0; i < numOfSample; i++)
        {
            cout<<i+1<<":"<<gWavDataIn[i]<<endl;
        }
#endif     /* -----  not in DEBUG  ----- */

        /*
         * Writing all samples on text file
         */
        for( int i = 0; i < numOfSample; i++)
        {
            outfile << gWavDataIn[i] << endl;
        }
        cout<<"File written successfully, check " << argv[2] << endl;
        infile.close();
        outfile.close();
        delete[] gWavDataIn;
        delete meta;

    }catch(const char * str){
        printERR(str);
    }

    return 0;
}

