#pragma once

#pragma pack (push,1)
struct sdt_entry {
	int  offset;
	int  size;
	int  freq;
	int  loopStart;
	int  loopEnd;
};
#pragma (pop)

#pragma pack (push,1)
struct sdt_entry_ps2_gta
{
	int offset;
	int size;
	int freq;
};
#pragma (pop)

#pragma pack (push,1)
struct sdt_entry_gta2 {
	int  offset;
	int  size;
	int  freq;
	int  unk;
	int  loopStart;
	int  loopEnd;
};
#pragma (pop)

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

#pragma pack (push,1)
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
#pragma (pop)
