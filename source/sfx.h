#pragma once

#pragma pack(push, 1)
struct sdt_entry {
	int  offset;
	int  size;
	int  freq;
	int  loopStart;
	int  loopEnd;
};
#pragma pack(pop)

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