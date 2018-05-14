#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "NormalParse.h"

enum VecType { INPUT, OUTPUT };

/** \brief Convert file into canonical space */
void create_canonical_verts(Args args);

/** \brief Export data into calibrated space by folder */
void export_canonical(const char *input_dir, const char *ground_dir,
                      const glm::vec2 canonical_iris_pos,
                      const float canonical_iris_dist);

/** \brief Get vector of xyz values from input file */
std::vector<glm::vec2> extract_vector2(const char *in_file, const VecType type);