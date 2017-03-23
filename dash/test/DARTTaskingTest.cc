
#include "DARTTaskingTest.h"
#include <dash/dart/if/dart_types.h>
#include <dash/dart/if/dart_globmem.h>

typedef struct testdata {
  int  expected;
  int *valptr;
} testdata_t;

static void testfn(void *data) {
  testdata_t *td = (testdata_t*)data;
  int *valptr = td->valptr;
  ASSERT_EQ(td->expected, *valptr);
  *valptr += 1;
}

TEST_F(DARTTaskingTest, LocalDirectDependency)
{
  if (!dash::is_multithreaded()) {
    SKIP_TEST_MSG("Thread-support required");
  }
  int i;
  int val = 0;

  dart_taskref_t prev_task = DART_TASK_NULL;

  for (i = 0; i < 100; i++) {
    testdata_t td;
    td.valptr   = &val;
    td.expected = i;
    dart_task_dep_t dep;
    dep.type = DART_DEP_DIRECT;
    dep.task = prev_task;
    ASSERT_EQ(
      DART_OK,
      dart_task_create_handle(
        &testfn,             // action to call
        &td,                 // argument to pass
        sizeof(td),          // size of the tasks's data (if to be copied)
        &dep,                // dependency
        1,                   // number of dependencies
        &prev_task           // handle to be returned
        )
    );
  }

  ASSERT_EQ(i, val);
}


TEST_F(DARTTaskingTest, LocalOutDependency)
{
  if (!dash::is_multithreaded()) {
    SKIP_TEST_MSG("Thread-support required");
  }
  int i;
  int val = 0;

  for (i = 0; i < 100; i++) {
    testdata_t td;
    td.valptr   = &val;
    td.expected = i;

    // force serialization through an output chain
    dart_task_dep_t dep;
    dep.type = DART_DEP_OUT;
    dep.gptr = DART_GPTR_NULL;
    dep.gptr.unitid = dash::myid();
    dep.gptr.teamid = dash::Team::All().dart_id();
    dep.gptr.addr_or_offs.addr = &val;
    ASSERT_EQ(
      DART_OK,
      dart_task_create(
        &testfn,              // action to call
        &td,                 // argument to pass
        sizeof(td),          // size of the tasks's data (if to be copied)
        &dep,                // dependency
        1                    // number of dependencies
        )
    );
  }

  ASSERT_EQ(i, val);
}

TEST_F(DARTTaskingTest, LocalInOutDependency)
{
  if (!dash::is_multithreaded()) {
    SKIP_TEST_MSG("Thread-support required");
  }
  int  i;
  int  val = 0;
  int *valptr = &val; // dummy pointer used for synchronization, never accessed

  for (i = 0; i < 100; i++) {
    testdata_t td;
    td.valptr   = &val;
    td.expected = i;

    dart_task_dep_t dep[2];
    dep[0].type = DART_DEP_IN;
    dep[0].gptr = DART_GPTR_NULL;
    dep[0].gptr.unitid = dash::myid();
    dep[0].gptr.teamid = dash::Team::All().dart_id();
    dep[0].gptr.addr_or_offs.addr = valptr;
    valptr++;
    dep[1].type = DART_DEP_OUT;
    dep[1].gptr = DART_GPTR_NULL;
    dep[1].gptr.unitid = dash::myid();
    dep[1].gptr.teamid = dash::Team::All().dart_id();
    dep[1].gptr.addr_or_offs.addr = valptr;
    ASSERT_EQ(
      DART_OK,
      dart_task_create(
        &testfn,              // action to call
        &td,                 // argument to pass
        sizeof(td),          // size of the tasks's data (if to be copied)
        dep,                // dependency
        2                    // number of dependencies
        )
    );
  }

  ASSERT_EQ(i, val);
}
