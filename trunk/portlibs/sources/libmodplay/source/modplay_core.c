/*
 * Copyright (c) 2002, 2003, 2004, 2005, 2007 by Christian Nowak <chnowak@web.de>
 * Last update: 20th October, 2007
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "modplay_core.h"
#include "mixer.h"
#include "effects.h"


static int linearperiodtab[768] = {

  133808, 133687, 133566, 133446, 133325, 133205, 133085, 132965, 
  132845, 132725, 132605, 132486, 132366, 132247, 132127, 132008, 
  131889, 131770, 131651, 131532, 131414, 131295, 131177, 131059, 
  130940, 130822, 130704, 130586, 130468, 130351, 130233, 130116, 
  129998, 129881, 129764, 129647, 129530, 129413, 129296, 129180, 
  129063, 128947, 128830, 128714, 128598, 128482, 128366, 128250, 
  128134, 128019, 127903, 127788, 127673, 127558, 127442, 127328, 
  127213, 127098, 126983, 126869, 126754, 126640, 126526, 126411, 
  126297, 126183, 126070, 125956, 125842, 125729, 125615, 125502, 
  125389, 125276, 125163, 125050, 124937, 124824, 124712, 124599, 
  124487, 124374, 124262, 124150, 124038, 123926, 123814, 123703, 
  123591, 123480, 123368, 123257, 123146, 123035, 122924, 122813, 
  122702, 122591, 122481, 122370, 122260, 122150, 122039, 121929, 
  121819, 121709, 121600, 121490, 121380, 121271, 121161, 121052, 
  120943, 120834, 120725, 120616, 120507, 120398, 120290, 120181, 
  120073, 119964, 119856, 119748, 119640, 119532, 119424, 119317, 
  119209, 119101, 118994, 118887, 118779, 118672, 118565, 118458, 
  118351, 118244, 118138, 118031, 117925, 117818, 117712, 117606, 
  117500, 117394, 117288, 117182, 117076, 116971, 116865, 116760, 
  116654, 116549, 116444, 116339, 116234, 116129, 116024, 115920, 
  115815, 115711, 115606, 115502, 115398, 115294, 115190, 115086, 
  114982, 114878, 114775, 114671, 114568, 114464, 114361, 114258, 
  114155, 114052, 113949, 113846, 113743, 113641, 113538, 113436, 
  113334, 113231, 113129, 113027, 112925, 112823, 112721, 112620, 
  112518, 112417, 112315, 112214, 112113, 112012, 111911, 111810, 
  111709, 111608, 111507, 111407, 111306, 111206, 111105, 111005, 
  110905, 110805, 110705, 110605, 110505, 110406, 110306, 110207, 
  110107, 110008, 109909, 109809, 109710, 109611, 109512, 109414, 
  109315, 109216, 109118, 109019, 108921, 108823, 108725, 108627, 
  108529, 108431, 108333, 108235, 108137, 108040, 107942, 107845, 
  107748, 107651, 107553, 107456, 107359, 107263, 107166, 107069, 
  106973, 106876, 106780, 106683, 106587, 106491, 106395, 106299, 
  106203, 106107, 106011, 105916, 105820, 105725, 105629, 105534, 
  105439, 105344, 105249, 105154, 105059, 104964, 104869, 104775, 
  104680, 104586, 104492, 104397, 104303, 104209, 104115, 104021, 
  103927, 103834, 103740, 103646, 103553, 103459, 103366, 103273, 
  103180, 103086, 102993, 102901, 102808, 102715, 102622, 102530, 
  102437, 102345, 102253, 102160, 102068, 101976, 101884, 101792, 
  101700, 101609, 101517, 101425, 101334, 101242, 101151, 101060, 
  100969, 100878, 100787, 100696, 100605, 100514, 100423, 100333, 
  100242, 100152, 100061, 99971,  99881,  99791,  99701,  99611,  
  99521,  99431,  99342,  99252,  99162,  99073,  98984,  98894,  
  98805,  98716,  98627,  98538,  98449,  98360,  98271,  98183,  
  98094,  98006,  97917,  97829,  97741,  97653,  97564,  97476,  
  97389,  97301,  97213,  97125,  97038,  96950,  96863,  96775,  
  96688,  96601,  96514,  96426,  96339,  96253,  96166,  96079,  
  95992,  95906,  95819,  95733,  95646,  95560,  95474,  95388,  
  95302,  95216,  95130,  95044,  94958,  94873,  94787,  94701,  
  94616,  94531,  94445,  94360,  94275,  94190,  94105,  94020,  
  93935,  93851,  93766,  93681,  93597,  93512,  93428,  93344,  
  93260,  93175,  93091,  93007,  92923,  92840,  92756,  92672,  
  92589,  92505,  92422,  92338,  92255,  92172,  92089,  92005,  
  91922,  91840,  91757,  91674,  91591,  91509,  91426,  91344,  
  91261,  91179,  91097,  91014,  90932,  90850,  90768,  90686,  
  90605,  90523,  90441,  90360,  90278,  90197,  90115,  90034,  
  89953,  89872,  89791,  89710,  89629,  89548,  89467,  89386,  
  89306,  89225,  89145,  89064,  88984,  88904,  88823,  88743,  
  88663,  88583,  88503,  88423,  88344,  88264,  88184,  88105,  
  88025,  87946,  87867,  87787,  87708,  87629,  87550,  87471,  
  87392,  87313,  87234,  87156,  87077,  86998,  86920,  86842,  
  86763,  86685,  86607,  86529,  86451,  86373,  86295,  86217,  
  86139,  86061,  85984,  85906,  85829,  85751,  85674,  85597,  
  85519,  85442,  85365,  85288,  85211,  85134,  85057,  84981,  
  84904,  84827,  84751,  84675,  84598,  84522,  84446,  84369,  
  84293,  84217,  84141,  84065,  83989,  83914,  83838,  83762,  
  83687,  83611,  83536,  83461,  83385,  83310,  83235,  83160,  
  83085,  83010,  82935,  82860,  82785,  82711,  82636,  82561,  
  82487,  82413,  82338,  82264,  82190,  82116,  82042,  81968,  
  81894,  81820,  81746,  81672,  81598,  81525,  81451,  81378,  
  81304,  81231,  81158,  81085,  81011,  80938,  80865,  80792,  
  80719,  80647,  80574,  80501,  80429,  80356,  80284,  80211,  
  80139,  80066,  79994,  79922,  79850,  79778,  79706,  79634,  
  79562,  79490,  79419,  79347,  79275,  79204,  79133,  79061,  
  78990,  78919,  78847,  78776,  78705,  78634,  78563,  78492,  
  78422,  78351,  78280,  78209,  78139,  78068,  77998,  77928,  
  77857,  77787,  77717,  77647,  77577,  77507,  77437,  77367,  
  77297,  77227,  77158,  77088,  77019,  76949,  76880,  76810,  
  76741,  76672,  76603,  76534,  76465,  76396,  76327,  76258,  
  76189,  76120,  76052,  75983,  75914,  75846,  75778,  75709,  
  75641,  75573,  75504,  75436,  75368,  75300,  75232,  75165,  
  75097,  75029,  74961,  74894,  74826,  74759,  74691,  74624,  
  74556,  74489,  74422,  74355,  74288,  74221,  74154,  74087,  
  74020,  73953,  73887,  73820,  73753,  73687,  73620,  73554,  
  73488,  73421,  73355,  73289,  73223,  73157,  73091,  73025,  
  72959,  72893,  72827,  72762,  72696,  72630,  72565,  72499,  
  72434,  72369,  72303,  72238,  72173,  72108,  72043,  71978,  
  71913,  71848,  71783,  71718,  71654,  71589,  71524,  71460,  
  71395,  71331,  71267,  71202,  71138,  71074,  71010,  70946,  
  70882,  70818,  70754,  70690,  70626,  70563,  70499,  70435,  
  70372,  70308,  70245,  70182,  70118,  70055,  69992,  69929,  
  69866,  69803,  69740,  69677,  69614,  69551,  69488,  69426,  
  69363,  69300,  69238,  69175,  69113,  69051,  68988,  68926,  
  68864,  68802,  68740,  68678,  68616,  68554,  68492,  68430,  
  68369,  68307,  68245,  68184,  68122,  68061,  67999,  67938,  
  67877,  67815,  67754,  67693,  67632,  67571,  67510,  67449,  
  67388,  67328,  67267,  67206,  67145,  67085,  67024,  66964
};



static char * mod_notes[] = {

  "C-0", "C#0", "D-0", "D#0", "E-0", "F-0", "F#0", "G-0", "G#0", "A-0", "A#0", "B-0", "???", "???", "???", "???",
  "C-1", "C#1", "D-1", "D#1", "E-1", "F-1", "F#1", "G-1", "G#1", "A-1", "A#1", "B-1", "???", "???", "???", "???",
  "C-2", "C#2", "D-2", "D#2", "E-2", "F-2", "F#2", "G-2", "G#2", "A-2", "A#2", "B-2", "???", "???", "???", "???",
  "C-3", "C#3", "D-3", "D#3", "E-3", "F-3", "F#3", "G-3", "G#3", "A-3", "A#3", "B-3", "???", "???", "???", "???",
  "C-4", "C#4", "D-4", "D#4", "E-4", "F-4", "F#4", "G-4", "G#4", "A-4", "A#4", "B-4", "???", "???", "???", "???",
  "C-5", "C#5", "D-5", "D#5", "E-5", "F-5", "F#5", "G-5", "G#5", "A-5", "A#5", "B-5", "???", "???", "???", "???",
  "C-6", "C#6", "D-6", "D#6", "E-6", "F-6", "F#6", "G-6", "G#6", "A-6", "A#6", "B-6", "???", "???", "???", "???",
  "C-7", "C#7", "D-7", "D#7", "E-7", "F-7", "F#7", "G-7", "G#7", "A-7", "A#7", "B-7", "???", "???", "???", "???"
};

static u32 mod_notetable[16][12] = {

  { 229079296, 216233728, 203923392, 192683520, 181443648, 171274240,
    161640064, 152541120, 143977408, 135948928, 128455680, 121363856 },

  { 114539648, 108116864, 101961696, 96341760,  90721824,  85637120,
    80820032,  76270560,  71988704,  67974464,  64227840,  60681928  },

  { 57269824,  54058432,  50980848,  48170880,  45360912,  42818560,
    40410016,  38135280,  35994352,  33987232,  32113920,  30340964  },

  { 28634912,  27029216,  25490424,  24085440,  22680456,  21409280,
    20205008,  19067640,  17997176,  16993616,  16056960,  15170482  },

  { 14317456,  13514608,  12745212,  12042720,  11340228,  10704640,
    10102504,  9533820,   8998588,   8496808,   8028480,   7585241   },

  { 7158728,   6757304,   6372606,   6021360,   5670114,   5352320,
    5051252,   4766910,   4499294,   4248404,   4014240,   3792620   },

  { 3579364,   3378652,   3186303,   3010680,   2835057,   2676160,
    2525626,   2383455,   2249647,   2124202,   2007120,   1896310   },

  { 1789682,   1689326,   1593151,   1505340,   1417528,   1338080,
    1262813,   1191727,   1124823,   1062101,   1003560,   948155    },

  { 894841,    844663,    796575,    752670,    708764,    669040,
    631406,    595863,    562411,    531050,    501780,    474077    },

  { 447420,    422331,    398287,    376335,    354382,    334520,
    315703,    297931,    281205,    265525,    250890,    237038    },

  { 223710,    211165,    199143,    188167,    177191,    167260,
    157851,    148965,    140602,    132762,    125445,    118519    },

  { 111855,    105582,    99571,     94083,     88595,     83630,
    78925,     74482,     70301,     66381,     62722,     59259     },

  { 55927,     52791,     49785,     47041,     44297,     41815,
    39462,     37241,     35150,     33190,     31361,     29629     },

  { 27963,     26395,     24892,     23520,     22148,     20907,
    19731,     18620,     17575,     16595,     15680,     14814     },

  { 13981,     13197,     12446,     11760,     11074,     10453,
    9865,      9310,      8787,      8297,      7840,      7407      },

  { 6990,      6598,      6223,      5880,      5537,      5226,
    4932,      4655,      4393,      4148,      3920,      3703      }
};

int MODFILE_BPM2SamplesPerTick(MODFILE *mod, int bpm) {

  return (mod->playfreq * 5) / (2 * bpm);
}


static MIXER_TYPE MODFILE_Period2Incval(MODFILE *mod, u32 period) {

  if (mod->period_type == 1) {

    int freq;
    int okt;
    MIXER_TYPE t;

    okt = period / 768;
    freq = linearperiodtab[period % 768];
    freq = freq >> okt;
    freq <<= 2;
    t = ((MIXER_TYPE)freq * ((MIXER_TYPE)1 << MIXER_SHIFT)) / (MIXER_TYPE)mod->playfreq;

    return t;
  } else {

    unsigned long long int temp = period;
    unsigned long long int playfreq = mod->playfreq;
    unsigned long long int temp2 = 14317056;
    unsigned long long int temp3 = 65536;

    if (period == 0)
      return 0;

    temp = (temp2 * temp3) / (temp * playfreq);

#if MIXER_SHIFT >= 16
    return ((MIXER_TYPE)temp) << (MIXER_SHIFT-16);
#else
    return ((MIXER_TYPE)temp) >> (16-MIXER_SHIFT);
#endif
  }
}

static int MODFILE_Note2Period(MODFILE *mod, u8 dnote, int middle_c_freq, s8 finetune) {

  u32 note = dnote & 0x0f;
  u32 octave = (dnote >> 4) & 0x0f;

  if (mod->period_type == 1) {

    int linnote = (octave * 12) + note;

    return (10 * 12 * 16 * 4) - (linnote * 16 * 4) - (finetune / 2);
  } else {

    if (middle_c_freq == 0)
      middle_c_freq = 8363;

    return mod_notetable[octave][note] / middle_c_freq;
  }
}

void MODFILE_SetNote(MODFILE *mod, int channel, u8 note, int middle_c, s8 finetune) {

  mod->channels[channel].st3_period = MODFILE_Note2Period(mod, note, middle_c, finetune);
  mod->channels[channel].voiceInfo.incval =
  	MODFILE_Period2Incval(mod,
  		mod->channels[channel].st3_period + mod->channels[channel].st3_periodofs);
}

void MODFILE_SetPeriod(MODFILE *mod, int channel, u32 period) {

  mod->channels[channel].st3_period = period;
  mod->channels[channel].voiceInfo.incval =
  	MODFILE_Period2Incval(mod,
  		mod->channels[channel].st3_period + mod->channels[channel].st3_periodofs);
}

void MODFILE_SetPeriodOfs(MODFILE *mod, int channel, s32 periodofs) {
	
	mod->channels[channel].st3_periodofs = periodofs;
  mod->channels[channel].voiceInfo.incval =
  	MODFILE_Period2Incval(mod,
  		mod->channels[channel].st3_period + mod->channels[channel].st3_periodofs);
}

void MODFILE_SetBPM(MODFILE *mod, int bpm) {

  if ((bpm >= 32) && (bpm < 256)) {

    mod->bpm = bpm;
    mod->samplespertick = MODFILE_BPM2SamplesPerTick(mod, bpm);
  }
}

static MOD_Note *MODFILE_GetCurPatternData(MODFILE *mod, int patternline, int channel) {

  return &mod->patterns[mod->playlist[mod->play_position]]
    [(patternline * mod->nChannels) + channel];
}

char *MODFILE_GetNoteString(u8 note) {

  if (note >= 128)
    return "---";
  else
    return mod_notes[note];
}

u8 MODFILE_GetNote(MODFILE *mod, int patternline, int channel) {

  MOD_Note *data = MODFILE_GetCurPatternData(mod, patternline, channel);

  return data->note;
}

u16 MODFILE_GetEffect(MODFILE *mod, int patternline, int channel, int ecol) {

  MOD_Note *data = MODFILE_GetCurPatternData(mod, patternline, channel);
  return data->effect[ecol] == 255 ? 0 : data->effect[ecol];
}

u8 MODFILE_GetEffectOp(MODFILE *mod, int patternline, int channel, int ecol) {

  MOD_Note *data = MODFILE_GetCurPatternData(mod, patternline, channel);
  return data->operand[ecol] == 255 ? 0 : data->operand[ecol];
}

u32 MODFILE_GetInstr(MODFILE *mod, int patternline, int channel) {

  MOD_Note *data = MODFILE_GetCurPatternData(mod, patternline, channel);
  return data->instrument == 255 ? 0 : data->instrument;
}

void MODFILE_SubVolume(MODFILE *mod, int channel, u8 sub) {

  if (sub >= mod->channels[channel].voiceInfo.volume)
    mod->channels[channel].voiceInfo.volume = 0;
  else
    mod->channels[channel].voiceInfo.volume -= sub;
}

void MODFILE_AddVolume(MODFILE *mod, int channel, u8 add) {

  if (64 - add <= mod->channels[channel].voiceInfo.volume)
    mod->channels[channel].voiceInfo.volume = 64;
  else
    mod->channels[channel].voiceInfo.volume += add;
}

/* TODO: Rewrite(!) - Sieht aus wie hingekotzt... */
BOOL MODFILE_TriggerNote(MODFILE *mod, int channel, u8 note, u8 instrument, u8 volume, u16 *commands) {

  BOOL ret = FALSE;
  BOOL effect_07h = FALSE,
    effect_0ch = FALSE;
  int c;

  if (instrument - 1 >= mod->nInstruments) {
	  
  	return FALSE;
	}
  
  if (instrument > 0) {
	  
	  if (!mod->instruments[instrument - 1].samples[note]) {
		  
	  	return FALSE;
  	}
  }
/*
  for (c = 0; c < MODPLAY_NUM_COMMANDS; c++) {

    if (commands[c] == 0x07) effect_07h = TRUE;
    if (commands[c] == 0x0c) effect_0ch = TRUE;
  }
*/
  if ((note < 254) && (instrument != 255) && (!effect_07h) && (!effect_0ch)) {

    mod->channels[channel].voiceInfo.playpos = 0;
    mod->channels[channel].voiceInfo.forward = TRUE;

    if (instrument > 0) {

			mod->channels[channel].last_instrument = instrument;
      mod->channels[channel].instrument = &mod->instruments[instrument - 1];
      mod->channels[channel].sample = mod->instruments[instrument - 1].samples[note];
      mod->channels[channel].voiceInfo.sampleInfo =
        &mod->channels[channel].sample->sampleInfo;

			ret = mod->channels[channel].voiceInfo.playing = TRUE;
    }

    for (c = 0; c < MODPLAY_NUM_COMMANDS; c++) {

      if (mod->channels[channel].effects[c].vibrato_wave < 4)
        mod->channels[channel].effects[c].vibrato_sintabpos = 0;

      if (mod->channels[channel].effects[c].tremolo_wave < 4)
        mod->channels[channel].effects[c].tremolo_sintabpos = 0;
    }
  }

  if ((note < 254) && !((effect_07h) || (effect_0ch))) {

    u8 dnote;

    mod->channels[channel].last_note = note;
    dnote = ((note >> 4) * 12) + (note & 0x0f);
    if (mod->channels[channel].last_instrument != 0)
    	mod->channels[channel].sample = mod->instruments[mod->channels[channel].last_instrument - 1].samples[note];

    if (mod->channels[channel].sample &&
    		mod->channels[channel].instrument) {
	    
    	mod->channels[channel].voiceInfo.sampleInfo =
      	&mod->channels[channel].sample->sampleInfo;
    	dnote += mod->channels[channel].sample->relative_note;
    	dnote = ((dnote / 12) << 4) | (dnote % 12);
    	ret = mod->channels[channel].voiceInfo.playing = TRUE;
    	mod->channels[channel].voiceInfo.forward = TRUE;

    	MODFILE_SetNote(mod, channel,
                    	mod->channels[channel].instrument->note[dnote],
                    	mod->channels[channel].sample->middle_c,
                    	mod->channels[channel].sample->finetune);
    	mod->channels[channel].cur_note = dnote;
  	}

    for (c = 0; c < MODPLAY_NUM_COMMANDS; c++)
      mod->channels[channel].effects[c].vibrato_base = mod->channels[channel].st3_period;
  }

  /* Really trigger */
  if ((instrument != 255) && (instrument > 0)/* && mod->channels[channel].sample*/) {

/*    mod->channels[channel].voiceInfo.playpos = 0;*/
		mod->channels[channel].voiceInfo.panning = mod->channels[channel].default_panning;
		mod->channels[channel].last_instrument = instrument;
		mod->channels[channel].instrument = &mod->instruments[instrument - 1];
		mod->channels[channel].voiceInfo.volume = 64;

    if (mod->channels[channel].sample != NULL) {

	    u8 tmp;
	    
    	mod->channels[channel].voiceInfo.volume = mod->channels[channel].sample->default_volume;
    	ret = mod->channels[channel].voiceInfo.playing = TRUE;
    	tmp = mod->channels[channel].sample->panning;
    	if (tmp <= 64) {

      	mod->channels[channel].voiceInfo.panning = tmp == 64 ? 255 : tmp * 4 ;
    	}
  	}

    mod->channels[channel].envVolume.envConfig = &mod->channels[channel].instrument->envVolume;
    EnvTrigger(&mod->channels[channel].envVolume);
    mod->channels[channel].envPanning.envConfig = &mod->channels[channel].instrument->envPanning;
    EnvTrigger(&mod->channels[channel].envPanning);
    mod->channels[channel].volumeFade = 32768;
    mod->channels[channel].volumeFadeDec = 0;
  }

  if (volume <= 64) {

    mod->channels[channel].voiceInfo.volume = volume;
  }

  if (note == 0xfe) {
    
    if (mod->channels[channel].voiceInfo.playing) {

      if (mod->channels[channel].envPanning.envConfig->enabled)
        EnvRelease(&mod->channels[channel].envPanning);

      if (mod->channels[channel].envVolume.envConfig->enabled)
        EnvRelease(&mod->channels[channel].envVolume);
      else
        mod->channels[channel].voiceInfo.playing = FALSE;

      mod->channels[channel].volumeFadeDec = mod->channels[channel].instrument->volumeFade;
    }
  }

  return ret;
}

