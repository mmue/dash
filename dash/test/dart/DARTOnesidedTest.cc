
#include "DARTOnesidedTest.h"

#include <dash/Array.h>
#include <dash/Onesided.h>


TEST_F(DARTOnesidedTest, GetBlockingSingleBlock)
{
  typedef int value_t;
  const size_t block_size = 10;
  size_t num_elem_total   = dash::size() * block_size;
  dash::Array<value_t> array(num_elem_total, dash::BLOCKED);
  // Array to store local copy:
  int local_array[block_size];
  // Assign initial values: [ 1000, 1001, 1002, ... 2000, 2001, ... ]
  for (size_t l = 0; l < block_size; ++l) {
    array.local[l] = ((dash::myid() + 1) * 1000) + l;
  }
  array.barrier();
  // Unit to copy values from:
  dart_unit_t unit_src  = (dash::myid() + 1) % dash::size();
  // Global start index of block to copy:
  int g_src_index       = unit_src * block_size;
  // Copy values:
  dash::dart_storage<value_t> ds(block_size);
  LOG_MESSAGE("DART storage: dtype:%d nelem:%zu", ds.dtype, ds.nelem);
  dart_get_blocking(
    local_array,                                // lptr dest
    (array.begin() + g_src_index).dart_gptr(),  // gptr start
    ds.nelem,
    ds.dtype
  );
  for (size_t l = 0; l < block_size; ++l) {
    value_t expected = array[g_src_index + l];
    ASSERT_EQ_U(expected, local_array[l]);
  }
}

TEST_F(DARTOnesidedTest, GetBlockingSingleBlockTeam)
{
  typedef int value_t;

  if (dash::size() < 4) {
    SKIP_TEST_MSG("requires at least 4 units");
  }

  auto& split_team = dash::Team::All().split(2);
  const size_t block_size = 10;
  size_t num_elem_total   = split_team.size() * block_size;


  dash::Array<value_t> array(num_elem_total, dash::BLOCKED, split_team);
  // Array to store local copy:
  int local_array[block_size];
  // Assign initial values: [ 1000, 1001, 1002, ... 2000, 2001, ... ]
  for (size_t l = 0; l < block_size; ++l) {
    array.local[l] = (split_team.myid() + 1) * 1000 +
                       (split_team.dart_id() * 100) + l;
  }
  array.barrier();
  // Unit to copy values from:
  dart_unit_t unit_src  = (split_team.myid() + 1) % split_team.size();
  // Global start index of block to copy:
  int g_src_index       = unit_src * block_size;
  // Copy values:
  dash::dart_storage<value_t> ds(block_size);
  LOG_MESSAGE("DART storage: dtype:%d nelem:%zu", ds.dtype, ds.nelem);
  dart_get_blocking(
    local_array,                                // lptr dest
    (array.begin() + g_src_index).dart_gptr(),  // gptr start
    ds.nelem,
    ds.dtype
  );
  for (size_t l = 0; l < block_size; ++l) {
    value_t expected = array[g_src_index + l];
    ASSERT_EQ_U(expected, local_array[l]);
  }
}

TEST_F(DARTOnesidedTest, GetBlockingTwoBlocks)
{
  typedef int value_t;
  const size_t block_size    = 10;
  const size_t num_elem_copy = 2 * block_size;
  size_t num_elem_total      = dash::size() * block_size;
  dash::Array<value_t> array(num_elem_total, dash::BLOCKED);
  if (dash::size() < 2) {
    return;
  }
  // Array to store local copy:
  int local_array[num_elem_copy];
  // Assign initial values: [ 1000, 1001, 1002, ... 2000, 2001, ... ]
  for (size_t l = 0; l < block_size; ++l) {
    array.local[l] = ((dash::myid() + 1) * 1000) + l;
  }
  array.barrier();
  // Copy values from first two blocks:
  dash::dart_storage<value_t> ds(num_elem_copy);
  LOG_MESSAGE("DART storage: dtype:%d nelem:%zu", ds.dtype, ds.nelem);
  dart_get_blocking(
    local_array,                      // lptr dest
    array.begin().dart_gptr(),        // gptr start
    ds.nelem,                         // number of elements
    ds.dtype                          // data type
  );
  // Fails for elements in second block, i.e. for l < num_elem_copy:
  for (size_t l = 0; l < block_size; ++l) {
    value_t expected = array[l];
    ASSERT_EQ_U(expected, local_array[l]);
  }
}

TEST_F(DARTOnesidedTest, GetHandleAllRemote)
{
  typedef int value_t;
  const size_t block_size = 5000;
  size_t num_elem_copy    = (dash::size() - 1) * block_size;
  size_t num_elem_total   = dash::size() * block_size;
  dash::Array<value_t> array(num_elem_total, dash::BLOCKED);
  if (dash::size() < 2) {
    return;
  }
  // Array to store local copy:
  int * local_array = new int[num_elem_copy];
  // Array of handles, one for each dart_get_handle:
  std::vector<dart_handle_t> handles;
  // Assign initial values: [ 1000, 1001, 1002, ... 2000, 2001, ... ]
  for (size_t l = 0; l < block_size; ++l) {
    array.local[l] = ((dash::myid() + 1) * 1000) + l;
  }
  array.barrier();

  LOG_MESSAGE("Requesting remote blocks");
  // Copy values from all non-local blocks:
  size_t block = 0;
  for (size_t u = 0; u < dash::size(); ++u) {
    if (u != static_cast<size_t>(dash::myid())) {
      LOG_MESSAGE("Requesting block %zu from unit %zu", block, u);
      dart_handle_t handle;

      dash::dart_storage<value_t> ds(block_size);
      LOG_MESSAGE("DART storage: dtype:%d nelem:%zu", ds.dtype, ds.nelem);
      EXPECT_EQ_U(
        DART_OK,
        dart_get_handle(
            local_array + (block * block_size),
            (array.begin() + (u * block_size)).dart_gptr(),
            ds.nelem,
            ds.dtype,
            &handle)
      );
      LOG_MESSAGE("dart_get_handle returned handle %p",
                  static_cast<void*>(handle));
      handles.push_back(handle);
      ++block;
    }
  }
  // Wait for completion of get operations:
  LOG_MESSAGE("Waiting for completion of async requests");
  dart_waitall_local(
    handles.data(),
    handles.size()
  );

  LOG_MESSAGE("Validating values");
  int l = 0;
  for (size_t g = 0; g < array.size(); ++g) {
    auto unit = array.pattern().unit_at(g);
    if (unit != dash::myid()) {
      value_t expected = array[g];
      ASSERT_EQ_U(expected, local_array[l]);
      ++l;
    }
  }
  delete[] local_array;
  ASSERT_EQ_U(num_elem_copy, l);
}
