#include <libdash.h>
#include <iostream>

// For more information on HDF5 files see
// https://www.hdfgroup.org/HDF5

#define FILENAME "example.hdf5"

using std::cout;
using std::cerr;
using std::endl;

typedef dash::Pattern<1, dash::ROW_MAJOR, long> pattern_t;
typedef dash::Array<int, long, pattern_t>       array_t;

void print_file(){
	// Print h5dump output
	int status;
	if(dash::myid() == 0){
		std::string syscall = "h5dump ";
		status = system((syscall + FILENAME).c_str());
	}
}

void print_separator(){
	if(dash::myid() == 0){
		cout << "=======================================================" << endl;
	}
}

int main(int argc, char * argv[])
{
  dash::init(&argc, &argv);

  long      extent = 100;
	int       myid   = dash::myid();

	pattern_t pattern_a(extent, dash::TILE(10));
	pattern_t pattern_b(extent, dash::TILE(7));
  array_t   array_a(pattern_a);
	array_t   array_b(pattern_b);

	// Fill Array
	dash::fill(array_a.begin(), array_a.end(), myid);
	dash::fill(array_b.begin(), array_a.end(), myid * 10);


	if(myid == 0){
		cout << "DASH HDF5 API example. After each change in the hdf5 file"
         << " the contents are printed using h5dump" << endl;
	}
	// Write Array to HDF5 file using defaults
	{
		if(myid == 0) {
			print_separator();
			cout << "Write Array A to " << FILENAME << " / data" << endl;
		}
		dash::io::StoreHDF::write(array_a, FILENAME, "data");
		dash::barrier();
		print_file();
	}
	// Restore values from HDF5 dataset. Pattern gets reconstructed from
	// hdf5 metadata
	{
		if(myid == 0){
			print_separator();
			cout << "Read " << FILENAME << " / data into Array C," 
					 << " reconstruct pattern" << endl;
		}
		// Use delayed allocation
		array_t array_c;
		dash::io::StoreHDF::read(array_c, FILENAME, "data");
	}

	// OK, that was easy. Now let's have a slightly more complex setup
	// Convert between two patterns
	//
	{
		if(myid == 0){
			print_separator();
			cout << "Read " << FILENAME << " / data into already allocated Array C" << endl;
		}
		// pass allocated array to define custom pattern
		array_t array_c(pattern_b); // tilesize=7
		dash::io::StoreHDF::read(array_c, FILENAME, "data");
		if(myid == 0){
			cout << "Array A Pattern: Tilesize: " << array_a.pattern().blocksize(0) << endl;
			cout << "Array C Pattern: Tilesize: " << array_c.pattern().blocksize(0) << endl;
		}
	}

	// Store multiple datasets in single file
	{
		// use options object to add a dataset instead
		// of overwriting the hdf5 file
		if(myid == 0){
			print_separator();
			cout << "Add dataset temperature to " << FILENAME << endl;
		}
		auto fopts = dash::io::StoreHDF::get_default_options();
		fopts.overwrite_file = false; // Do not overwrite existing file

		dash::io::StoreHDF::write(array_b, FILENAME, "temperature", fopts);
		dash::barrier();
		print_file();
	}

	// Update dataset. IMPORTANT: the dataset extents must not change!
	{
		if(myid == 0){
			print_separator();
			cout << "Modify " << FILENAME << " / temperature dataset" << endl;
		}
		auto fopts = dash::io::StoreHDF::get_default_options();
		fopts.overwrite_file = false;
		fopts.modify_dataset = true;

		dash::io::StoreHDF::write(array_a, FILENAME, "temperature", fopts);
		dash::barrier();
		print_file();
	}

	sleep(1);
	// Clean up
	if(myid == 0){
		remove(FILENAME);
	}

  dash::finalize();

  return EXIT_SUCCESS;
}