u32 MODFILE_Process(MODFILE *mod) {

  int i;
  u32 retval = 0;
  BOOL advancePattern = TRUE;

  for (i = 0; i < mod->nChannels; i++) {

/*    if (mod->channels[i].voiceInfo.enabled)*/ {

      int c;
      MOD_Note *patternData = MODFILE_GetCurPatternData(mod, mod->pattern_line, i);
      u8 note = patternData->note;
      u32 instrument = patternData->instrument;
      u8 volume = patternData->volume;

      BOOL doTrigger = TRUE;
      BOOL resetSamplePos = TRUE;
      BOOL changePeriod = TRUE;
      BOOL changePanning = TRUE;
      BOOL changeVolume = TRUE;

      for (c = 0; c < MODPLAY_NUM_COMMANDS; c++) {

        u16 command = patternData->effect[c];
        u8 operand = patternData->operand[c];
        u16 curcommand = mod->channels[i].effects[c].cur_effect;
/*        u8 curoperand = mod->channels[i].effects[c].cur_operand;*/

        if (note < 254 && mod->channels[i].sample != NULL) {
            
          u8 realnote = ((note >> 4) * 12) + (note & 0x0f);

          realnote += (int)(s8)mod->channels[i].sample->relative_note;
          realnote = ((realnote / 12) << 4) | (realnote % 12);

          mod->channels[i].effects[c].toneporta_dest =
            MODFILE_Note2Period(mod, realnote,
                                mod->channels[i].sample->middle_c,
                                mod->channels[i].sample->finetune);
        }

        /* Stop the last effect */
	      if (curcommand < EFFECT_NUMEFFECTS) {
		        
	      	if (MODPLAY_EffectHandlers[curcommand].stop != NULL) {
		        	
	        	MODPLAY_EffectHandlers[curcommand].stop(mod, i, c);
        	}
      	}

        mod->channels[i].effects[c].cur_effect = command;
        mod->channels[i].effects[c].cur_operand = operand;
        mod->channels[i].effects[c].last_effect = command;
        
        if (volume <= 64)
        	mod->channels[i].voiceInfo.volume = volume;

        /* Start the next effect */
				if (command < EFFECT_NUMEFFECTS) {
						
	       	if (MODPLAY_EffectHandlers[command].start != NULL) {
	        	
		       	int r = MODPLAY_EffectHandlers[command].start(mod, i, c, patternData);
		        	
		       	if (r & EFFECT_DONOTTRIGGERNOTE)
			       	doTrigger = FALSE;

			      if (r & EFFECT_DONOTADVANCEPATTERN)
			       	advancePattern = FALSE;

			      if (r & EFFECT_DONOTRESETSAMPLEPOS)
			       	resetSamplePos = FALSE;

			      if (r & EFFECT_DONOTCHANGEPERIOD)
			       	changePeriod = FALSE;
			       
			      if (r & EFFECT_DONOTCHANGEPANNING)
			      	changePanning = FALSE;
			      
			      if (r & EFFECT_DONOTCHANGEVOLUME)
			      	changeVolume = FALSE;
        	}
      	}
      }
      
      if (doTrigger) {
        
	    	MIXER_TYPE playpos = mod->channels[i].voiceInfo.playpos;
	      u32 period = mod->channels[i].st3_period;
	      u8 panning = mod->channels[i].voiceInfo.panning;
	      u8 vol = mod->channels[i].voiceInfo.volume;
	        
      	if (MODFILE_TriggerNote(mod, i, note, instrument, volume, patternData->effect)) {

        	retval |= 1 << i;
      	}

       	if (!resetSamplePos) {
	       		
       		mod->channels[i].voiceInfo.playpos = playpos;
     		}
     			
     		if (!changePeriod) {

	     		MODFILE_SetPeriod(mod, i, period);
     		}
     		
     		if (!changePanning) {
	     		
	     		mod->channels[i].voiceInfo.panning = panning;
     		}

     		if (!changeVolume) {
	     		
	     		mod->channels[i].voiceInfo.volume = vol;
     		}
    	}

		  if (volume <= 64) {

		    mod->channels[i].voiceInfo.volume = volume;
  		}

    	if (note != 0xff)
    		mod->channels[i].cur_note = note;
	  }
  }

  if (advancePattern) {
	  
  	mod->pattern_line++;
  	if (mod->pattern_line >= mod->patternLengths[mod->playlist[mod->play_position]]) {

    	mod->pattern_line = 0;
    	do {

      	mod->play_position++;
      	if (mod->play_position >= mod->songlength)
        	mod->play_position = 0;
    	} while (mod->playlist[mod->play_position] >= 0xfe);
  	}

  	if (mod->play_position >= mod->songlength)
    	mod->play_position = 0;
	}

  return retval;
}

