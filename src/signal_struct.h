#ifndef DATAVIEWER_SIGNAL_STRUCT_H
#define DATAVIEWER_SIGNAL_STRUCT_H

#include <cstdint>

struct signal_file_t {
    char signature[4];
    uint32_t n_channels = 0;
    uint32_t samples_per_block = 0;
    uint32_t n_spectra_lines = 0;
    uint32_t cutoff_freq = 0;
    float spectra_step_hz = 0;
    float block_time_s = 0;
    uint32_t total_time_s = 0;
    uint32_t n_blocks = 0;
    uint32_t data_size = 0;
    uint32_t n_blocks_captured = 0;
    float max_value = 0;
    float min_value = 0;
    float data[];
};

#endif //DATAVIEWER_SIGNAL_STRUCT_H
