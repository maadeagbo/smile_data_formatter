#include "CanonicalParse.h"
#include <map>
#include "StringLib.h"
#include "ddFileIO.h"

void create_canonical_verts(Args args) {
  // check if necessary info is present

  if (args.canon_in == "" || args.canon_gt == "") {
    printf(
        "Error: Need to provide both input and ground truth directories:\n"
        "  input: %s\n  ground truth: %s\n",
        args.canon_in.c_str(), args.canon_gt.c_str());
    return;
  }

  export_canonical(args.canon_in.c_str(), args.canon_gt.c_str(), glm::vec2(),
                   1.f);
}

/** \brief Export data into calibrated space */
void export_canonical_data(SmileData &s_data, const char *dir, const char *gdir,
                           const unsigned d_idx, std::string &f_id,
                           const glm::vec2 canonical_iris_pos,
                           const float canonical_iris_dist, const bool append) {
  // create new file
  f_id = f_id.substr(0, 7);
  std::string out_f_name, out_fg_name;
  out_f_name = dir + std::string("/") + f_id + "_canon.csv";
  out_fg_name = gdir + std::string("/") + f_id + "_canon.csv";

  // get translation offset
  cbuff<64> map_idx = "Lateral canthus (R) x";
  const unsigned pf_r_l = s_data.gt_keys[map_idx] / 2;
  map_idx = "Lateral canthus (L) x";
  const unsigned pf_l_l = s_data.gt_keys[map_idx] / 2;
  
  glm::vec2 delta_pos = glm::vec2(-s_data.ground_data[d_idx][pf_r_l]);

  // apply delta translation to all points
  dd_array<glm::vec2> input_n(s_data.input_data[d_idx].size());
  dd_array<glm::vec2> ground_n(s_data.ground_data[d_idx].size());

  DD_FOREACH(glm::vec2, vec, s_data.input_data[d_idx]) {  // input
    input_n[vec.i] = *vec.ptr + delta_pos;
  }
  DD_FOREACH(glm::vec2, vec, s_data.ground_data[d_idx]) {  // ground truth
    ground_n[vec.i] = *vec.ptr + delta_pos;
  }

  // get rotation offset b/t lateral & medial iris
  const float rot_offset = atan2(ground_n[pf_l_l].y, ground_n[pf_l_l].x);
  glm::mat2 r_mat;
  r_mat[0][0] = glm::cos(-rot_offset);
  r_mat[0][1] = glm::sin(-rot_offset);
  r_mat[1][0] = -glm::sin(-rot_offset);
  r_mat[1][1] = glm::cos(-rot_offset);

  // apply negative rotation to all points (at the current pos)
  DD_FOREACH(glm::vec2, vec, input_n) {  // input
    input_n[vec.i] = r_mat * (*vec.ptr);
  }
  DD_FOREACH(glm::vec2, vec, ground_n) {  // ground
    ground_n[vec.i] = r_mat * (*vec.ptr);
  }

  // scale points so that iris distance is set to a canonical distance
  const float dist = glm::distance(ground_n[pf_r_l], ground_n[pf_l_l]);
  const float scale_factor = canonical_iris_dist / dist;
  glm::mat2 s_mat;
  s_mat[0][0] = s_mat[1][1] = scale_factor;
  s_mat[0][1] = s_mat[1][0] = 0.f;

  DD_FOREACH(glm::vec2, vec, input_n) {  // input
    input_n[vec.i] = s_mat * (*vec.ptr);
  }
  DD_FOREACH(glm::vec2, vec, ground_n) {  // ground
    ground_n[vec.i] = s_mat * (*vec.ptr);
  }

  // apply translation to all points to move iris to canonical position
  DD_FOREACH(glm::vec2, vec, input_n) {
    *vec.ptr += canonical_iris_pos;
  }
  DD_FOREACH(glm::vec2, vec, ground_n) {
    *vec.ptr += canonical_iris_pos;
  }

  // write out input and ground file
  ddFileIO<> i_out, g_out;
  if (append) {
    i_out.open(out_f_name.c_str(), ddIOflag::APPEND);
  } else {
    i_out.open(out_f_name.c_str(), ddIOflag::WRITE);
  }

  std::string out_str;
  std::string _sp(" ");

  // record time if if exists
  if (s_data.time_stamps_i.size() > 0) {
    out_str = std::to_string(s_data.time_stamps_i[d_idx]) + _sp;
  }

  DD_FOREACH(glm::vec2, vec, input_n) {
    out_str +=
        std::to_string(vec.ptr->x) + _sp + std::to_string(vec.ptr->y) + _sp;
  }
  out_str.pop_back();
  out_str += "\n";
  i_out.writeLine(out_str.c_str());

  if (append) {
    g_out.open(out_fg_name.c_str(), ddIOflag::APPEND);
  } else {
    g_out.open(out_fg_name.c_str(), ddIOflag::WRITE);
  }

  out_str = "";
  // record time if if exists
  if (s_data.time_stamps_gt.size() > 0) {
    out_str = std::to_string(s_data.time_stamps_gt[d_idx]) + _sp;
  }

  DD_FOREACH(glm::vec2, vec, ground_n) {
    out_str +=
        std::to_string(vec.ptr->x) + _sp + std::to_string(vec.ptr->y) + _sp;
  }
  out_str.pop_back();
  out_str += "\n";
  g_out.writeLine(out_str.c_str());
}

