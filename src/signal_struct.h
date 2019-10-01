#ifndef DATAVIEWER_SIGNAL_STRUCT_H
#define DATAVIEWER_SIGNAL_STRUCT_H

#include <cstdint>

struct signal_file {
    char sign[4];
    uint32_t n_channels;
    uint32_t samples_per_block;
    uint32_t n_spectra_lines;
    uint32_t cutoff_freq;
    float spectra_step_hz;
    float block_time_s;
    uint32_t total_time_s;
    uint32_t n_blocks;
    uint32_t data_size;
    uint32_t n_blocks_captured;
    float max_value;
    float min_value;
    float data[];
};

#endif //DATAVIEWER_SIGNAL_STRUCT_H