u32 MODFILE_EffectHandler(MODFILE *mod) {

  u32 retval = 0;
  int i;
/*
  for (i = mod->nChannels; i < mod->nChannels + mod->nSFXChannels; i++) {

    if (mod->channels[i].voiceInfo.enabled) {

      if (EnvProcess(&mod->channels[i].envVolume))
        mod->channels[i].voiceInfo.envVolume = mod->channels[i].envVolume.value;
      else
        mod->channels[i].voiceInfo.envVolume = 64;

      if (EnvProcess(&mod->channels[i].envPanning)) {
        mod->channels[i].voiceInfo.envPanning = mod->channels[i].envPanning.value;
      }
      else
        mod->channels[i].voiceInfo.envPanning = 32;
    }
  }
*/
  for (i = 0; i < mod->nChannels; i++) {

    if (mod->channels[i].voiceInfo.enabled) {

      int c;

      /* Process envelopes */
      if (EnvProcess(&mod->channels[i].envVolume))
        mod->channels[i].voiceInfo.envVolume = mod->channels[i].envVolume.value;
      else
        mod->channels[i].voiceInfo.envVolume = 64;

      if (EnvProcess(&mod->channels[i].envPanning))
        mod->channels[i].voiceInfo.envPanning = mod->channels[i].envPanning.value;
      else
        mod->channels[i].voiceInfo.envPanning = 32;

      if (mod->channels[i].volumeFadeDec >= mod->channels[i].volumeFade)
        mod->channels[i].volumeFade = 0;
      else
        mod->channels[i].volumeFade -= mod->channels[i].volumeFadeDec;

      for (c = 0; c < MODPLAY_NUM_COMMANDS; c++ ) {

	      if (mod->channels[i].effects[c].cur_effect < EFFECT_NUMEFFECTS)
	      	if (MODPLAY_EffectHandlers[mod->channels[i].effects[c].cur_effect].process != NULL)
	      		MODPLAY_EffectHandlers[mod->channels[i].effects[c].cur_effect].process(mod, i, c);
      }
    }
  }

  return retval;
}