void export_canonical(const char *input_dir, const char *ground_dir,
                      const glm::vec2 canonical_iris_pos,
                      const float canonical_iris_dist) {
  // export input files
  ddFileIO<> io_input, io_ground;
  bool success = io_input.open(input_dir, ddIOflag::DIRECTORY);
  success |= io_ground.open(ground_dir, ddIOflag::DIRECTORY);
  if (success) {
    // for each file:
    dd_array<std::string> i_files = io_input.get_directory_files();
    dd_array<std::string> g_files = io_ground.get_directory_files();
    printf("Opening in dir: %s..\n", input_dir);
    printf("Opening ground dir: %s..\n", ground_dir);
    DD_FOREACH(std::string, file, i_files) {
      const char *g_file = g_files[file.i].c_str();
      // get name of file
      const std::string f_name = file.ptr->c_str();
      const size_t idx = f_name.find_last_of("\\/");

      if (file.ptr->find("_s_out.csv") != std::string::npos ||
          file.ptr->find("_v_out.csv") != std::string::npos) {
        printf("  Exporting: %s\n", f_name.c_str());

        // TODO: Change smile data to parameter and not return
        SmileData s_data;
        extract_vector2(file.ptr->c_str(), VecType::INPUT, s_data);
        extract_vector2(g_file, VecType::OUTPUT, s_data);

        // loop thru lines fo each and write to output file
        for (size_t j = 0; j < s_data.input_data.size(); j++) {
          const bool append = (j == 0) ? false : true;

          std::string file_substr = f_name.substr(idx + 1);

          export_canonical_data(s_data, input_dir, ground_dir, j,
                                file_substr, canonical_iris_pos,
                                canonical_iris_dist, append);
        }
        // ddTerminal::post("---> Done.");
      }
    }
  }
}

void extract_vector2(const char *in_file, const VecType type,
                     SmileData &sdata) {
  // set up handles
  std::vector<dd_array<glm::vec2>> &out_vec =
      (type == VecType::INPUT) ? sdata.input_data : sdata.ground_data;
  std::map<cbuff<64>, unsigned> &out_keys =
      (type == VecType::INPUT) ? sdata.i_keys : sdata.gt_keys;
  std::vector<float> &t_stamp =
      (type == VecType::INPUT) ? sdata.time_stamps_i : sdata.time_stamps_gt;

  // file/directory reader
  ddFileIO<> vec_io;

  bool success = vec_io.open(in_file, ddIOflag::READ);

  if (success) {
    // get vector size
    const char *line = vec_io.readNextLine();
    dd_array<cbuff<64>> indices;
    const std::string _f = in_file;
    bool time_flag = false;

    indices = StrSpace::tokenize1024<64>(line, ",");
    DD_FOREACH(cbuff<64>, _key, indices) {
      // set offset if time column is present (must be 1st column)
      if (_key.ptr->contains("time")) {
        time_flag = true;
      } else {
        out_keys[*_key.ptr] = time_flag ? _key.i - 1 : _key.i;
      }
    }
    line = vec_io.readNextLine();

    const unsigned vec_size =
        time_flag ? (indices.size() - 1) / 2 : (indices.size()) / 2;

    printf("    Creating new input vectors(%u)...\n", (unsigned)vec_size);
    // populate vector
    unsigned r_idx = 0;
    while (line) {
      // printf("%s\n", line);
      out_vec.push_back(dd_array<glm::vec2>(vec_size));

      // loop thru columns per row
      unsigned c_idx = 0;
      bool time_recorded = false;
      const char *curr_row = line;
      while (*curr_row) {
        char *nxt_dbl = nullptr;
        // printf("%s\n", curr_row);
        if (time_flag && !time_recorded) {
          // get time info
          t_stamp.push_back(std::strtod(curr_row, &nxt_dbl));
          time_recorded = true;
        } else {
          // get x & y axis
          out_vec[r_idx][c_idx].x = std::strtod(curr_row, &nxt_dbl);
          curr_row = nxt_dbl;
          POW2_VERIFY_MSG(nxt_dbl != nullptr, "Y axis error: column %u", c_idx);
          out_vec[r_idx][c_idx].y = std::strtod(curr_row, &nxt_dbl);
          c_idx++;
        }
        curr_row = nxt_dbl;
      }
      // // std::cout << out_vec[idx] << "\n\n";

      line = vec_io.readNextLine();
      r_idx++;
    }
  }
}
