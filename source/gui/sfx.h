#pragma once
#include <Windows.h>


struct sdt_entry {
	int  offset;
	int  size;
	int  freq;
	int  loopStart;
	int  loopEnd;
};



struct sdt_entry_ps2_gta
{
	int offset;
	int size;
	int freq;
};


struct sdt_entry_gta {
	int  offset;
	int  size;
	int  freq;
};

struct sdt_entry_gta2 {
	int  offset;
	int  size;
	int  freq;
	int  unk;
	int  loopStart;
	int  loopEnd;
};


struct wav_header {
	int        header; // RIFF
	int        filesize;
	int        waveheader; // WAVE
	int        format; // FMT
	int        sectionsize;
	short      waveformat;
	short      channels;
	int        samplespersecond;
	int        bytespersecond;
	short      blockalign;
	short      bitspersample;
	int        dataheader;
	int        datasize;

};


struct wav_header_adpcm {
	int        header; // RIFF
	int        filesize;
	int        waveheader; // WAVE
	int        format; // FMT
	int        sectionsize;
	short      waveformat;
	short      channels;
	int        samplespersecond;
	int        bytespersecond;
	short      blockalign;
	short      bitspersample;
	short      bit1;
	short      bit2;
	int        dataheader;
	int        datasize;

};

struct wav_header_xbox {
	int        header; // RIFF
	int        filesize;
	int        waveheader; // WAVE
	int        format; // FMT
	int        sectionsize;
	short      waveformat;
	short      channels;
	int        samplespersecond;
	int        bytespersecond;
	short      blockalign;
	short      bitspersample;
	short      bit1;
	short      bit2;
	int        factid;
	int        factsize;
	int        uncompressedsize;
	int        dataheader;
	int        datasize;
};

struct vag_header {
	int header;
	int version;
	int unk;
	int dataSize;
	int freq;
	char pad[12] = {};
	char name[16] = {};
	char _pad[16] = {};
};


struct fsb4_header {
	int             header;
	int             samples;
	int             headersize;
	int             datasize;
	int             ver;
	int             mode;
	char            pad[8] = {};
	int             data1;
	int             data2;
	int             data3;
	int             data4;
};

struct fsb4_sample {
	short           size;
	char            name[30] = {};
	int             lenghtsamples;
	int             compressed;
	int             loopstart;
	int             loopend;
	int             mode;
	int             freq;
	short           vol;
	short           pan;
	short           pri;
	short           channels;
	float           min;
	float           max;
	float           varFreq;
	short           varVol;
	short           varPan;
};


struct fsb3_header {
	int             header;
	int             samples;
	int             headersize;
	int             datasize;
	int             ver;
	int             mode;
};

struct fsb3_sample {
	short           size;
	char            name[30];
	int             lenghtsamples;
	int             compressed;
	int             loopstart;
	int             loopend;
	int             mode;
	int             freq;
	short           vol;
	short           pan;
	short           pri;
	short           channels;
	float             data1;
	float             data2;
	int             data3;
	int             data4;
};

struct fsb3_sample_small_header {
	int  samples;
	int  size;
};

