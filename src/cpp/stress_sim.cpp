#include <Rcpp.h>
using namespace Rcpp;

// Simulates the chart on CPP rather than R
// Input
  // Note that the first 2 arguments must match in length
  // offsets: Offsets in a vector
  // is_spikes: Logical Vector indicating if it's a spike
  // args_list: A List indicating different arguments for
  //  spike vs. decay
  // spike & decay_func: Spike and Decay Functions, the 
  //  arguments must match those provided in args_list
  // stress: The value to initialize stress with
// Output
  // A list containing stress_base and stress_spike, 
  //  accessible via "base" & "spike" names
List simulate_key(NumericVector offsets,
                  LogicalVector is_spikes,
                  List args_list,
                  Function spike_func,
                  Function decay_func,
                  double stress = 0.0) {
  
  unsigned int rows = offsets.length();
  
  // Assert length
  assert((is_spikes.length() == rows,
          "Both vectors must be equal in length."));
  
  // Initialize with -1 as default
  NumericVector stress_base(rows, -1.0);
  NumericVector stress_spike(rows, -1.0);
  
  double offset_buffer = 0.0;
  double offset = 0.0;
  double duration = 0.0;
  bool is_spike = false;
  
  for (unsigned int row; row < rows; row ++) {
    // Get all the required parameters
    offset = offsets[row];
    is_spike = is_spikes[row];
    duration = offset - offset_buffer; 
    
    // If spike, we will calculate via decay then spike_func
    // and commit into stress var.
    // Else, will calculate via decay_func but do not commit.
    
    // This conditional is reversed since non-spikes are more
    // common
    if (!is_spike){
      stress_base[row] = as<double>(decay_func(_["stress"] = stress,
                                               _["duration"] = duration,
                                               _["args"] = args_list["decay"]));
    } else {
      stress = as<double>(spike_func(_["stress"] = stress,
                                     _["args"] = args_list["decay"]));
      stress_base[row] = stress;
      stress = as<double>(decay_func(_["stress"] = stress,
                                     _["duration"] = duration,
                                     _["args"] = args_list["spike"]));
      stress_spike[row] = stress;
    }

    offset_buffer = offset;
  }
  
  List stress_list = List::create(
    _["base"] = stress_base,
    _["spike"] = stress_spike
  );
  
  return stress_list;
}