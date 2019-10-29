
#include "PileupGenerator.hpp"
#include "Base.hpp"
#include <iostream>
#include <experimental/filesystem>
#include <cmath>

using std::cout;
using std::ostream;
using std::min;
using std::experimental::filesystem::path;


PileupGenerator::PileupGenerator(path bam_path, uint16_t maximum_depth){
    this->bam_path = bam_path;
    this->bam_reader = BamReader(bam_path);
    this->maximum_depth = maximum_depth;
}


int64_t PileupGenerator::find_depth_index(int64_t start_index){
    ///
    /// Decide where to insert a read in the pileup, depending on what space is available.
    ///

    int64_t depth_index = -1;
    int64_t lowest_width_index;

    // Sort a vector of pairs
    sort(this->lowest_free_index_per_depth.begin(), this->lowest_free_index_per_depth.end(), [](auto &left, auto &right) {
        return left.second < right.second;
    });

    lowest_width_index = this->lowest_free_index_per_depth[0].second;

    // If this row is not empty
    if (lowest_width_index != 0){
        // Check if there is at least 1 space between the lowest free index and the start index for this read
        if (start_index > lowest_width_index + 1){
            depth_index = this->lowest_free_index_per_depth[0].first;
        }
        // If there is not, then just add another row, and set the depth index to that row
        else{
            depth_index = this->lowest_free_index_per_depth.size();
            this->lowest_free_index_per_depth.emplace_front(this->lowest_free_index_per_depth.size(), start_index);
        }
    }
    else{
        depth_index = 0;
    }

    return depth_index;
}


void PileupGenerator::print_lowest_free_indexes(){
    for (auto& item: this->lowest_free_index_per_depth){
        cout << item.first << " " << item.second << "\n";
    }
}


void PileupGenerator::print(Pileup& pileup){
//    vector<string> pileup_strings;

    vector<vector<string>> pileup_strings_per_channel(pileup.pileup[0][0].size());

    size_t i = 0;
    string s_value;
    float value;
    for (size_t width_index = 0; width_index<pileup.pileup.size(); width_index++){
        for (size_t depth_index = 0; depth_index < pileup.pileup[width_index].size(); depth_index++){

            i = 0;
            for (auto& pileup_strings: pileup_strings_per_channel) {
                if (depth_index >= pileup_strings.size()) {
                    pileup_strings.push_back("");
                }

                value = pileup.pileup[width_index][depth_index][i];
                if (i == 0) {
                    s_value = float_to_base(value);
                }
                else{
                    s_value = to_string(min(int(9), int(value)));
                }

                pileup_strings[depth_index] += s_value;

                // If there are inserts in this column, append them to the strings
                if (pileup.inserts.count(width_index) > 0) {
                    for (auto& column: pileup.inserts.at(width_index)) {
                        value = column[depth_index][i];
                        if (i == 0) {
                            s_value = float_to_base(value);
                        } else {
                            s_value = to_string(min(int(9), int(value)));
                        }

                        pileup_strings[depth_index] += s_value;
                    }
                }
                i++;
            }
        }
    }

    for (auto& pileup_strings: pileup_strings_per_channel){
        for (auto& s: pileup_strings) {
            cout << s << "\n";
        }
        cout << '\n';
    }
}


void PileupGenerator::parse_insert(Pileup& pileup, int64_t pileup_width_index, int64_t pileup_depth_index, AlignedSegment& aligned_segment, vector<float>& read_data){
    uint64_t insert_anchor_index = pileup_width_index + aligned_segment.reversal;
    uint64_t insert_index = aligned_segment.subcigar_index - 1;

    // If there is already another insert anchored at this ref position:
    if (pileup.inserts.count(insert_anchor_index) > 0) {

        // If this insert will fit within the width of the insert columns already present
        if (size_t(insert_index) < pileup.inserts.at(insert_anchor_index).size()) {
            // Simply fill in the base
            pileup.inserts.at(insert_anchor_index)[insert_index][pileup_depth_index] = read_data;
        } else {
            // Add another column and then fill in the base
            pileup.inserts.at(insert_anchor_index).push_back(this->default_insert_column);      // copy value because value is stored as class member
            pileup.inserts.at(insert_anchor_index)[insert_index][pileup_depth_index] = read_data;
        }
    } else {
        // Add a new entry at this position, initialize with a vector, and then fill in the base
        pileup.inserts.insert({insert_anchor_index, this->default_insert_pileup});        // copy value because value is stored as class member
        pileup.inserts.at(insert_anchor_index)[insert_index][pileup_depth_index] = read_data;
    }
}

