#include "StringLib.h"
#include "CanonicalParse.h"
#include "ddFileIO.h"
#include <map>

namespace {
std::map<cbuff<64>, unsigned> i_keys;
std::map<cbuff<64>, unsigned> gt_keys;
std::vector<float> time_stamps;
}

void create_canonical_verts(Args args) {
	// check if necessary info is present
  
  if (args.canon_in == "" || args.canon_gt == "") {
    printf("Error: Need to provide both input and ground truth directories:\n"
            "  input: %s\n  ground truth: %s\n", args.canon_in.c_str(),
            args.canon_gt.c_str());
    return;
  }
  
  export_canonical(args.canon_in.c_str(), args.canon_gt.c_str(), glm::vec2(), 
                   1.f);
}

/** \brief Export data into calibrated space */
void export_canonical_data(dd_array<glm::vec3> &input,
                           dd_array<glm::vec3> &ground,
                           const char *dir, const char *gdir, std::string &f_id,
                           const glm::vec2 canonical_iris_pos,
                           const float canonical_iris_dist, const bool append) {
  // create new file
  f_id = f_id.substr(0, 7);
  std::string out_f_name, out_fg_name;
  //out_f_name.format("%s/%s_canon.csv", dir, f_id.c_str());
  out_f_name = dir + std::string("/") + f_id + "_canon.csv";
  //out_fg_name.format("%s/%s_canon.csv", gdir, f_id.c_str());
  out_fg_name = gdir + std::string("/") + f_id + "_canon.csv";
  // ddTerminal::f_post("Creating: %s", out_f_name.str());

  // get translation offset (Iris (M) x, Iris (M) y)
  cbuff<64> map_idx = "Iris (M) x";
  const unsigned iris_m_idx = i_keys[map_idx]/2;
	map_idx = "Iris (L) x";
  const unsigned iris_l_idx = i_keys[map_idx]/2;
  // glm::vec2 delta_pos = glm::vec2(-input[iris_l_idx]);

  // palpebral fissure delta and center
	map_idx = "Palpebral fissure (RL) x";
  const unsigned pf_r_l = gt_keys[map_idx] / 2;
	map_idx = "Palpebral fissure (LL) x";
  const unsigned pf_l_l = gt_keys[map_idx] / 2;
  glm::vec2 delta_pos = glm::vec2(-ground[pf_r_l]);
  // ddTerminal::f_post("PF R L: %.3f", -delta_pos.y);

  // apply delta translation to all points
  dd_array<glm::vec2> input_n(input.size());
  dd_array<glm::vec2> ground_n(ground.size());

  DD_FOREACH(glm::vec3, vec, input) {  // input
    input_n[vec.i] = glm::vec2(*vec.ptr) + delta_pos;
  }
  DD_FOREACH(glm::vec3, vec, ground) {  // ground truth
    ground_n[vec.i] = glm::vec2(*vec.ptr) + delta_pos;
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
    // ddTerminal::f_post("#%u : %.3f, %.3f", vec.i, vec.ptr->x, vec.ptr->y);
		*vec.ptr += canonical_iris_pos;
  }
  DD_FOREACH(glm::vec2, vec, ground_n) {
    // ddTerminal::f_post("----> %.3f, %.3f", input_n[vec.i].x,
    // input_n[vec.i].y);
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
  DD_FOREACH(glm::vec2, vec, input_n) {
    out_str +=
        std::to_string(vec.ptr->x) + _sp + std::to_string(vec.ptr->y) + _sp;
  }
  out_str.pop_back();
  out_str += "\n";
  // ddTerminal::post(out_str.c_str());
  i_out.writeLine(out_str.c_str());

  if (append) {
    g_out.open(out_fg_name.c_str(), ddIOflag::APPEND);
  } else {
    g_out.open(out_fg_name.c_str(), ddIOflag::WRITE);
  }

  out_str = "";
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
      const std::string temp = file.ptr->c_str();
      const size_t idx = temp.find_last_of("\\/");
      const std::string f_name = temp.substr(idx + 1);

      if (file.ptr->find("_s_out.csv") != std::string::npos || 
				  file.ptr->find("_v_out.csv") != std::string::npos) {
        printf("  Exporting: %s\n", f_name.c_str());

        std::vector<glm::vec2> i_vec = 
          extract_vector2(file.ptr->c_str(), VecType::INPUT);

        // // extract contents of each file and convert to glm vectors
        // std::vector<Eigen::VectorXd> i_vec =
        //     extract_vector2(file.ptr->str(), VectorOut::INPUT);
        // std::vector<Eigen::VectorXd> g_vec =
        //     extract_vector2(g_file, VectorOut::OUTPUT);

        // // loop thru lines fo each and write to output file
        // for (size_t j = 0; j < i_vec.size(); j++) {
        //   dd_array<glm::vec3> i_p, g_p;
        //   get_points(i_vec, i_p, j, VectorOut::INPUT);
        //   get_points(g_vec, g_p, j, VectorOut::OUTPUT);

        //   const bool append = (j == 0) ? false : true;
        //   export_canonical_data(i_p, g_p, input_dir, ground_dir, f_name.c_str(),
        //                         canonical_iris_pos, canonical_iris_dist,
        //                         append);
        // }
        // ddTerminal::post("---> Done.");
      }
    }
  }
}

std::vector<glm::vec2> extract_vector2(const char *in_file, const VecType type) {
  std::vector<glm::vec2> out_vec;
  ddFileIO<> vec_io;

  bool success = vec_io.open(in_file, ddIOflag::READ);

  if (success) {
    // get vector size
    const char *line = vec_io.readNextLine();
    dd_array<cbuff<64>> indices;
    const std::string _f = in_file;
    bool time_flag = false;

    switch (type) {
      case VecType::INPUT:
        // get input keys
        indices = StrSpace::tokenize1024<64>(line, ",");
        if (i_keys.size() == 0) {
          DD_FOREACH(cbuff<64>, _key, indices) {
            // set offset if time column is present (must be 1st column)
            if (_key.ptr->contains("time")) {
							time_flag = true;
            } else {
              i_keys[*_key.ptr] = time_flag ? _key.i - 1 : _key.i;
            }
          }
        }
        // skip to next line in file
        line = vec_io.readNextLine();
        break;
      case VecType::OUTPUT:
        // get output keys
        indices = StrSpace::tokenize1024<64>(line, ",");
        if (gt_keys.size() == 0) {
          DD_FOREACH(cbuff<64>, _key, indices) {
            if (_key.ptr->contains("time")) {
							time_flag = true;
            } else {
              gt_keys[*_key.ptr] = time_flag ? _key.i - 1 : _key.i;
            }
          }
        }
        // skip to next line in file
        line = vec_io.readNextLine();
        break;
    }
    const unsigned vec_size = time_flag ? indices.size() - 1 : indices.size();

    // ddTerminal::f_post("Creating new input vectors(%lu)...", vec_size);
    // populate vector
    unsigned idx = 0;
    while (line) {
      printf("%s\n", line);
      // out_vec.push_back(Eigen::VectorXd::Zero(vec_size));

      // // loop thru columns per row
      // unsigned r_idx = 0;
      // const char *curr_row = line;
      // while (*curr_row) {
      //   char *nxt_dbl = nullptr;
      //   // printf("%s\n", curr_row);
      //   out_vec[idx](r_idx) = std::strtod(curr_row, &nxt_dbl);
      //   curr_row = nxt_dbl;

      //   r_idx++;
      // }
      // // std::cout << out_vec[idx] << "\n\n";

      line = vec_io.readNextLine();
      idx++;
    }
  }

  return out_vec;
}