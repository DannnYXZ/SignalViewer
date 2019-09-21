#ifndef DATAVIEWER_SIGNAL_DATA_STRUCT_H
#define DATAVIEWER_SIGNAL_DATA_STRUCT_H

#include <cstdint>

struct signal_file {
    char sign[4];
    uint32_t nchannel;
    uint32_t sample_size;
    uint32_t n_spectra_lines;
    uint32_t cutoff_freq;
    float freq_res;
    float chunk_time_s;
    uint32_t total_time_s;
    uint32_t n_blocks;
    uint32_t data_size;
    uint32_t actual_blocks;
    float max_value;
    float min_value;
    float data[];
};

#endif //DATAVIEWER_SIGNAL_DATA_STRUCT_H
