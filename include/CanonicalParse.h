#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "NormalParse.h"
#include "StringLib.h"
#include <map>

/** \brief Data struct for csv file information */
struct SmileData {
  std::vector<dd_array<glm::vec2>> input_data;
  std::vector<dd_array<glm::vec2>> ground_data;
  std::map<cbuff<64>, unsigned> i_keys;
  std::map<cbuff<64>, unsigned> gt_keys;
	std::vector<float> time_stamps_i;
	std::vector<float> time_stamps_gt;
};

enum VecType { INPUT, OUTPUT };

/** \brief Convert file into canonical space */
void create_canonical_verts(Args args);

/** \brief Export data into calibrated space by folder */
void export_canonical(const char *input_dir, const char *ground_dir,
                      const glm::vec2 canonical_iris_pos,
                      const float canonical_iris_dist);

/** \brief Get vector of xyz values from input file */
void extract_vector2(const char *in_file, const VecType type, SmileData& sdata);