int MODFILE_Mix(MODFILE *mod, int flags, void *buf, int nSamples) {

  int i;

  clearbuf_final(flags, mod->tempmixbuf, nSamples);

  for (i = 0; i < mod->nChannels - 1; i++) {

    MOD_Channel *channel = &mod->channels[i];
    MOD_Sample *sample = channel->sample;

    if (channel->voiceInfo.enabled &&
        channel->voiceInfo.playing &&
        sample->sampleInfo.sampledata != NULL &&
        channel->voiceInfo.sampleInfo != NULL) {

/*	        if(i == 0)*/
      mix_final_1616bit(flags,
                        mod->tempmixbuf,
                        nSamples,
                        &channel->voiceInfo,
                        ((u32)mod->cur_master_volume *
                         (u32)mod->master_volume *
                         (u32)mod->musicvolume *
                         (u32)(channel->volumeFade >> 9)) >> 18
                        );
    }
  }

  for (i = mod->nChannels + 1; i < mod->nChannels + mod->nSFXChannels; i++) {

    MOD_Channel *channel = &mod->channels[i];
    MOD_Sample *sample = channel->sample;

    if (channel->voiceInfo.enabled &&
        channel->voiceInfo.playing &&
        sample->sampleInfo.sampledata != NULL &&
        channel->voiceInfo.sampleInfo != NULL) {

      mix_final_1616bit(flags,
                        mod->tempmixbuf,
                        nSamples,
                        &channel->voiceInfo,
                        ((u32)mod->cur_master_volume *
                         (u32)mod->master_volume *
                         (u32)mod->sfxvolume *
                         (u32)(channel->volumeFade >> 9)) >> 18
                        );
    }
  }

  copybuf_final(flags, buf, mod->tempmixbuf, nSamples);

  return nSamples;
}

void MODFILE_ClearPattern(MODFILE *mod, int pattern) {

  int pline, pchan;

  if (mod == NULL)
    return;

  if (pattern < 0)
    return;

  if (pattern >= mod->nPatterns)
    return;

  for (pline = 0; pline < mod->patternLengths[pattern]; pline++) {

    for (pchan = 0; pchan < mod->nChannels; pchan++) {

      MOD_Note *n;
      int c;

      n = &mod->patterns[pattern][(pline * mod->nChannels) + pchan];
      n->instrument = 0;
      n->volume = 0xff;
      n->note = 0xff;

      for (c = 0; c < MODPLAY_NUM_COMMANDS; c++) {

        n->effect[c] = EFFECT_NONE;
        n->operand[c] = 0;
      }
    }
  }
}
