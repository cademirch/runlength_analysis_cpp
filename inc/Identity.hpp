#ifndef RUNLENGTH_ANALYSIS_IDENTITY_HPP
#define RUNLENGTH_ANALYSIS_IDENTITY_HPP

#include <iostream>
#include <experimental/filesystem>

using std::cout;
using std::experimental::filesystem::path;


void measure_identity_from_fasta(path reads_fasta_path,
        path reference_fasta_path,
        path output_directory,
        string minimap_preset,
        uint16_t max_threads,
        uint64_t chunk_size=1*1000*1000);

void measure_identity_from_bam(path bam_path,
        path reference_fasta_path,
        uint16_t max_threads,
        uint64_t chunk_size=1*1000*1000);


#endif //RUNLENGTH_ANALYSIS_IDENTITY_HPP
